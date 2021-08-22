/*
    My implementation of the assembler for hack machine language from "The Elements of Computing Systems".
Only for learning purpose; it has no assembly input validation.
              */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <map>

#define BUFFER_SIZE 1024
#define ASSEMBLY_LINE_SIZE 16
#define global static

int length_line(char *line);
bool empty_line(char *line, int size);

enum Instruction_Type {
    A_INSTRUCTION, // Addressing instruction
    C_INSTRUCTION, // Compute instruction
    L_INSTRUCTION  // Pseudo command (for locations)
};

Instruction_Type get_instruction_type(char *line);

struct Instruction {
    char assembly_line[ASSEMBLY_LINE_SIZE + 1];
};

global int ROM_SP = 0;  // pointer to save the ROM address of labels in the symbol table
global int RAM_SP = 16; // pointer to save the RAM address of variables in the symbol table

struct cmp_str
{
    bool operator()(char const *a, char const *b) const
    {
        return std::strcmp(a, b) < 0;
    }
};

global std::map<char*, int, cmp_str> symbols;

void add_symbol(char *key, int value)
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
    
    char *symbol = (char *) malloc(length + 1);
    char *symbolp = symbol;
    while (length > 0) {
        if (*key == '(' || *key == ')') {
            ++key;
            continue;
        }
        
        *symbolp++ = *key++;
        --length;
    }
    
    *symbolp = '\0';
    
    symbols[symbol] = value;
}

void process_L_instruction(char *line)
{
    add_symbol(line, ROM_SP);
}

bool is_number(char *string)
{
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        if (string[i] == '\n' || string[i] == '\0')  {
            if (i == 0) return false; // Empty string
            
            break;
        }
        
        if (!isdigit(string[i])) {
            return false;
        }
    }
    
    return true;
}

char *trim(char *string)
{
    while (isspace(*string) || *string == '\t') ++string;
    
    if (*string == 0) return string;
    
    char *end = string + strlen(string) - 1;
    while (end > string && (isspace(*end) || *end == '\t')) --end;
    
    end[1] = '\0';
    
    return string;
}

void process_A_instruction(char *line, Instruction *instruction)
{
    char assembly_line[ASSEMBLY_LINE_SIZE + 1] = "0000000000000000";
    
    char *symbol = line + 1; // To remove the "@"
    
    int number;
    
    if (is_number(symbol)) {
        number = atoi(symbol);
    } else {
        if (symbols.find(symbol) == symbols.end()) { // not found
            add_symbol(symbol, RAM_SP++);
        }
        
        number = symbols[symbol];
    }
    
    
    // Convert number to binary
    // index > 0 because there are 15 bits available; the first bit is for identify the instruction type
    for (int digit = number, index = ASSEMBLY_LINE_SIZE - 1; digit > 0 && index > 0 ; digit /= 2, --index) {
        int remainder = digit % 2;
        assembly_line[index] = (char) (remainder + '0');
    }
    
    strcpy_s(instruction->assembly_line, assembly_line);
    
}

