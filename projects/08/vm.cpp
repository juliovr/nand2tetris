#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <dirent.h>
#include <map>

#define global static
#define BUFFER_SIZE 1024

uint64_t line = 0;

typedef uint64_t u64;


#define STRING_BUFFER 100
#define STATIC_BASE_ADDRESS 16

global int static_variables_count = 0;


char filename[STRING_BUFFER];
int filename_index = 0;

struct cmp_str
{
    bool operator()(char const *a, char const *b) const
    {
        return std::strcmp(a, b) < 0;
    }
};

global std::map<char*, int, cmp_str> map_file_base_address_static_variables;

void add_base_address(char *key, int value)
{
    char *lengthp = key;
    int length = 0;
    while (*lengthp != 0) {
        if (*lengthp == '(' || *lengthp == ')') {
            ++lengthp;
            continue;
        }
        
        ++lengthp;
        ++length;
    }
    
    char *filename = (char *) malloc(length + 1);
    char *filenamep = filename;
    while (length > 0) {
        if (*key == '(' || *key == ')') {
            ++key;
            continue;
        }
        
        *filenamep++ = *key++;
        --length;
    }
    
    *filenamep = '\0';
    
    map_file_base_address_static_variables[filename] = value;
}

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

bool ends_with(const char *str, const char *suffix)
{
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (str_len < suffix_len) return false;
    
    return memcmp(str + (str_len - suffix_len), suffix, suffix_len) == 0;
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
    if      (start_with(line, "add"))      return C_ARITHMETIC_ADD;
    else if (start_with(line, "sub"))      return C_ARITHMETIC_SUB;
    else if (start_with(line, "neg"))      return C_ARITHMETIC_NEG;
    else if (start_with(line, "eq"))       return C_ARITHMETIC_EQ;
    else if (start_with(line, "gt"))       return C_ARITHMETIC_GT;
    else if (start_with(line, "lt"))       return C_ARITHMETIC_LT;
    else if (start_with(line, "and"))      return C_ARITHMETIC_AND;
    else if (start_with(line, "or"))       return C_ARITHMETIC_OR;
    else if (start_with(line, "not"))      return C_ARITHMETIC_NOT;
    else if (start_with(line, "push"))     return C_PUSH;
    else if (start_with(line, "pop"))      return C_POP;
    else if (start_with(line, "label"))    return C_LABEL;
    else if (start_with(line, "goto"))     return C_GOTO;
    else if (start_with(line, "if-goto"))  return C_IF;
    else if (start_with(line, "function")) return C_FUNCTION;
    else if (start_with(line, "return"))   return C_RETURN;
    else if (start_with(line, "call"))     return C_CALL;
    
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
            
            char *token_str = (char *) malloc(strlen(token) + 1);
            strcpy(token_str, token);
            command->arg_1 = token_str;
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

global u64 index_symbols = 0;

void write_arithmetic(Command command, FILE *output)
{
    switch (command.type) {
        case C_ARITHMETIC_ADD: {
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "D=M                    // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "M=M+D                  // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M+1                  // L-%lld\n", line++);
        } break;
        
        case C_ARITHMETIC_SUB: {
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "D=M                    // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "M=M-D                  // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M+1                  // L-%lld\n", line++);
        } break;
        
        case C_ARITHMETIC_AND: {
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "D=M                    // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "M=M&D                  // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M+1                  // L-%lld\n", line++);
        } break;
        
        case C_ARITHMETIC_OR: {
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "D=M                    // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "M=M|D                  // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M+1                  // L-%lld\n", line++);
        } break;
        
        case C_ARITHMETIC_EQ: {
            fprintf(output, "@SP                         // L-%lld\n", line++);
            fprintf(output, "M=M-1                       // L-%lld\n", line++);
            fprintf(output, "A=M                         // L-%lld\n", line++);
            fprintf(output, "D=M                         // L-%lld\n", line++);
            fprintf(output, "@SP                         // L-%lld\n", line++);
            fprintf(output, "M=M-1                       // L-%lld\n", line++);
            fprintf(output, "A=M                         // L-%lld\n", line++);
            fprintf(output, "M=M-D                       // L-%lld\n", line++);
            fprintf(output, "D=M                         // L-%lld\n", line++);
            fprintf(output, "@EQ_%lld                    // L-%lld\n", index_symbols, line++);
            fprintf(output, "D;JEQ                       // L-%lld\n", line++);
            fprintf(output, "(NEQ_%lld)\n", index_symbols);
            fprintf(output, "    @SP                     // L-%lld\n", line++);
            fprintf(output, "    A=M                     // L-%lld\n", line++);
            fprintf(output, "    M=0                     // L-%lld\n", line++);
            fprintf(output, "    @CONTINUE_%lld          // L-%lld\n", index_symbols, line++);
            fprintf(output, "    0;JMP                   // L-%lld\n", line++);
            fprintf(output, "(EQ_%lld)\n", index_symbols);
            fprintf(output, "    @SP                     // L-%lld\n", line++);
            fprintf(output, "    A=M                     // L-%lld\n", line++);
            fprintf(output, "    M=-1                    // L-%lld\n", line++);
            fprintf(output, "(CONTINUE_%lld)\n", index_symbols);
            fprintf(output, "@SP                      // L-%lld\n", line++);
            fprintf(output, "M=M+1                    // L-%lld\n", line++);
            
            ++index_symbols;
        } break;
        
        case C_ARITHMETIC_GT: {
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "M=M-1                      // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "M=M-1                      // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "M=M-D                      // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@GT_%lld                   // L-%lld\n", index_symbols, line++);
            fprintf(output, "D;JGT                      // L-%lld\n", line++);
            fprintf(output, "(NGT_%lld)\n", index_symbols);
            fprintf(output, "    @SP                    // L-%lld\n", line++);
            fprintf(output, "    A=M                    // L-%lld\n", line++);
            fprintf(output, "    M=0                    // L-%lld\n", line++);
            fprintf(output, "    @CONTINUE_%lld         // L-%lld\n", index_symbols, line++);
            fprintf(output, "    0;JMP                  // L-%lld\n", line++);
            fprintf(output, "(GT_%lld)\n", index_symbols);
            fprintf(output, "    @SP                    // L-%lld\n", line++);
            fprintf(output, "    A=M                    // L-%lld\n", line++);
            fprintf(output, "    M=-1                   // L-%lld\n", line++);
            fprintf(output, "(CONTINUE_%lld)\n", index_symbols);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M+1                  // L-%lld\n", line++);
            
            ++index_symbols;
        } break;
        
        case C_ARITHMETIC_LT: {
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "D=M                    // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "M=M-D                  // L-%lld\n", line++);
            fprintf(output, "D=M                    // L-%lld\n", line++);
            fprintf(output, "@LT_%lld               // L-%lld\n", index_symbols, line++);
            fprintf(output, "D;JLT                  // L-%lld\n", line++);
            fprintf(output, "(NLT_%lld)\n", index_symbols);
            fprintf(output, "    @SP                    // L-%lld\n", line++);
            fprintf(output, "    A=M                    // L-%lld\n", line++);
            fprintf(output, "    M=0                    // L-%lld\n", line++);
            fprintf(output, "    @CONTINUE_%lld         // L-%lld\n", index_symbols, line++);
            fprintf(output, "    0;JMP                  // L-%lld\n", line++);
            fprintf(output, "(LT_%lld)\n", index_symbols);
            fprintf(output, "    @SP                    // L-%lld\n", line++);
            fprintf(output, "    A=M                    // L-%lld\n", line++);
            fprintf(output, "    M=-1                   // L-%lld\n", line++);
            fprintf(output, "(CONTINUE_%lld)\n", index_symbols);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M+1                  // L-%lld\n", line++);
            
            ++index_symbols;
        } break;
        
        //
        // Unary operations
        //
        case C_ARITHMETIC_NEG: {
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "M=-M                   // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M+1                  // L-%lld\n", line++);
        } break;
        
        case C_ARITHMETIC_NOT: {
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M-1                  // L-%lld\n", line++);
            fprintf(output, "A=M                    // L-%lld\n", line++);
            fprintf(output, "M=!M                   // L-%lld\n", line++);
            fprintf(output, "@SP                    // L-%lld\n", line++);
            fprintf(output, "M=M+1                  // L-%lld\n", line++);
        } break;
    }
}

