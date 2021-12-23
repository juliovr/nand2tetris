2#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <map>
#include <dirent.h>

#define BUFFER_SIZE 1024

#define global static

typedef uint64_t u64;

global bool in_multiline_comment = false;

u64 line_number = 0;

#define FILENAME_BUFFER 100
char filename[FILENAME_BUFFER];
char filename_output[FILENAME_BUFFER];
char filename_output_xml[FILENAME_BUFFER];

int generate_file = 0;

char *class_name;
char *subroutine_name;
char *function_or_method;


inline
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
        } else {
            ++line_number;
        }

        strcat(line, buffer);
        
        // Line completed
        // TODO: last lines are skiped when there is no final new line. Last char != EOF. FIX!!!!
        char *last_char = &line[strlen(line) - 1];
        if (*last_char == '\n' || *last_char == EOF) {
            *last_char = '\0'; // Remove new line or EOF char
            
            if (strlen(line) == 0) continue; // Empty line
            
            if (line[0] == '/' && line[1] == '/') { // Skip comments
                line[0] = '\0';
                continue;
            } else {
                bool same_line = false;
                int index_start_multiline_comment = 0;
                
                for (int i = 1; i < strlen(line); ++i) {
                    if (in_multiline_comment) {
                        if (line[i-1] == '*' && line[i] == '/') { // End of multiline comment
                            in_multiline_comment = false;
                            
                            int new_index = 0;
                            if (same_line) {
                                new_index = i - 1;
                            } else {
                                for (int j = i+1; j < strlen(line); ++j) {
                                    line[new_index++] = line[j];
                                }
                            }
                            
                            line[new_index] = '\0';
                            break;
                            
                        } else {
                            continue; // Still in multiline comment
                        }
                    } else {
                        if (line[i-1] == '/' && line[i] == '*') { // Beginning of multiline comment
                            index_start_multiline_comment = i - 1;
                            in_multiline_comment = true;
                            same_line = true;
                        }
                    }
                }
                
                if (in_multiline_comment) {
                    line[0] = '\0';
                } else if (same_line) {
                    line[index_start_multiline_comment] = '\0';
                    
                    // TODO: Trim line because the left blanks are considered to be part of the next line
                } else {
                    break;
                }
            }
        }
    }
    
    if (!read) {
        if (line) free(line);
        return NULL;
    }
    
    return line;
}

inline
bool string_equals(const char *s1, const char *s2)
{
    bool result = strcmp(s1, s2) == 0;
    return result;
}

inline
bool start_with(const char *str, const char *pre)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

inline
bool end_with(const char *str, const char *suffix)
{
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (str_len < suffix_len) return false;
    
    return memcmp(str + (str_len - suffix_len), suffix, suffix_len) == 0;
}

inline
bool is_number(char *string)
{
    int len = strlen(string);
    for (int i = 0; i < len; ++i) {
        if (string[i] == '\n' || string[i] == '\0' || string[i] == EOF)  {
            if (i == 0) return false; // Empty string
            
            break;
        }
        
        if (!isdigit(string[i])) {
            return false;
        }
    }
    
    return true;
}

enum Token_Type {
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_INT_CONSTANT,
    TOKEN_TYPE_STRING_CONSTANT,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_UNKNOWN
};

enum KEYWORD_TYPE {
    KEYWORD_CLASS,
    KEYWORD_METHOD,
    KEYWORD_FUNCTION,
    KEYWORD_CONSTRUCTOR,
    KEYWORD_INT,
    KEYWORD_BOOLEAN,
    KEYWORD_CHAR,
    KEYWORD_VOID,
    KEYWORD_VAR,
    KEYWORD_STATIC,
    KEYWORD_FIELD,
    KEYWORD_LET,
    KEYWORD_DO,
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_RETURN,
    KEYWORD_TRUE,
    KEYWORD_FALSE,
    KEYWORD_NULL,
    KEYWORD_THIS
};


struct Token {
    char *value;
    u64 line_number;
    Token *next;
    Token_Type type;
};


struct Tokens {
    Token *head;
    Token *tail;
};

Token_Type get_token_type(Token *token)
{
    char *value = token->value;
    if (   string_equals(value, "class")
        || string_equals(value, "constructor")
        || string_equals(value, "function")
        || string_equals(value, "method")
        || string_equals(value, "field")
        || string_equals(value, "static")
        || string_equals(value, "var")
        || string_equals(value, "int")
        || string_equals(value, "char")
        || string_equals(value, "boolean")
        || string_equals(value, "void")
        || string_equals(value, "true")
        || string_equals(value, "false")
        || string_equals(value, "null")
        || string_equals(value, "this")
        || string_equals(value, "let")
        || string_equals(value, "do")
        || string_equals(value, "if")
        || string_equals(value, "else")
        || string_equals(value, "while")
        || string_equals(value, "return")) {
        
        return TOKEN_TYPE_KEYWORD;
    } else if (string_equals(value, "{")
               || string_equals(value, "}")
               || string_equals(value, "(")
               || string_equals(value, ")")
               || string_equals(value, "[")
               || string_equals(value, "]")
               || string_equals(value, ".")
               || string_equals(value, ",")
               || string_equals(value, ";")
               || string_equals(value, "+")
               || string_equals(value, "-")
               || string_equals(value, "*")
               || string_equals(value, "/")
               || string_equals(value, "&")
               || string_equals(value, "|")
               || string_equals(value, "<")
               || string_equals(value, ">")
               || string_equals(value, "&lt;")
               || string_equals(value, "&gt;")
               || string_equals(value, "=")
               || string_equals(value, "~")) {
        
        return TOKEN_TYPE_SYMBOL;
        
    } else if (is_number(value)) {
        
        int number = atoi(value);
        if (number >= 0 && number <= 32767) {
            return TOKEN_TYPE_INT_CONSTANT;
        } else {
            return TOKEN_TYPE_UNKNOWN;
        }
        
        
    } else if (start_with(value, "\"") && end_with(value, "\"")) {
        
        return TOKEN_TYPE_STRING_CONSTANT;
        
    }
    
    // Default
    return TOKEN_TYPE_IDENTIFIER;
}


Tokens* create_token_list()
{
    Tokens *tokens = (Tokens *) malloc(sizeof(Tokens));
    tokens->head = NULL;
    tokens->tail = NULL;
    
    return tokens;
}

