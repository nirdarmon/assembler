#ifndef LIBRARIES
#define LIBRARIES


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#endif


#include "constants.h"
#include "structs.h"


#ifndef UTILS_H
#define UTILS_H


/**
 * @brief Concatenates a base name with an extension to form a file name.
 * 
 * @param file_name_without_ext The base name of the file (e.g., "document").
 * @param file_name_with_ext A pointer to the newly allocated string containing the full file name.
 * @param file_ptr A pointer to the newly allocated string containing the full file name.
 * @param extension The file extension to append (e.g., ".txt").
 * @param mode The mode to open the file in. (e.g "r", "w").
 * 
 * @return -2 if file creation failed, -1 if memory allocation failed, 0 otherwise.
 */
int create_file_and_save_name_with_ext(char* file_name_without_ext, char** file_name_with_ext, FILE** file_ptr, char* extension, char* mode);


/**
 * @brief Frees a matrix of char pointers.
 * 
 * @param matrix the matrix to free.
 * @param counter the number of rows in the matrix.
 * 
 * @return no return as function is void, editing in place. 
 */
void free_matrix_char_pointer(char*** matrix, int counter);


/**
 * @brief Checks if the given word is a reserved keyword.
 *
 * @param word Pointer to the string to check.
 * 
 * @return 1 if the word is reserved, 0 otherwise.
 */
int is_reserved_word(char* word);


/**
 * Checks if a word is a macro name.
 * 
 * @param word The string to check.
 * @param macro_names Array of known macro names.
 * @param macro_counter Number of macros in the array.
 * 
 * @return The position of the macro in the array if found, -1 otherwise.
 */
int is_macro(char* word, char** macro_names, int macro_counter);


/**
 * @brief Skips white spaces in a line.
 * 
 * @param line The string to process.
 * @param idx Pointer to the current index in the string.
 * 
 * @return 1 if end of line or file is reached, 0 otherwise.
 */
int skip_white_spaces(char* line, int* idx);


/**
 * @brief Calculates the length of the current line in a file.
 * 
 * @param input The file pointer to read from.
 * 
 * @return The length of the line, including the newline character if present.
 */
int line_length(FILE* input);


/**
 * @brief Checks if a given word exists in the label array.
 * 
 * @param word The label name to search for.
 * @param label_array The array of label pointers.
 * @param label_array_counter The number of labels in the array.
 * 
 * @return The postion of the label in the array if found, -1 otherwise.
 */
int check_if_label_exist(char* word, Labels** label_array, int label_array_counter);


void free_label_data(Labels* label);


/**
 * @brief Frees dynamically allocate memory of the label array.
 * 
 * @param label_array Pointer to the array of label pointers.
 * @param label_array_counter The number of labels in the array.
 * 
 * @return no return as function is void, editing in place. 
 */
void free_label_array(Labels*** label_array, int label_array_counter);


/**
 * @brief frees dynamically allocate memory of the ast structure.
 * 
 * @param ast Pointer to the ast structure.
 * 
 * @return no return as function is void, editing in place. 

 */
void free_ast_data(Ast* ast);


/**
 * @brief frees dynamically allocate memory of the label array.
 * 
 * @param ast_array Pointer to the array of ast pointers.
 * @param ast_array_counter The number of ast structures in the array.
 * 
 * @return no return as function is void, editing in place. 
 */
void free_ast_array(Ast*** ast_array, int ast_array_counter);


/**
 * @brief Removes a label from the label array.
 * 
 * @param label_array Pointer to the array of label pointers.
 * @param label_array_counter Pointer to the number of labels in the array.
 * @param label_name The name of the label to remove.
 * 
 * @return no return as function is void, editing in place. 
 */
void remove_label(Labels** label_array, int* label_array_counter, char* label_name);


#endif