void write_push_pop(Command command, FILE *output)
{
    switch (command.type) {
        case C_PUSH: {
            if (strcmp(command.arg_1, "constant") == 0) {
                fprintf(output, "@%d                    // L-%lld\n", command.arg_2, line++);
                fprintf(output, "D=A                    // L-%lld\n", line++);
                fprintf(output, "@SP                    // L-%lld\n", line++);
                fprintf(output, "A=M                    // L-%lld\n", line++);
                fprintf(output, "M=D                    // L-%lld\n", line++);
                fprintf(output, "@SP                    // L-%lld\n", line++);
                fprintf(output, "M=M+1                  // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "local") == 0) {
                fprintf(output, "@%d                    // L-%lld\n", command.arg_2, line++);
                fprintf(output, "D=A                    // L-%lld\n", line++);
                fprintf(output, "@LCL                   // L-%lld\n", line++);
                fprintf(output, "A=M+D                  // L-%lld\n", line++);
                fprintf(output, "D=M                    // L-%lld\n", line++);
                fprintf(output, "@SP                    // L-%lld\n", line++);
                fprintf(output, "A=M                    // L-%lld\n", line++);
                fprintf(output, "M=D                    // L-%lld\n", line++);
                fprintf(output, "@SP                    // L-%lld\n", line++);
                fprintf(output, "M=M+1                  // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "argument") == 0) {
                fprintf(output, "@%d                    // L-%lld\n", command.arg_2, line++);
                fprintf(output, "D=A                    // L-%lld\n", line++);
                fprintf(output, "@ARG                   // L-%lld\n", line++);
                fprintf(output, "A=M+D                  // L-%lld\n", line++);
                fprintf(output, "D=M                    // L-%lld\n", line++);
                fprintf(output, "@SP                    // L-%lld\n", line++);
                fprintf(output, "A=M                    // L-%lld\n", line++);
                fprintf(output, "M=D                    // L-%lld\n", line++);
                fprintf(output, "@SP                    // L-%lld\n", line++);
                fprintf(output, "M=M+1                  // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "this") == 0) {
                fprintf(output, "@%d                      // L-%lld\n", command.arg_2, line++);
                fprintf(output, "D=A                      // L-%lld\n", line++);
                fprintf(output, "@THIS                    // L-%lld\n", line++);
                fprintf(output, "A=M+D                    // L-%lld\n", line++);
                fprintf(output, "D=M                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M+1                    // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "that") == 0) {
                fprintf(output, "@%d                      // L-%lld\n", command.arg_2, line++);
                fprintf(output, "D=A                      // L-%lld\n", line++);
                fprintf(output, "@THAT                    // L-%lld\n", line++);
                fprintf(output, "A=M+D                    // L-%lld\n", line++);
                fprintf(output, "D=M                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M+1                    // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "temp") == 0) {
                int temp_address = command.arg_2 + 5; // Temp[0] register is in RAM[5]
                
                fprintf(output, "@R%d                     // L-%lld\n", temp_address, line++);
                fprintf(output, "D=M                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M+1                    // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "pointer") == 0) {
                if (command.arg_2 == 0) {
                    // THIS pointer
                    fprintf(output, "@THIS                    // L-%lld\n", line++);
                    fprintf(output, "D=M                      // L-%lld\n", line++);
                    fprintf(output, "@SP                      // L-%lld\n", line++);
                    fprintf(output, "A=M                      // L-%lld\n", line++);
                    fprintf(output, "M=D                      // L-%lld\n", line++);
                    fprintf(output, "@SP                      // L-%lld\n", line++);
                    fprintf(output, "M=M+1                    // L-%lld\n", line++);
                } else if (command.arg_2 == 1) {
                    // THAT pointer
                    fprintf(output, "@THAT                    // L-%lld\n", line++);
                    fprintf(output, "D=M                      // L-%lld\n", line++);
                    fprintf(output, "@SP                      // L-%lld\n", line++);
                    fprintf(output, "A=M                      // L-%lld\n", line++);
                    fprintf(output, "M=D                      // L-%lld\n", line++);
                    fprintf(output, "@SP                      // L-%lld\n", line++);
                    fprintf(output, "M=M+1                    // L-%lld\n", line++);
                }
            } else if (strcmp(command.arg_1, "static") == 0) {
                int base_address = map_file_base_address_static_variables[filename];
                
                int address = base_address + command.arg_2;
                
                fprintf(output, "@%d                      // L-%lld\n", address, line++);
                fprintf(output, "D=M                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M+1                    // L-%lld\n", line++);
            }
            
        } break;
        
        case C_POP: {
            if (strcmp(command.arg_1, "local") == 0) {
                fprintf(output, "@%d                      // L-%lld\n", command.arg_2, line++);
                fprintf(output, "D=A                      // L-%lld\n", line++);
                fprintf(output, "@LCL                     // L-%lld\n", line++);
                fprintf(output, "D=M+D                    // L-%lld\n", line++);
                fprintf(output, "@R13                     // L-%lld\n", line++); // Location of local base address + padding (RAM[local + i])
                fprintf(output, "M=D                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M-1                    // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "D=M                      // L-%lld\n", line++);
                fprintf(output, "@R13                     // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "argument") == 0) {
                fprintf(output, "@%d                      // L-%lld\n", command.arg_2, line++);
                fprintf(output, "D=A                      // L-%lld\n", line++);
                fprintf(output, "@ARG                     // L-%lld\n", line++);
                fprintf(output, "D=M+D                    // L-%lld\n", line++);
                fprintf(output, "@R13                     // L-%lld\n", line++); // Location of argument base address + padding (RAM[local + i])
                fprintf(output, "M=D                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M-1                    // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "D=M                      // L-%lld\n", line++);
                fprintf(output, "@R13                     // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "this") == 0) {
                fprintf(output, "@%d                      // L-%lld\n", command.arg_2, line++);
                fprintf(output, "D=A                      // L-%lld\n", line++);
                fprintf(output, "@THIS                    // L-%lld\n", line++);
                fprintf(output, "D=M+D                    // L-%lld\n", line++);
                fprintf(output, "@R13                     // L-%lld\n", line++); // Location of this base address + padding (RAM[local + i])
                fprintf(output, "M=D                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M-1                    // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "D=M                      // L-%lld\n", line++);
                fprintf(output, "@R13                     // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "that") == 0) {
                fprintf(output, "@%d                      // L-%lld\n", command.arg_2, line++);
                fprintf(output, "D=A                      // L-%lld\n", line++);
                fprintf(output, "@THAT                    // L-%lld\n", line++);
                fprintf(output, "D=M+D                    // L-%lld\n", line++);
                fprintf(output, "@R13                     // L-%lld\n", line++); // Location of that base address + padding (RAM[local + i])
                fprintf(output, "M=D                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M-1                    // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "D=M                      // L-%lld\n", line++);
                fprintf(output, "@R13                     // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "temp") == 0) {
                int temp_address = command.arg_2 + 5; // Temp[0] register is in RAM[5]
                
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M-1                    // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "D=M                      // L-%lld\n", line++);
                fprintf(output, "@R%d                     // L-%lld\n", temp_address, line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
            } else if (strcmp(command.arg_1, "pointer") == 0) {
                if (command.arg_2 == 0) {
                    // THIS pointer
                    fprintf(output, "@SP                      // L-%lld\n", line++);
                    fprintf(output, "M=M-1                    // L-%lld\n", line++);
                    fprintf(output, "A=M                      // L-%lld\n", line++);
                    fprintf(output, "D=M                      // L-%lld\n", line++);
                    fprintf(output, "@THIS                    // L-%lld\n", line++);
                    fprintf(output, "M=D                      // L-%lld\n", line++);
                } else if (command.arg_2 == 1) {
                    // THAT pointer
                    fprintf(output, "@SP                      // L-%lld\n", line++);
                    fprintf(output, "M=M-1                    // L-%lld\n", line++);
                    fprintf(output, "A=M                      // L-%lld\n", line++);
                    fprintf(output, "D=M                      // L-%lld\n", line++);
                    fprintf(output, "@THAT                    // L-%lld\n", line++);
                    fprintf(output, "M=D                      // L-%lld\n", line++);
                }
            } else if (strcmp(command.arg_1, "static") == 0) {
                if (map_file_base_address_static_variables.find(filename) == map_file_base_address_static_variables.end()) {
                    int base_address = STATIC_BASE_ADDRESS + static_variables_count;
                    add_base_address(filename, base_address);
                }
                
                ++static_variables_count;
                
                int base_address = map_file_base_address_static_variables[filename];
                
                int address = base_address + command.arg_2;
                
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M-1                    // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "D=M                      // L-%lld\n", line++);
                fprintf(output, "@%d                      // L-%lld\n", address, line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
            }
            
        } break;
    }
}

void write_label(Command command, FILE *output)
{
    switch(command.type) {
        case C_LABEL: {
            fprintf(output, "(%s)\n", command.arg_1);
        } break;
    }
}


void write_goto(Command command, FILE *output)
{
    switch(command.type) {
        case C_GOTO: {
            fprintf(output, "@%s                        // L-%lld\n", command.arg_1, line++);
            fprintf(output, "0;JMP                      // L-%lld\n", line++);
        } break;
    }
}


void write_if(Command command, FILE *output)
{
    switch(command.type) {
        case C_IF: {
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "M=M-1                      // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@%s                        // L-%lld\n", command.arg_1, line++);
            fprintf(output, "D;JNE                      // L-%lld\n", line++);
        } break;
    }
}

int return_address_number = 0;

void write_call(Command command, FILE *output)
{
    switch(command.type) {
        case C_CALL: {
            fprintf(output, "\n// call %s %d\n", command.arg_1, command.arg_2);
            
            // push return address
            fprintf(output, "@return_%d                 // L-%lld\n", return_address_number, line++);
            fprintf(output, "D=A                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "M=M+1                      // L-%lld\n", line++);
            
            // Save LCL
            fprintf(output, "@LCL                       // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "M=M+1                      // L-%lld\n", line++);
            
            // Save ARG
            fprintf(output, "@ARG                       // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "M=M+1                      // L-%lld\n", line++);
            
            // Save THIS
            fprintf(output, "@THIS                      // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "M=M+1                      // L-%lld\n", line++);
            
            // Save THAT
            fprintf(output, "@THAT                      // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "M=M+1                      // L-%lld\n", line++);
            
            // Reposition ARG
            int offset = 5 + command.arg_2;
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@%d                        // L-%lld\n", offset, line++);
            fprintf(output, "D=D-A                      // L-%lld\n", line++);
            fprintf(output, "@ARG                       // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            
            // Reposition LCL
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@LCL                       // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            
            // GOTO function
            fprintf(output, "@%s                        // L-%lld\n", command.arg_1, line++);
            fprintf(output, "0;JMP                      // L-%lld\n", line++);
            
            // Return address to come back
            fprintf(output, "(return_%d)\n", return_address_number);
            
            fprintf(output, "\n// end call statement\n");
            
            ++return_address_number;
        } break;
    }
}


void write_return(Command command, FILE *output)
{
    switch(command.type) {
        case C_RETURN: {
            fprintf(output, "\n// beginning return\n");
            
            
            // Restore caller's pointers
            // Save LCL in @R12
            fprintf(output, "@LCL                       // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@R12                       // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++); // LCL pointer dereferenced
            
            // Save return address
            fprintf(output, "@5                         // L-%lld\n", line++);
            fprintf(output, "A=D-A                      // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@R15                       // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            
            
            // pop argument 0
            fprintf(output, "@0                         // L-%lld\n", line++);
            fprintf(output, "D=A                        // L-%lld\n", line++);
            fprintf(output, "@ARG                       // L-%lld\n", line++);
            fprintf(output, "D=M+D                      // L-%lld\n", line++);
            fprintf(output, "@R13                       // L-%lld\n", line++); // Location of argument base address + padding (RAM[local + i])
            fprintf(output, "M=D                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "M=M-1                      // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@R13                       // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            
            // SP = ARG + 1
            fprintf(output, "@ARG                       // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@SP                        // L-%lld\n", line++);
            fprintf(output, "M=D+1                      // L-%lld\n", line++);
            
            fprintf(output, "@R12                       // L-%lld\n", line++);
            fprintf(output, "A=M-1                      // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@THAT                      // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            
            fprintf(output, "@2                         // L-%lld\n", line++);
            fprintf(output, "D=A                        // L-%lld\n", line++);
            fprintf(output, "@R12                       // L-%lld\n", line++);
            fprintf(output, "A=M-D                      // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@THIS                      // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            
            fprintf(output, "@3                         // L-%lld\n", line++);
            fprintf(output, "D=A                        // L-%lld\n", line++);
            fprintf(output, "@R12                       // L-%lld\n", line++);
            fprintf(output, "A=M-D                      // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@ARG                       // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            
            fprintf(output, "@4                         // L-%lld\n", line++);
            fprintf(output, "D=A                        // L-%lld\n", line++);
            fprintf(output, "@R12                       // L-%lld\n", line++);
            fprintf(output, "A=M-D                      // L-%lld\n", line++);
            fprintf(output, "D=M                        // L-%lld\n", line++);
            fprintf(output, "@LCL                       // L-%lld\n", line++);
            fprintf(output, "M=D                        // L-%lld\n", line++);
            
            fprintf(output, "@R15                       // L-%lld\n", line++);
            fprintf(output, "A=M                        // L-%lld\n", line++);
            fprintf(output, "0;JMP                      // L-%lld\n", line++);
            
            fprintf(output, "\n// end return\n");
            
        } break;
    }
}


void write_function(Command command, FILE *output)
{
    switch(command.type) {
        case C_FUNCTION: {
            fprintf(output, "\n// function %s %d\n", command.arg_1, command.arg_2);
            fprintf(output, "(%s)\n", command.arg_1);
            
            int local_variables = command.arg_2;
            for (int i = 0; i < local_variables; ++i) {
                fprintf(output, "@0                       // L-%lld\n", line++);
                fprintf(output, "D=A                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "A=M                      // L-%lld\n", line++);
                fprintf(output, "M=D                      // L-%lld\n", line++);
                fprintf(output, "@SP                      // L-%lld\n", line++);
                fprintf(output, "M=M+1                    // L-%lld\n", line++);
            }
            
            fprintf(output, "\n// end function declaration\n");
        } break;
    }
}


void write_bootstrap_init_code(FILE *output)
{
    fprintf(output, "@256                   // L-%lld\n", line++);
    fprintf(output, "D=A                    // L-%lld\n", line++);
    fprintf(output, "@SP                    // L-%lld\n", line++);
    fprintf(output, "M=D                    // L-%lld\n", line++);
    
    Command command = {};
    command.type = C_CALL;
    command.arg_1 = "Sys.init\0";
    command.arg_2 = 0;
    
    write_call(command, output);
}


int main(int argc, char *argv[])
{
    //char *output_filename = "FunctionCalls/FibonacciElement/FibonacciElement.asm";
    //char *output_filename = "FunctionCalls/NestedCall/NestedCall.asm";
    char *output_filename = "FunctionCalls/StaticsTest/StaticsTest.asm";
    FILE *output = fopen(output_filename, "w");
    if (!output) {
        printf("Could not open file %s to write on.", output_filename);
        goto free_resources;
    }
    
    write_bootstrap_init_code(output);
    
    
    //DIR *dir = opendir("FunctionCalls/FibonacciElement");
    //DIR *dir = opendir("FunctionCalls/NestedCall");
    DIR *dir = opendir("FunctionCalls/StaticsTest");
    
	if (dir == NULL)
        return 1;
    
    struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
        if (!ends_with(entry->d_name, ".vm")) continue;
        
        //sprintf(filename, "FunctionCalls/FibonacciElement/%s", entry->d_name);
        //sprintf(filename, "FunctionCalls/NestedCall/%s", entry->d_name);
        sprintf(filename, "FunctionCalls/StaticsTest/%s", entry->d_name);
        
        FILE *file = fopen(filename, "r");
        if (!file) {
            printf("Could not open file %s.\n", filename);
            goto free_resources;
        }
        
        
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
            write_label(command, output);
            write_goto(command, output);
            write_if(command, output);
            write_call(command, output);
            write_return(command, output);
            write_function(command, output);
            
            free(command.line);
            if (*command.arg_1 != '\0') free(command.arg_1);
        }
        
        if (file) fclose(file);
	}
    
    
    free_resources:
    if (output) fclose(output);
    
    for (auto& s : map_file_base_address_static_variables) {
        free(s.first);
    }
}

