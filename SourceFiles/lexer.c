#include "lexer.h"


/* static functions declarations */
static int op_type_vs_addressing_type(int inst_name, int op_type, int op_counter);
static int add_token_to_struct(Lexed_Line* ret, char* curr_tok, int* curr_tok_len);
static int set_instruction(Lexed_Line* ret,int inst_num, char* name_of_instruction, int op_amount, char* curr_tok, int* curr_tok_len, int* curr_idx);
static int get_instruction_name(int* curr_idx, char* buffer, Lexed_Line* ret, char* curr_tok, int* curr_tok_len);
static int get_rest_of_number(int* curr_idx, char* buffer,Lexed_Line* ret, char* curr_tok, int* curr_tok_len, int line_counter);
static int get_number(int* curr_idx, char* buffer,Lexed_Line* ret, char* curr_tok, int* curr_tok_len, int line_counter);
static int is_label(int* curr_idx, char* buffer, char* curr_tok, int* curr_tok_len, char** macro_array, int macro_counter, Lexed_Line* ret, int line_counter, int target);
static int point(int* curr_idx, char* buffer, Lexed_Line* ret, char* curr_tok, int* curr_tok_len, int line_counter, char** macro_array, int macro_counter);
static int get_operand(int* curr_idx, char* buffer, Lexed_Line* ret, char* curr_tok, int* curr_tok_len, int line_counter, char** macro_array, int macro_counter);
static int get_operands(int* curr_idx, char* buffer, Lexed_Line* ret, char* curr_tok, int* curr_tok_len, int line_counter, char** macro_array, int macro_counter);


void sep_line_to_tokens(Lexed_Line* ret, char* buffer, int line_counter, char** macro_array, int macro_counter, Labels** label_array, int label_array_counter)
{
    int curr_tok_len = 0;/* token length indicator */
    int curr_idx = 0;/* current reading position in line */
    int i;
    char curr_tok[MAX_LINE_LEN];
    memset(curr_tok, 0, MAX_LINE_LEN);

    /* dynamic allocation for the line tokens */
    ret->tokens = (char**)calloc(1, sizeof(char*));
    if(ret->tokens == NULL)
    {
        fprintf(stderr, "Error: memory allocation failure\n");
        ret->is_error = -1;
        return;
    }     
    
    /* move pointer to the first char in line */
    skip_white_spaces(buffer, &curr_idx);

    /* check for comment line, line length can be longer than 80 chars in this case */
    if (buffer[curr_idx] == ';')
    {
        ret->line_type = COMMENT_LINE;
        return;
    }

    /* not a comment line */

    i = strlen(buffer);
    if (i > MAX_LINE_LEN-1)
    {/* any command line that exceeds the MAX_LINE_LEN is invalid and an error will be printed */
        sprintf(ret->error_message, "in line %d: line length is over %d characters\n", line_counter, MAX_LINE_LEN-1);
        ret->is_error = 1;
        return;
    }

    /* buffer[curr_idx] points to the first char in line after whitespaces */
    /* separate the line to tokens */
    while (buffer[curr_idx] != '\n' && buffer[curr_idx] != '\0')
    {
        if(ret->token_counter == 0)
        {/* valid cases: first char is a letter or a point (a beginning of a label declaration, a beginning of a directive and a beginning of an instruction) */
            if (buffer[curr_idx] == '.')
            {/* check if the command line is a valid directive line */
                i = point(&curr_idx, buffer, ret, curr_tok, &curr_tok_len, line_counter, macro_array, macro_counter);
                if (i == -1)
                {
                    return;
                }
                else if (i == -2)
                {
                    return;
                }
                else 
                {
                    ret->line_type = DIRECTIVE;
                    ret->dir_name = i;
                    return;
                }
            }/* end of directive case */
            else if (isalpha(buffer[curr_idx]))
            {/* check if the first token is an instruction or a label declaration */
                if((i = get_instruction_name(&curr_idx, buffer, ret, curr_tok, &curr_tok_len)) == -1)
                {
                    return;
                }
                else if (i == 1)
                {/* instruction */
                    continue;/* continue to next iteration after line_type is set to instruction */
                }
                else 
                {/* not an instruction */
                    while (isdigit(buffer[curr_idx]) || isalpha(buffer[curr_idx]))
                    {
                        curr_tok[curr_tok_len] = buffer[curr_idx];
                        curr_tok_len++;
                        curr_idx++;
                    }
                    if (buffer[curr_idx] == ':' && (buffer[curr_idx +1] == '\t' || buffer[curr_idx +1] == ' '))
                    {/* case of label declaration */
                        if(is_macro(curr_tok, macro_array, macro_counter) != -1)
                        {
                            sprintf(ret->error_message, "in line %d: error in label declaration, label name cannot be the same as an existing macro name\n", line_counter);
                            ret->is_error = 1;
                            return;
                        }
                        else if (is_reserved_word(curr_tok))
                        {
                            sprintf(ret->error_message, "in line %d: label name (\"%s\") is a reserved keyword\n", line_counter, curr_tok);
                            ret->is_error = 1;
                            return;
                        }
                        else if(check_if_label_exist(curr_tok, label_array, label_array_counter) != -1)
                        {
                            sprintf(ret->error_message, "in line %d: label name (\"%s\") already declared therefore cannot be declared again\n", line_counter, curr_tok);
                            ret->is_error = 1;
                            return;
                        }
                        else 
                        {
                            curr_tok[curr_tok_len] = ':';
                            curr_tok_len++;
                            curr_idx++;
                            if(is_macro(curr_tok, macro_array, macro_counter) != -1)
                            {/* case of macro name that ends with ":" that was not implemented due to the extraneous text in line */
                                sprintf(ret->error_message, "in line %d: macro \"%s\" was not implemented due to extraneous text in line after end of macro name\n", line_counter, curr_tok);
                                ret->is_error = 1;
                                return;
                            }
                            /* not a macro name, save token and continue line tokenization */
                            if (add_token_to_struct(ret, curr_tok, &curr_tok_len) == -1)
                            {
                                return;
                            }
                            else 
                            {/* continue to next iteration after label_dec is set to 1 */
                                if (skip_white_spaces(buffer, &curr_idx))
                                {/* empty label */
                                    sprintf(ret->error_message, "in line %d: cannot declare an empty label\n", line_counter);
                                    ret->is_error = 1;
                                    return;
                                }   
                                ret->label_declaration = 1;
                                continue;
                            }
                        }
                    }
                    else
                    {/* case first token is not a label declaration and not and instruction */
                        sprintf(ret->error_message, "in line %d: syntax error\n", line_counter);
                        ret->is_error = 1;
                        return;
                    }
                }
            }/* end of case first token in line starts with a letter */
            else
            {/* first token in line starts with a symbol */
                sprintf(ret->error_message, "in line %d: syntax error\n", line_counter);
                ret->is_error = 1;
                return;
            }
        }/* end of case token counter == 0 */
        else if (ret->line_type == INSTRUCTION)
        {/* case of instruction line -> get operands */
            get_operands(&curr_idx, buffer, ret, curr_tok, &curr_tok_len, line_counter, macro_array, macro_counter);  
            return;  
        }
        else if (ret->label_declaration == 1)
        {/* optional cases: beginning of directive (point) and beginning of instruction */
            if (buffer[curr_idx] == '.')
            {/* check if the directive line is valid */
                i = point(&curr_idx, buffer, ret, curr_tok, &curr_tok_len, line_counter, macro_array, macro_counter);
                if (i == -1 || i == -2)
                {
                    return;
                }
                else
                {
                    ret->line_type = DIRECTIVE;
                    ret->dir_name = i;
                    return; 
                }
            }
            else 
            {/* check if the instruction line is valid */
                i = get_instruction_name(&curr_idx, buffer, ret, curr_tok, &curr_tok_len);
                if (i == -1)
                {
                    return;
                }
                else if (i == 1)
                {/* case token is an instruction */
                    /* index was updated inside function and line_type changed to instruction mode */
                    continue;
                }
                else 
                {/* not an instruction */
                    sprintf(ret->error_message, "in line %d: invalid command after label declaration\n", line_counter);
                    ret->is_error = 1;
                    return;
                }
            }
        }
        else 
        {
            sprintf(ret->error_message, "in line %d: syntax error\n", line_counter);
            ret->is_error = 1;
            return; 
        }
    }/* end of while loop, finished line tokenization */
    return;  
}/* end of sep_line func */ 


