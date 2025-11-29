#include "output_generator.h"


int printer_func(FILE* ob_ptr, FILE* ent_ptr, FILE* ext_ptr  , Ast* *ast_array, int ast_array_counter, Labels** label_array, int label_counter)
{
    /* initialize variables for function */
	int ent_print_return = -1;
	int ext_print_return = -1;
	int ob_print_return = -1;
	int main_print_return = 0;
	int max_used_extern_len = 0;
   
    /* call for printer function - entries, externals and object */
	ob_print_return = print_ob_file(ob_ptr, ast_array, ast_array_counter, label_array, label_counter, &max_used_extern_len);
    /* failed to open internal file in print_ob_file function, exiting program */
    if (ob_print_return == -1)
    {
        return -1;
    }
    else if (ob_print_return == 0)
    {
        return 4;
    }
    
    ext_print_return = print_ext_file(ext_ptr, ast_array, ast_array_counter, label_array, label_counter, &max_used_extern_len);
	ent_print_return = print_ent_file(ent_ptr,label_array, label_counter);
    
    
    /* return based on if files were empty or not */
    if (ent_print_return != 0  && ext_print_return !=0)  main_print_return = 0;
    else if (ent_print_return != 0  && ext_print_return ==0) main_print_return = 1;
    else if (ent_print_return == 0  && ext_print_return !=0) main_print_return = 2;
    else if (ent_print_return == 0  && ext_print_return ==0) main_print_return = 3; 
    
	
	return main_print_return; 
}





int print_ext_file(FILE* ext_ptr, Ast** ast_array, int ast_array_counter, Labels** label_array, int label_counter, int* max_used_extern_len)
{
    /* initialize variables for function */
	int i = 0, j = 0, k = 0;
	int curr_row = 0;
	int num_of_rows = 0;
	int total_rows_counter = 0;
	int extern_counter = 0;
    int src_or_dest_phase = -1;



    /* for loop going over ast array counter to find all array elements that has externals */
	for(i = 0 ; i < ast_array_counter; i++)
	{
        /* skip COMMENT_LINE lines, lines with error in it and DIRECTIVE lines (which we are generating ast cell in ast array for but dont want
        to output). */
		if((ast_array[i]->ast_type == COMMENT_LINE) || 
		   ((ast_array[i]->is_error == 1) || (ast_array[i]->is_error == -1) || (ast_array[i]->is_error == 2)) ||  
		   (ast_array[i]->ast_type == DIRECTIVE))
		{
			continue;
		}
        /* get the number of output rows for each ast cell(row of source code) and then we will be able to go over it and
         * check each operand\s to see if external */
		num_of_rows = get_num_of_output_rows(ast_array[i]);
        

        /* main function loop, checks each operand and prints to ext file if necessary */
		for(k=0; k < num_of_rows - 1 ; k++)
		{
			/* helper function to phase the operand we check according to the instruction type */
			src_or_dest_phase = src_or_dest_phaser(ast_array[i]->ast_options.Instruction.inst_name);

			if(ast_array[i]->ast_options.Instruction.Operand[k + src_or_dest_phase].op_type == DIRECT)
			{   
				/* going over and comparing to label array to get label row from label structure array */
				for(j=0; j<label_counter; j++)
				{
					if((strcmp(label_array[j]->l_name, ast_array[i]->ast_options.Instruction.Operand[k + src_or_dest_phase].op_data.label)==0) &&
                       (label_array[j]->l_type == EXTERNAL_L))
                    {
						extern_counter ++;
						/*add to ext file output*/
						curr_row = STARTING_ROW + (k+1) + total_rows_counter ;
						fprintf(ext_ptr, "%-*s %.4d\n", *max_used_extern_len,  label_array[j]->l_name, curr_row);
						
					}
				}		
			}
			
	    }
        /* updating rows counter */
        total_rows_counter += num_of_rows;
	}
	return  extern_counter;
}



