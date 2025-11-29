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


#ifndef PREPROC
#define PREPROC


/**
 * @brief This function is responsible for the preprocessor stage of the assembler.
 * 
 * @param as_file_name_with_ext The name of the .as file.
 * @param macro_names A pointer to a pointer to a char array that will hold the macro names.
 * @param macro_counter A pointer to an integer that will hold the amount of macros.
 * @param as_file .as file pointer.
 * @param am_file .am file pointer.
 * 
 * @return -2 if file opening failed, -1 if memory allocation failed, 1 if preprocessing was successful and 0 if an error occurred.
 */
int preproc(char* as_file_name_with_ext, char*** macro_names, int* macro_counter, FILE* as_file, FILE* am_file);


#endif /* PREPROC*/