/**
Insert new token at the end of the list.
*/
void insert_token(Tokens *tokens, char value[], int len, u64 line_number, Token_Type token_type)
{
    for (int i = 0; i < len; ++i) {
        char current_char = value[i];
        if (current_char == '\n' || current_char == EOF) break;
    }
    
    Token *new_token = (Token *) malloc(sizeof(Token));
    new_token->value = (char *) malloc(len + 1);
    new_token->line_number = line_number;
    new_token->next = NULL;
    
    strncpy(new_token->value, value, len);
    new_token->value[len] = '\0';
    
    // NOTE(julio): Only lookup for the type if is not pass a concrete type in the parameter.
    if (token_type == TOKEN_TYPE_UNKNOWN) {
        new_token->type = get_token_type(new_token);
    } else {
        new_token->type = token_type;
    }
    
    Token *token = tokens->tail;
    if (token == NULL) {
        tokens->head = new_token;
        tokens->tail = new_token;
    } else {
        token->next = new_token;
        tokens->tail = new_token;
    }
}

enum TOKENIZER_STATE {
    TOKENIZER_STATE_START_TOKEN,
    TOKENIZER_STATE_END_TOKEN,
    TOKENIZER_STATE_STRING_TOKEN,
    TOKENIZER_STATE_SINGLE_CHAR_TOKEN
};


char current_token[4096];
int current_token_length = 0;
TOKENIZER_STATE state = TOKENIZER_STATE_START_TOKEN;

void add_char_to_token(char c)
{
    if (current_token_length < sizeof(current_token)) {
        current_token[current_token_length++] = c;
    }
}


struct Tokenizer {
    Tokens *tokens;
    Token *current_token;
};


Tokenizer* tokenize(FILE *file)
{
    Tokens *tokens = create_token_list();
    
    char *line;
    while ((line = read_line(file)) != NULL) {
        int len = strlen(line);
        
        if (len == 0) {
            continue;
        }
        
        int index = 0;
        while (index < len) {
            char current_char = line[index];
            
            switch (state) {
                case TOKENIZER_STATE_START_TOKEN: {
                    
                    // Skip one-line comment
                    if (current_char == '/') {
                        if (current_token[current_token_length - 1] == '/') {
                            // Reset state
                            current_token_length = 0;
                            state = TOKENIZER_STATE_START_TOKEN;
                            
                            goto end_while;
                        }
                    }
                    
                    // Skip blank
                    if (current_char == ' ' || current_char == '\t') {
                        ++index;
                        state = TOKENIZER_STATE_END_TOKEN;
                        break;
                    }
                    
                    // Single char tokens
                    if (current_char == '{'
                        || current_char == '}'
                        || current_char == '('
                        || current_char == ')'
                        || current_char == '['
                        || current_char == ']'
                        || current_char == '.'
                        || current_char == ','
                        || current_char == ';'
                        || current_char == '+'
                        || current_char == '-'
                        || current_char == '*'
                        //|| current_char == '/'
                        || current_char == '&'
                        || current_char == '|'
                        || current_char == '<'
                        || current_char == '>'
                        || current_char == '='
                        || current_char == '~') {
                        
                        state = TOKENIZER_STATE_SINGLE_CHAR_TOKEN;
                        break;
                    }
                    
                    if (current_char == '"') {
                        ++index;
                        state = TOKENIZER_STATE_STRING_TOKEN;
                        break;
                    }
                    
                    ++index;
                    add_char_to_token(current_char);
                    
                } break;
                
                case TOKENIZER_STATE_END_TOKEN: {
                    if (current_token_length > 0) {
                        insert_token(tokens, current_token, current_token_length, line_number, TOKEN_TYPE_UNKNOWN);
                    }
                    
                    current_token_length = 0;
                    state = TOKENIZER_STATE_START_TOKEN;
                } break;
                
                case TOKENIZER_STATE_SINGLE_CHAR_TOKEN: {
                    // Process current accumulated token without the current char
                    if (current_token_length > 0) {
                        insert_token(tokens, current_token, current_token_length, line_number, TOKEN_TYPE_UNKNOWN);
                    }
                    
                    // Add current char
                    current_token_length = 0;
                    
                    ++index;
                    if (current_char == '<') {
                        insert_token(tokens, "&lt;", 4, line_number, TOKEN_TYPE_UNKNOWN);
                    } else if (current_char == '>') {
                        insert_token(tokens, "&gt;", 4, line_number, TOKEN_TYPE_UNKNOWN);
                    } else {
                        add_char_to_token(current_char);
                        insert_token(tokens, current_token, current_token_length, line_number, TOKEN_TYPE_UNKNOWN);
                    }
                    
                    
                    // Reset state
                    current_token_length = 0;
                    state = TOKENIZER_STATE_START_TOKEN;
                } break;
                
                case TOKENIZER_STATE_STRING_TOKEN: {
                    ++index;
                    
                    // End string
                    if (current_char == '"') {
                        insert_token(tokens, current_token, current_token_length, line_number, TOKEN_TYPE_STRING_CONSTANT);
                        
                        current_token_length = 0;
                        state = TOKENIZER_STATE_START_TOKEN;
                        
                        break;
                    }
                    
                    add_char_to_token(current_char);
                } break;
            }
            
        } end_while:
        
        free(line);
    }
    
    Tokenizer *tokenizer = (Tokenizer *) malloc(sizeof(Tokenizer));
    tokenizer->tokens = tokens;
    tokenizer->current_token = tokens->head;
    
    return tokenizer;
}

void free_tokenizer(Tokenizer *tokenizer)
{
    Token *token = tokenizer->tokens->head;
    while (token != NULL)
    {
        Token *next_token = token->next;
        free(token->value);
        free(token);

        token = next_token;
    }
    
    free(tokenizer);
}

Token* next_token(Tokenizer *tokenizer)
{
    if (tokenizer->current_token != NULL) {
        tokenizer->current_token = tokenizer->current_token->next;
    }
    
    return tokenizer->current_token;
}





struct cmp_str
{
    bool operator()(char const *a, char const *b) const
    {
        return std::strcmp(a, b) < 0;
    }
};

enum SYMBOL_ENTRY_CATEGORY {
    SYMBOL_ENTRY_CATEGORY_VAR,
    SYMBOL_ENTRY_CATEGORY_ARGUMENT,
    SYMBOL_ENTRY_CATEGORY_STATIC,
    SYMBOL_ENTRY_CATEGORY_FIELD
};

struct Symbol_Entry {
    char *name;
    char *type;
    char *kind;
    int index;
    Symbol_Entry *next;
};

void free_symbol_entry(Symbol_Entry *entry)
{
    while (entry != NULL)
    {
        Symbol_Entry *next_entry = entry->next;
        free(entry->name);
        free(entry->type);
        free(entry->kind);
        free(entry);
        entry = next_entry;
    }
}


