#ifndef LIBRARIES
#define LIBRARIES


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#endif


#include "constants.h"
#include "utils.h"


#ifndef LEXER_HEADER
#define LEXER_HEADER


typedef struct 
{
    int is_error; /* flag for error case, set to 0 by default, set to 1 if general error and to -1 if memory allocation error*/
    char error_message[200];
    int label_declaration; /* flag for label declaration case, 0 if none, 1 if valid, -1 if irrelevant (case of .entry or .extern directive after)*/
    int amount_of_operands;
    Addressing_Mode operand_types[2]; /* 0 for immediate, 1 for label, 2 for register indirect, 3 for register direct*/
    char** tokens;
    int token_counter;
    Line_Type line_type;
    Directive_Name dir_name;
    Instruction_Name inst_name;
} Lexed_Line;


/**
 * @brief Splits a line into tokens and processes them.
 * 
 * This function parses a line of text, separating it into tokens. It uses the provided
 * macro array and label array to process the tokens and stores the results in the 
 * `Lexed_Line` structure.
 * 
 * @param ret Pointer to a `Lexed_Line` structure where the tokens will be stored.
 * @param buffer The line of text to be tokenized.
 * @param line_counter The current line number.
 * @param macro_array Array of macro names used for token processing.
 * @param macro_counter Number of macros in the macro array.
 * @param label_array Array of label pointers used for token processing.
 * @param label_array_counter Number of labels in the label array.
 * 
 * @return no return as function is void, editing in place. 
 */
void sep_line_to_tokens(Lexed_Line* ret, char* buffer, int line_counter, char** macro_array, int macro_counter, Labels** label_array, int label_array_counter);


#endif