int print_ent_file(FILE* ent_ptr, Labels** label_array, int label_array_counter)
{

    /* initialize variables for function */
	int i = 0;
	int entry_counter = 0;
    int max_label_len = 0;

    /* get the maximum entry(1) label name length in order to align printing */ 
    max_label_len = get_max_label_len(label_array, label_array_counter,1);
	
    /* going over label array and checking if label is entry. if so then we print to .ent file along with label definition row
     * number */
	for(i = 0 ; i < label_array_counter ; i++)
	{
		if(label_array[i]->l_type == ENTRY_L)
		{
			entry_counter++;  /* keep entries counter updated */
			fprintf(ent_ptr, "%-*s %d\n", max_label_len, label_array[i]->l_name ,label_array[i]->l_row);
		}
	}
	return entry_counter;
}





/* ob_out_arr stores all the row codes in binary using 2 char array. In this function we convert to octal and printing */
int print_ob_file(FILE* ob_ptr, Ast** ast_array, int ast_array_counter, Labels** label_array, int label_counter, int* max_used_extern_len)
{
	
	/* initialize variables for function */
    char c;
	char octal_rep[6];
	int i = 0, k = 0;
	int curr_row_num = 0;
	int num_of_rows = 0;
	int total_IC_lines_so_far = 0;
	int total_DC_lines_so_far = 0;
	short int curr_binary_row = 0;
	FILE* temp_ob;
    int src_or_dest_phase = -1;

    /* opening temp file, used later to copy content after printing IC and DC counters */
	temp_ob = fopen("temp_ob_file.ob", "w");
    if (temp_ob == NULL)
    {
       	printf("Error: failed to open temp_ob_file.ob file\n");
	    return -1;
    }

	for(i = 0 ; i< ast_array_counter; i++)
	{
		/* counter for comment or empty lines */
		if((ast_array[i]->ast_type == COMMENT_LINE) ||
		   ((ast_array[i]->is_error == 1) || (ast_array[i]->is_error == -1) || (ast_array[i]->is_error == 2) ) ||
		   ((ast_array[i]->ast_type == DIRECTIVE) ))
		{
			continue;
		}
		
        /* get the number of operands on current command line */
		num_of_rows = get_num_of_output_rows(ast_array[i]);
		
        if((STARTING_ROW + total_IC_lines_so_far + num_of_rows) > MAX_MEMORY_SIZE)
        {
            printf("\nExceeded total memory size, stopped output generator and deleted all files. \n\n");
            fclose(temp_ob);
            remove ("temp_ob_file.ob");
            return 0;
        }
        
        if (ast_array[i]->ast_type == INSTRUCTION) 
		{
			/* first we convert the first word of the instruction */
			/* get row number before adding current instruction counter */
			curr_row_num = (STARTING_ROW + total_IC_lines_so_far) ;
			
			/* updating instruction counter */
			total_IC_lines_so_far += (num_of_rows);
			
			/* here we will call helper functions in order to build binary representation for each of current ast command or operand */
			curr_binary_row =  first_word_of_instruction_to_binary(ast_array[i]);
			
			/* convert to output format and print to file */
			binary_to_octal(curr_binary_row , octal_rep);
            
            fprintf(temp_ob, "%.4d %.5s\n", curr_row_num, octal_rep);

			
			/* check if two operands on current command are registers and if so create binary rep outside of k loop and continue to next i iteration */
			if ( (num_of_rows == 2) &&
		        ((ast_array[i]->ast_options.Instruction.Operand[1].op_type == REGISTER_DIRECT || ast_array[i]->ast_options.Instruction.Operand[1].op_type == REGISTER_INDIRECT) &&
			     (ast_array[i]->ast_options.Instruction.Operand[0].op_type == REGISTER_DIRECT || ast_array[i]->ast_options.Instruction.Operand[0].op_type == REGISTER_INDIRECT)) )
			{
				/* get row number before adding current instruction counter (-1 because its 2 registers and instruction
                 * command line already printed.) */
				curr_row_num = (STARTING_ROW + total_IC_lines_so_far - 1 );
				
				/* create 2 registers binary rep */
				curr_binary_row = instruction_double_registers_to_binary(ast_array[i]);
				
				/* convert to output format and print to file */
				binary_to_octal(curr_binary_row , octal_rep);
				
                fprintf(temp_ob, "%.4d %.5s\n", curr_row_num, octal_rep);
				continue;
			}
			for(k = 0; k < num_of_rows - 1; k++)
			{
                /* here we call src_or_dest_phaser function in order to address the correct cell in ast array (src or dest) */
                src_or_dest_phase = src_or_dest_phaser(ast_array[i]->ast_options.Instruction.inst_name);
                /* here we will call helper functions in order to build binary representation for each of current ast command or operand */
				curr_binary_row = instruction_operand_to_binary(ast_array[i], label_array, label_counter, k + src_or_dest_phase, max_used_extern_len);
				
				/* get row number and offset (k - num_of_operands) to make sure we are giving correct row number for each operand
				 * because we already incremented instruction counter at top of i loop */
				curr_row_num = (STARTING_ROW + total_IC_lines_so_far  + (k + 1 - num_of_rows) );

				/* convert to output format and print to file */
				binary_to_octal(curr_binary_row , octal_rep);
				
                fprintf(temp_ob, "%.4d %.5s\n", curr_row_num, octal_rep);
			}
		}

    }

    for(i = 0 ; i< ast_array_counter; i++)
    {
        /* counter for comment or empty lines */
        if((ast_array[i]->ast_type == COMMENT_LINE) ||
           ((ast_array[i]->is_error == 1) || (ast_array[i]->is_error == -1) || (ast_array[i]->is_error == 2) ) ||
           (ast_array[i]->ast_type == INSTRUCTION) ||
           ((ast_array[i]->ast_type ==DIRECTIVE) && (((ast_array[i]->ast_options.Directive.dir_name == ENTRY) || (ast_array[i]->ast_options.Directive.dir_name == EXTERNAL)))))
            
        {
            continue;
        }
        /* get the number of operands on current command line */
        num_of_rows = get_num_of_output_rows(ast_array[i]);

        if((STARTING_ROW + total_IC_lines_so_far + total_DC_lines_so_far + num_of_rows) > MAX_MEMORY_SIZE)
        {
            printf("\nExceeded total memory size, stopped output generator and deleted all files.\n\n");
            fclose(temp_ob);
            remove ("temp_ob_file.ob");
            return 0;
        }

        /* ob file header counters update */
        if (ast_array[i]->ast_type == DIRECTIVE) 
        {			
            for (k = 0; k < num_of_rows ; k++)
            {
                /* here we will call helper functions in order to build binary representation for each of current ast command or operand */
                curr_binary_row = directive_operand_to_binary(ast_array[i], k);
            
                /* get row number and offset (k - num_of_operands) to make sure we are giving correct row number for each operand
                 * because we already incremented data counter before k loop */
                curr_row_num = (STARTING_ROW + total_IC_lines_so_far + total_DC_lines_so_far + k);

                /* convert to output format and print to file */
                binary_to_octal(curr_binary_row , octal_rep);
                
                fprintf(temp_ob, "%.4d %.5s\n", curr_row_num, octal_rep);
    
            }
            total_DC_lines_so_far += num_of_rows;
        } 
    }

    /* Read contents from ob temp file and print to ob output file */
    fclose(temp_ob);
    
    temp_ob = fopen("temp_ob_file.ob", "r");
    if (temp_ob == NULL)
    {
        printf("Error: failed to open temp_ob_file.ob file\n");
        remove("temp_ob_file.ob");
        return -1;
    }

    fprintf(ob_ptr, "   %d %d\n", total_IC_lines_so_far, total_DC_lines_so_far);
    
    while ((c = fgetc(temp_ob)) != EOF)
    {
        fputc(c, ob_ptr);
    }
    
    fclose(temp_ob);
    remove("temp_ob_file.ob");
    
    return (total_IC_lines_so_far + total_DC_lines_so_far) ;
}