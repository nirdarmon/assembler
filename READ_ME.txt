Project: Assembler Program
Course: "Systems Programming Laboratory"
University - The Open University
Semester: 2024B
 
 
Authors:
    - Nir Darmon, ID: 206454076
    - Adar Amir, ID: 302660717

Description:
    This program serves as an assembler for a custom assembly language.
    It processes source files, performs macro preprocessing, parses the code,
    and generates output files (.ob, .ent, .ext) if the source code is free of errors.
 
The assembler handles multiple files provided as arguments without the extension, processes each one,
and generates appropriate output or error messages for each file.

 
1.  In order to run your own testers on our program you will need to make sure you save those testers in the main/parent
    directory and NOT in the Testers/ directory (this folder holds only the testers we used including output files and
    terminal screenshots).

2.  The executable is in build/bin/ directory.

3.  You can use 'make clean' command in terminal to remove build/ directory along with tester output files.

4.  You can use 'make valgrind' after updating ARGS in Makefile file.

5.  Compiling command currently includes '-g' flag in it. 