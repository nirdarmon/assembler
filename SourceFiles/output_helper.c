#include "output_helper.h"

/* static declarations */
static int get_op_methods(Ast curr_ast, int operand_num);
/* end of static declarations */




int get_max_label_len(Labels** label_array, int label_array_counter, int entry_or_extern)
{
    /* initialize variables */
	int i=0;
    int max_label_len = 0;

    /* going over label array and storing the maximum length value if found used in other function for printing reasons */
	for (i=0; i < label_array_counter; i++)
	{
			if((strlen(label_array[i]->l_name) > max_label_len) && (label_array[i]->l_type == entry_or_extern))
            {
                max_label_len  = strlen(label_array[i]->l_name);
            }
	}
	return max_label_len;
}





void add_row_num_to_label_arr(Ast** ast_array, int ast_array_counter, Labels** label_array, int label_array_counter)
{

    /* initialize variables */
	int i=0, j=0;
	int curr_row = 0;
	int total_IC_lines_so_far = 0;
    int total_DC_lines_so_far = 0;


    /* in the first for loop in this function we will go over the ast and label arrays looking for instructions only */
	for (i=0; i < ast_array_counter; i++)
	{
		/* check if curr line is not supposed to be in output and decrement index accordingly */
		if((ast_array[i]->ast_type == COMMENT_LINE) || 
		   ((ast_array[i]->is_error == 1) || (ast_array[i]->is_error == -1) || (ast_array[i]->is_error == 2) ) ||
		   ((ast_array[i]->ast_type ==DIRECTIVE) ))
		{
    		continue;
		}
        /* if label declaration then field label_dec_name has value stored */
		else if ((ast_array[i]->is_label_declaration == 1))
		{
            /* going over label array comparing each label to the label_dec_name field in our ast cell */
			for (j=0; j<label_array_counter; j++)
			{
				if(strcmp(label_array[j]->l_name, ast_array[i]->label_dec_name) == 0)
				{
					if (label_array[j]->l_type == ENTRY_L || label_array[j]->l_type == NONE)
					{
                        /* assigning value to label array cell row number field */
						curr_row = (STARTING_ROW + total_IC_lines_so_far);
						label_array[j]->l_row = curr_row;
						break;
					}
				}		
			}
		}
        /* keeping IC counter updated in order to know where to start the data part of the code at the next loop */
		total_IC_lines_so_far += get_num_of_output_rows(ast_array[i]);
	}

    /* in the second for loop in this function we will go over the ast and label arrays looking for directives only */    
    for (i=0; i < ast_array_counter; i++)
	{
		/* check if curr line is not supposed to be in output and decrement index accordingly */
		if((ast_array[i]->ast_type == COMMENT_LINE) || 
           (ast_array[i]->ast_type == INSTRUCTION) || 
		   ((ast_array[i]->is_error == 1) || (ast_array[i]->is_error == -1) || (ast_array[i]->is_error == 2) ) ||
		   ((ast_array[i]->ast_type ==DIRECTIVE) && (((ast_array[i]->ast_options.Directive.dir_name == ENTRY) || (ast_array[i]->ast_options.Directive.dir_name == EXTERNAL)))))
		{
    		continue;
		}
		else if ((ast_array[i]->is_label_declaration == 1) && (ast_array[i]->ast_type == DIRECTIVE))
		{
            /* going over label array comparing each label to the label_dec_name field in our ast cell */            
			for (j=0; j<label_array_counter; j++)
			{
				if(strcmp(label_array[j]->l_name, ast_array[i]->label_dec_name) == 0)
				{
					if (label_array[j]->l_type == ENTRY_L || label_array[j]->l_type == NONE)
					{
                        /* assigning value to label array cell row number field */
						curr_row = (STARTING_ROW + total_IC_lines_so_far + total_DC_lines_so_far);
						label_array[j]->l_row = curr_row;
						break;
					}
				}		
			}
		}
		total_DC_lines_so_far += get_num_of_output_rows(ast_array[i]);
	}

	return;
}




