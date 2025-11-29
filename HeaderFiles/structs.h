#include "constants.h"


#ifndef LABELS_STRUCT
#define LABELS_STRUCT


typedef struct 
{
    char* l_name;
    int l_row;
    Label_Type l_type;
}Labels;


#endif /* LABELS_STRUCT*/


#ifndef AST_STRUCT
#define AST_STRUCT


typedef struct {
    char error_message[200];
    int am_line_number;
    int is_error;
    int is_label_declaration;
    char* label_dec_name;
    Line_Type ast_type;
    union {
        struct {
            Directive_Name dir_name;
            union {
                char* label;
                char* dir_string_str;
                int numbers[MAX_LINE_LEN]; /* set numbers from cell 1 and use cell 0 to count the total amount of numbers*/
            } dir_content;
        } Directive;
        struct {
            Instruction_Name inst_name;
            int num_of_operands;
            struct {
                Addressing_Mode op_type;
                union {
                    int immed;
                    char* label;
                    int reg_num;
                } op_data;
            } Operand[2];
        } Instruction;
    } ast_options;
} Ast;


#endif /* AST_STRUCT*/