global std::map<char *, std::map<char *, Symbol_Entry *, cmp_str>, cmp_str> symbol_table_classes;
global std::map<char *, std::map<char *, Symbol_Entry *, cmp_str>, cmp_str> symbol_table_subroutines;
global std::map<char *, std::map<char *, int, cmp_str>, cmp_str> symbol_table_os_subroutines;

void add_symbol_table_os_subroutines()
{
    symbol_table_os_subroutines["Memory"]["deAlloc"] = 1;
    symbol_table_os_subroutines["Keyboard"]["keyPressed"] = 0;
    symbol_table_os_subroutines["Sys"]["wait"] = 1;
    symbol_table_os_subroutines["Screen"]["setColor"] = 1;
    symbol_table_os_subroutines["Screen"]["drawRectangle"] = 4;
    symbol_table_os_subroutines["Math"]["abs"] = 1;
    symbol_table_os_subroutines["Output"]["printString"] = 1;
}


void add_value_symbol_table_classes(char *class_name_key, Symbol_Entry *value)
{
    int index = 0;
    auto& entries_classes = symbol_table_classes[class_name_key];
    Symbol_Entry *entry = entries_classes[value->kind];
    if (entry == NULL)
    {
        value->index = index;
        symbol_table_classes[class_name_key][value->kind] = value;
    }
    else
    {
        Symbol_Entry *same_kind = NULL;
        while (entry != NULL)
        {
            if (strcmp(entry->kind, value->kind) == 0)
            {
                ++index;
                same_kind = entry;
            }
            
            if (entry->next == NULL) break;
            
            entry = entry->next;
        }
        
        value->index = index;
        if (same_kind == NULL) 
        {
            value->next = entry->next;
            entry->next = value;
        }
        else
        {
            value->next = same_kind->next;
            same_kind->next = value;
        }
    }
}

Symbol_Entry* get_variable_entry_from_class(char *class_name, char *variable_name)
{
    if (symbol_table_classes.find(class_name) != symbol_table_classes.end())
    {
        for (auto& symbol : symbol_table_classes[class_name])
        {
            Symbol_Entry *entry = symbol.second;
            
            while (entry != NULL)
            {
                if (strcmp(entry->name, variable_name) == 0)
                {
                    return entry;
                }
                
                entry = entry->next;
            }
        }
    }
    
    return NULL;
}


void add_value_symbol_table_subroutines(char *classname_subroutinename_key, Symbol_Entry *value)
{
    int index = 0;
    auto& entries_subroutines = symbol_table_subroutines[classname_subroutinename_key];
    Symbol_Entry *entry = entries_subroutines[value->kind];
    if (entry == NULL)
    {
        value->index = index;
        symbol_table_subroutines[classname_subroutinename_key][value->kind] = value;
    }
    else
    {
        Symbol_Entry *same_kind = NULL;
        while (entry != NULL)
        {
            if (strcmp(entry->kind, value->kind) == 0)
            {
                ++index;
                same_kind = entry;
            }
            
            if (entry->next == NULL) break;
            
            entry = entry->next;
        }
        
        value->index = index;
        if (same_kind == NULL)
        {
            value->next = entry->next;
            entry->next = value;
        }
        else
        {
            value->next = same_kind->next;
            same_kind->next = value;
        }
    }
}

Symbol_Entry* get_variable_entry_from_subroutine(char *class_name, char *subroutine_name, char *variable_name)
{
    char key[BUFFER_SIZE];
    sprintf(key, "%s_%s", class_name, subroutine_name);
    if (symbol_table_subroutines.find(key) != symbol_table_subroutines.end())
    {
        for (auto& symbol : symbol_table_subroutines[key])
        {
            Symbol_Entry *entry = symbol.second;
            
            while (entry != NULL)
            {
                if (strcmp(entry->name, variable_name) == 0)
                {
                    return entry;
                }
                
                entry = entry->next;
            }
        }
    }
    
    return NULL;
}



