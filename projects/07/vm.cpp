#include <stdio.h>
#include <stdlib.h>
#include <string>

#define BUFFER_SIZE 1024

typedef uint64_t u64;

char* read_line(FILE *file)
{
    char buffer[BUFFER_SIZE];
    int len = sizeof(buffer);
    
    char *line = (char *) malloc(BUFFER_SIZE);
    line[0] = '\0';
    
    char *read;
    while (read = fgets(buffer, sizeof(buffer), file)) {
        // Still processing the previous line
        if (len - strlen(line) < sizeof(buffer)) {
            len *= 2;
            line = (char *) realloc(line, len);
            if (!line) {
                printf("Error: unable to reallocate memory for line buffer");
                if (line) free(line);
                return NULL;
            }
        }
        
        strcat(line, buffer);
        
        // Line completed
        // TODO: last lines are skiped when there is no final new line. Last char != EOF. FIX!!!!
        char *last_char = &line[strlen(line) - 1];
        if (*last_char == '\n' || *last_char == 'EOF') {
            *last_char = '\0'; // Remove new line or EOF char
            
            if (strlen(line) == 0) continue; // Empty line
            
            if (line[0] == '/' && line[1] == '/') { // Skip comments
                line[0] = '\0';
                continue;
            } else {
                break;
            }
        }
    }
    
    if (!read) {
        if (line) free(line);
        return NULL;
    }
    
    return line;
}

bool start_with(const char *str, const char *pre)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

enum Command_Type {
    C_ARITHMETIC_ADD,
    C_ARITHMETIC_SUB,
    C_ARITHMETIC_NEG,
    C_ARITHMETIC_EQ,
    C_ARITHMETIC_GT,
    C_ARITHMETIC_LT,
    C_ARITHMETIC_AND,
    C_ARITHMETIC_OR,
    C_ARITHMETIC_NOT,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL,
    
    C_UNKNOWN
};

struct Command {
    char *line;
    Command_Type type;
    char *arg_1;
    int arg_2;
};

Command_Type get_command_type(char *line)
{
    if      (start_with(line, "add"))  return C_ARITHMETIC_ADD;
    else if (start_with(line, "sub"))  return C_ARITHMETIC_SUB;
    else if (start_with(line, "neg"))  return C_ARITHMETIC_NEG;
    else if (start_with(line, "eq"))   return C_ARITHMETIC_EQ;
    else if (start_with(line, "gt"))   return C_ARITHMETIC_GT;
    else if (start_with(line, "lt"))   return C_ARITHMETIC_LT;
    else if (start_with(line, "and"))  return C_ARITHMETIC_AND;
    else if (start_with(line, "or"))   return C_ARITHMETIC_OR;
    else if (start_with(line, "not"))  return C_ARITHMETIC_NOT;
    else if (start_with(line, "push")) return C_PUSH;
    else if (start_with(line, "pop"))  return C_POP;
    
    return C_UNKNOWN;
}

bool is_arithmetic_type(Command_Type command_type)
{
    return command_type == C_ARITHMETIC_ADD
        || command_type == C_ARITHMETIC_SUB
        || command_type == C_ARITHMETIC_NEG
        || command_type == C_ARITHMETIC_EQ
        || command_type == C_ARITHMETIC_GT
        || command_type == C_ARITHMETIC_LT
        || command_type == C_ARITHMETIC_AND
        || command_type == C_ARITHMETIC_OR
        || command_type == C_ARITHMETIC_NOT;
}

void get_arguments(Command *command)
{
    char *line = (char *) malloc(strlen(command->line) + 1);
    strcpy(line, command->line);
    char *token = strtok(line, " ");
    int arg_num = 0;
    while (token != NULL) {
        // Arg 1
        if (arg_num == 1) {
            if (command->type == C_RETURN) {
                command->arg_1 = "";
            } else if (is_arithmetic_type(command->type)) {
                command->arg_1 = command->line;
            }
            
            command->arg_1 = token;
        }
        
        // Arg 2
        if (arg_num == 2) {
            if (command->type == C_PUSH
                || command->type == C_POP
                || command->type == C_FUNCTION
                || command->type == C_CALL) {
                
                command->arg_2 = atoi(token);
            } else {
                command->arg_2 = -1;
            }
        }
        
        ++arg_num;
        token = strtok(NULL, " ");
    }
    
    free(line);
}