/*
 *    STATIC FUNCTIONS ONLY
 *            ||
 *            ||
 *            ||
 *           \  /
 *            \/                                               
 */


/*
 * This function checks whether the operand type is appropriate for the specific instruction and operand
 * position. Each instruction has different requirements for addressing types, and this function ensures that
 * the operand type conforms to these requirements. 
 * 
 * Returns:
 * 1 if the addressing type is valid for the given instruction and operand position.
 * 0 if the addressing type is invalid for the given instruction and operand position.
 */
static int op_type_vs_addressing_type(int inst_name, int op_type, int op_counter)
{
    switch (inst_name)
    {
        case MOV:
        case ADD:
        case SUB:
            if (op_counter == 0)
            {
                return 1;
            }
            else 
            {/* (op_counter == 1) */
                if (op_type == IMMEDIATE)
                {
                    return 0;
                }
                else return 1;
            }
        case CMP:
            return 1;
        case LEA:
            if (op_counter == 0)
            {
                if (op_type == DIRECT)
                {
                    return 1;
                }
                else return 0;
            }
            else 
            {/* (op_counter == 1) */
                if (op_type == IMMEDIATE)
                {
                    return 0;
                }
                else return 1;
            }
        case CLR:
        case NOT:
        case INC:
        case DEC:
        case RED:
            if (op_type == IMMEDIATE)
            {
                return 0;
            }
            else return 1;
        case JMP:
        case BNE:
        case JSR:
            if (op_type == DIRECT || op_type == REGISTER_INDIRECT)
            {
                return 1;
            }
            else return 0;
        case PRN:
            return 1;
    }
    return 0;
}