//
// Compiler methods declaration
//
void parse(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_class(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_class_var_dec(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_subroutine(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_parameter_list(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_subroutine_body(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
int compile_var_dec(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_statements(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_let(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_expression(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_term(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_do(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_subroutine_call(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
int  compile_expression_list(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_return(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_if(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);
void compile_while(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm);

inline
void parse(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    Token *token = tokenizer->current_token;
    if (token == NULL || !string_equals(token->value, "class")) {
        printf("Error(%lld): The file should start with a class definition.\n", token->line_number);
        return;
    }
    
    compile_class(tokenizer, output_xml, output_vm);
}

int class_var_dec_count;

inline
void compile_class(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<class>\n");

    class_var_dec_count = 0;
    
    Token *token = next_token(tokenizer);
    if (token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error(%lld): Invalid class name: %s\n", token->line_number, token->value);
        return;
    }
    
    class_name = token->value;
    fprintf(output_xml, "<keyword> class </keyword>\n");
    fprintf(output_xml, "<identifier> %s </identifier>\n", class_name);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "{")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '{' in class declaration\n", token->line_number);
        return;
    }
    
    
    token = next_token(tokenizer);
    while (string_equals(token->value, "static")
           || string_equals(token->value, "field")) {
        
        compile_class_var_dec(tokenizer, output_xml, output_vm);
        
        token = next_token(tokenizer);
    }
    
    
    token = tokenizer->current_token;
    while (string_equals(token->value, "constructor")
           || string_equals(token->value, "function")
           || string_equals(token->value, "method")) {
        
        compile_subroutine(tokenizer, output_xml, output_vm);
        
        token = next_token(tokenizer);
    }
    
    token = tokenizer->current_token;
    if (string_equals(token->value, "}")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '}' after class declaration\n", token->line_number);
        return;
    }
    
    fprintf(output_xml, "</class>\n");
}



inline
void compile_class_var_dec(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<classVarDec>\n");
    
    Token *token = tokenizer->current_token;
    
    char *kind;
    if (strcmp(token->value, "field") == 0)
    {
        kind = "this";
    }
    else
    {
        kind = token->value;
    }
    
    fprintf(output_xml, "<keyword> %s </keyword>\n", kind);
    
    
    token = next_token(tokenizer);
    
    char *type = token->value;
    
    if (token->type == TOKEN_TYPE_KEYWORD || token->type == TOKEN_TYPE_IDENTIFIER) {
        fprintf(output_xml, "<keyword> %s </keyword>\n", type);
    } else {
        fprintf(stderr, "Error(%lld): Specify type for class variable declaration\n", token->line_number);
        return;
    }
    
    
    while (true) {
        token = next_token(tokenizer);
        if (token->type == TOKEN_TYPE_IDENTIFIER) {
            char *name = (char *) malloc(strlen(token->value) + 1);
            strcpy(name, token->value);
            fprintf(output_xml, "<identifier> %s </identifier>\n", name);

            if (strcmp(kind, "this") == 0) ++class_var_dec_count;
            
            Symbol_Entry *entry = (Symbol_Entry *) malloc(sizeof(Symbol_Entry));
            entry->name = name;
            entry->type = (char *) malloc(strlen(type) + 1);
            strcpy(entry->type, type);
            entry->kind = (char *) malloc(strlen(kind) + 1);
            strcpy(entry->kind, kind);
            entry->next = NULL;
            
            char *classname = (char *) malloc(strlen(class_name) + 1);
            strcpy(classname, class_name);
            
            if (generate_file == 0) add_value_symbol_table_classes(classname, entry);
        } else {
            fprintf(stderr, "Error(%lld): Specify name for class variable declaration\n", token->line_number);
            return;
        }
        
        Token *eval_next_token = token->next;
        if (string_equals(eval_next_token->value, ",")) {
            token = next_token(tokenizer);
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        } else {
            break;
        }
    }
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, ";")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing ';' in class variable declaration\n", token->line_number);
        return;
    }
    
    
    fprintf(output_xml, "</classVarDec>\n");
}



inline
void compile_subroutine(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<subroutineDec>\n");
    
    Token *token = tokenizer->current_token;
    function_or_method = token->value;
    fprintf(output_xml, "<keyword> %s </keyword>\n", function_or_method);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "void") 
        || string_equals(token->value, "int")
        || string_equals(token->value, "char")
        || string_equals(token->value, "boolean")
        || token->type == TOKEN_TYPE_IDENTIFIER)
    {
        fprintf(output_xml, "<keyword> %s </keyword>\n", token->value);
    }
    else
    {
        fprintf(stderr, "Error(%lld): Invalid identifier: %s\n", token->line_number, token->value);
        return;
    }
    
    token = next_token(tokenizer);
    subroutine_name = token->value;
    fprintf(output_xml, "<identifier> %s </identifier>\n", subroutine_name);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "(")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '(' in subroutine declaration\n", token->line_number);
        return;
    }
    
    if (strcmp(function_or_method, "method") == 0)
    {
        Symbol_Entry *entry = (Symbol_Entry *) malloc(sizeof(Symbol_Entry));
        entry->name = (char *) malloc(strlen("this") + 1);
        strcpy(entry->name, "this");
        entry->type = (char *) malloc(strlen(class_name) + 1);
        strcpy(entry->type, class_name);
        entry->kind = (char *) malloc(strlen("argument") + 1);
        strcpy(entry->kind, "argument");
        entry->next = NULL;

        char *classname_subroutinename = (char *) malloc(strlen(subroutine_name) + strlen(class_name) + 1);
        sprintf(classname_subroutinename, "%s_%s", class_name, subroutine_name);
        
        if (generate_file == 0) add_value_symbol_table_subroutines(classname_subroutinename, entry);
    }
    
    compile_parameter_list(tokenizer, output_xml, output_vm);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, ")")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing ')' in subroutine declaration\n", token->line_number);
        return;
    }
    
    compile_subroutine_body(tokenizer, output_xml, output_vm);
    
    if (generate_file == 1) fprintf(output_vm, "return\n");
    fprintf(output_xml, "</subroutineDec>\n");
}



inline
void compile_parameter_list(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<parameterList>\n");
    
    Token *token = tokenizer->current_token;
    while (true) {
        // No parameters
        if (string_equals(token->next->value, ")")) {
            break;
        }
        
        token = next_token(tokenizer);
        char *type = (char *) malloc(strlen(token->value) + 1);
        strcpy(type, token->value);
        if (token->type == TOKEN_TYPE_IDENTIFIER) {
            fprintf(output_xml, "<identifier> %s </identifier>\n", type);
        } else if (string_equals(token->value, "int")
                   || string_equals(token->value, "char")
                   || string_equals(token->value, "boolean")) {
            
            fprintf(output_xml, "<keyword> %s </keyword>\n", type);
        } else {
            fprintf(stderr, "Error(%lld): Specify type for parameter declaration\n", token->line_number);
            return;
        }
        
        token = next_token(tokenizer);
        if (token->type == TOKEN_TYPE_IDENTIFIER) {
            char *name = (char *) malloc(strlen(token->value) + 1);
            strcpy(name, token->value);
            fprintf(output_xml, "<identifier> %s </identifier>\n", name);
            
            Symbol_Entry *entry = (Symbol_Entry *) malloc(sizeof(Symbol_Entry));
            entry->name = name;
            entry->type = type;
            entry->kind = (char *) malloc(strlen("argument") + 1);
            strcpy(entry->kind, "argument");
            entry->next = NULL;

            char *classname_subroutinename = (char *) malloc(strlen(subroutine_name) + strlen(class_name) + 1);
            sprintf(classname_subroutinename, "%s_%s", class_name, subroutine_name);
            
            if (generate_file == 0) add_value_symbol_table_subroutines(classname_subroutinename, entry);
        } else {
            fprintf(stderr, "Error(%lld): Specify name for parameter declaration\n", token->line_number);
            free(type);
            return;
        }
        
        Token *eval_next_token = token->next;
        if (string_equals(eval_next_token->value, ",")) {
            token = next_token(tokenizer);
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        } else {
            break;
        }
    }
    
    fprintf(output_xml, "</parameterList>\n");
}

inline
void compile_subroutine_body(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<subroutineBody>\n");
    
    int subroutine_var_count = 0;
    
    Token *token = next_token(tokenizer);
    if (string_equals(token->value, "{")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '{' in subroutine body\n", token->line_number);
        return;
    }
    
    while (true) {
        Token *eval_next_token = tokenizer->current_token->next;
        if (string_equals(eval_next_token->value, "var")) {
            token = next_token(tokenizer);
            subroutine_var_count += compile_var_dec(tokenizer, output_xml, output_vm);
        } else {
            break;
        }
    }
    
    if (generate_file == 1) fprintf(output_vm, "function %s.%s %d\n", class_name, subroutine_name, subroutine_var_count);
    if (strcmp(function_or_method, "method") == 0)
    {
        if (generate_file == 1) fprintf(output_vm, "push argument 0\n");
        if (generate_file == 1) fprintf(output_vm, "pop pointer 0\n");
    }
    else if (strcmp(function_or_method, "constructor") == 0)
    {
        if (generate_file == 1) fprintf(output_vm, "push constant %d\n", class_var_dec_count);
        if (generate_file == 1) fprintf(output_vm, "call Memory.alloc 1\n");
        if (generate_file == 1) fprintf(output_vm, "pop pointer 0\n");
    }
    
    compile_statements(tokenizer, output_xml, output_vm);
    
    token = tokenizer->current_token;
    if (string_equals(token->value, "}")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '}' in subroutine body\n", token->line_number);
        return;
    }
    
    
    fprintf(output_xml, "</subroutineBody>\n");
}

inline
int compile_var_dec(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<varDec>\n");
    
    int var_count = 0;
    
    Token *token = tokenizer->current_token;
    fprintf(output_xml, "<keyword> %s </keyword>\n", token->value);
    
    token = next_token(tokenizer);
    char *type = token->value;
    
    if (token->type == TOKEN_TYPE_IDENTIFIER) {
        fprintf(output_xml, "<identifier> %s </identifier>\n", type);
    } else if (string_equals(token->value, "int")
               || string_equals(token->value, "char")
               || string_equals(token->value, "boolean")) {
        
        fprintf(output_xml, "<keyword> %s </keyword>\n", type);
    } else {
        fprintf(stderr, "Error(%lld): Specify type for variable declaration\n", token->line_number);
        return 0;
    }
    
    
    while (true) {
        token = next_token(tokenizer);
        char *variable_name = (char *) malloc(strlen(token->value) + 1);
        strcpy(variable_name, token->value);
        
        if (token->type == TOKEN_TYPE_IDENTIFIER) {
            fprintf(output_xml, "<identifier> %s </identifier>\n", variable_name);
            
            Symbol_Entry *entry = (Symbol_Entry *) malloc(sizeof(Symbol_Entry));
            entry->name = variable_name;
            entry->type = (char *) malloc(strlen(type) + 1);
            strcpy(entry->type, type);
            entry->kind = (char *) malloc(strlen("local") + 1);
            strcpy(entry->kind, "local");
            entry->next = NULL;

            
            char *classname_subroutinename = (char *) malloc(strlen(subroutine_name) + strlen(class_name) + 1);
            sprintf(classname_subroutinename, "%s_%s", class_name, subroutine_name);
        
            if (generate_file == 0) add_value_symbol_table_subroutines(classname_subroutinename, entry);
            
            ++var_count;
        } else {
            fprintf(stderr, "Error(%lld): Specify name for variable declaration\n", token->line_number);
            free(variable_name);
            return 0;
        }
        
        Token *eval_next_token = token->next;
        if (string_equals(eval_next_token->value, ",")) {
            token = next_token(tokenizer);
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        } else {
            break;
        }
    }
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, ";")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing ';' in variable declaration\n", token->line_number);
        return 0;
    }
    
    
    fprintf(output_xml, "</varDec>\n");
    
    return var_count;
}

//
// Advance the token even if it's not used. After the call, to get the actual token use tokenizer->current_token,
// NOT next_token function.
//
inline
void compile_statements(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<statements>\n");
    
    Token *token = next_token(tokenizer);
    while (string_equals(token->value, "let")
           || string_equals(token->value, "do")
           || string_equals(token->value, "return")
           || string_equals(token->value, "if")
           || string_equals(token->value, "while")) {
        
        if (string_equals(token->value, "let")) {
            compile_let(tokenizer, output_xml, output_vm);
        } else if (string_equals(token->value, "do")) {
            compile_do(tokenizer, output_xml, output_vm);
        } else if (string_equals(token->value, "return")) {
            compile_return(tokenizer, output_xml, output_vm);
        } else if (string_equals(token->value, "if")) {
            compile_if(tokenizer, output_xml, output_vm);
        } else if (string_equals(token->value, "while")) {
            compile_while(tokenizer, output_xml, output_vm);
        } else {
            fprintf(stderr, "Error(%lld): Statement '%s' unrecognized\n", token->line_number, token->value);
            return;
        }
        
        
        token = next_token(tokenizer);
    }
    
    
    fprintf(output_xml, "</statements>\n");
}


inline
void compile_let(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<letStatement>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output_xml, "<keyword> %s </keyword>\n", token->value);
    
    token = next_token(tokenizer);
    if (token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error(%lld): Specify name for variable assignment\n", token->line_number);
        return;
    }
    
    char *variable_name = token->value;
    fprintf(output_xml, "<identifier> %s </identifier>\n", variable_name);


    bool array_variable = false;
    token = next_token(tokenizer);
    if (string_equals(token->value, "[")) {
        array_variable = true;
        
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        
        compile_expression(tokenizer, output_xml, output_vm);
        
        token = next_token(tokenizer);
        if (!string_equals(token->value, "]")) {
            fprintf(stderr, "Error(%lld): Missing ']' in array assignment\n", token->line_number);
            return;
        }
        
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);

        Symbol_Entry *entry = get_variable_entry_from_subroutine(class_name, subroutine_name, variable_name);
        if (entry == NULL)
        {
            entry = get_variable_entry_from_class(class_name, variable_name);
            if (entry == NULL)
            {
                fprintf(stderr, "Error(%lld): Variable %s is not defined\n", token->line_number, variable_name);
                return;
            }
        }

        fprintf(output_vm, "push %s %d\n", entry->kind, entry->index);
        fprintf(output_vm, "add\n");
        
        token = next_token(tokenizer);
    }
    
    if (string_equals(token->value, "=")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '=' in  assignment statement\n", token->line_number);
        return;
    }
    
    
    compile_expression(tokenizer, output_xml, output_vm);

    if (array_variable)
    {
        if (generate_file == 1) fprintf(output_vm, "pop temp 0\n");
        if (generate_file == 1) fprintf(output_vm, "pop pointer 1\n");
        if (generate_file == 1) fprintf(output_vm, "push temp 0\n");
        if (generate_file == 1) fprintf(output_vm, "pop that 0\n");
        
    }
    else
    {
        Symbol_Entry *entry = get_variable_entry_from_subroutine(class_name, subroutine_name, variable_name);
        if (entry == NULL)
        {
            entry = get_variable_entry_from_class(class_name, variable_name);
            if (entry == NULL)
            {
                fprintf(stderr, "Error(%lld): Variable %s is not defined\n", token->line_number, variable_name);
                return;
            }
        }
        
        if (generate_file == 1) fprintf(output_vm, "pop %s %d\n", entry->kind, entry->index);
    }

    
    token = next_token(tokenizer);
    if (!string_equals(token->value, ";")) {
        fprintf(stderr, "Error(%lld): Missing ';' in let\n", token->line_number);
        return;
    }
    
    fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    
    
    fprintf(output_xml, "</letStatement>\n");
}

inline
void do_compile_expression(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    Token *token = next_token(tokenizer);

    compile_term(tokenizer, output_xml, output_vm);
    
    Token *eval_next_token = tokenizer->current_token->next;
    if (   string_equals(eval_next_token->value, "+")
        || string_equals(eval_next_token->value, "-")
        || string_equals(eval_next_token->value, "*")
        || string_equals(eval_next_token->value, "/")
        || string_equals(eval_next_token->value, "&")
        || string_equals(eval_next_token->value, "|")
        || string_equals(eval_next_token->value, "&lt;")
        || string_equals(eval_next_token->value, "&gt;")
        || string_equals(eval_next_token->value, "="))
    {
        token = next_token(tokenizer);
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        
        do_compile_expression(tokenizer, output_xml, output_vm);
        
        if (string_equals(token->value, "+"))
        {
            if (generate_file == 1) fprintf(output_vm, "add\n");
        }
        else if (string_equals(token->value, "-"))
        {
            if (generate_file == 1) fprintf(output_vm, "sub\n");
        }
        else if (string_equals(token->value, "*"))
        {
            if (generate_file == 1) fprintf(output_vm, "call Math.multiply 2\n");
        }
        else if (string_equals(token->value, "/"))
        {
            if (generate_file == 1) fprintf(output_vm, "call Math.divide 2\n");
        }
        else if (string_equals(token->value, "&"))
        {
            if (generate_file == 1) fprintf(output_vm, "and\n");
        }
        else if (string_equals(token->value, "|"))
        {
            if (generate_file == 1) fprintf(output_vm, "or\n");
        }
        else if (string_equals(token->value, "&lt;"))
        {
            if (generate_file == 1) fprintf(output_vm, "lt\n");
        }
        else if (string_equals(token->value, "&gt;"))
        {
            if (generate_file == 1) fprintf(output_vm, "gt\n");
        }
        else if (string_equals(token->value, "="))
        {
            if (generate_file == 1) fprintf(output_vm, "eq\n");
        }
    }
}


inline
void compile_expression(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<expression>\n");
    
    do_compile_expression(tokenizer, output_xml, output_vm);
    
    fprintf(output_xml, "</expression>\n");
}

inline
void compile_term(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<term>\n");
    
    Token *token = tokenizer->current_token;
    if (token->type == TOKEN_TYPE_IDENTIFIER)
    {
        Token *eval_next_token = token->next;
        
        // Array
        if (string_equals(eval_next_token->value, "["))
        {
            char *array_variable_name = token->value;
            fprintf(output_xml, "<identifier> %s </identifier>\n", array_variable_name);
            
            token = next_token(tokenizer);
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);

            
            compile_expression(tokenizer, output_xml, output_vm);

            
            Symbol_Entry *entry = get_variable_entry_from_subroutine(class_name, subroutine_name, array_variable_name);
            if (entry == NULL)
            {
                entry = get_variable_entry_from_class(class_name, array_variable_name);
                if (entry == NULL)
                {
                    fprintf(stderr, "Error(%lld): Variable %s is not defined\n", token->line_number, array_variable_name);
                    return;
                }
            }
            
            if (generate_file == 1) fprintf(output_vm, "push %s %d\n", entry->kind, entry->index);
            
            token = next_token(tokenizer);
            if (string_equals(token->value, "]")) {
                fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
            } else {
                fprintf(stderr, "Error(%lld): Missing ']' in array term\n", token->line_number);
                return;
            }

            
            if (generate_file == 1) fprintf(output_vm, "add\n");
            if (generate_file == 1) fprintf(output_vm, "pop pointer 1\n");
            if (generate_file == 1) fprintf(output_vm, "push that 0\n");
            
        }
        else if (string_equals(eval_next_token->value, "(") || 
                 string_equals(eval_next_token->value, "."))
        {
            compile_subroutine_call(tokenizer, output_xml, output_vm);
        }
        else // Normal variable
        {
            char *variable_name = token->value;
            Symbol_Entry *entry = get_variable_entry_from_subroutine(class_name, subroutine_name, variable_name);
            if (entry == NULL)
            {
                entry = get_variable_entry_from_class(class_name, variable_name);
                if (entry == NULL)
                {
                    fprintf(stderr, "Error(%lld): Variable %s is not defined\n", token->line_number, variable_name);
                    return;
                }
            }
            
            int variable_index = entry->index;
            
            fprintf(output_xml, "<identifier> %s </identifier>\n", variable_name);
            if (generate_file == 1) fprintf(output_vm, "push %s %d\n", entry->kind, entry->index);
        }
    }
    else if (token->type == TOKEN_TYPE_INT_CONSTANT)
    {
        int int_constant = atoi(token->value);
        fprintf(output_xml, "<integerConstant> %d </integerConstant>\n", int_constant);
        if (generate_file == 1) fprintf(output_vm, "push constant %d\n", int_constant);
    }
    else if (token->type == TOKEN_TYPE_STRING_CONSTANT)
    {
        int string_length = (int) strlen(token->value);
        fprintf(output_xml, "<stringConstant> %s </stringConstant>\n", token->value);
        fprintf(output_vm, "push constant %d\n", string_length);
        fprintf(output_vm, "call String.new 1\n");

        for (int i = 0; i < string_length; ++i)
        {
            int char_at_ascii = token->value[i];
            fprintf(output_vm, "push constant %d\n", char_at_ascii);
            fprintf(output_vm, "call String.appendChar 2\n");
        }
    }
    else if (   string_equals(token->value, "true")
             || string_equals(token->value, "false")
             || string_equals(token->value, "null")
             || string_equals(token->value, "this"))
    { // Keyword constants
        
        fprintf(output_xml, "<keyword> %s </keyword>\n", token->value);
        
        if (string_equals(token->value, "true"))
        {
            if (generate_file == 1) fprintf(output_vm, "push constant 0\n");
            if (generate_file == 1) fprintf(output_vm, "not\n");
        }
        else if (string_equals(token->value, "false") || string_equals(token->value, "null"))
        {
            if (generate_file == 1) fprintf(output_vm, "push constant 0\n");
        }
        else if (string_equals(token->value, "this"))
        {
            if (generate_file == 1) fprintf(output_vm, "push pointer 0\n");
        }
    }
    else if (string_equals(token->value, "-")
             || string_equals(token->value, "~"))
    { // Unary operands
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        
        next_token(tokenizer);
        compile_term(tokenizer, output_xml, output_vm);
        
        if (string_equals(token->value, "-"))
        {
            if (generate_file == 1) fprintf(output_vm, "neg\n");
        }
        else if (string_equals(token->value, "~"))
        {
            if (generate_file == 1) fprintf(output_vm, "not\n");
        }
    }
    else if (string_equals(token->value, "("))
    {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        
        compile_expression(tokenizer, output_xml, output_vm);
        
        token = next_token(tokenizer);
        if (string_equals(token->value, ")")) {
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        } else {
            fprintf(stderr, "Error(%lld): Missing ')' in enclosed term\n", token->line_number);
            return;
        }
    }
    
    
    fprintf(output_xml, "</term>\n");
}