static u64 index_symbols = 0;

void write_arithmetic(Command command, FILE *output)
{
    switch (command.type) {
        case C_ARITHMETIC_ADD: {
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "D=M\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "M=M+D\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M+1\n");
        } break;
        
        case C_ARITHMETIC_SUB: {
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "D=M\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "M=M-D\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M+1\n");
        } break;
        
        case C_ARITHMETIC_AND: {
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "D=M\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "M=M&D\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M+1\n");
        } break;
        
        case C_ARITHMETIC_OR: {
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "D=M\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "M=M|D\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M+1\n");
        } break;
        
        case C_ARITHMETIC_EQ: {
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "D=M\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "M=M-D\n");
            fprintf(output, "D=M\n");
            fprintf(output, "@EQ_%lld\n", index_symbols);
            fprintf(output, "D;JEQ\n");
            fprintf(output, "(NEQ_%lld)\n", index_symbols);
            fprintf(output, "    @SP\n");
            fprintf(output, "    A=M\n");
            fprintf(output, "    M=0\n");
            fprintf(output, "    @CONTINUE_%lld\n", index_symbols);
            fprintf(output, "    0;JMP\n");
            fprintf(output, "(EQ_%lld)\n", index_symbols);
            fprintf(output, "    @SP\n");
            fprintf(output, "    A=M\n");
            fprintf(output, "    M=-1\n");
            fprintf(output, "(CONTINUE_%lld)\n", index_symbols);
            fprintf(output, "@SP\n");
            fprintf(output, "M=M+1\n");
            
            ++index_symbols;
        } break;
        
        case C_ARITHMETIC_GT: {
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "D=M\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "M=M-D\n");
            fprintf(output, "D=M\n");
            fprintf(output, "@GT_%lld\n", index_symbols);
            fprintf(output, "D;JGT\n");
            fprintf(output, "(NGT_%lld)\n", index_symbols);
            fprintf(output, "    @SP\n");
            fprintf(output, "    A=M\n");
            fprintf(output, "    M=0\n");
            fprintf(output, "    @CONTINUE_%lld\n", index_symbols);
            fprintf(output, "    0;JMP\n");
            fprintf(output, "(GT_%lld)\n", index_symbols);
            fprintf(output, "    @SP\n");
            fprintf(output, "    A=M\n");
            fprintf(output, "    M=-1\n");
            fprintf(output, "(CONTINUE_%lld)\n", index_symbols);
            fprintf(output, "@SP\n");
            fprintf(output, "M=M+1\n");
            
            ++index_symbols;
        } break;
        
        case C_ARITHMETIC_LT: {
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "D=M\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "M=M-D\n");
            fprintf(output, "D=M\n");
            fprintf(output, "@LT_%lld\n", index_symbols);
            fprintf(output, "D;JLT\n");
            fprintf(output, "(NLT_%lld)\n", index_symbols);
            fprintf(output, "    @SP\n");
            fprintf(output, "    A=M\n");
            fprintf(output, "    M=0\n");
            fprintf(output, "    @CONTINUE_%lld\n", index_symbols);
            fprintf(output, "    0;JMP\n");
            fprintf(output, "(LT_%lld)\n", index_symbols);
            fprintf(output, "    @SP\n");
            fprintf(output, "    A=M\n");
            fprintf(output, "    M=-1\n");
            fprintf(output, "(CONTINUE_%lld)\n", index_symbols);
            fprintf(output, "@SP\n");
            fprintf(output, "M=M+1\n");
            
            ++index_symbols;
        } break;
        
        //
        // Unary operations
        //
        case C_ARITHMETIC_NEG: {
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "M=-M\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M+1\n");
        } break;
        
        case C_ARITHMETIC_NOT: {
            fprintf(output, "@SP\n");
            fprintf(output, "M=M-1\n");
            fprintf(output, "A=M\n");
            fprintf(output, "M=!M\n");
            fprintf(output, "@SP\n");
            fprintf(output, "M=M+1\n");
        } break;
    }
}

