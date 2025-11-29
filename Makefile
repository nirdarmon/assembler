CC			= gcc
CFLAGS		= -ansi -pedantic -Wall -g -I$(HEADER_DIR)
LFDFLAGS	= 
PROG_NAME	= assembler
BUILD_DIR	= build
OBJ_DIR 	= $(BUILD_DIR)/obj
BIN_DIR		= $(BUILD_DIR)/bin
SOURCE_DIR	= SourceFiles
HEADER_DIR	= HeaderFiles
ARGS		= 


.PHONY: clean bulid_env all valgrind

all: build_env $(PROG_NAME)

$(PROG_NAME): assembler.o utils.o preproc.o  parser.o lexer.o output_generator.o output_helper.o
	$(CC) $(CFLAGS) $(OBJ_DIR)/*.o -o $(BIN_DIR)/$@ $(LFDFLAGS)



assembler.o: $(SOURCE_DIR)/assembler.c $(HEADER_DIR)/assembler.h $(HEADER_DIR)/constants.h $(HEADER_DIR)/structs.h $(HEADER_DIR)/utils.h \
 $(HEADER_DIR)/preproc.h $(HEADER_DIR)/parser.h $(HEADER_DIR)/lexer.h $(HEADER_DIR)/output_helper.h
utils.o: $(SOURCE_DIR)/utils.c $(HEADER_DIR)/utils.h
preproc.o: $(SOURCE_DIR)/preproc.c $(HEADER_DIR)/preproc.h $(HEADER_DIR)/constants.h $(HEADER_DIR)/structs.h $(HEADER_DIR)/utils.h
parser.o: $(SOURCE_DIR)/parser.c $(HEADER_DIR)/parser.h $(HEADER_DIR)/constants.h $(HEADER_DIR)/structs.h $(HEADER_DIR)/utils.h $(HEADER_DIR)/lexer.h
lexer.o: $(SOURCE_DIR)/lexer.c $(HEADER_DIR)/lexer.h $(HEADER_DIR)/constants.h $(HEADER_DIR)/utils.h
output_helper.o: $(SOURCE_DIR)/output_helper.c $(HEADER_DIR)/output_helper.h $(HEADER_DIR)/constants.h $(HEADER_DIR)/structs.h
output_generator.o: $(SOURCE_DIR)/output_generator.c $(HEADER_DIR)/output_generator.h $(HEADER_DIR)/constants.h \
 $(HEADER_DIR)/structs.h $(HEADER_DIR)/output_helper.h




%.o:
	$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$@

clean:
	rm -rf $(BUILD_DIR)
	rm -rf *.o *.am *.ob *.ent *.ext

build_env:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)

valgrind: $(PROG_NAME)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s $(BIN_DIR)/$(PROG_NAME) $(ARGS)
	
	