int get_num_of_output_rows(Ast* curr_ast)
{
    /* initialize variables */
	int num_of_rows = 0;

	if(curr_ast->ast_type == DIRECTIVE)
	{
		if(curr_ast->ast_options.Directive.dir_name == STRING)
		{
        	/* using strlen to check the number of lines needed to represent the string where each line is a char
             * +1 for the termination  */
			num_of_rows = strlen(curr_ast->ast_options.Directive.dir_content.dir_string_str)+1;
		}
		else if(curr_ast->ast_options.Directive.dir_name == DATA)
		{
			/* in data directive we will use the first cell of ast as a counter to how many values 
			 * this data directive stores */
			num_of_rows = curr_ast->ast_options.Directive.dir_content.numbers[0];
		}
	}
	else if (curr_ast->ast_type == INSTRUCTION)
	{
        /* special case - 2 registers will be stored in one line even though 2 operands */
        if ((curr_ast->ast_options.Instruction.num_of_operands == 2) &&
		    ((curr_ast->ast_options.Instruction.Operand[1].op_type == REGISTER_DIRECT || curr_ast->ast_options.Instruction.Operand[1].op_type == REGISTER_INDIRECT) &&
		     (curr_ast->ast_options.Instruction.Operand[0].op_type == REGISTER_DIRECT || curr_ast->ast_options.Instruction.Operand[0].op_type == REGISTER_INDIRECT)))
        {
            num_of_rows = 2;
        }
        /* regular instruction case */
        else num_of_rows = curr_ast->ast_options.Instruction.num_of_operands + 1;
	}	
	return num_of_rows;
}






short int first_word_of_instruction_to_binary(Ast* curr_ast)
{
    /* initialize variables */
	short int opcode_mask = 15; /* 1111 */
	short int bin_rep = 0;
	short int src_method = 0, dest_method = 0;
    int src_or_dest_checker = -1;
	
	/* assign op code to binary representation string */  
	bin_rep = (curr_ast->ast_options.Instruction.inst_name & opcode_mask) << 11; 
	
	/* get op type and prepare mask */
    src_or_dest_checker = src_or_dest_phaser(curr_ast->ast_options.Instruction.inst_name);
    if (src_or_dest_checker == 0)
    {
        src_method = get_op_methods(*curr_ast,0);
	    /* assign source operand method code to binary representation string */
        bin_rep |= (src_method << 7);

       	dest_method = get_op_methods(*curr_ast,1);
	    /* assign destination operand method code to binary representation string */
	    bin_rep |= (dest_method << 3);
    }
    else if (src_or_dest_checker == 1)
    {
        src_method = 0;
	    /* assign source operand method code to binary representation string */
        bin_rep |= (src_method << 7);

        dest_method = get_op_methods(*curr_ast,1);
        /* assign destination operand method code to binary representation string */
        bin_rep |= (dest_method << 3);
    }
    else if (src_or_dest_checker == 2)
    {
        src_method = 0;
	    /* assign source operand method code to binary representation string */
	    bin_rep |= (src_method << 7);

       	dest_method = 0;
	    /* assign destination operand method code to binary representation string */
	    bin_rep |= (dest_method << 3);
    }
    
	/* assign ARE code to binary representation - if instruction then always are = 100 */
	bin_rep|= 4;

	return bin_rep;	
}



short int instruction_operand_to_binary(Ast* curr_ast, Labels** label_array, int label_counter, int operand_num, int* max_used_extern_len)
{
    /* initialize variables */
	short int bin_rep = 0;
	int j=0;
    int reg_phase = 0;
    int temp_len = 0;

    
	/* instruction immediate operand input conversion (#7 for example, 111) */
	if(curr_ast->ast_options.Instruction.Operand[operand_num].op_type == IMMEDIATE)
	{

		bin_rep |= (curr_ast->ast_options.Instruction.Operand[operand_num].op_data.immed << 3);
        bin_rep |= 4; /* ARE 100 */
		return bin_rep;
	}
	/* label operand conversion */
	else if (curr_ast->ast_options.Instruction.Operand[operand_num].op_type == DIRECT)
	{
		for(j=0; j< label_counter; j++)
		{
			if(strcmp(curr_ast->ast_options.Instruction.Operand[operand_num].op_data.label, label_array[j]->l_name)==0)
			{
				if(label_array[j]->l_type == EXTERNAL_L) 
				{
                    temp_len = strlen(label_array[j]->l_name);
                    if (temp_len > *max_used_extern_len)
                    {
                        *max_used_extern_len = temp_len;
                    }
					bin_rep |= 1;  /* ARE 001 */
					return bin_rep;
				}
				else 
				{
					bin_rep |= ((label_array[j]->l_row) << 3);
                    bin_rep |= 2; /* ARE 010 */
					return bin_rep;
				}			
			}
		}
	}
	/* single register operand conversion */
	else if (curr_ast->ast_options.Instruction.Operand[operand_num].op_type == REGISTER_DIRECT || curr_ast->ast_options.Instruction.Operand[operand_num].op_type == REGISTER_INDIRECT)
	{
		/* example 000000SSSDDDARE
         * where    S = source operand    
         * and      D = destination operand */
        /* checking which operand we need to mask and setting the corresponding phase used to shift */
		if (operand_num == 0) reg_phase = 6;
		else if (operand_num == 1) reg_phase = 3;

		bin_rep |= (curr_ast->ast_options.Instruction.Operand[operand_num].op_data.reg_num << reg_phase);
		bin_rep |= 4;  /* ARE 100 */
		
        return bin_rep;
	}
	return bin_rep;
}



