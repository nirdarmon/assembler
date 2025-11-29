#ifndef LIBRARIES
#define LIBRARIES


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#endif


#include "constants.h"
#include "structs.h"
#include "output_helper.h"


#ifndef OUT_GENERATOR
#define OUT_GENERATOR


/**
 * @brief This function is the main function of printing procedure. Calls all other printing functions(ob, ent, ext), stores value
 * from printing functions returns and based on those values removes corresponding files if necessary. 
 * 
 * @param ob_ptr a pointer to object output file.
 * @param ent_ptr a pointer to entries output file.
 * @param ext_ptr a pointer to externals output file.
 * @param ast_array is the main ast array in use to store all source code lines data.
 * @param ast_array_counter counter of our ast_array size, used most in for loops.
 * @param label_array an array of Labels structure elements, stores all labels in source code with relevant data.
 * @param label_array_counter counter of our label_array size, used most in for loops. 
 *
 * @return an integer we later use to understand which files are empty and need to be removed.
 *         -1,0,1,2,3,4 are the return options. Where:
 *         -1:  temp ob file failed.
 *         0:   ob, ent, ext files are not empty.
 *         1:   ob and ent files are not empty.
 *         2:   ob and ext files are not empty.
 *         3:   ob only is not empty.
 *         4:   all files are empty.
 */
int printer_func(FILE* ob_ptr, FILE* ent_ptr, FILE* ext_ptr  , Ast** ast_array, int ast_array_counter, Labels** label_array, int label_counter);


/**
 * @brief This function prints our externals file (.ext) using ast and labels arrays to check for each label if
 * external label and if so prints use line to externals file.
 *
 * @param ext_ptr a pointer to externals output file.
 * @param ast_array is the main ast array in use to store all source code lines data.
 * @param ast_array_counter counter of our ast_array size, used most in for loops.
 * @param label_array an array of Labels struct elements, stores all labels in source code with relevant data.
 * @param label_array_counter counter of our label_array size, used most in for loops. 
 * @param max_used_extern_len helper variable used to align printing in ext file.
 *
 * @return an integer we later use to remove file if empty.
 *         0: no external labels were used.
 *         else: return the amount of times an external label was used.
 */
int print_ext_file(FILE* ext_ptr, Ast** ast_array, int ast_array_counter, Labels** label_array, int label_counter, int* max_used_extern_len);


/**
 * @brief This function prints our entries file (.ent) using labels array to check for each label if
 * is entry label and if so prints corresponding line to entries file.
 *
 * @param ent_ptr a pointer to entries output file.
 * @param label_array an array of Labels structure elements, stores all labels in source code with relevant data.
 * @param label_array_counter counter of our label_array size, used most in for loops. 
 *
 * @return an integer we later use to remove file if empty.
 *         0:    no entry labels were defined.
 *         else: return the amount of times an entry label was defined.*/
int print_ent_file(FILE* ent_ptr, Labels** label_array, int label_array_counter);


/**
 * @brief This function prints our object file (.ob) going over ast and labels arrays while using helper function to
 * get correct values.
 *
 * @param ob_ptr a pointer to object (.ob) output file.
 * @param ast_array is the main ast array in use to store all source code lines data.
 * @param ast_array_counter counter of our ast_array size, used most in for loops.
 * @param label_array an array of Labels struct elements, stores all labels in source code with relevant data.
 * @param label_array_counter counter of our label_array size, used most in for loops. 
 * @param max_used_extern_len helper variable used to align printing in ext file.
 *
 * @return an integer we later use to remove file if empty.
 *         -1:   temp ob file failed to open.
 *         0:    exceeded memory size or empty file.
 *         else: return the sum of IC and DC counters.

 */
int print_ob_file(FILE* ob_ptr, Ast** ast_array, int ast_array_counter, Labels** label_array, int label_counter, int* max_used_extern_len);


#endif