/*
 * This function adds the current token to the `tokens` array in the `Lexed_Line` structure. It first attempts
 * to resize the `tokens` array using `realloc`. If successful, it allocates memory for the new token and copies
 * the token into this newly allocated space. The function also handles potential memory allocation errors and
 * resets the token buffer.  
 * 
 * Returns:
 * 1 on success, indicating that the token was added to the `tokens` array successfully.
 * -1 if memory allocation fails, with an error message printed to `stderr`.
 */
static int add_token_to_struct(Lexed_Line* ret, char* curr_tok, int* curr_tok_len)
{
    char** temp;
    temp = (char**)realloc(ret->tokens, (ret->token_counter+1)*sizeof(char*));
    if (temp == NULL)
    {
        fprintf(stderr, "Error: memory allocation failure\n");
        ret->is_error = -1;
        return -1;
    }
    ret->tokens = temp;
    ret->tokens[ret->token_counter] = (char*)calloc(*curr_tok_len+1, sizeof(char));
    if (ret->tokens[ret->token_counter] == NULL)
    {
        fprintf(stderr, "Error: memory allocation failure\n");
        ret->is_error = -1;
        return -1;
    }
    strncpy(ret->tokens[ret->token_counter], curr_tok, *curr_tok_len);
    ret->tokens[ret->token_counter][*curr_tok_len] = '\0';
    memset(curr_tok, 0, *curr_tok_len);
    *curr_tok_len = 0;
    ret->token_counter++;
    return 1;
} 


/*
 * This function sets the appropriate fields in the `Lexed_Line` structure to represent an instruction.
 * It updates the token buffer with the instruction name, adjusts the current index to point after the instruction name,
 * and attempts to add the token to the structure. The index will point to the next character after the instruction name
 * upon successful completion.
 * 
 * Returns:
 * 1 on success, indicating that the instruction was set and the token was added successfully.
 * -1 if there is a failure in adding the token due to memory allocation issues.
 */
static int set_instruction(Lexed_Line* ret,int inst_num, char* name_of_instruction, int op_amount, char* curr_tok, int* curr_tok_len, int* curr_idx)
{
    ret->line_type = INSTRUCTION;
    ret->inst_name = inst_num;
    ret->amount_of_operands = op_amount;
    strncpy(curr_tok, name_of_instruction, strlen(name_of_instruction));
    *curr_tok_len = strlen(name_of_instruction);
    *curr_idx += strlen(name_of_instruction);
    return (add_token_to_struct(ret, curr_tok, curr_tok_len));
}


/*
 * This function checks if the current segment of the buffer matches any known instruction. If a match is found, 
 * it updates the instruction details using `set_instruction` and adjusts the index and token buffer accordingly.
 * 
 * Returns:
 * 1 if an instruction is successfully identified and set.
 * -1 if there is a failure in adding the token due to memory allocation issues.
 * 0 if the current buffer segment does not match any known instruction.
 */
static int get_instruction_name(int* curr_idx, char* buffer, Lexed_Line* ret, char* curr_tok, int* curr_tok_len)
{/* check instruction name and set data in struct */
    int i;
    if(strncmp(buffer+(*curr_idx), "mov ", 4) == 0)
    {
        i = set_instruction(ret, MOV, "mov", 2, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "cmp ", 4) == 0)
    {
        i = set_instruction(ret, CMP, "cmp", 2, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "add ", 4) == 0)
    {
        i = set_instruction(ret, ADD, "add", 2, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "sub ", 4) == 0)
    {
        i = set_instruction(ret, SUB, "sub", 2, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "not ", 4) == 0)
    {
        i = set_instruction(ret, NOT, "not", 1, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "clr ", 4) == 0)
    {
        i = set_instruction(ret, CLR, "clr", 1, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "lea ", 4) == 0)
    {
        i = set_instruction(ret, LEA, "lea", 2, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "inc ", 4) == 0)
    {
        i = set_instruction(ret, INC, "inc", 1, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "dec ", 4) == 0)
    {
        i = set_instruction(ret, DEC, "dec", 1, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "jmp ", 4) == 0)
    {
        i = set_instruction(ret, JMP, "jmp", 1, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "bne ", 4) == 0)
    {
        i = set_instruction(ret, BNE, "bne", 1, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "red ", 4) == 0)
    {
        i = set_instruction(ret, RED, "red", 1, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "prn ", 4) == 0)
    {
        i = set_instruction(ret, PRN, "prn", 1, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "jsr ", 4) == 0)
    {
        i = set_instruction(ret, JSR, "jsr", 1, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "rts ", 4) == 0
            || strncmp(buffer+(*curr_idx), "rts\0", 4) == 0
            || strncmp(buffer+(*curr_idx), "rts\n", 4) == 0)
    {
        i = set_instruction(ret, RTS, "rts", 0, curr_tok, curr_tok_len, curr_idx);
    }
    else if(strncmp(buffer+(*curr_idx), "stop ", 5) == 0
            || strncmp(buffer+(*curr_idx), "stop\n", 5) == 0
            || strncmp(buffer+(*curr_idx), "stop\0", 5) == 0)
    {
        i = set_instruction(ret, STOP, "stop", 0, curr_tok, curr_tok_len, curr_idx);
    }
    else
    {
        return 0;
    }
    return i;
}