void process_C_instruction(char *line, int length, Instruction *instruction)
{
    char assembly_line[ASSEMBLY_LINE_SIZE + 1] = "1110000000000000";
    
    // Destination
    char *destp = strstr(line, "=");
    if (destp) {
        while (line != destp) {
            char dest = *line;
            
            if (dest == 'M') {
                assembly_line[ASSEMBLY_LINE_SIZE - 4] = '1';
            }
            
            if (dest == 'D') {
                assembly_line[ASSEMBLY_LINE_SIZE - 5] = '1';
            }
            
            if (dest == 'A') {
                assembly_line[ASSEMBLY_LINE_SIZE - 6] = '1';
            }
            
            line++;
        }
    }
    
    // Comp
    if (*line == '=') line++;
    
    char *tmp_line = line;
    int length_computation = 0;
    char *jmpp = strstr(line, ";");
    if (jmpp) {
        while (tmp_line != jmpp) {
            ++length_computation;
            ++tmp_line;
        }
    } else {
        while (*tmp_line != '\n' && *tmp_line != '\0') {
            if (*tmp_line == '/' && *(tmp_line + 1) == '/') break; // Inline comment
            if (isspace(*tmp_line)) {
                ++tmp_line;
                continue;
            }
            
            ++length_computation;
            ++tmp_line;
        }
    }
    
    char *computation = (char *) malloc(length_computation + 1);
    char *tmp = computation;
    while (length_computation-- > 0) {
        *tmp = *line;
        ++tmp;
        ++line;
    }
    
    *tmp = '\0';
    
    // NOTE: Maybe replace with map??
    char comp_part[8];
    if (strcmp(computation, "0") == 0)        strcpy_s(comp_part, "0101010");
    else if (strcmp(computation, "1") == 0)   strcpy_s(comp_part, "0111111");
    else if (strcmp(computation, "-1") == 0)  strcpy_s(comp_part, "0111010");
    else if (strcmp(computation, "D") == 0)   strcpy_s(comp_part, "0001100");
    else if (strcmp(computation, "A") == 0)   strcpy_s(comp_part, "0110000");
    else if (strcmp(computation, "!D") == 0)  strcpy_s(comp_part, "0001101");
    else if (strcmp(computation, "!A") == 0)  strcpy_s(comp_part, "0110001");
    else if (strcmp(computation, "-D") == 0)  strcpy_s(comp_part, "0001111");
    else if (strcmp(computation, "-A") == 0)  strcpy_s(comp_part, "0110011");
    else if (strcmp(computation, "D+1") == 0) strcpy_s(comp_part, "0011111");
    else if (strcmp(computation, "A+1") == 0) strcpy_s(comp_part, "0110111");
    else if (strcmp(computation, "D-1") == 0) strcpy_s(comp_part, "0001110");
    else if (strcmp(computation, "A-1") == 0) strcpy_s(comp_part, "0110010");
    else if (strcmp(computation, "D+A") == 0) strcpy_s(comp_part, "0000010");
    else if (strcmp(computation, "D-A") == 0) strcpy_s(comp_part, "0010011");
    else if (strcmp(computation, "A-D") == 0) strcpy_s(comp_part, "0000111");
    else if (strcmp(computation, "D&A") == 0) strcpy_s(comp_part, "0000000");
    else if (strcmp(computation, "D|A") == 0) strcpy_s(comp_part, "0010101");
    else if (strcmp(computation, "M") == 0)   strcpy_s(comp_part, "1110000");
    else if (strcmp(computation, "!M") == 0)  strcpy_s(comp_part, "1110001");
    else if (strcmp(computation, "-M") == 0)  strcpy_s(comp_part, "1110011");
    else if (strcmp(computation, "M+1") == 0) strcpy_s(comp_part, "1110111");
    else if (strcmp(computation, "M-1") == 0) strcpy_s(comp_part, "1110010");
    else if (strcmp(computation, "D+M") == 0) strcpy_s(comp_part, "1000010");
    else if (strcmp(computation, "D-M") == 0) strcpy_s(comp_part, "1010011");
    else if (strcmp(computation, "M-D") == 0) strcpy_s(comp_part, "1000111");
    else if (strcmp(computation, "D&M") == 0) strcpy_s(comp_part, "1000000");
    else if (strcmp(computation, "D|M") == 0) strcpy_s(comp_part, "1010101");
    
    for (int i = 0; i < 7; ++i) {
        assembly_line[i + 3] = comp_part[i];
    }
    
    if (computation) {
        free(computation);
    }
    
    // Jump
    if (strstr(line, ";")) {
        if (strstr(line, "JGT")) {
            assembly_line[ASSEMBLY_LINE_SIZE - 1] = '1';
        }
        
        if (strstr(line, "JEQ")) {
            assembly_line[ASSEMBLY_LINE_SIZE - 2] = '1';
        }
        
        if (strstr(line, "JGE")) {
            assembly_line[ASSEMBLY_LINE_SIZE - 1] = '1';
            assembly_line[ASSEMBLY_LINE_SIZE - 2] = '1';
        }
        
        if (strstr(line, "JLT")) {
            assembly_line[ASSEMBLY_LINE_SIZE - 3] = '1';
        }
        
        if (strstr(line, "JNE")) {
            assembly_line[ASSEMBLY_LINE_SIZE - 1] = '1';
            assembly_line[ASSEMBLY_LINE_SIZE - 3] = '1';
        }
        
        if (strstr(line, "JLE")) {
            assembly_line[ASSEMBLY_LINE_SIZE - 2] = '1';
            assembly_line[ASSEMBLY_LINE_SIZE - 3] = '1';
        }
        
        if (strstr(line, "JMP")) {
            assembly_line[ASSEMBLY_LINE_SIZE - 1] = '1';
            assembly_line[ASSEMBLY_LINE_SIZE - 2] = '1';
            assembly_line[ASSEMBLY_LINE_SIZE - 3] = '1';
        }
    }
    
    strcpy_s(instruction->assembly_line, assembly_line);
}


