#ifndef LIBRARIES
#define LIBRARIES


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#endif


#include "constants.h"
#include "structs.h"


#ifndef OUT_HELPER
#define OUT_HELPER

/**
 * @brief getting the size of the longest label name on label array for printing purposes. 
 * 
 * @param label_array an array of Labels struct elements, stores all labels in source code with relevant data.
 * @param label_array_counter counter of our label_array size, used most in for loops.
 * @param entry_or_extern is a flag we use to check if label is entry or is it external.
 *
 * @return an integer which holds the maximum label length.
 * */
int get_max_label_len(Labels** label_array, int label_array_counter, int entry_or_extern);


/**
 * @brief adding row numbers to each element in label array, to be used later for output printing.
 * 
 * @param ast_array is the main ast array in use to store all source code lines data.
 * @param ast_array_counter counter of our ast_array size, used most in for loops.
 * @param label_array an array of Labels struct elements, stores all labels in source code with relevant data.
 * @param label_array_counter counter of our label_array size, used most in for loops. 
 *
 * @return no return as function is void, editing in place.
 */
void add_row_num_to_label_arr(Ast** ast_array, int ast_array_counter, Labels** label_array, int label_array_counter);


/**
 * @brief This function will be used to get the correct binary representation only for the first word of instruction
 *  commands using different masks acquired from our ast based on the directions in our project.
 *
 * @param curr_ast ast struct that stores all data from source code line.
 *
 * @return a short int that holds the binary value translation of said source code line.
 */
short int first_word_of_instruction_to_binary(Ast* curr_ast);


/**
 * @brief This function will be used to get the correct binary representation just for the first word of instruction
 * commands(the command itself) using different masks acquired from our ast based on the directions in our project.
 * 
 * @param curr_ast ast struct that stores all data from source code line.
 * @param label_array an array of Labels struct elements, stores all labels in source code with relevant data.
 * @param label_array_counter counter of our label_array size, used most in for loops. 
 * @param max_used_extern_len helper variable used to align printing in ext file.
 *
 * @return a short int that holds the binary value translation of current operand.
 */
short int instruction_operand_to_binary(Ast* curr_ast, Labels** label_array, int label_counter, int operand_num, int* max_used_extern_len);


/**
 * @brief This function will be used to get the correct binary representation for directive commands 
 *  where each line represent a number or a char. We are using those number or chars to mask
 *  our binary return.
 *
 * @param curr_ast ast struct that stores all data from source code line.
 * @param operand_num an integer used to address the correct operand number in curr_ast.
 *
 * @return a short int that holds the binary value translation of current operand.
 */
short int directive_operand_to_binary(Ast* curr_ast, int operand_num);


/**
 * @brief This function will be used to get the correct binary representation for a special case - 
 * a line with to registers in it. In that case we will set the binary representation to store 
 * the values of both registers in one line.
 * we use the register numbers from the ast to mask the corresponding bits in our binary representation.
 *
 * @param curr_ast ast struct that stores all data from source code line.
 *
 * @return a short int that holds the binary value translation of both registers.
 */
short int instruction_double_registers_to_binary(Ast* curr_ast);


/**
 * @brief This function will be used to get the correct octal representation for each binary representation in 
 * our object file. Assigning value in place to a string sent to function when called upon.
 *
 * @param bin_rep a short integer that holds correct binary value of output line, used as source for translation. 
 * @param octal_rep a string with length of 6 char used to store the octal representation of bin_rep, later used
 * for output printing.
 * 
 * @return no return as function is void, editing in place. 
 */
void binary_to_octal(short int bin_rep , char octal_rep[6]);


/**
 * @brief This function will be used to delete empty files at the final commands of our main loop.
 * Deletion will be based on return of printer functions.
 *
 * @param ob_f_name object file name used as parameter in remove() to delete file if necessary.  
 * @param ent_f_name entries file name used as parameter in remove() to delete file if necessary.
 * @param ext_f_name externals file name used as parameter in remove() to delete file if necessary.
 * @param printer_return an integer returned from printer function we use to understand which files are empty and
 *  therefor needs to be removed.
 * 
 * @return no return as function is void, editing in place. 
 */
void empty_file_remover(char* ob_f_name, char* ent_f_name, char* ext_f_name, int printer_return);


/**
 * @brief This function will be used to get the correct number of rows for each line in our object output file.
 * it will be doing so be checking our ast struct data. One ast struct is called each time the function runs.
 *
 * @param curr_ast ast struct that stores all data from source code line.
 *
 * @return an integer that holds the number of output code lines we need to generate for said ast. Possible return
 * values range from 1-3 for instructions. And at least 1 for directives (length of string or amount of data numbers)
 */
int get_num_of_output_rows(Ast* curr_ast);


/**
 * @brief This function will be used to decide which operand to check in array based on the command 
 * given in inst_name. 
 *
 * @param inst_name instruction name, based on this parameter we will return the correct phase.  
 *  
 * @return an integer - 1 if the operand in use is only destination, 0 if uses both operands, 2 if no operands are being used.
 */
int src_or_dest_phaser(int inst_name);


#endif /* OUT_HELPER */