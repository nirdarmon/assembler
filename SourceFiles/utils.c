#include "utils.h"


/* static functions declarations */
static char* generate_name_of_file_with_ext(char* base_name, char* extension);
static int is_instruction(char* word);
static int is_directive(char* word);
static int is_register(char* word);


int create_file_and_save_name_with_ext(char* file_name_without_ext, char** file_name_with_ext, FILE** file_ptr, char* extension, char* mode)
{
    *file_name_with_ext = (generate_name_of_file_with_ext(file_name_without_ext, extension));
    if (!(*file_name_with_ext))
    {
        fprintf(stderr, "Error: Memory allocation failure\n");
        return -1;/* memory allocation failure */
    }
    (*file_ptr) = fopen(*file_name_with_ext, mode);
    if(!(*file_ptr))
    {
        fprintf(stderr, "Error: could not open %s file\n", *file_name_with_ext);
        free(*file_name_with_ext);
        *file_name_with_ext = NULL; /* avoid dangling pointer */
        return -2;/* file creation failure */
    }
    else return 0;
}


void free_matrix_char_pointer(char*** matrix, int counter)
{
    int i;
    for ( i = 0; i < counter; i++)
    {
        if ((*matrix)[i] != NULL)
        {
            free((*matrix)[i]);
            (*matrix)[i] = NULL; /* avoid dangling pointer */
        }
    }
    if (*matrix != NULL)
    {
        free(*matrix);
        *matrix = NULL; /* avoid dangling pointer */
    } 
}




int is_reserved_word(char* word)
{
    if (is_instruction(word)
        || is_directive(word)
        || is_register(word)
        || (strcmp(word, "endmacr") == 0)
        || (strcmp(word, "macr") == 0))
    {
        return 1;
    }
    else return 0;
}


/*
 * Checks if a word is a macro name.
 * 
 * Return the position of the macro in the macro_names array if it is a macro, -1 otherwise.
 */
int is_macro(char* word, char** macro_names,int macro_counter)
{
    int i;
    for (i = 0; i < macro_counter; i++)
    {
        if (strcmp(word, macro_names[i]) == 0)
        {
            return i;/* return the position of the macro in the macro_names array */
        }
    }
    return -1;
}


/* Return 1 if reached end of line or file, else return 0 */
int skip_white_spaces(char* line, int* idx)
{
    while(line[*idx] == ' ' || line[*idx] == '\t')
    {
        (*idx)++;
    }
    if (line[*idx] == '\n' || line[*idx] == '\0')
    {
        return 1;/* end of line or file */
    }
    else return 0;
}


/* Return the length of the current line in the given file including \n */
int line_length(FILE* input)
{
    /* store original file position */
    char c;
    int count = 0;
    long int current_pos = ftell(input);
    /* count characters until newline or end of file */
    while ((c = fgetc(input)) != '\n' && c != EOF) 
    {
        count++;
    }
    if (c == '\n')
    {
        count++;
    }
    /* move file pointer back to the beginning of the line */
    while (ftell(input) != current_pos)
    {
        fseek(input, -1, SEEK_CUR);
    }
    return count;
}


int check_if_label_exist(char* word, Labels** label_array, int label_array_counter)
{
    int i;
    for (i = 0; i < label_array_counter; i++)
    {
        if (strcmp(word, label_array[i]->l_name) == 0)
        {
            return i;/* return the position of the label in the label_array */
        }
    }
    return -1;
}


void remove_label(Labels** label_array, int* label_array_counter, char* label_name)
{
    int i;
    for (i = 0; i < *label_array_counter; i++)
    {
        if (strcmp(label_name, label_array[i]->l_name) == 0)
        {
            free_label_data(label_array[i]);
            if (label_array[i] != NULL)
            {
                free(label_array[i]);
                label_array[i] = NULL; /* avoid dangling pointer */
            }
            while (i < *label_array_counter - 1)
            {
                label_array[i] = label_array[i + 1];
                i++;
            }
            (*label_array_counter)--;
            return;
        }
    }
    return;
}


void free_label_data(Labels* label)
{
    if (label->l_name != NULL)
    {
        free(label->l_name);
        label->l_name = NULL; /* avoid dangling pointer */
    }
    return;
}


void free_label_array(Labels*** label_array, int label_array_counter)
{

    int i=0;
    for (i = 0; i < label_array_counter; ++i) 
    {
        free_label_data((*label_array)[i]);
        if ((*label_array)[i] != NULL)
        {
            free((*label_array)[i]);
            (*label_array)[i] = NULL; /* avoid dangling pointer */
        }
    }
    if (*label_array != NULL)
    {
        free(*label_array);
        *label_array = NULL; /* avoid dangling pointer */
    }
    return;
}