void write_push_pop(Command command, FILE *output)
{
    switch (command.type) {
        case C_PUSH: {
            if (strcmp(command.arg_1, "constant") == 0) {
                fprintf(output, "@%d\n", command.arg_2);
                fprintf(output, "D=A\n");
                fprintf(output, "@SP\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M+1\n");
            } else if (strcmp(command.arg_1, "local") == 0) {
                fprintf(output, "@%d\n", command.arg_2);
                fprintf(output, "D=A\n");
                fprintf(output, "@LCL\n");
                fprintf(output, "A=M+D\n");
                fprintf(output, "D=M\n");
                fprintf(output, "@SP\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M+1\n");
            } else if (strcmp(command.arg_1, "argument") == 0) {
                fprintf(output, "@%d\n", command.arg_2);
                fprintf(output, "D=A\n");
                fprintf(output, "@ARG\n");
                fprintf(output, "A=M+D\n");
                fprintf(output, "D=M\n");
                fprintf(output, "@SP\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M+1\n");
            } else if (strcmp(command.arg_1, "this") == 0) {
                fprintf(output, "@%d\n", command.arg_2);
                fprintf(output, "D=A\n");
                fprintf(output, "@THIS\n");
                fprintf(output, "A=M+D\n");
                fprintf(output, "D=M\n");
                fprintf(output, "@SP\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M+1\n");
            } else if (strcmp(command.arg_1, "that") == 0) {
                fprintf(output, "@%d\n", command.arg_2);
                fprintf(output, "D=A\n");
                fprintf(output, "@THAT\n");
                fprintf(output, "A=M+D\n");
                fprintf(output, "D=M\n");
                fprintf(output, "@SP\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M+1\n");
            } else if (strcmp(command.arg_1, "temp") == 0) {
                int temp_address = command.arg_2 + 5; // Temp[0] register is in RAM[5]
                
                fprintf(output, "@R%d\n", temp_address);
                fprintf(output, "D=M\n");
                fprintf(output, "@SP\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M+1\n");
            } else if (strcmp(command.arg_1, "pointer") == 0) {
                if (command.arg_2 == 0) {
                    // THIS pointer
                    fprintf(output, "@THIS\n");
                    fprintf(output, "D=M\n");
                    fprintf(output, "@SP\n");
                    fprintf(output, "A=M\n");
                    fprintf(output, "M=D\n");
                    fprintf(output, "@SP\n");
                    fprintf(output, "M=M+1\n");
                } else if (command.arg_2 == 1) {
                    // THAT pointer
                    fprintf(output, "@THAT\n");
                    fprintf(output, "D=M\n");
                    fprintf(output, "@SP\n");
                    fprintf(output, "A=M\n");
                    fprintf(output, "M=D\n");
                    fprintf(output, "@SP\n");
                    fprintf(output, "M=M+1\n");
                }
            } else if (strcmp(command.arg_1, "static") == 0) {
                int address = command.arg_2 + 16; // static[0] address is in RAM[16]
                
                fprintf(output, "@%d\n", address);
                fprintf(output, "D=M\n");
                fprintf(output, "@SP\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M+1\n");
            }
            
        } break;
        
        case C_POP: {
            if (strcmp(command.arg_1, "local") == 0) {
                fprintf(output, "@%d\n", command.arg_2);
                fprintf(output, "D=A\n");
                fprintf(output, "@LCL\n");
                fprintf(output, "D=M+D\n");
                fprintf(output, "@R13\n"); // Location of local base address + padding (RAM[local + i])
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M-1\n");
                fprintf(output, "A=M\n");
                fprintf(output, "D=M\n");
                fprintf(output, "@R13\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
            } else if (strcmp(command.arg_1, "argument") == 0) {
                fprintf(output, "@%d\n", command.arg_2);
                fprintf(output, "D=A\n");
                fprintf(output, "@ARG\n");
                fprintf(output, "D=M+D\n");
                fprintf(output, "@R13\n"); // Location of argument base address + padding (RAM[local + i])
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M-1\n");
                fprintf(output, "A=M\n");
                fprintf(output, "D=M\n");
                fprintf(output, "@R13\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
            } else if (strcmp(command.arg_1, "this") == 0) {
                fprintf(output, "@%d\n", command.arg_2);
                fprintf(output, "D=A\n");
                fprintf(output, "@THIS\n");
                fprintf(output, "D=M+D\n");
                fprintf(output, "@R13\n"); // Location of this base address + padding (RAM[local + i])
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M-1\n");
                fprintf(output, "A=M\n");
                fprintf(output, "D=M\n");
                fprintf(output, "@R13\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
            } else if (strcmp(command.arg_1, "that") == 0) {
                fprintf(output, "@%d\n", command.arg_2);
                fprintf(output, "D=A\n");
                fprintf(output, "@THAT\n");
                fprintf(output, "D=M+D\n");
                fprintf(output, "@R13\n"); // Location of that base address + padding (RAM[local + i])
                fprintf(output, "M=D\n");
                fprintf(output, "@SP\n");
                fprintf(output, "M=M-1\n");
                fprintf(output, "A=M\n");
                fprintf(output, "D=M\n");
                fprintf(output, "@R13\n");
                fprintf(output, "A=M\n");
                fprintf(output, "M=D\n");
            } else if (strcmp(command.arg_1, "temp") == 0) {
                int temp_address = command.arg_2 + 5; // Temp[0] register is in RAM[5]
                
                fprintf(output, "@SP\n");
                fprintf(output, "M=M-1\n");
                fprintf(output, "A=M\n");
                fprintf(output, "D=M\n");
                fprintf(output, "@R%d\n", temp_address);
                fprintf(output, "M=D\n");
            } else if (strcmp(command.arg_1, "pointer") == 0) {
                if (command.arg_2 == 0) {
                    // THIS pointer
                    fprintf(output, "@SP\n");
                    fprintf(output, "M=M-1\n");
                    fprintf(output, "A=M\n");
                    fprintf(output, "D=M\n");
                    fprintf(output, "@THIS\n");
                    fprintf(output, "M=D\n");
                } else if (command.arg_2 == 1) {
                    // THAT pointer
                    fprintf(output, "@SP\n");
                    fprintf(output, "M=M-1\n");
                    fprintf(output, "A=M\n");
                    fprintf(output, "D=M\n");
                    fprintf(output, "@THAT\n");
                    fprintf(output, "M=D\n");
                }
            } else if (strcmp(command.arg_1, "static") == 0) {
                int address = command.arg_2 + 16; // static[0] address is in RAM[16]
                
                fprintf(output, "@SP\n");
                fprintf(output, "M=M-1\n");
                fprintf(output, "A=M\n");
                fprintf(output, "D=M\n");
                fprintf(output, "@%d\n", address);
                fprintf(output, "M=D\n");
            }
            
        } break;
    }
}

int main(int argc, char *argv[])
{
    char *filename = "MemoryAccess/StaticTest/StaticTest.vm";
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file %s.\n", filename);
        goto free_resources;
    }
    
    char *output_filename = "MemoryAccess/StaticTest/StaticTest.asm";
    FILE *output = fopen(output_filename, "w");
    if (!output) {
        printf("Could not open file %s to write on.", output_filename);
        goto free_resources;
    }
    
    fprintf(output, "@256\n");
    fprintf(output, "D=A\n");
    fprintf(output, "@SP\n");
    fprintf(output, "M=D\n");
    
    char *line;
    while ((line = read_line(file)) != NULL) {
        Command command = {};
        command.line = line;
        command.type = get_command_type(line);
        command.arg_1 = "";
        command.arg_2 = -1;
        get_arguments(&command);
        
        write_arithmetic(command, output);
        write_push_pop(command, output);
        
        free(line);
    }
    
    fprintf(output, "(END)\n");
    fprintf(output, "@END\n");
    fprintf(output, "0;JMP\n");
    
    free_resources:
    if (output) fclose(output);
    if (file) fclose(file);
}