/*
 * This function processes the remaining part of a number in the buffer, appending each digit to the token. 
 * It stops when a non-digit character is encountered, such as a space, tab, comma, or end of line. 
 * The function updates the token and index accordingly.
 * 
 * Returns:
 * 1 if the number is valid and the first non-whitespace character after the number is a comma or end of line.
 * 0 if there is a missing comma after the number or an error occurs.
 * -1 if there is a memory allocation failure (handled in `add_token_to_struct`).
 */
static int get_rest_of_number(int* curr_idx, char* buffer,Lexed_Line* ret, char* curr_tok, int* curr_tok_len, int line_counter)
{
    while (buffer[*curr_idx] >= '0' && buffer[*curr_idx] <= '9')
    {
        curr_tok[*curr_tok_len] = buffer[*curr_idx];
        (*curr_tok_len)++;
        (*curr_idx)++;
    }
    if (skip_white_spaces(buffer, curr_idx) == 1 || buffer[*curr_idx] == ',')
    {
        return(add_token_to_struct(ret, curr_tok, curr_tok_len));
    }
    else 
    {
        sprintf(ret->error_message, "in line %d: missing comma after number\n", line_counter);
        ret->is_error = 1;
        return 0;
    }
}


/*
 * This function processes a number from the current buffer segment. It handles optional signs (`+` or `-`) and checks for valid formatting, including leading zeros. 
 * If the number is correctly formatted, it updates the token buffer and index accordingly. 
 * After successful execution, the index will point to the next character after the number (end of line or comma).
 * 
 * Returns:
 * 1 if the number is successfully parsed and validated.
 * 0 if the number is invalid or improperly formatted.
 * -1 if there is a memory allocation failure (handled in `add_token_to_struct`).
 */
static int get_number(int* curr_idx, char* buffer,Lexed_Line* ret, char* curr_tok, int* curr_tok_len, int line_counter)
{
    if (buffer[*curr_idx] != '+' && buffer[*curr_idx] != '-' && (buffer[*curr_idx] < '0' || buffer[*curr_idx] > '9'))
    {/* invalid case */
        sprintf(ret->error_message, "in line %d: syntax error\n", line_counter);
        ret->is_error = 1;
        return 0;
    }
    else if(buffer[*curr_idx] == '0')
    {/* zero case */
        (*curr_idx)++;
        while (buffer[*curr_idx] == '\t' || buffer[*curr_idx] == ' ')
        {
            (*curr_idx)++;
        }
        if (buffer[*curr_idx] != ',' && buffer[*curr_idx] != '\0' && buffer[*curr_idx] != '\n')
        {
            sprintf(ret->error_message, "in line %d: missing comma\n", line_counter);
            ret->is_error = 1;
            return 0;
        }
        else
        {
            curr_tok[*curr_tok_len] = '0';
            (*curr_tok_len)++;
            return add_token_to_struct(ret, curr_tok, curr_tok_len);
        }
    }
    else if (buffer[*curr_idx] == '+' || buffer[*curr_idx] == '-')
    {/* signed number case */
        curr_tok[*curr_tok_len] = buffer[*curr_idx];
        (*curr_tok_len)++;
        (*curr_idx)++;
        if (buffer[*curr_idx] < '1' || buffer[*curr_idx] > '9')
        {
            sprintf(ret->error_message, "in line %d: invalid number\n", line_counter);
            ret->is_error = 1;
            return 0;
        }
        else {
            curr_tok[*curr_tok_len] = buffer[*curr_idx];
            (*curr_tok_len)++;
            (*curr_idx)++;
            return (get_rest_of_number(curr_idx, buffer, ret, curr_tok, curr_tok_len, line_counter));
        }
    }
    else
    {/* case first char is 1-9 */
        return (get_rest_of_number(curr_idx, buffer, ret, curr_tok, curr_tok_len, line_counter));
    }
}


/*
 * This function checks if the current segment of the buffer represents a valid label name. It updates
 * the token buffer with the label name if valid and advances the index to the next character after the label.
 * It also handles errors related to labels duplications, invalid characters, reserved keywords, and macro names.
 * Target indicates if the call for this function is for an operand inside an instruction line (1) or a directive line(0).
 * 
 * Returns 1 if the label is valid and successfully processed, 0 if the label is invalid or in case of syntax error.
 */