short int directive_operand_to_binary(Ast* curr_ast, int operand_num)
{
    /* initialize variables */
	short int bin_rep = 0;

    /* masking the binary return using the data in the ast structure sent to function */
	if (curr_ast->ast_options.Directive.dir_name == DATA)
	{
		bin_rep |= curr_ast->ast_options.Directive.dir_content.numbers[operand_num+1]; /* +1 because we use first cell as a data counter for said .data line */
		return bin_rep;
	}
	else if (curr_ast->ast_options.Directive.dir_name == STRING)
	{
		bin_rep |= curr_ast->ast_options.Directive.dir_content.dir_string_str[operand_num]; /* masking based on ascii value */
		return bin_rep;
	}
	else return bin_rep;
}



short int instruction_double_registers_to_binary(Ast* curr_ast)
{
    /* initialize variables */
	short int bin_rep = 0;
	
	/* creating masks for double registers instruction shifting by the format:
     * 000000SSSDDDARE
     * where S = source operand
     * and   D = destination operand */
	bin_rep |= (curr_ast->ast_options.Instruction.Operand[0].op_data.reg_num << 6);
	bin_rep |= (curr_ast->ast_options.Instruction.Operand[1].op_data.reg_num << 3);
	bin_rep |= 4;

	return bin_rep;
}




void binary_to_octal(short int bin_rep , char octal_rep[6]) 
{
    /* Convert binary char array to a single integer */
	char octa_table[8] = {'0','1','2','3','4','5','6','7'};	
	
    /* Initialize variables */
	int i =0, digit = 0;
	unsigned short int mask = 070000;  /* Binary mask 111 000 000 000 000 */

	/* Process each group of three bits from left to right */
	for (i = 0; i < 5; ++i) 
	{
		/* Extract three bits at a time */
		digit = (bin_rep & mask) >> (3 * (4 - i));
		/* Convert the extracted bits to octal digit */
		octal_rep[i] = octa_table[digit]  ;
		/* Shift the mask to the right to get the next three bits */
		mask >>= 3;
   	}
	
    /* Null-terminate the octal string */
    octal_rep[5] = '\0';
	
    return;
}



void empty_file_remover(char* ob_f_name, char* ent_f_name, char* ext_f_name, int printer_return)
{
    /* function based on the return from the main printer function which removes empty files using the next switch, case */
	switch(printer_return)
	{

        case 0:
            break;

		case 1:
			remove(ext_f_name);
			break;

		case 2:
			remove(ent_f_name);
			break;

		case 3:
	        remove(ext_f_name);
			remove(ent_f_name);
			break;

        case (-1):
		case 4:
			remove(ext_f_name);
			remove(ent_f_name);
			remove(ob_f_name);
			break;

	}
	return;

}




int src_or_dest_phaser(int inst_name)
{
    /* using the next switch, case over the instruction name given to return the correct operand phase we should use in
     * the calling function */
    switch (inst_name)
    {
        case MOV:
        case CMP:
        case ADD:
        case SUB:
        case LEA:
            return 0;
        case CLR:
        case NOT:
        case INC:
        case DEC:
        case JMP:
        case BNE:
        case RED:
        case PRN:
        case JSR:
            return 1;
        case RTS:
        case STOP:
            return 2;
    }
    return 0;
}






/*
 *    STATIC FUNCTIONS ONLY
 *            ||
 *            ||
 *            ||
 *           \  /
 *            \/                                               
 */





/* This function will get ast struct and will return an int we will later use to mask our object lines with
 * based on the operand type we are currently checking */
static int get_op_methods(Ast curr_ast, int operand_num)
{
    /* initialize variable */
	int op_method = 0;
    
    /* using the next switch, case over the instruction operand type given to return the correct mask we should use in
     * the calling function */

	switch (curr_ast.ast_options.Instruction.Operand[operand_num].op_type)
	{
		case (IMMEDIATE):
			op_method = 1;
			break;
		case (DIRECT):
 			op_method = 2;
			break;
		case (REGISTER_INDIRECT):
			op_method = 4;
			break;
		case (REGISTER_DIRECT):
			op_method = 8;
			break;
	}
	return op_method;
}