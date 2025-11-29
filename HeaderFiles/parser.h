#ifndef LIBRARIES
#define LIBRARIES


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#endif /* LIBRARIES*/


#include "constants.h"
#include "structs.h"
#include "utils.h"
#include "lexer.h"


#ifndef PARSER
#define PARSER


/**
 * @brief Parses the given file and creates an array of ASTs and an array of labels.
 * 
 * @param am_file The file to parse.
 * @param am_file_name_with_ext The name of the file.
 * @param ast_array A pointer to the array of ASTs.
 * @param ast_array_counter A pointer to the counter of ASTs.
 * @param label_array A pointer to the array of labels.
 * @param label_array_counter A pointer to the counter of labels.
 * @param macro_array An array of macro names.
 * @param macro_array_counter The number of macros in the array.
 * 
 * @return 1 if parsing was successful, -1 if memory allocation failed.
 */
int parse_file(FILE* am_file, char* am_file_name_with_ext, Ast*** ast_array, int* ast_array_counter, Labels*** label_array,
 int* label_array_counter, char** macro_array, int macro_array_counter);


#endif /* PARSER*/
