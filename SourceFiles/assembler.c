/**
 * Project: Assembler Program
 * Course: "Systems Programming Laboratory"
 * University - The Open University
 * Semester: 2024B
 * 
 * 
 * @authors:
 * - Nir Darmon, ID: 206454076
 * - Adar Amir, ID: 302660717
 *
 * @description:
 * This program serves as an assembler for a custom assembly language.
 * It processes source files, performs macro preprocessing, parses the code,
 * and generates output files (.ob, .ent, .ext) if the source code is free of errors.
 * 
 * The assembler handles multiple files provided as arguments without the extension, processes each one,
 * and generates appropriate output or error messages for each file.
 */

#include "assembler.h"


int main(int argc, char* argv[])
{
    int i;
    int current_file = 1; /* start from 1 to skip program name */

    printf("Starting assembler\n\n");  

    if (argc < 2)
    {
        printf("Error: no files were given\n\n");
        return 0;
    }
    printf("Number of files to work on: %d\n\n", argc-1);

    while (current_file < argc)
    {
        Ast** ast_array; /* an array of ast structs that will store the parsed source code */
        int ast_array_counter = 0;
        Labels** label_array; /* what about the line number of the code after the preprocessor */
        int label_array_counter = 0;
        char** macro_names; /* an array that will store the macro names without */
        int macro_counter = 0; /* macro array counter */
        char* as_file_name_with_ext; /* the source code file name with .as extension */
        char* am_file_name_with_ext; /* the source code after macros implementation name */
        FILE* as_file; /* a pointer to the original .as source code file */
        FILE* am_file; /* a pointer to the source code after macros implementation file */
        int create_file_return = 0;
    
        printf("--------------------------------------------\n");
        printf("Started working on file: %s.as\n\n", argv[current_file]);

        /* open .as file */
        create_file_return = create_file_and_save_name_with_ext(argv[current_file], &as_file_name_with_ext, &as_file, ".as", "r");
        if (create_file_return == -1)/* memory allocation failure */
        {
            return create_file_return;
        }
        else if (create_file_return == -2)/* file does not exist */
        {
            current_file++;
            if (current_file < argc)
            {/* there is at least one more file to handle */
                printf("--------------------------------------------\n");
                printf("\nProceeding to the next file: %s.as\n\n", argv[current_file]);
                continue;
            }
            else break;
        }

        /* create and open .am file */
        create_file_return = create_file_and_save_name_with_ext(argv[current_file], &am_file_name_with_ext, &am_file, ".am", "w");
        if (create_file_return == -1 || create_file_return == -2)/* memory allocation failure */
        {
            fclose(as_file);
            free(as_file_name_with_ext);
            as_file_name_with_ext = NULL; /* avoid dangling pointer */
            return create_file_return;
        }

        printf("\nStarting preprocessor\n\n");
        {/* preprocessor phase */
            i = preproc(as_file_name_with_ext, &macro_names, &macro_counter, as_file, am_file);
            if (i == -1 || i == -2)
            {
                fclose(as_file);
                fclose(am_file);
                remove(am_file_name_with_ext);
                free(as_file_name_with_ext);
                as_file_name_with_ext = NULL; /* avoid dangling pointer */
                free(am_file_name_with_ext);
                am_file_name_with_ext = NULL; /* avoid dangling pointer */
                if (macro_names != NULL)
                {
                    free_matrix_char_pointer(&macro_names, macro_counter);
                }
                return i;
            }
            else if (i == 0)
            {
                printf("\npreprocessor completed with errors and deleted file %s\n", am_file_name_with_ext);
                printf("Assembler finished working on file: %s.as\n", argv[current_file]);
                printf("--------------------------------------------\n");
                fclose(as_file);
                fclose(am_file);
                remove(am_file_name_with_ext);
                if (macro_names != NULL)
                {
                    free_matrix_char_pointer(&macro_names, macro_counter);
                }
                free(as_file_name_with_ext);
                as_file_name_with_ext = NULL; /* avoid dangling pointer */
                free(am_file_name_with_ext);
                am_file_name_with_ext = NULL; /* avoid dangling pointer */
                current_file++;
                if (current_file < argc)
                {/* there is at least one more file to handle */
                    printf("\nProceeding to the next file: %s.as\n\n", argv[current_file]);
                    continue;
                }
                else break;
            }
            else /* i == 1 */
            {
                printf("\nPreprocessor completed successfully\n\n");
            }
        }/* end of preprocessor phase */

        /* check if am_file is empty using ftell */
        if (ftell(am_file) == 0)
        {
            printf("%s.as file had only empty lines, therefore preprocessor deleted %s.am file\n", argv[current_file], argv[current_file]);
            printf("Assembler finished working on file: %s.as\n", argv[current_file]);
            printf("--------------------------------------------\n");
            fclose(as_file);
            free(as_file_name_with_ext);
            as_file_name_with_ext = NULL; /* avoid dangling pointer */
            fclose(am_file);
            remove(am_file_name_with_ext);
            free_matrix_char_pointer(&macro_names, macro_counter);
            free(am_file_name_with_ext);
            am_file_name_with_ext = NULL; /* avoid dangling pointer */
            current_file++;
            if (current_file < argc)
            {/* there is at least one more file to handle */
                printf("\nProceeding to the next file: %s.as\n\n", argv[current_file]);
                continue;
            }
            else 
            {
                free_label_array(&label_array, label_array_counter);
                free_ast_array(&ast_array, ast_array_counter);
                break;
            }
        }

        /* finished working with .as file */
        fclose(as_file);
        free(as_file_name_with_ext);
        as_file_name_with_ext = NULL; /* avoid dangling pointer */

        /* reopen .am file for text parsing */
        fclose(am_file);
        am_file = fopen(am_file_name_with_ext, "r");
        if (am_file == NULL)
        {
            fprintf(stderr, "Error: failed to open %s file\n", am_file_name_with_ext);
            free_matrix_char_pointer(&macro_names, macro_counter);
            free(am_file_name_with_ext);
            am_file_name_with_ext = NULL; /* avoid dangling pointer */
            free_label_array(&label_array, label_array_counter);
            free_ast_array(&ast_array, ast_array_counter);
            return -2;
        }

        {/* parser phase */
            printf("\nStarting parser\n\n");
            i = parse_file(am_file, am_file_name_with_ext , &ast_array, &ast_array_counter, &label_array, &label_array_counter, macro_names, macro_counter);
            fclose(am_file);
            free (am_file_name_with_ext);
            am_file_name_with_ext = NULL; /* avoid dangling pointer */
            free_matrix_char_pointer(&macro_names, macro_counter);
            if (i == -1)
            {
                printf("\nParser failed due to memory allocation failure\n\n");
                free_label_array(&label_array, label_array_counter);
                free_ast_array(&ast_array, ast_array_counter);
                return -1;
            }
            else if (i == 1)
            {
                printf("\nParser completed and found errors (all errors that were found are printed above)\n\n");
            }
            else
            {/* i == 0 */
                printf("\nParser completed successfully\n\n");
            } 
        }/* end of parser phase */

        if(i == 0)/* create output files only if parser completed without errors */
        {/*  Output generator phase  */
            FILE* ent_ptr;
            FILE* ext_ptr;
            FILE* ob_ptr;
            char* ent_f_name_w_ext;
            char* ext_f_name_w_ext;   
            char* ob_f_name_w_ext;
            int printer_return = 0;
            
            printf("\nStarting output generator\n\n");


            /* creating files for output generation and freeing if failed to open */
            create_file_return = create_file_and_save_name_with_ext(argv[current_file], &ent_f_name_w_ext, &ent_ptr, ".ent", "w");
            if (create_file_return == -1 || create_file_return == -2)
            {
                free_label_array(&label_array, label_array_counter);
                free_ast_array(&ast_array, ast_array_counter);
                return create_file_return;
            }

            create_file_return = create_file_and_save_name_with_ext(argv[current_file], &ext_f_name_w_ext, &ext_ptr, ".ext", "w");
            if (create_file_return == -1 || create_file_return == -2)
            {
                free_label_array(&label_array, label_array_counter);
                free_ast_array(&ast_array, ast_array_counter);
                fclose(ent_ptr);
                remove(ent_f_name_w_ext);
                free(ent_f_name_w_ext);
                return create_file_return;
            }

            create_file_return = create_file_and_save_name_with_ext(argv[current_file], &ob_f_name_w_ext, &ob_ptr, ".ob", "w");
            if (create_file_return == -1 || create_file_return == -2)
            {
                free_label_array(&label_array, label_array_counter);
                free_ast_array(&ast_array, ast_array_counter);
                fclose(ent_ptr);
                remove(ent_f_name_w_ext);
                free(ent_f_name_w_ext);
                fclose(ext_ptr);
                remove(ext_f_name_w_ext);
                free(ext_f_name_w_ext);
                return create_file_return;
            }

            /* adding row numbers to each cell in label array to be used later on when printing ent file mainly */
            add_row_num_to_label_arr(ast_array, ast_array_counter, label_array, label_array_counter);

            /* calling for main printing function with all the necessary file pointers and arrays. This return value
             * will be used in the empty_file_remover function later*/
            printer_return = printer_func(ob_ptr,ent_ptr,ext_ptr, ast_array, ast_array_counter, label_array, label_array_counter);
        
            fclose(ob_ptr);
            fclose(ent_ptr);
            fclose(ext_ptr);
            
           
            /* removing Empty files based on printer_return */
            empty_file_remover(ob_f_name_w_ext, ent_f_name_w_ext, ext_f_name_w_ext, printer_return);

            free(ent_f_name_w_ext);
            free(ext_f_name_w_ext);
            free(ob_f_name_w_ext);

            printf("\nOutput generator phase completed\n\n");

        }/* end of output generator phase */

        /* free dynamically allocated data before next file */
        free_label_array(&label_array, label_array_counter);
        free_ast_array(&ast_array, ast_array_counter);
    	printf("Assembler finished working on file: %s.as\n", argv[current_file]);
        printf("--------------------------------------------\n");
        
        /* continue to next file */
        current_file++;
        if (current_file < argc)
        {/* there is at least one more file to handle */
            printf("\nProceeding to the next file: %s.as\n\n", argv[current_file]);
        }

    }/* end of while, finished working on all files */

    printf("Assembler finished working on all files\n\n");
    printf("Assembler completed successfully\n\n");
    return 0;
}