static int is_label(int* curr_idx, char* buffer, char* curr_tok, int* curr_tok_len, char** macro_array, int macro_counter, Lexed_Line* ret, int line_counter, int target)
{
    char word[31];/* store current token */
    int i = 0;
    if (skip_white_spaces(buffer, curr_idx))
    {/* empty word */
        sprintf(ret->error_message, "in line %d: missing label name\n", line_counter);
        ret->is_error = 1;
        return 0;
    }
    /* check that label name starts with a letter */
    if (!isalpha(buffer[*curr_idx]))
    {
        sprintf(ret->error_message, "in line %d: syntax error, invalid label name (label name must start with a letter)\n", line_counter);
        ret->is_error = 1;
        return 0;
    }

    while (buffer[*curr_idx] != '\0'
         && buffer[*curr_idx] != '\n'
         && buffer[*curr_idx] != ' '
         && buffer[*curr_idx] != '\t'
         && (isdigit(buffer[*curr_idx]) || isalpha(buffer[*curr_idx]))
         && i < MAX_LABEL_LEN)
    {
        word[i] = buffer[*curr_idx];
        i++;
        (*curr_idx)++;
    }

    if (i == MAX_LABEL_LEN)
    {/* case of label name longer than 30 chars */
        if (target == 0)/* case of instruction line */
        {
            sprintf(ret->error_message, "in line %d: invalid operand\n", line_counter);
            ret->is_error = 1;
            return 0;
        }
        else /* case of directive line */
        {
            sprintf(ret->error_message, "in line %d: label name is over %d chars\n", line_counter, MAX_LABEL_LEN);
            ret->is_error = 1;
            return 0;
        }
    }

    if (buffer[*curr_idx] != '\0' && buffer[*curr_idx] != '\n' && buffer[*curr_idx] != ' ' && buffer[*curr_idx] != '\t')
    {/* case of symbol char in label name */
        if (target == 0 && (buffer)[*curr_idx] == ',')/* case of instruction line and a comma right after the label name */
        {/* a direct operand can end with a comma without separation of whitespaces from the label name */
            word[i] = '\0';
            strcpy(curr_tok, word);
            *curr_tok_len += i;
        }
        else 
        {/* invalid case */
            target == 0? sprintf(ret->error_message, "in line %d: invalid operand\n", line_counter) : sprintf(ret->error_message, "in line %d: label name can contain digits and letters only\n", line_counter);
            ret->is_error = 1;
            return 0;
        }    
    }
    word[i] = '\0';
    if(is_reserved_word(word))
    {
        target == 0? sprintf(ret->error_message, "in line %d: operand (\"%s\") is a reserved keyword\n", line_counter, word) : sprintf(ret->error_message, "in line %d: label name (\"%s\") is a reserved keyword\n", line_counter, word);
        ret->is_error = 1;
        (*curr_idx) -= i;
        return 0;
    }

    else if (is_macro(word, macro_array, macro_counter) != -1)
    {
        target == 0? sprintf(ret->error_message, "in line %d: operand (\"%s\") is a macro name\n", line_counter, word) : sprintf(ret->error_message, "in line %d: label name (\"%s\") is a macro name\n", line_counter, word);
        ret->is_error = 1;
        (*curr_idx) -= i;
        return 0;
    }

    else 
    {
        strcpy(curr_tok, word);
        *curr_tok_len += i;
        return 1;
    }  
}


/*
 * This function parses directive lines from the source code buffer and updates the `Lexed_Line`
 * structure based on the directive type. It handles syntax errors, missing components, and validates
 * directive content. It ensures that directives are followed by the correct format and components.
 * 
 * Returns:
 * -2 if an error occurs.
 * -1 if a memory allocation error occurs.
 * 0 if the line is valid and the directive is .data.
 * 1 if the line is valid and the directive is .string.
 * 2 if the line is valid and the directive is .entry.
 * 3 if the line is valid and the directive is .extern.
 */