inline
void compile_do(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<doStatement>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output_xml, "<keyword> %s </keyword>\n", token->value);
    
    next_token(tokenizer);
    compile_subroutine_call(tokenizer, output_xml, output_vm);
    
    if (generate_file == 1) fprintf(output_vm, "pop temp 0\n");
    
    
    token = next_token(tokenizer);
    if (!string_equals(token->value, ";")) {
        fprintf(stderr, "Error(%lld): Missing ';' in do\n", token->line_number);
        return;
    }
    
    fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    
    
    fprintf(output_xml, "</doStatement>\n");
}

inline
bool os_method(char *variable_caller, char *subroutine_call_name, FILE *output_vm)
{
    if (symbol_table_os_subroutines.find(variable_caller) != symbol_table_os_subroutines.end())
    {
        auto& subroutine = symbol_table_os_subroutines[variable_caller];
        
        if (subroutine.find(subroutine_call_name) != subroutine.end())
        {
            int count_local_variables = subroutine[subroutine_call_name];
            
            if (generate_file == 1) fprintf(output_vm, "call %s.%s %d\n", variable_caller, subroutine_call_name, count_local_variables);
            
            return true;
        }
        else
        {
            return false;
        }
    }
    
    return false;
}

inline
void compile_subroutine_call(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    Token *token = tokenizer->current_token;
    if (token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error(%lld): Incorrect identifier %s in subroutine call\n", token->line_number, token->value);
        return;
    }
    
    char *subroutine_call_name = token->value;
    fprintf(output_xml, "<identifier> %s </identifier>\n", subroutine_call_name);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "(")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        
        if (generate_file == 1) fprintf(output_vm, "push pointer 0\n");

        int expression_count = compile_expression_list(tokenizer, output_xml, output_vm);

        Symbol_Entry *entry = get_variable_entry_from_subroutine(class_name, subroutine_call_name, "this");
        if (entry != NULL) ++expression_count;

        if (generate_file == 1) fprintf(output_vm, "call %s.%s %d\n", class_name, subroutine_call_name, expression_count);
        
        
        token = tokenizer->current_token;
        if (!string_equals(token->value, ")")) {
            fprintf(stderr, "Error(%lld): Missing ')' after subroutine call\n", token->line_number);
            return;
        }
    } else if (string_equals(token->value, ".")) {
        char *variable_caller = subroutine_call_name;
        
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        
        token = next_token(tokenizer);
        if (token->type != TOKEN_TYPE_IDENTIFIER) {
            fprintf(stderr, "Error(%lld): Incorrect identifier %s in subroutine call\n", token->line_number, token->value);
            return;
        }
        
        subroutine_call_name = token->value;
        fprintf(output_xml, "<identifier> %s </identifier>\n", subroutine_call_name);
        
        token = next_token(tokenizer);
        if (string_equals(token->value, "(")) {
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        } else {
            fprintf(stderr, "Error(%lld): Missing '(' in subroutine call\n", token->line_number);
            return;
        }
        
        
        Symbol_Entry *variable_entry = get_variable_entry_from_subroutine(class_name, subroutine_name, variable_caller);
        if (variable_entry == NULL)
        {
            variable_entry = get_variable_entry_from_class(class_name, variable_caller);
        }

        if (variable_entry != NULL)
        {
            if (generate_file == 1) fprintf(output_vm, "push %s %d\n", variable_entry->kind, variable_entry->index);
        }
        
        int expression_count = compile_expression_list(tokenizer, output_xml, output_vm);
        if (variable_entry != NULL) ++expression_count;


        bool is_os_method = os_method(variable_caller, subroutine_call_name, output_vm);
        if (!is_os_method)
        {
            if (variable_entry == NULL)
            {
                // NOTE: this only apply because the files from the book are error-free.
                // This should work only if all functions are loaded in the symbol table, which, at the moment
                // does not do.
                if (generate_file == 1) fprintf(output_vm, "call %s.%s %d\n", variable_caller, subroutine_call_name, expression_count);
            }
            else
            {
                if (generate_file == 1) fprintf(output_vm, "call %s.%s %d\n", variable_entry->type, subroutine_call_name, expression_count);
            }
        }
        
        token = tokenizer->current_token;
        if (string_equals(token->value, ")")) {
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        } else {
            fprintf(stderr, "Error(%lld): Missing ')' in subroutine call\n", token->line_number);
            return;
        }
        
    } else {
        fprintf(stderr, "Error(%lld): Malformed %s for subroutine call\n", token->line_number, token->value);
        return;
    }
}