int main(int argc, char *argv[])
{
    int status_code = 0;
    
    if (argc == 1) {
        printf("Should specify the .asm and .hack file.");
        exit(1);
    }
    
    if (argc > 3) {
        printf("Should specify the .asm and .hack file.");
        exit(1);
    }
    
    char *filename = argv[1];
    
    printf("Processing file %s...\n", filename);
    
    FILE *file = fopen(filename, "r");
    
    if (!file) {
        printf("File %s does not exists.\n", filename);
        goto free_resources;
    }
    
    char *output_filename = argv[2];
    
    FILE *output = fopen(output_filename, "w");
    if (!output) {
        printf("Could not open file %s to write on.", output_filename);
        goto free_resources;
    }
    
    char buffer[BUFFER_SIZE];
    
    FILE *file_looking_symbols = fopen(filename, "r");
    if (!file_looking_symbols) {
        printf("File %s does not exists.\n", filename);
        goto free_resources;
    }
    
    
    // Predefined symbols
    add_symbol("SP", 0);
    add_symbol("LCL", 1);
    add_symbol("ARG", 2);
    add_symbol("THIS", 3);
    add_symbol("THAT", 4);
    add_symbol("R0", 0);
    add_symbol("R1", 1);
    add_symbol("R2", 2);
    add_symbol("R3", 3);
    add_symbol("R4", 4);
    add_symbol("R5", 5);
    add_symbol("R6", 6);
    add_symbol("R7", 7);
    add_symbol("R8", 8);
    add_symbol("R9", 9);
    add_symbol("R10", 10);
    add_symbol("R11", 11);
    add_symbol("R12", 12);
    add_symbol("R13", 13);
    add_symbol("R14", 14);
    add_symbol("R15", 15);
    add_symbol("SCREEN", 16384);
    add_symbol("KBD", 24576);
    
    
    while (fgets(buffer, BUFFER_SIZE - 1, file_looking_symbols)) {
        if (buffer[0] == '/' && buffer[1] == '/') {
            continue;
        }
        
        int length = length_line(buffer);
        
        if (empty_line(buffer, length)) continue;
        
        char *line = trim(buffer);
        
        Instruction_Type instruction_type = get_instruction_type(line);
        if (instruction_type == L_INSTRUCTION) {
            process_L_instruction(line);
        } else {
            ++ROM_SP;
        }
    }
    
    while (fgets(buffer, BUFFER_SIZE - 1, file)) {
        if (buffer[0] == '/' && buffer[1] == '/') {
            continue;
        }
        
        int length = length_line(buffer);
        
        if (empty_line(buffer, length)) continue;
        
        char *line = trim(buffer);
        Instruction_Type instruction_type = get_instruction_type(line);
        
        Instruction instruction = {};
        switch (instruction_type) {
            case A_INSTRUCTION:
            process_A_instruction(line, &instruction);
            break;
            
            case C_INSTRUCTION:
            process_C_instruction(line, length, &instruction);
            break;
            
            case L_INSTRUCTION: continue;
        }
        
        fputs(instruction.assembly_line, output);
        fputc('\n', output);
    }
    
    free_resources:
    if (output) fclose(output);
    if (file_looking_symbols) fclose(file_looking_symbols);
    if (file) fclose(file);
    
    for (auto& s : symbols) {
        free(s.first);
    }
    
    
    if (status_code == 0) {
        printf("Complete");
    }
    
    return status_code;
}

int length_line(char *line)
{
    int length = 0;
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        if (line[i] == '\n' || line[i] == '\0') break;
        
        ++length;
    }
    
    return length;
}

bool empty_line(char *line, int size)
{
    bool blank = true;
    for (int i = 0; i < size; ++i) {
        if (line[i] == '\n' && blank)
            return true;
        
        if (line[i] != ' ')
            return false;
    }
    
    return true;
}

Instruction_Type get_instruction_type(char *line)
{
    if (line[0] == '@')      return A_INSTRUCTION;
    else if (line[0] == '(') return L_INSTRUCTION;
    else                     return C_INSTRUCTION;
}