static int point(int* curr_idx, char* buffer, Lexed_Line* ret, char* curr_tok, int* curr_tok_len, int line_counter, char** macro_array, int macro_counter)
{
    int i;
    int len = 0;
    while (buffer[*curr_idx] != ' ' && buffer[*curr_idx] != '\t' && buffer[*curr_idx] != '\n' && buffer[*curr_idx] != '\0') 
    {/* get the current token */
        (*curr_idx)++;
        len++;
    }
    (*curr_idx) -= len;
    curr_tok[*curr_tok_len] = '.';
    (*curr_tok_len)++;
    (*curr_idx)++;
    if (len < 5)
    {/* a directive without data or an invalid directive */
        sprintf(ret->error_message, "in line %d: syntax error\n", line_counter);
        ret->is_error = 1;
        return -2;
    }
    else if (strncmp(buffer + *curr_idx, "data ", 5) == 0)
    {/* data directive */
        strcpy(curr_tok + 1, "data");
        (*curr_tok_len) += 4;
        (*curr_idx) += 5;
        if(add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
        {
            return -1;
        }
        if(skip_white_spaces(buffer, curr_idx) == 1)
        {/* skip white spaces */
            sprintf(ret->error_message, "in line %d: at least one number must appear after .data directive\n", line_counter);
            ret->is_error = 1;
            return -2;
        }
        /* check for at least one number after .data directive */
        i = get_number(curr_idx, buffer, ret, curr_tok, curr_tok_len, line_counter);
        if (i == 0)
        {/* invalid number */
            return -2;
        }
        else if (i == -1)
        {/* memory allocation failure */
            return -1;
        }
        if (buffer[*curr_idx] == '\n' || buffer[*curr_idx] == '\0')
        {/* data with only one number */
            return DATA;
        }

        else
        {/* case of more than one number, check that rest of the numbers are valid, that line ends with a number and that there are no cases of consecutive commas */
            while (buffer[*curr_idx] != '\n' && buffer[*curr_idx] != '0')
            {/* read the entire line */
                if (buffer[*curr_idx] == '\t' || buffer[*curr_idx] == ' ')
                {/* skip white spaces */
                    (*curr_idx)++;
                }
                else if (buffer[*curr_idx] == ',')
                {/* current token is a comma, check that the previous token was a number */
                    if(ret->tokens[ret->token_counter-1][0] == ',')
                    {/* check if previous token was a comma */
                        sprintf(ret->error_message, "in line %d: consecutive commas\n", line_counter);
                        ret->is_error = 1;
                        return -2;
                    }
                    else 
                    {/* update current token and continue reading line */
                        curr_tok[*curr_tok_len] = ',';
                        (*curr_tok_len)++;
                        (*curr_idx)++;
                        i = add_token_to_struct(ret, curr_tok, curr_tok_len);
                        if (i == -1)
                        {
                            return -1;
                        }
                    }
                }
                else if ((buffer[*curr_idx] >= '0' && buffer[*curr_idx] <= '9')
                         || buffer[*curr_idx] == '+'
                         || buffer[*curr_idx] == '-')
                {/* a beginning of a number */
                    if (ret->tokens[ret->token_counter-1][0] != ',')
                    {/* case previous token is not a comma */
                        sprintf(ret->error_message, "in line %d: missing comma\n", line_counter);
                        ret->is_error = 1;
                        return -2;
                    }
                    else 
                    {/* case previous token is a comma, get number */
                        i = get_number(curr_idx, buffer, ret, curr_tok, curr_tok_len, line_counter);
                        if (i == 0)
                        {/* invalid number */
                            return -2;
                        }
                        else if (i == -1)
                        {/* memory allocation failure */
                            return -1;
                        }
                        else
                        {/* valid number */
                            continue;
                        }
                    }
                }
                else 
                {
                    sprintf(ret->error_message, "in line %d: syntax error\n", line_counter);
                    ret->is_error = 1;
                    return -2;
                }
            }
            if (ret->tokens[ret->token_counter-1][0] == ',')
            {/* case line ends with a comma */
                sprintf(ret->error_message, "in line %d: line ends with a comma\n", line_counter);
                ret->is_error = 1;
                return -2;
            }
            /* line ends with a number */
            else return DATA;
        }
    }
    else if (len < 6)
    {
        sprintf(ret->error_message, "in line %d: syntax error\n", line_counter);
        ret->is_error = 1;
        return -2;
    }
    else if (strncmp(buffer + *curr_idx, "entry ", 6) == 0)
    {/* entry directive */
        if (ret->label_declaration == 1)
        {/* ignore label declaration before entry directive (set flag to -1) */
            ret->label_declaration = -1;
        }
        strcpy(curr_tok + 1, "entry");
        *curr_tok_len += 5;
        *curr_idx += 6;
        if(add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
        {
            return -1;
        }
        if (skip_white_spaces(buffer, curr_idx) == 1)
        {
            sprintf(ret->error_message, "in line %d: missing label name\n", line_counter);
            ret->is_error = 1;
            return -2;
        }
        if (is_label(curr_idx, buffer, curr_tok, curr_tok_len, macro_array, macro_counter, ret, line_counter, 1))
        {/* check if current token is a valid label name */
            if(add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
            {/* memory allocation failure */
                return -1;
            }
            else 
            {/* valid label name */
                if (skip_white_spaces(buffer, curr_idx) != 1)
                {
                    sprintf(ret->error_message, "in line %d: extraneous text after end of command\n", line_counter);
                    ret->is_error = 1;
                    return -2;
                }
                else
                {
                    return ENTRY;   
                } 
            }
        }
        else 
        {/* invalid label name */
            return -2;
        }
    }
    else if (len < 7)
    {/* ignore label declaration before extern directive (set flag to -1) */
        sprintf(ret->error_message, "in line %d: syntax error\n", line_counter);
        ret->is_error = 1;
        return -2;
    }
    else if (strncmp(buffer + *curr_idx, "extern ", 7) == 0)
    {/* extern directive */
        if (ret->label_declaration == 1)
        {
            ret->label_declaration = -1;
        }
        strcpy(curr_tok + 1, "extern");
        *curr_tok_len += 6;
        *curr_idx += 7;
        if(add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
        {
            return -1;
        }
        if (skip_white_spaces(buffer, curr_idx) == 1)
        {
            sprintf(ret->error_message, "in line %d: missing label name\n", line_counter);
            ret->is_error = 1;
            return -2;
        }
        if (is_label(curr_idx, buffer, curr_tok, curr_tok_len, macro_array, macro_counter, ret, line_counter, 1))
        {/* check if current token is a valid label name */
            if(add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
            {/* memory allocation failure */
                return -1;
            }
            else 
            {/* valid label name */
                if (skip_white_spaces(buffer, curr_idx) != 1)
                {
                    sprintf(ret->error_message, "in line %d: extraneous text after end of command\n", line_counter);
                    ret->is_error = 1;
                    return -2;
                }
                return EXTERNAL;
            }
        }
        else 
        {/* invalid label name */
            return -2;
        }
    }
    else if (strncmp(buffer + *curr_idx, "string ", 7) == 0)
    {/* string directive */
        strcpy(curr_tok + 1, "string");
        (*curr_tok_len) += 6;
        (*curr_idx) += 7;
        if(add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
        {
            return -1;
        }
        if (skip_white_spaces(buffer, curr_idx) == 1)
        {
            sprintf(ret->error_message, "in line %d: missing opening brackets after .string directive\n", line_counter);
            ret->is_error = 1;
            return -2;
        }
        else 
        {/* check for opening and closing brackets */
            if (buffer[*curr_idx] != '\"')
            {
                sprintf(ret->error_message, "in line %d: syntax error, missing opening brackets before string\n", line_counter);
                ret->is_error = 1;
                return -2;
            }
            curr_tok[*curr_tok_len] = '\"';
            (*curr_tok_len)++;
            (*curr_idx)++;
            while (buffer[*curr_idx] != '\"'
             && buffer[*curr_idx] != '\0'
             && buffer[*curr_idx] != '\n')
            {
                curr_tok[*curr_tok_len] = buffer[*curr_idx];
                (*curr_tok_len)++;
                (*curr_idx)++;
            }

            if (buffer[*curr_idx] != '\"')
            {
                sprintf(ret->error_message, "in line %d: missing closing brackets after string\n", line_counter);
                ret->is_error = 1;
                return -2;
            }
            else
            {
                curr_tok[*curr_tok_len] = '\"';
                (*curr_tok_len)++;
                (*curr_idx)++;
                if (skip_white_spaces(buffer, curr_idx) != 1)
                {
                    sprintf(ret->error_message, "in line %d: extraneous text after end of command\n", line_counter);
                    ret->is_error = 1;
                    return -2;
                }
                else 
                {
                    if(add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
                    {
                        return -1;
                    }
                    else return STRING;
                }
            }
        }
    }
    else
    {
        sprintf(ret->error_message, "in line %d: syntax error\n", line_counter);
        ret->is_error = 1;
        return -2;
    }
}


/*
 * This function parses an operand from the given buffer and updates the `Lexed_Line` structure
 * accordingly. It identifies the addressing mode of the operand, which can be immediate, direct,
 * register indirect, or register direct. It also handles syntax errors and reports them.
 * 
 * Returns:
 * -2 if an error occurs.
 * -1 if a memory allocation error occurs.
 * 0 if the addressing mode immediate.
 * 1 if the addressing mode is direct.
 * 2 if the addressing mode is register indirect.
 * 3 if the addressing mode is register direct.
 */
static int get_operand(int* curr_idx, char* buffer, Lexed_Line* ret, char* curr_tok, int* curr_tok_len, int line_counter, char** macro_array, int macro_counter)
{
    while(buffer[(*curr_idx)] == ' ' || buffer[(*curr_idx)] == '\t')
    {/* skip white spaces */
        (*curr_idx)++;
    }
    if(buffer[(*curr_idx)] == '\n' || buffer[(*curr_idx)] == '\0')
    {/* check that operand exist */
        sprintf(ret->error_message, "in line %d: missing operand\n", line_counter);
        ret->is_error = 1;
        return -2;
    }
    else if(buffer[*curr_idx] == '#')
    {/* check for immediate operand */
        int i;
        curr_tok[*curr_tok_len] = '#';
        (*curr_tok_len)++;
        (*curr_idx)++;
        if ((i = get_number(curr_idx, buffer, ret, curr_tok, curr_tok_len, line_counter)) == -1)
        {
            return -1;
        }
        else if (i == 0)
        {
            ret->is_error = 1;
            return -2;
        }
        else return IMMEDIATE;
    }
    else if (buffer[*curr_idx] == '*')
    {/* check for indirect register operand */
        curr_tok[*curr_tok_len] = '*';
        (*curr_tok_len)++;
        (*curr_idx)++;
        if ((buffer[*curr_idx] == 'r'
            && buffer[(*curr_idx) + 1] >= '0'
            && buffer[(*curr_idx) + 1] <= '7')
            && (buffer[(*curr_idx) + 2] == '\0'
            || buffer[(*curr_idx) + 2] == '\n'
            || buffer[(*curr_idx) + 2] == '\t'
            || buffer[(*curr_idx) + 2] == ' '
            || buffer[(*curr_idx) + 2] == ','))
        {
            curr_tok[*curr_tok_len] = buffer[*curr_idx];
            (*curr_tok_len)++;
            (*curr_idx)++;
            curr_tok[*curr_tok_len] = buffer[*curr_idx];
            (*curr_tok_len)++;
            (*curr_idx)++;
            if(add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
            {
                return -1;
            }
            else return REGISTER_INDIRECT;
        }
        else 
        {
            sprintf(ret->error_message, "in line %d: invalid register\n", line_counter);
            ret->is_error = 1;
            return -2;
        }
    }
    else if (isalpha(buffer[*curr_idx]))
    {/* check for direct operand (label) or register direct operand */
        if ((buffer[*curr_idx] == 'r'
         && buffer[(*curr_idx) + 1] >= '0'
         && buffer[(*curr_idx) + 1] <= '7')
         && (buffer[(*curr_idx) + 2] == '\0'
         || buffer[(*curr_idx) + 2] == '\n' 
         || buffer[(*curr_idx) + 2] == '\t' 
         || buffer[(*curr_idx) + 2] == ' '
         || buffer[(*curr_idx) + 2] == ','))
        {
            curr_tok[*curr_tok_len] = buffer[*curr_idx];
            (*curr_tok_len)++;
            (*curr_idx)++;
            curr_tok[*curr_tok_len] = buffer[*curr_idx];
            (*curr_tok_len)++;
            (*curr_idx)++;
            if (add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
            {
                return -1;
            }
            else return REGISTER_DIRECT;
        }
        else 
        {/* not a register direct operand, check for direct operand (valid label name) */
            if (is_label(curr_idx, buffer, curr_tok, curr_tok_len, macro_array, macro_counter, ret, line_counter, 0))
            {
                if (add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
                {
                    return -1;
                }
                else return DIRECT;
            }
            else 
            {
                return -2;
            }
        }
    }
    else
    {/* invalid operand */
        sprintf(ret->error_message, "in line %d: invalid operand\n", line_counter);
        ret->is_error = 1;
        return -2;
    }        
}


/*
 * This function parses and validates the operands of an instruction from a buffer. It updates the `Lexed_Line` structure
 * with the operand types and checks for syntax errors and correct addressing modes. The index will point to the end of 
 * the line if the function completes successfully.
 * 
 * Returns:
 * -2 if an error occurs.
 * -1 if a memory allocation error occurs.
 * 0 if the line has errors.
 * 1 if the line is valid and processing is complete.
 */
static int get_operands(int* curr_idx, char* buffer, Lexed_Line* ret, char* curr_tok, int* curr_tok_len, int line_counter, char** macro_array, int macro_counter)
{
    int i;
    int op_counter = 0;
    int comma_counter = 0;
    while (op_counter < ret->amount_of_operands)
    {/* get the correct amount of operands */
        if (op_counter == comma_counter)
        {/* get an operand */
            if((i = get_operand(curr_idx, buffer, ret, curr_tok, curr_tok_len, line_counter, macro_array, macro_counter)) == -1)
            {/* memory allocation failure while adding token to array */
                return -1;
            }
            else if (i == -2)
            {/* invalid operand */
                return 0;
            }
            else 
            {/* valid operand */
                if (ret->amount_of_operands == 1)
                {
                    op_counter++;
                }
                ret->operand_types[op_counter] = i;
                if(op_type_vs_addressing_type(ret->inst_name, ret->operand_types[op_counter], op_counter))
                {/* check if current operand addressing mode is valid */
                    op_counter++;
                }
                else 
                {/* invalid addressing mode of current operand */
                    /* update error flag and message and stop getting more operands */
                    if (op_counter == 0)
                    {/* source operand */
                        sprintf(ret->error_message, "in line %d: source operand \"%s\" does not comply with the required addressing mode\n",line_counter, ret->tokens[ret->token_counter-1]);
                    } 
                    else
                    {/* destination operand */
                        sprintf(ret->error_message, "in line %d: destination operand \"%s\" does not comply with the required addressing mode\n",line_counter, ret->tokens[ret->token_counter-1]);
                    }
                    ret->is_error = 1;
                    return 0;
                }
            }
        }
        else 
        {/* get a comma */
            while(buffer[(*curr_idx)] == ' ' || buffer[(*curr_idx)] == '\t')
            {
                (*curr_idx)++;
            }
            if(buffer[(*curr_idx)] != ',')
            {
                if((buffer[(*curr_idx)] == '\n' || buffer[(*curr_idx)] == '\0') && op_counter < ret->amount_of_operands)
                {
                    sprintf(ret->error_message, "in line %d: missing operand\n", line_counter);
                    ret->is_error = 1;
                    return 0;
                }
                else 
                {
                    sprintf(ret->error_message, "in line %d: missing comma\n", line_counter);
                    ret->is_error = 1;
                    return 0;
                }
            }
            else
            {/* curr char is a comma */
                /* add to token and update */
                curr_tok[*curr_tok_len] = ',';
                (*curr_tok_len)++;
                if (add_token_to_struct(ret, curr_tok, curr_tok_len) == -1)
                {
                    return -1;
                }
                comma_counter++;
                (*curr_idx)++;
            }
        }
    }/* end of getting the desired amount of operands */
    if (skip_white_spaces(buffer, curr_idx))
    {
        return 1;
    }
    else 
    {
        sprintf(ret->error_message, "in line %d: extraneous text after end of command\n", line_counter);
        ret->is_error = 1;
        return 0;
    }
}