//
// Advance the token even if it's not used. After the call, to get the actual token use tokenizer->current_token,
// NOT next_token function.
//
inline
int compile_expression_list(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    int expression_count = 0;
    fprintf(output_xml, "<expressionList>\n");
    
    Token *token = tokenizer->current_token; // Initially, current_token should be '('
    while (true) {
        if (string_equals(token->next->value, ")")) {
            token = next_token(tokenizer);
            break;
        }
        
        compile_expression(tokenizer, output_xml, output_vm);
        
        ++expression_count;
        
        token = next_token(tokenizer);
        if (string_equals(token->value, ",")) {
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        } else {
            break;
        }
        
    }

#if 0
    if (strcmp(function_or_method, "method") == 0)
    {
        ++expression_count;
    }
#endif    
    fprintf(output_xml, "</expressionList>\n");
    
    return expression_count;
}

inline
void compile_return(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    fprintf(output_xml, "<returnStatement>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output_xml, "<keyword> %s </keyword>\n", token->value);
    
    Token *eval_next_token = token->next;
    if (string_equals(eval_next_token->value, ";")) {
        token = next_token(tokenizer);
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        
        if (generate_file == 1) fprintf(output_vm, "push constant 0\n");
    } else {
        compile_expression(tokenizer, output_xml, output_vm);
        
        token = next_token(tokenizer);
        if (string_equals(token->value, ";")) {
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        } else {
            fprintf(stderr, "Error(%lld): Missing ';' in return statement\n", token->line_number);
            return;
        }
    }
    
    fprintf(output_xml, "</returnStatement>\n");
}


int global_if_counter = 0;

inline
void compile_if(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    int if_counter = global_if_counter++;
    fprintf(output_xml, "<ifStatement>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output_xml, "<keyword> %s </keyword>\n", token->value);
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "(")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '(' in if statement\n", token->line_number);
        return;
    }
    
    
    compile_expression(tokenizer, output_xml, output_vm);
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, ")")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing ')' in if statement\n", token->line_number);
        return;
    }
    
    if (generate_file == 1) fprintf(output_vm, "if-goto IF_TRUE%d\n", if_counter);
    if (generate_file == 1) fprintf(output_vm, "goto IF_FALSE%d\n", if_counter);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "{")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '{' in if statement\n", token->line_number);
        return;
    }
    
    if (generate_file == 1) fprintf(output_vm, "label IF_TRUE%d\n", if_counter);
    
    
    compile_statements(tokenizer, output_xml, output_vm);
    
    if (generate_file == 1) fprintf(output_vm, "goto IF_END%d\n", if_counter);
    
    token = tokenizer->current_token;
    if (string_equals(token->value, "}")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '}' in if statement\n", token->line_number);
        return;
    }
    
    if (generate_file == 1) fprintf(output_vm, "label IF_FALSE%d\n", if_counter);
    
    Token *possible_else_token = token->next;
    if (string_equals(possible_else_token->value, "else")) {
        token = next_token(tokenizer);
        fprintf(output_xml, "<keyword> %s </keyword>\n", token->value);
        
        
        token = next_token(tokenizer);
        if (string_equals(token->value, "{")) {
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        } else {
            fprintf(stderr, "Error(%lld): Missing '{' in else statement\n", token->line_number);
            return;
        }
        
        
        compile_statements(tokenizer, output_xml, output_vm);
        
        
        token = tokenizer->current_token;
        if (string_equals(token->value, "}")) {
            fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
        } else {
            fprintf(stderr, "Error(%lld): Missing '}' in else statement\n", token->line_number);
            return;
        }
        
    }
    
    
    fprintf(output_xml, "</ifStatement>\n");
    if (generate_file == 1) fprintf(output_vm, "label IF_END%d\n", if_counter);
}


