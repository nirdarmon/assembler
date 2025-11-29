#include "preproc.h"


/* static functions declarations */
static void close_macro_files(FILE** files, int counter);
static void remove_macro_file(char* file_name);
static void remove_macro_files_array(char** macro_name, int counter);
static int is_endmacr(char* line, int line_len);
static int check_if_macro_use_and_execute(char* line, char** macro_names, int macro_counter, FILE* am_file, FILE** macro_files);
static int is_macro_declaration(char* line, int* curr_idx, char* source_code_file_name, int line_length, int line_counter);
static int is_valid_macro_name(char* buffer, char current_macro_name[31], char** macro_names, int macro_counter, int* curr_idx, 
 char* source_code_file_name, int line_counter);


int preproc(char* as_file_name_with_ext, char*** macro_names, int* macro_counter, FILE* as_file, FILE* am_file)
{
    char* buffer; /* a buffer that will store the current line of the source code */
    int in_macro_declaration = 0;
    FILE** macro_files;
    int line_len;
    int i;
    int curr_idx = 0;
    char current_macro_name[MAX_LABEL_LEN + 4 + 1];/* +4 for the .txt if used as file name */
    int current_macro_name_len;
    int line_counter = 0;
    int is_error = 0;

    *macro_names = (char**)calloc(1, sizeof(char*));
    if (*macro_names == NULL)
    {
        fprintf(stderr, "Error: memory allocation failure\n");
        return -1;
    }
    
    macro_files = (FILE**)calloc(1, sizeof(FILE*));
    if (macro_files == NULL)
    {
        fprintf(stderr, "Error: memory allocation failure\n");
        return -1;
    }


    while (1)
    {/* read the source code file line by line */
        /* Reallocate memory (using calloc because buffer set to free before at the end of while) for the new line */
        {
            line_len = line_length(as_file);
            if (line_len == 0)
            {/* finished copying .as file after macro expansion to .am file */
                if(in_macro_declaration == 1)
                {
                    printf("Error in file %s in line %d: EOF in the middle of macro declaration\n", as_file_name_with_ext, line_counter);
                    is_error = 1;
                }
                close_macro_files(macro_files, *macro_counter);
                remove_macro_files_array(*macro_names, *macro_counter);
                free(macro_files);
                macro_files = NULL; /* avoid dangling pointer */
                break;
            }
            buffer = (char*)calloc(line_len + 1, sizeof(char));
            if(buffer == NULL)
            {
                fprintf(stderr, "Error: memory allocation failure\n");
                close_macro_files(macro_files, *macro_counter);
                remove_macro_files_array(*macro_names, *macro_counter);
                free(macro_files);
                macro_files = NULL; /* avoid dangling pointer */
                return -1;
            }
        }

        {/* update line data */
            fgets(buffer, line_len+1, as_file);/* get a new line */
            buffer[line_len] = '\0';/* add null terminator */
            line_counter++; /* update line counter */
            curr_idx = 0; /* reset line index */
            
        }

        while (buffer[curr_idx] == ' ' || buffer[curr_idx] == '\t')
        {
            curr_idx++;
        }

        if (buffer[curr_idx] == '\n' || line_len == 0)
        {
        /* case of empty line - ignore */
        }
        
        else if (!in_macro_declaration)
        {/* case not in macro state */
            if ((i = is_macro_declaration(buffer, &curr_idx, as_file_name_with_ext, line_len, line_counter)) == 1)
            {/* the first token in line is "macr""" */
                /* check that the second token in line is a valid macro name and that line ends after this token */
                if (skip_white_spaces(buffer, &curr_idx))
                {
                    printf("Error in file %s in line %d: error in macro declaration opening line, missing macro name\n", as_file_name_with_ext, line_counter);
                    is_error = 1;
                }
                if (!is_valid_macro_name(buffer, current_macro_name, *macro_names, *macro_counter, &curr_idx, as_file_name_with_ext, line_counter))
                {/* case not a valid macro name or case of extraneous text after end of macro name */
                    is_error = 1; /* set flag and continue checking for more macro errors */
                }
                else 
                {/* case second token in line is a valid macro name & line ends after that token */
                    char** temp;
                    FILE** temp_files;
                    current_macro_name_len = strlen(current_macro_name);
                    temp = (char**)realloc(*macro_names, (*macro_counter + 1) * sizeof(char*));
                    if (temp == NULL)
                    {
                        fprintf(stderr, "Error: memory allocation failure\n");
                        close_macro_files(macro_files, *macro_counter);
                        remove_macro_files_array(*macro_names, *macro_counter);
                        free(macro_files);
                        macro_files = NULL; /* avoid dangling pointer */
                        free(buffer);
                        buffer = NULL; /* avoid dangling pointer */
                        return -1;
                    }
                    *macro_names = temp;
                    (*macro_counter)++;
                    (*macro_names)[*macro_counter-1] = (char*)calloc(current_macro_name_len + 1, sizeof(char));
                    if ((*macro_names)[*macro_counter-1] == NULL)
                    {
                        fprintf(stderr, "Error: memory allocation failure\n");
                        close_macro_files(macro_files, *macro_counter-1);
                        remove_macro_files_array(*macro_names, *macro_counter-1);
                        free(macro_files);
                        macro_files = NULL; /* avoid dangling pointer */
                        free(buffer);
                        buffer = NULL; /* avoid dangling pointer */
                        return -1;
                    }
                    strcpy((*macro_names)[*macro_counter-1], current_macro_name);
                    strcat(current_macro_name, ".txt");
                    current_macro_name_len += 4;
                    current_macro_name[current_macro_name_len] = '\0';

                    (*macro_counter)--;/* decrement counter to use it as index for the macro_files array */
                    temp_files = (FILE**)realloc(macro_files, ((*macro_counter) + 1) * sizeof(FILE*));
                    if (temp_files == NULL)
                    {
                        fprintf(stderr, "Error: memory allocation failure\n");
                        close_macro_files(macro_files, *macro_counter);
                        remove_macro_files_array(*macro_names, *macro_counter);
                        free(macro_files);
                        macro_files = NULL; /* avoid dangling pointer */
                        free(buffer);
                        buffer = NULL; /* avoid dangling pointer */
                        return -1;
                    }
                    macro_files = temp_files;
                    (*macro_counter)++;
                    macro_files[*macro_counter-1] = fopen(current_macro_name, "w");
                    if(macro_files[*macro_counter-1] == NULL)
                    {
                        close_macro_files(macro_files, *macro_counter-1);
                        remove_macro_files_array(*macro_names, *macro_counter-1);
                        free(macro_files);
                        macro_files = NULL; /* avoid dangling pointer */
                        free(buffer);
                        buffer = NULL; /* avoid dangling pointer */                       
                        return -2;
                    }
                    else {/* create file success */
                        in_macro_declaration = 1;
                    } 
                }
            }
            else if (i == -1)
            {/* error in macro declaration line */
                is_error = 1;
            } 
            else
            {/* i == 0, not a macro declaration line */
                if ((i = check_if_macro_use_and_execute(buffer, *macro_names, *macro_counter, am_file, macro_files)) == 1)
                {/* case macro usage line and macro content was implemented successfully in the .am file */
                    /* do nothing(actions inside the function) */
                }
                else
                {/* non macro related line, copy line to am_file */
                    fprintf(am_file, "%s", buffer);
                }
            }
        }
        else{/* case in macro state */
            if ((i = is_endmacr(buffer, line_len)) == 1)
            {/* reset macro set without copying line */
                in_macro_declaration = 0;
                /* reopen macro file in reading mode */
                fclose(macro_files[*macro_counter-1]);
                macro_files[*macro_counter-1] = fopen(current_macro_name, "r");
                if(macro_files[*macro_counter-1] == NULL)
                {
                    close_macro_files(macro_files, *macro_counter);
                    remove_macro_files_array(*macro_names, *macro_counter);
                    free(macro_files);
                    macro_files = NULL; /* avoid dangling pointer */
                    free(buffer);
                    buffer = NULL; /* avoid dangling pointer */
                    return -2;
                }
            }
            else if (i == -1 || i == -2)
            {/* case of line length or extraneous text after endmacr token */
                if (i == -1)
                {
                    printf("Error in file %s in line %d: error in macro declaration closing line, extraneous text after \"endmacr\" token\n", as_file_name_with_ext, line_counter);
                }
                else if(i == -2)
                {
                    printf("Error in file %s in line %d: error in macro declaration closing line, line length is over %d chars\n", as_file_name_with_ext, line_counter, MAX_LINE_LEN);
                }
                is_error = 1;
                in_macro_declaration = 0;
                fclose(macro_files[*macro_counter-1]);
                remove(current_macro_name);
                free((*macro_names)[*macro_counter-1]);
                (*macro_names)[*macro_counter-1] = NULL; /* avoid dangling pointer */
                (*macro_counter)--;
            }
            else
            {/* copy line to macro file and keep current state */
                fprintf(macro_files[*macro_counter-1], "%s", buffer);
            }
        }

        if(feof(as_file))
        {/* finished copying .as file after macro expansion to .am file */
            close_macro_files(macro_files, *macro_counter);
            remove_macro_files_array(*macro_names, *macro_counter);
            free(macro_files);
            macro_files = NULL; /* avoid dangling pointer */
            free(buffer);
            buffer = NULL; /* avoid dangling pointer */
            break;
        }
        free(buffer);
        buffer = NULL; /* avoid dangling pointer */
    }
    if (is_error)
    {
        return 0;
    }
    return 1;
}


