#include "parser.h"


static int add_label_to_array(Labels*** label_array, int* label_array_counter, char* label_name, int label_len, int l_type);
static int parse_line(Ast* curr_ast,char* line, int line_counter, Labels*** label_array, int* label_array_counter ,char** macro_array, int macro_array_counter, char* am_file_name_with_ext);


int parse_file(FILE* am_file, char* am_file_name_with_ext, Ast*** ast_array, int* ast_array_counter, Labels*** label_array, int* label_array_counter, char** macro_array, int macro_array_counter)
{
    char* buffer;
    int line_len;
    int line_counter = 0;
    int i;
    int parse_file_error = 0;

    /* allocate memory for unknown amount of asts and Labels */
    (*ast_array) = (Ast**)calloc(1, sizeof(Ast*));
    if (*ast_array == NULL)
    {
        fprintf(stderr, "Error: memory allocation failure\n");
        return -1;
    }
    
    (*label_array) = (Labels**)calloc(1, sizeof(Labels*));
    if (label_array == NULL)
    {
        fprintf(stderr, "Error: memory allocation failure\n");
        return -1;
    }

    while (1)
    {
        /* Reallocate memory for the new line */
        {
            line_len = line_length(am_file);
            if (line_len == 0)
            {
                break;
            }
            buffer = (char*)calloc(line_len + 1, sizeof(char));
            if(buffer == NULL)
            {
                fprintf(stderr, "Error: memory allocation failure\n");
                return -1;
            }
        }

        {/* update line data */
            fgets(buffer, line_len+1, am_file);/* get a new line */
            buffer[line_len] = '\0';/* add null terminator */
            line_counter++; /* update line counter */
        }

        {/* allocate memory for the new ast */
            Ast** temp;
            temp = (Ast**)realloc(*ast_array, (*ast_array_counter + 1) * sizeof(Ast*));
            if (temp == NULL)
            {
                fprintf(stderr, "Error: memory allocation failure\n");
                free(buffer);
                buffer = NULL; /* avoid dangling pointer */
                return -1;
            }
            (*ast_array_counter)++;
            (*ast_array) = temp;
            (*ast_array)[*ast_array_counter-1] = calloc(1, sizeof(Ast));
            if ((*ast_array)[*ast_array_counter-1] == NULL)
            {
                fprintf(stderr, "Error: memory allocation failure\n");
                free(buffer);
                buffer = NULL; /* avoid dangling pointer */
                return -1;
            }
        }

        {/* parse line and update current ast contents */
            if(parse_line((*ast_array)[*ast_array_counter-1], buffer, line_counter, label_array, label_array_counter, macro_array, macro_array_counter, am_file_name_with_ext) == -1)
            {
                return -1;
            }
        }
        

        if(feof(am_file))
        {/* finished parsing .am file */
            free(buffer);
            buffer = NULL; /* avoid dangling pointer */
            break;
        }
        free(buffer);
        buffer = NULL; /* avoid dangling pointer */
    }

    /* 
     * all asts created
     * check if all labels that are used as operands in instruction line are defined and exist in the label array
     * check if all labels that are defined as entries are defined in the label array and make sure that there are no redefinitions
     * if any error occurs, update ast is_error flag, print error to output file and free ast.tokens
     */

    for(i = 0; i < *ast_array_counter; i++)
    {
        if ((*ast_array)[i]->is_error == 1)
        {
            printf("%s", (*ast_array)[i]->error_message);
            if (parse_file_error == 0)
            {
                parse_file_error = 1;
            }
        }
        else
        {
            if ((*ast_array)[i]->ast_type == DIRECTIVE && (*ast_array)[i]->ast_options.Directive.dir_name == ENTRY)
            {
                int j;
                if((j = check_if_label_exist((*ast_array)[i]->ast_options.Directive.dir_content.label, *label_array, *label_array_counter)) == -1)
                {
                    printf("Error in source code file %s in line %d: label \"%s\" is not declared in this file and therefore cannot be defined as an entry\n", am_file_name_with_ext, (*ast_array)[i]->am_line_number ,(*ast_array)[i]->ast_options.Directive.dir_content.label);
                    if (parse_file_error == 0)
                    {
                        parse_file_error = 1;
                    }
                }
                else
                {
                    (*label_array)[j]->l_type = ENTRY_L;
                }
            }
            else if ((*ast_array)[i]->ast_type == INSTRUCTION && (*ast_array)[i]->ast_options.Instruction.num_of_operands > 0)
            {
                int j;
                for(j = 0; j < 2; j++)
                {
                    if ((*ast_array)[i]->ast_options.Instruction.Operand[j].op_type == DIRECT)
                    {
                        if(check_if_label_exist((*ast_array)[i]->ast_options.Instruction.Operand[j].op_data.label, *label_array, *label_array_counter) == -1)
                        {
                            printf("Error in source code file %s in line %d: undefined label \"%s\" is used as an operand\n", am_file_name_with_ext, (*ast_array)[i]->am_line_number ,(*ast_array)[i]->ast_options.Instruction.Operand[j].op_data.label);
                            if (parse_file_error == 0)
                            {
                                parse_file_error = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return parse_file_error;
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
 * This function reallocates memory to increase the size of the label array, 
 * then allocates memory for a new label and its name, and finally adds 
 * the label to the array. The label's name and type are set accordingly.
 * 
 * Returns 1 if the label was added successfully, -1 if memory allocation failed.
 */
static int add_label_to_array(Labels*** label_array, int* label_array_counter, char* label_name, int label_len, int l_type)
{
    Labels** temp;
    temp = (Labels**)realloc(*label_array, (*label_array_counter + 1) * sizeof(Labels*));
    if (temp == NULL)
    {
        fprintf(stderr, "Error: memory allocation failure\n");
        return -1;
    }
    *label_array = temp;
    (*label_array)[*label_array_counter] = calloc(1, sizeof(Labels));
    if((*label_array)[*label_array_counter] == NULL)
    {
        fprintf(stderr, "Error: memory allocation failure\n");
        return -1;
    }
    (*label_array)[*label_array_counter]->l_name = (char*)calloc(label_len + 1, sizeof(char));
    if ((*label_array)[*label_array_counter]->l_name == NULL)
    {
        fprintf(stderr, "Error: memory allocation failure\n");
        return -1;
    }
    strcpy((*label_array)[*label_array_counter]->l_name, label_name);
    (*label_array)[*label_array_counter]->l_type = l_type;
    (*label_array_counter)++;
    return 1;
}


/*
 * Parses a line from the .am file and updates the ast accordingly.
 * Returns -1 if encountered memory allocation failure, 1 if ast was updated.
 */
static int parse_line(Ast* curr_ast,char* line, int line_counter, Labels*** label_array, int* label_array_counter ,char** macro_array, int macro_array_counter, char* am_file_name_with_ext)
{
    Lexed_Line curr_lexed_line = {0};
    int curr_tok = 0;
    curr_lexed_line.line_type = -1;
    curr_ast->am_line_number = line_counter;

    sep_line_to_tokens(&curr_lexed_line, line, line_counter, macro_array, macro_array_counter, *label_array, *label_array_counter);
    if(curr_lexed_line.is_error == -1)
    {
        free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
        return -1;
    }
    
    else if (curr_lexed_line.is_error == 1)
    {
        sprintf(curr_ast->error_message, "Error in source code file %s %s", am_file_name_with_ext, curr_lexed_line.error_message);
        curr_ast->is_error = 1;
        free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
        return 1;
    }

    if (curr_lexed_line.label_declaration != 0)
    {
        if (curr_lexed_line.label_declaration == 1)
        {
            int len = 0;
            len = strlen(curr_lexed_line.tokens[curr_tok]);
            curr_lexed_line.tokens[curr_tok][len-1] = '\0';/* remove the ':' from the label name */
            len--;
            if(add_label_to_array(label_array, label_array_counter, curr_lexed_line.tokens[curr_tok], len, NONE) == -1)
            {
                free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                return -1;
            }
            curr_ast->label_dec_name = (char*)calloc(len+1, sizeof(char));
            if (curr_ast->label_dec_name == NULL)
            {
                fprintf(stderr, "Error: memory allocation failure\n");
                free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                return -1;
            }
            strcpy(curr_ast->label_dec_name, curr_lexed_line.tokens[curr_tok]);
            curr_ast->is_label_declaration = 1;
        }
        else{/* case of label declaration before .entry or .extern: ignore declaration and error note */
            printf("Warning, in source code file %s in line %d: label declaration before entry/extern directive ignored\n", am_file_name_with_ext, line_counter);
        }
        curr_tok++;
    }

    if (curr_lexed_line.line_type == COMMENT_LINE)
    {
        curr_ast->ast_type = COMMENT_LINE;
        free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
        return 1;
    }

    
    else if (curr_lexed_line.line_type == DIRECTIVE)
    {
        curr_ast->ast_type = DIRECTIVE;
        curr_ast->ast_options.Directive.dir_name = curr_lexed_line.dir_name;
        curr_tok++;
        if (curr_lexed_line.dir_name == DATA)
        {
            int numbers_counter = 0;
            int i = 1;/* numbers[0] is an indicator for the amount of numbers in the array and will be set to numbers_counter */
            while(curr_lexed_line.token_counter > curr_tok)
            {
                if (curr_lexed_line.tokens[curr_tok][0] == ',')
                {
                    curr_tok++;
                }
                else
                {
                    int j;
                    int num_of_bits = 15;
                    j = atoi(curr_lexed_line.tokens[curr_tok]);
                    if (curr_lexed_line.tokens[curr_tok][0] == '-' || curr_lexed_line.tokens[curr_tok][0] == '+')
                    {/* signed number */
                        if (j > MAX_SIGNED_15_BITS || j < MIN_SIGNED_15_BITS)
                        {
                            sprintf(curr_ast->error_message, "Error in source code file %s in line %d: immediate value %d is out of range for an unsigned %d bits number\n", am_file_name_with_ext, line_counter, j, num_of_bits);
                            curr_ast->is_error = 1;
                            free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                            return 1;
                        }
                    }
                    else 
                    {/* unsigned number */
                        if (j > MAX_UNSIGNED_15_BITS)
                        {
                            sprintf(curr_ast->error_message, "Error in source code file %s in line %d: immediate value %d is out of range for an unsigned %d bits number\n", am_file_name_with_ext, line_counter, j, num_of_bits);
                            curr_ast->is_error = 1;
                            free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                            return 1;
                        }
                    }
                    curr_ast->ast_options.Directive.dir_content.numbers[i] = j;
                    numbers_counter++;
                    i++;
                    curr_tok++;
                }
            }
            curr_ast->ast_options.Directive.dir_content.numbers[0] = numbers_counter;
            free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
            return 1;
        }
        else if(curr_lexed_line.dir_name == STRING)
        {
            int len;
            len = strlen(curr_lexed_line.tokens[curr_tok]);
            len -= 2;/* remove the opening and closing brackets from the string */
            curr_ast->ast_options.Directive.dir_content.dir_string_str = (char*)calloc(len+1, sizeof(char));
            if (curr_ast->ast_options.Directive.dir_content.dir_string_str == NULL)
            {
                fprintf(stderr, "Error: memory allocation failure\n");
                free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                if (curr_ast->is_label_declaration == 1)
                {
                    remove_label(*label_array, label_array_counter, curr_ast->label_dec_name);
                    free(curr_ast->label_dec_name);
                    curr_ast->label_dec_name = NULL; /* avoid dangling pointer */
                }
                return -1;
            }
            strncpy(curr_ast->ast_options.Directive.dir_content.dir_string_str, curr_lexed_line.tokens[curr_tok]+1, len);
            curr_ast->ast_options.Directive.dir_content.dir_string_str[len] = '\0';
            free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
            return 1;
        }
        else if(curr_lexed_line.dir_name == ENTRY)
        {
            int label_len;
            label_len = strlen(curr_lexed_line.tokens[curr_tok]);
            curr_ast->ast_options.Directive.dir_content.label = (char*)calloc(label_len + 1, sizeof(char));
            if (curr_ast->ast_options.Directive.dir_content.label == NULL)
            {
                fprintf(stderr, "Error: memory allocation failure\n");
                free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                return -1;
            }
            strcpy(curr_ast->ast_options.Directive.dir_content.label, curr_lexed_line.tokens[curr_tok]);
            free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
            return 1;
        }
        else
        {/* extern directive */
            int i;
            int label_len;
            if ((i = check_if_label_exist(curr_lexed_line.tokens[curr_tok], *label_array, *label_array_counter)) != -1)
            {
                if ((*label_array)[i]->l_type != EXTERNAL_L)
                {
                    if ((*label_array)[i]->l_type == ENTRY_L)
                    {
                        sprintf(curr_ast->error_message, "Error in source code file %s in line %d: label %s cannot be defined as entry and external label simultaneously\n", am_file_name_with_ext, line_counter, curr_lexed_line.tokens[curr_tok]);
                        curr_ast->is_error = 1;
                        free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                        return 1;
                    }
                    else
                    {/* (*label_array)[i]->l_type == NONE */
                        sprintf(curr_ast->error_message, "Error in source code file %s at line %d: Label \"%s\" is already declared in the scope therefore cannot be defined as extern\n", am_file_name_with_ext, line_counter, curr_lexed_line.tokens[curr_tok]);
                        curr_ast->is_error = 1;
                        free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                        return 1;
                    }
                }
                else
                {/* update ast data without adding the label to the array */
                    label_len = strlen((*label_array)[i]->l_name);
                    curr_ast->ast_options.Directive.dir_content.label = (char*)calloc(label_len + 1, sizeof(char));
                    if (curr_ast->ast_options.Directive.dir_content.label == NULL)
                    {
                        fprintf(stderr, "Error: memory allocation failure\n");
                        free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                        return -1;
                    }
                    strcpy(curr_ast->ast_options.Directive.dir_content.label, curr_lexed_line.tokens[curr_tok]);
                    free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                    return 1;
                }
            }
            /* add label to label array and update ast data */
            label_len = strlen(curr_lexed_line.tokens[curr_tok]);
            if(add_label_to_array(label_array, label_array_counter, curr_lexed_line.tokens[curr_tok], label_len, EXTERNAL_L) == -1)
            {
                free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                return -1;
            }
            curr_ast->ast_options.Directive.dir_content.label = (char*)calloc(label_len + 1, sizeof(char));
            if (curr_ast->ast_options.Directive.dir_content.label == NULL)
            {
                fprintf(stderr, "Error: memory allocation failure\n");
                free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                if (curr_ast->is_label_declaration == 1)
                return -1;
            }
            strcpy(curr_ast->ast_options.Directive.dir_content.label, curr_lexed_line.tokens[curr_tok]);
            free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
            return 1;
        }
    }
    else 
    {/* curr_lexed_line.line_type == INSTRUCTION) */
        int i = 0;
        curr_ast->ast_type = INSTRUCTION;
        curr_ast->ast_options.Instruction.inst_name = curr_lexed_line.inst_name;
        curr_ast->ast_options.Instruction.num_of_operands = curr_lexed_line.amount_of_operands;
        curr_tok++;

        if (curr_lexed_line.amount_of_operands == 0)
        {
            free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
            return 1;
        }

        if (curr_lexed_line.amount_of_operands == 1)
        {
            i++;
        }

        while (i < 2)
        {
            curr_ast->ast_options.Instruction.Operand[i].op_type = curr_lexed_line.operand_types[i];
            if (curr_lexed_line.operand_types[i] == IMMEDIATE)
            {/* 12 bits total max number */
                int j;
                int num_of_bits = 12;
                j = atoi(curr_lexed_line.tokens[curr_tok]+1);/* skip '#' with +1 to get to the beginning of the number */
                if (curr_lexed_line.tokens[curr_tok][2] == '-' || curr_lexed_line.tokens[curr_tok][2] == '+')
                {/* signed number */
                    if (j > MAX_SIGNED_12_BITS || j < MIN_SIGNED_12_BITS)
                    {
                        sprintf(curr_ast->error_message, "Error in source code file %s in line %d: immediate value %d is out of range for a signed %d bits number\n", am_file_name_with_ext, line_counter, j, num_of_bits);
                        curr_ast->is_error = 1;
                        free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                        return 1;
                    }
                }
                else 
                {/* unsigned number */
                    if (j > MAX_UNSIGNED_12_BITS)
                    {
                        sprintf(curr_ast->error_message, "Error in source code file %s in line %d: immediate value %d is out of range for an unsigned %d bits number\n", am_file_name_with_ext, line_counter, j, num_of_bits);
                        curr_ast->is_error = 1;
                        free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
                        return 1;
                    }
                }
                curr_ast->ast_options.Instruction.Operand[i].op_data.immed = j;
            }
            else if (curr_lexed_line.operand_types[i] == DIRECT)
            {
                int label_len;
                label_len = strlen(curr_lexed_line.tokens[curr_tok]);
                curr_ast->ast_options.Instruction.Operand[i].op_data.label = (char*)calloc(label_len + 1, sizeof(char));
                if (curr_ast->ast_options.Instruction.Operand[i].op_data.label == NULL)
                {
                    fprintf(stderr, "Error: memory allocation failure\n");
                    return -1;
                }
                else 
                {
                    strcpy(curr_ast->ast_options.Instruction.Operand[i].op_data.label, curr_lexed_line.tokens[curr_tok]);
                }
                /* optional but not needed for now */
                /* case where in line of a label declaration there is a usage of the label itself as an operand for the instruction */
                /*
                if (curr_ast->is_label_declaration == 1)
                {
                    if (strcmp(curr_ast->label_dec_name, curr_ast->ast_options.Instruction.Operand[i].op_data.label) == 0)
                    {
                        sprintf(curr_ast->error_message, "Error in source code file %s in line %d: label \"%s\" is declared and used in the same line\n", am_file_name_with_ext, line_counter, curr_ast->label_dec_name);
                        curr_ast->is_error = 1;
                        return 1;
                    }
                } 
                 */
            }
            else if (curr_lexed_line.operand_types[i] == REGISTER_DIRECT)
            {
                curr_ast->ast_options.Instruction.Operand[i].op_data.reg_num = atoi(curr_lexed_line.tokens[curr_tok]+1);/* skip 'r' with +1 to get reg number */
            }
            else if (curr_lexed_line.operand_types[i] == REGISTER_INDIRECT)
            {
                curr_ast->ast_options.Instruction.Operand[i].op_data.reg_num = atoi(curr_lexed_line.tokens[curr_tok]+2);/* skip '*r' with +2 to get reg number */
            } 
            i++;
            curr_tok += 2;/* operand token and comma token */
        }
    }
    free_matrix_char_pointer(&(curr_lexed_line.tokens), curr_lexed_line.token_counter);
    return 1;    
}
