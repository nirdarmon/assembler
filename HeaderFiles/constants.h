#ifndef CONSTANTS
#define CONSTANTS


#define MAX_LINE_LEN 81 /* including the ending "\n" or "\0"*/
#define MAX_LABEL_LEN 31
#define STARTING_ROW 100
#define MAX_MEMORY_SIZE 4096 
#define MAX_SIGNED_12_BITS 2047
#define MIN_SIGNED_12_BITS -2048
#define MAX_UNSIGNED_12_BITS 4095
#define MAX_SIGNED_15_BITS 16383
#define MIN_SIGNED_15_BITS -16384
#define MAX_UNSIGNED_15_BITS 32767

typedef enum 
{
        INSTRUCTION,
        DIRECTIVE,
        COMMENT_LINE
}Line_Type;


typedef enum 
{
        MOV, CMP, ADD, SUB, LEA, CLR, NOT, INC, DEC, JMP, BNE, RED, PRN, JSR, RTS, STOP
} Instruction_Name;


typedef enum 
{
        DATA,
        STRING,
        ENTRY,
        EXTERNAL        
} Directive_Name;


typedef enum 
{
        IMMEDIATE,
        DIRECT,
        REGISTER_INDIRECT,
        REGISTER_DIRECT
        
} Addressing_Mode;


typedef enum
{
        NONE,
        ENTRY_L,
        EXTERNAL_L
} Label_Type;


#endif /* CONSTANTS*/