int global_while_counter = 0;

inline
void compile_while(Tokenizer *tokenizer, FILE *output_xml, FILE *output_vm)
{
    int while_counter = global_while_counter++;
    
    fprintf(output_xml, "<whileStatement>\n");
    if (generate_file == 1) fprintf(output_vm, "label WHILE_EXP%d\n", while_counter);
    
    Token *token = tokenizer->current_token;
    fprintf(output_xml, "<keyword> %s </keyword>\n", token->value);
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "(")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '(' in while statement\n", token->line_number);
        return;
    }
    
    
    compile_expression(tokenizer, output_xml, output_vm);
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, ")")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing ')' in while statement\n", token->line_number);
        return;
    }
    
    if (generate_file == 1) fprintf(output_vm, "not\n");
    if (generate_file == 1) fprintf(output_vm, "if-goto WHILE_END%d\n", while_counter);
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "{")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '{' in while statement\n", token->line_number);
        return;
    }
    
    
    compile_statements(tokenizer, output_xml, output_vm);
    
    
    token = tokenizer->current_token;
    if (string_equals(token->value, "}")) {
        fprintf(output_xml, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '}' in while statement\n", token->line_number);
        return;
    }
    
    if (generate_file == 1) fprintf(output_vm, "goto WHILE_EXP%d\n", while_counter);
    fprintf(output_xml, "</whileStatement>\n");
    if (generate_file == 1) fprintf(output_vm, "label WHILE_END%d\n", while_counter);
}