void free_ast_data(Ast* ast)
{
    if (ast->is_label_declaration == 1)
    {
        if (ast->label_dec_name != NULL)
        {
            free(ast->label_dec_name);
            ast->label_dec_name = NULL; /* avoid dangling pointer */
        }
    }
    if (ast->ast_type == DIRECTIVE)
    {
        if (ast->ast_options.Directive.dir_name == ENTRY ||
            ast->ast_options.Directive.dir_name == EXTERNAL)
        {
            if (ast->ast_options.Directive.dir_content.label != NULL)
            {
                free(ast->ast_options.Directive.dir_content.label);
                ast->ast_options.Directive.dir_content.label = NULL; /* avoid dangling pointer */
            }
        }
        if (ast->ast_options.Directive.dir_name ==  STRING &&
            ast->ast_options.Directive.dir_content.dir_string_str != NULL)
        {
            free(ast->ast_options.Directive.dir_content.dir_string_str);
            ast->ast_options.Directive.dir_content.dir_string_str = NULL; /* avoid dangling pointer */
        } 
    }
    else if (ast->ast_type == INSTRUCTION)
    {
        if (ast->ast_options.Instruction.Operand[0].op_type == DIRECT &&
            ast->ast_options.Instruction.Operand[0].op_data.label != NULL)
        {
            free(ast->ast_options.Instruction.Operand[0].op_data.label);
            ast->ast_options.Instruction.Operand[0].op_data.label = NULL; /* avoid dangling pointer */
        }
        if (ast->ast_options.Instruction.Operand[1].op_type == DIRECT &&
            ast->ast_options.Instruction.Operand[1].op_data.label != NULL)
        {
            free(ast->ast_options.Instruction.Operand[1].op_data.label);
            ast->ast_options.Instruction.Operand[1].op_data.label = NULL; /* avoid dangling pointer */
        }
    }
    return;
}


void free_ast_array(Ast*** ast_array, int ast_array_counter)
{
    int i = 0;

    for (i = 0; i < ast_array_counter; ++i) {
        if (((*ast_array)[i])->is_error)
        {
            free_ast_data((*ast_array)[i]);
            free((*ast_array)[i]);
            (*ast_array)[i] = NULL; /* avoid dangling pointer */
            continue;
        }
        free_ast_data((*ast_array)[i]);
        if ((*ast_array)[i] != NULL)
        {
            free((*ast_array)[i]);
            (*ast_array)[i] = NULL; /* avoid dangling pointer */
        }
    } 
    if (*ast_array != NULL)
    {
        free(*ast_array);
        *ast_array = NULL; /* avoid dangling pointer */
    }
    return;
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
 * Concatenates a base name with an extension to form a file name.
 * prints an error message to stderr if memory allocation fails.
 * Returns a pointer to the newly allocated string containing the full file name, or NULL if memory allocation fails.
 */
static char* generate_name_of_file_with_ext(char* base_name, char* extension)
{
    char* file_name_with_ext;
    int len = 0;
    len += strlen(base_name) + strlen(extension);
    file_name_with_ext = (char*)calloc(len + 1, sizeof(char));
    if (!file_name_with_ext)
    {
        fprintf(stderr, "Error: Memory allocation failure\n");
        return NULL;/* memory allocation failure */
    }
    strcpy(file_name_with_ext, base_name);
    strcat(file_name_with_ext, extension);
    return file_name_with_ext;
}


/*
 * Checks if a word is an instruction.
 * Returns 1 if it is an instruction, 0 otherwise.
 */
static int is_instruction(char* word)
{
    if ((strcmp(word, "mov") == 0)
        || (strcmp(word, "cmp") == 0) || (strcmp(word, "add") == 0)
        || (strcmp(word, "sub") == 0) || (strcmp(word, "lea") == 0)
        || (strcmp(word, "clr") == 0) || (strcmp(word, "not") == 0)
        || (strcmp(word, "inc") == 0) || (strcmp(word, "dec") == 0)
        || (strcmp(word, "jmp") == 0) || (strcmp(word, "bne") == 0)
        || (strcmp(word, "red") == 0) || (strcmp(word, "prn") == 0)
        || (strcmp(word, "jsr") == 0) || (strcmp(word, "rts") == 0)
        || (strcmp(word, "stop") == 0))
    {
        return 1;
    }
    else return 0;
}


/*
 * Checks if a word is a directive.
 * Returns 1 if it is a directive, 0 otherwise.
 */
static int is_directive(char* word)
{
    if ((strcmp(word, "data") == 0)
        || (strcmp(word, "string") == 0)
        || (strcmp(word, "entry") == 0)  
        || (strcmp(word, "extern") == 0))
    {
        return 1;
    }
    else return 0;
}


/*
 * Checks if a word is a register.
 * Returns 1 if it is a register, 0 otherwise.
 */
static int is_register(char* word)
{
    if (strlen(word) != 2)
    {
        return 0;
    }
    if (word[0] != 'r')
    {
        return 0;
    }
    if (word[1] < '0' || word[1] > '7')
    {
        return 0;
    }
    return 1;
}