/*
 *    STATIC FUNCTIONS ONLY
 *            ||
 *            ||
 *            ||
 *           \  /
 *            \/                                               
 */


/*
 * Closes an array of macro files.
 */
static void close_macro_files(FILE** files, int counter)
{
    int i;
    if (files == NULL)
    {
        return;
    }
    if (counter == 0)
    {
        return;
    }
    for (i = 0; i < counter; i++)
    {
        fclose(files[i]);
    }
}


/*
 * Deletes a macro file with the given name.
 */
static void remove_macro_file(char* file_name)
{
    char file_name_with_ext[MAX_LABEL_LEN + 5];/* .txt + '\0' */
    strcpy(file_name_with_ext, file_name);
    strcat(file_name_with_ext, ".txt");
    remove(file_name_with_ext);
}


/*
 * Removes macro files specified in the array.
 */
static void remove_macro_files_array(char** macro_name, int counter)
{
    int i;
    for (i = 0; i < counter; i++)
    {
        remove_macro_file(macro_name[i]);
    }
}


/*
 * Checks for end of macro declaration line.
 * 
 * Returns:
 * 1 if the only token in line is "endmacr" and the line ends after it.
 * -1 if the first token in line is "endmacr" but there is extraneous text after it.
 * 0 otherwise.
 */