inline
void process_file(FILE *file, FILE *output_xml, FILE *output_vm)
{
    Tokenizer *tokenizer = tokenize(file);
    if (tokenizer->tokens) {
        parse(tokenizer, output_xml, output_vm);
    }
    
    free_tokenizer(tokenizer);
}


int replace_str(char *i_str, char *i_orig, char *i_rep)
{
    char l_before[2024];
    char l_after[2024];
    char* l_p;
    int l_origLen;
    
    l_origLen = strlen(i_orig);
    while (l_p = strstr(i_str, i_orig)) {
        sprintf(l_before, "%.*s", (int) (l_p - i_str), i_str);
        sprintf(l_after, "%s", l_p + l_origLen);
        sprintf(i_str, "%s%s%s", l_before, i_rep, l_after);
    }
    return(strlen(i_str));
}

int main()
{
    printf("Starting compilation...\n");
    
    int status_code = 0;
    
//    char *foldername = "Seven";
//    char *foldername = "ConvertToBin";
//    char *foldername = "Square";
//    char *foldername = "Average";
//    char *foldername = "Pong";
    char *foldername = "ComplexArrays";
    
    add_symbol_table_os_subroutines();

    generate_file = 0;

    DIR *dir;
    for (int i = 0; i < 2; ++i)
    {
        dir = opendir(foldername);
        if (dir == NULL)
        {
            printf("Could not find folder %s\n", foldername);
            goto free_resources;
        }
        
        struct dirent *folder_entry;
        while ((folder_entry = readdir(dir)) != NULL)
        {
            if (!end_with(folder_entry->d_name, ".jack")) continue;
            
            sprintf(filename, "%s/%s", foldername, folder_entry->d_name);
            sprintf(filename_output, "%s/%s", foldername, folder_entry->d_name);
            sprintf(filename_output_xml, "%s/%s", foldername, folder_entry->d_name);
            
            replace_str(filename_output, ".jack", ".vm");
            replace_str(filename_output_xml, ".jack", ".xml");

            FILE *output_xml = fopen(filename_output_xml, "w");
            if (output_xml)
            {
                FILE *output_vm = fopen(filename_output, "w");
                if (output_vm)
                {
                    FILE *file = fopen(filename, "r");
                    if (file)
                    {
                        process_file(file, output_xml, output_vm);
                        
                        fclose(file);
                    }
                    else
                    {
                        printf("Could not open file %s.\n", filename);
                    }
                    
                    fclose(output_vm);
                }
                else
                {
                    printf("Could not open file %s to write on.\n", filename_output);
                }
                
            }
        }

        free(folder_entry);
        free(dir);

        generate_file = 1;
    }
    
    
    free_resources:
    //if (output) fclose(output);
    //if (dir) free(dir);
    
    
    for (auto& class_table : symbol_table_classes) {
        for (auto& symbol : class_table.second) {
            Symbol_Entry *entry = symbol.second;
            free_symbol_entry(entry);
        }
        
        free(class_table.first);
    }
    
    for (auto& subroutine_table : symbol_table_subroutines) {
        for (auto& subroutine : subroutine_table.second) {
            Symbol_Entry *entry = subroutine.second;
            free_symbol_entry(entry);
        }
        
        free(subroutine_table.first);
    }
    
    
    printf("Done\n");
    
    return status_code;
}