static int is_endmacr(char* line, int line_len)
{
    int i = 0;
    while (line[i] == ' ' || line[i] == '\t')
    {
        i++;
    }
    if (strncmp(line + i, "endmacr", 7) == 0 && (line[i + 7] == ' ' || line[i + 7] == '\t' || line[i + 7] == '\n' || line[i + 7] == '\0'))
    {
        if (line_len > MAX_LINE_LEN)
        {
            return -2;
        }
        i += 7;
        if(skip_white_spaces(line, &i))
        {
            return 1;
        }
        else return -1;
    }
    else return 0;
}


/*
 * Checks if the only token in line is an existing macro name. 
 * Implements the macro content (from the opened macro file) to the am file.
 * 
 * Return: 
 * 1 if line is a macro usage and if the macro content was implemented successfully in the .am file,
 * 0 if the first token in line is not a macro name, or if there is extraneous text after the macro name.
 */
static int check_if_macro_use_and_execute(char* line, char** macro_names, int macro_counter, FILE* am_file, FILE** macro_files)
{/* add the operands missing to print from macro file to am file123456*/
    int i = 0;
    int j = 0;
    char curr_name[MAX_LABEL_LEN + 1];
    while (line[i] == ' ' || line[i] == '\t')
    {
        i++;
    }
    while (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\0')
    {
        if (j > MAX_LABEL_LEN-1)
        {
            return 0;
        }
        curr_name[j] = line[i];
        i++;
        j++;
    }
    curr_name[j] = '\0';
    j = -1;
    if ((j = is_macro(curr_name, macro_names, macro_counter)) != -1)
    {
        if(skip_white_spaces(line, &i))
        {
            int ch;
            rewind(macro_files[j]);
            while ((ch = fgetc(macro_files[j])) != EOF)
            {
                fputc(ch, am_file);
            }
            return 1;
        }
        else 
        {
            return 0;
        }
    }
    else return 0;
}


/*
 * Checks if the line is a macro declaration starting with "macr ".
 * Updates the current index and checks if the line length exceeds the maximum allowed length.
 * 
 * Returns:
 * 1 if the first token in line is "macr " and it is not the only token in line.
 * -1 if the first and only token in line is "macr ".
 * 0 otherwise.
 */
static int is_macro_declaration(char* line, int* curr_idx, char* source_code_file_name, int line_length, int line_counter)
{
    if (strncmp(line + *curr_idx, "macr", 4) == 0 && (line[*curr_idx + 4] == ' ' || line[*curr_idx + 4] == '\t' || line[*curr_idx + 4] == '\n' || line[*curr_idx + 4] == '\0'))
    {
        if(line_length > MAX_LINE_LEN)
        {
            printf("Error in file %s in line %d: error in macro declaration opening line, line length is over %d chars\n", source_code_file_name, line_counter, MAX_LINE_LEN);
            return -1;
        }
        else if (line[*curr_idx + 4] == '\n' || line[*curr_idx + 4] == '\0')
        {
            printf("Error in file %s in line %d: error in macro declaration opening line, missing macro name\n", source_code_file_name, line_counter);
            return -1;
        }
        else 
        {
            (*curr_idx) += 5;
            return 1;
        }
    }
    else return 0;
}


/*
 * Validates a macro name from the given buffer.
 * Updates the buffer index, the current_macro_name string and provides an error message if validation fails.
 * 
 * Returns:
 * 1 if the macro name is valid and the line has no syntax errors.
 * 0 if invalid, with an error message printed.
 */
static int is_valid_macro_name(char* buffer, char current_macro_name[31], char** macro_names, int macro_counter, int* curr_idx, 
 char* source_code_file_name, int line_counter)
{
    int i = 0;
    if(!isalpha(buffer[*curr_idx]))
    {
        printf("Error in file %s in line %d: error in macro declaration opening line, macro name must start with a letter\n", source_code_file_name, line_counter);
        return 0;
    }
    while (buffer[*curr_idx] != ' ' && buffer[*curr_idx] != '\t' && buffer[*curr_idx] != '\0' && buffer[*curr_idx] != '\n')
    {
        if (i > MAX_LABEL_LEN)
        {
            printf("Error in file %s in line %d: error in macro declaration opening line, macro name is over %d chars\n", source_code_file_name, line_counter, MAX_LABEL_LEN);
            return 0;
        }
        current_macro_name[i] = buffer[*curr_idx];
        (*curr_idx)++;
        i++;
        
    }
    if(!skip_white_spaces(buffer, curr_idx))
    {
        printf("Error in file %s in line %d: error in macro declaration opening line, macro name is not followed by a new line or end of line\n", source_code_file_name, line_counter);
        return 0;
    }
    else {
        current_macro_name[i] = '\0';
        if (is_reserved_word(current_macro_name))
        {
            printf("Error in file %s in line %d: error in macro declaration opening line, macro name (\"%s\") is a reserved keyword\n", source_code_file_name, line_counter, current_macro_name);
            return 0;
        }
        else if (is_macro(current_macro_name, macro_names, macro_counter) != -1)
        {
            printf("Error in file %s in line %d: error in macro declaration opening line, macro name is already in use\n", source_code_file_name, line_counter);
            return 0;
        }
        else{
            return 1;
        }
    }
}
