#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>

#define BUFFER_SIZE 1024

#define global static

typedef uint64_t u64;

global bool in_multiline_comment = false;

u64 line_number = 0;

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
            
            if (same_line) {
                line[index_start_multiline_comment] = '\0';
            }
            
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

void free_tokens(Tokens *tokens)
{
    if (tokens == NULL) return;
    
    Token *token = tokens->head;
    while (token != NULL) {
        Token *current = token;
        token = token->next;
        free(current->value);
        free(current);
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

Token* next_token(Tokenizer *tokenizer)
{
    if (tokenizer->current_token != NULL) {
        tokenizer->current_token = tokenizer->current_token->next;
    }
    
    return tokenizer->current_token;
}

//
// Compiler methods declaration
//
void parse(Tokenizer *tokenizer, FILE *output);
void compile_class(Tokenizer *tokenizer, FILE *output);
void compile_class_var_dec(Tokenizer *tokenizer, FILE *output);
void compile_subroutine(Tokenizer *tokenizer, FILE *output);
void compile_parameter_list(Tokenizer *tokenizer, FILE *output);
void compile_subroutine_body(Tokenizer *tokenizer, FILE *output);
void compile_var_dec(Tokenizer *tokenizer, FILE *output);
void compile_statements(Tokenizer *tokenizer, FILE *output);
void compile_let(Tokenizer *tokenizer, FILE *output);
void compile_expression(Tokenizer *tokenizer, FILE *output);
void compile_term(Tokenizer *tokenizer, FILE *output);
void compile_do(Tokenizer *tokenizer, FILE *output);
void compile_subroutine_call(Tokenizer *tokenizer, FILE *output);
void compile_expression_list(Tokenizer *tokenizer, FILE *output);
void compile_return(Tokenizer *tokenizer, FILE *output);
void compile_if(Tokenizer *tokenizer, FILE *output);
void compile_while(Tokenizer *tokenizer, FILE *output);

inline
void parse(Tokenizer *tokenizer, FILE *output)
{
    Token *token = tokenizer->current_token;
    if (token == NULL || !string_equals(token->value, "class")) {
        printf("Error(%lld): The file should start with a class definition.\n", token->line_number);
        free_tokens(tokenizer->tokens);
        return;
    }
    
    compile_class(tokenizer, output);
    
    free_tokens(tokenizer->tokens);
}


inline
void compile_class(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<class>\n");
    
    Token *token = next_token(tokenizer);
    if (token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error(%lld): Invalid class name: %s\n", token->line_number, token->value);
        return;
    }
    
    fprintf(output, "<keyword> class </keyword>\n");
    fprintf(output, "<identifier> %s </identifier>\n", token->value);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "{")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '{' in class declaration\n", token->line_number);
        return;
    }
    
    Token *eval_next_token = token->next;
    if (string_equals(eval_next_token->value, "static") || string_equals(eval_next_token->value, "field")) {
        token = next_token(tokenizer);
        compile_class_var_dec(tokenizer, output);
    }
    
    token = next_token(tokenizer);
    while (string_equals(token->value, "constructor")
           || string_equals(token->value, "function")
           || string_equals(token->value, "method")) {
        
        compile_subroutine(tokenizer, output);
        
        token = next_token(tokenizer);
    }
    
    token = tokenizer->current_token;
    if (string_equals(token->value, "}")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '}' after class declaration\n", token->line_number);
        return;
    }
    
    fprintf(output, "</class>\n");
}



inline
void compile_class_var_dec(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<classVarDec>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output, "<keyword> %s </keyword>\n", token->value);
    
    token = next_token(tokenizer);
    if (token->type != TOKEN_TYPE_KEYWORD) {
        fprintf(stderr, "Error(%lld): Specify type for class variable declaration\n", token->line_number);
        return;
    }
    
    fprintf(output, "<keyword> %s </keyword>\n", token->value);
    
    token = next_token(tokenizer);
    if (token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error(%lld): Specify name for class variable declaration\n", token->line_number);
        return;
    }
    
    fprintf(output, "<identifier> %s </identifier>\n", token->value);
    
    token = next_token(tokenizer);
    if (!string_equals(token->value, ";")) {
        fprintf(stderr, "Error(%lld): Missing ';'\n", token->line_number);
        return;
    }
    
    fprintf(output, "<symbol> %s </symbol>\n", token->value);
    
    fprintf(output, "</classVarDec>\n");
}



inline
void compile_subroutine(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<subroutineDec>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output, "<keyword> %s </keyword>\n", token->value);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "void") || token->type == TOKEN_TYPE_IDENTIFIER) {
        fprintf(output, "<keyword> %s </keyword>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Invalid identifier: %s\n", token->line_number, token->value);
        return;
    }
    
    token = next_token(tokenizer);
    fprintf(output, "<identifier> %s </identifier>\n", token->value);
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "(")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '(' in subroutine declaration\n", token->line_number);
        return;
    }
    
    compile_parameter_list(tokenizer, output);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, ")")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing ')' in subroutine declaration\n", token->line_number);
        return;
    }
    
    compile_subroutine_body(tokenizer, output);
    
    fprintf(output, "</subroutineDec>\n");
}



inline
void compile_parameter_list(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<parameterList>\n");
    
    // TODO(julio): implement
    
    fprintf(output, "</parameterList>\n");
}

inline
void compile_subroutine_body(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<subroutineBody>\n");
    
    Token *token = next_token(tokenizer);
    if (string_equals(token->value, "{")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '{' in subroutine body\n", token->line_number);
        return;
    }
    
    while (true) {
        Token *eval_next_token = tokenizer->current_token->next;
        if (string_equals(eval_next_token->value, "var")) {
            token = next_token(tokenizer);
            compile_var_dec(tokenizer, output);
        } else {
            break;
        }
    }
    
    
    compile_statements(tokenizer, output);
    
    token = tokenizer->current_token;
    if (string_equals(token->value, "}")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '}' in subroutine body\n", token->line_number);
        return;
    }
    
    
    fprintf(output, "</subroutineBody>\n");
}

inline
void compile_var_dec(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<varDec>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output, "<keyword> %s </keyword>\n", token->value);
    
    token = next_token(tokenizer);
    if (token->type == TOKEN_TYPE_IDENTIFIER) {
        fprintf(output, "<identifier> %s </identifier>\n", token->value);
    } else if (string_equals(token->value, "int")
               || string_equals(token->value, "char")
               || string_equals(token->value, "boolean")) {
        
        fprintf(output, "<keyword> %s </keyword>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Specify type for variable declaration\n", token->line_number);
        return;
    }
    
    
    while (true) {
        token = next_token(tokenizer);
        if (token->type == TOKEN_TYPE_IDENTIFIER) {
            fprintf(output, "<identifier> %s </identifier>\n", token->value);
        } else {
            fprintf(stderr, "Error(%lld): Specify name for variable declaration\n", token->line_number);
            return;
        }
        
        Token *eval_next_token = token->next;
        if (string_equals(eval_next_token->value, ",")) {
            token = next_token(tokenizer);
            fprintf(output, "<symbol> %s </symbol>\n", token->value);
        } else {
            break;
        }
    }
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, ";")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing ';' in variable declaration\n", token->line_number);
        return;
    }
    
    
    fprintf(output, "</varDec>\n");
}

//
// Advance the token even if it's not used. After the call, to get the actual token use tokenizer->current_token,
// NOT next_token function.
//
inline
void compile_statements(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<statements>\n");
    
    Token *token = next_token(tokenizer);
    while (string_equals(token->value, "let")
           || string_equals(token->value, "do")
           || string_equals(token->value, "return")
           || string_equals(token->value, "if")
           || string_equals(token->value, "while")) {
        
        if (string_equals(token->value, "let")) {
            compile_let(tokenizer, output);
        } else if (string_equals(token->value, "do")) {
            compile_do(tokenizer, output);
        } else if (string_equals(token->value, "return")) {
            compile_return(tokenizer, output);
        } else if (string_equals(token->value, "if")) {
            compile_if(tokenizer, output);
        } else if (string_equals(token->value, "while")) {
            compile_while(tokenizer, output);
        } else {
            fprintf(stderr, "Error(%lld): Statement '%s' unrecognized\n", token->line_number, token->value);
            return;
        }
        
        
        token = next_token(tokenizer);
    }
    
    
    fprintf(output, "</statements>\n");
}


inline
void compile_let(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<letStatement>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output, "<keyword> %s </keyword>\n", token->value);
    
    token = next_token(tokenizer);
    if (token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error(%lld): Specify name for variable assignment\n", token->line_number);
        return;
    }
    
    fprintf(output, "<identifier> %s </identifier>\n", token->value);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "[")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
        
        compile_expression(tokenizer, output);
        
        token = next_token(tokenizer);
        if (!string_equals(token->value, "]")) {
            fprintf(stderr, "Error(%lld): Missing ']' in array assignment\n", token->line_number);
            return;
        }
        
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
        
        token = next_token(tokenizer);
    }
    
    if (string_equals(token->value, "=")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '=' in  assignment statement\n", token->line_number);
        return;
    }
    
    
    compile_expression(tokenizer, output);
    
    
    token = next_token(tokenizer);
    if (!string_equals(token->value, ";")) {
        fprintf(stderr, "Error(%lld): Missing ';' in let\n", token->line_number);
        return;
    }
    
    fprintf(output, "<symbol> %s </symbol>\n", token->value);
    
    
    fprintf(output, "</letStatement>\n");
}

inline
void compile_expression(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<expression>\n");
    
    while (true) {
        Token *token = next_token(tokenizer);
        compile_term(tokenizer, output);
        
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
            fprintf(output, "<symbol> %s </symbol>\n", token->value);
        }
        else
        {
            break;
        }
    }
    
    fprintf(output, "</expression>\n");
}

inline
void compile_term(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<term>\n");
    
    Token *token = tokenizer->current_token;
    if (token->type == TOKEN_TYPE_IDENTIFIER)
    {
        Token *eval_next_token = token->next;
        
        // Array
        if (string_equals(eval_next_token->value, "["))
        {
            fprintf(output, "<identifier> %s </identifier>\n", token->value);
            
            token = next_token(tokenizer);
            fprintf(output, "<symbol> %s </symbol>\n", token->value);
            
            compile_expression(tokenizer, output);
            
            token = next_token(tokenizer);
            if (string_equals(token->value, "]")) {
                fprintf(output, "<symbol> %s </symbol>\n", token->value);
            } else {
                fprintf(stderr, "Error(%lld): Missing ']' in array term\n", token->line_number);
                return;
            }
        } else if (string_equals(eval_next_token->value, "(")
                   || string_equals(eval_next_token->value, ".")) {
            
            compile_subroutine_call(tokenizer, output);
        } else {
            fprintf(output, "<identifier> %s </identifier>\n", token->value);
        }
    }
    else if (token->type == TOKEN_TYPE_INT_CONSTANT)
    {
        fprintf(output, "<integerConstant> %s </integerConstant>\n", token->value);
    }
    else if (token->type == TOKEN_TYPE_STRING_CONSTANT)
    {
        fprintf(output, "<stringConstant> %s </stringConstant>\n", token->value);
    }
    else if (   string_equals(token->value, "true")
             || string_equals(token->value, "false")
             || string_equals(token->value, "null")
             || string_equals(token->value, "this"))
    { // Keyword constants
        
        fprintf(output, "<keyword> %s </keyword>\n", token->value);
    }
    else if (string_equals(token->value, "-")
             || string_equals(token->value, "~"))
    { // Unary operands
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
        
        next_token(tokenizer);
        compile_term(tokenizer, output);
    }
    else if (string_equals(token->value, "("))
    {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
        
        compile_expression(tokenizer, output);
        
        token = next_token(tokenizer);
        if (string_equals(token->value, ")")) {
            fprintf(output, "<symbol> %s </symbol>\n", token->value);
        } else {
            fprintf(stderr, "Error(%lld): Missing ')' in enclosed term\n", token->line_number);
            return;
        }
    }
    
    
    fprintf(output, "</term>\n");
}

inline
void compile_do(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<doStatement>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output, "<keyword> %s </keyword>\n", token->value);
    
    next_token(tokenizer);
    compile_subroutine_call(tokenizer, output);
    
    
    token = next_token(tokenizer);
    if (!string_equals(token->value, ";")) {
        fprintf(stderr, "Error(%lld): Missing ';' in do\n", token->line_number);
        return;
    }
    
    fprintf(output, "<symbol> %s </symbol>\n", token->value);
    
    
    fprintf(output, "</doStatement>\n");
}

inline
void compile_subroutine_call(Tokenizer *tokenizer, FILE *output)
{
    Token *token = tokenizer->current_token;
    if (token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error(%lld): Incorrect identifier %s in subroutine call\n", token->line_number, token->value);
        return;
    }
    
    fprintf(output, "<identifier> %s </identifier>\n", token->value);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "(")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
        
        compile_expression_list(tokenizer, output);
        
        token = tokenizer->current_token;
        if (!string_equals(token->value, ")")) {
            fprintf(stderr, "Error(%lld): Missing ')' after subroutine call\n", token->line_number);
            return;
        }
    } else if (string_equals(token->value, ".")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
        
        token = next_token(tokenizer);
        if (token->type != TOKEN_TYPE_IDENTIFIER) {
            fprintf(stderr, "Error(%lld): Incorrect identifier %s in subroutine call\n", token->line_number, token->value);
            return;
        }
        
        fprintf(output, "<identifier> %s </identifier>\n", token->value);
        
        token = next_token(tokenizer);
        if (!string_equals(token->value, "(")) {
            fprintf(stderr, "Error(%lld): Missing '(' in subroutine call\n", token->line_number);
            return;
        }
        
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
        
        
        compile_expression_list(tokenizer, output);
        
        
        token = tokenizer->current_token;
        if (!string_equals(token->value, ")")) {
            fprintf(stderr, "Error(%lld): Missing ')' in subroutine call\n", token->line_number);
            return;
        }
        
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
        
    } else {
        fprintf(stderr, "Error(%lld): Malformed 'do' instruction %s for subroutine call\n", token->line_number, token->value);
        return;
    }
}

//
// Advance the token even if it's not used. After the call, to get the actual token use tokenizer->current_token,
// NOT next_token function.
//
inline
void compile_expression_list(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<expressionList>\n");
    
    // TODO(julio): only compiles 1 parameter. Make it multiple
    Token *eval_next_token = tokenizer->current_token->next;
    if (!string_equals(eval_next_token->value, ")")) {
        compile_expression(tokenizer, output);
    }
    
    Token *token = next_token(tokenizer);
    
    fprintf(output, "</expressionList>\n");
}

inline
void compile_return(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<returnStatement>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output, "<keyword> %s </keyword>\n", token->value);
    
    token = next_token(tokenizer);
    if (string_equals(token->value, ";")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        compile_expression(tokenizer, output);
    }
    
    fprintf(output, "</returnStatement>\n");
}

inline
void compile_if(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<ifStatement>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output, "<keyword> %s </keyword>\n", token->value);
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "(")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '(' in if statement\n", token->line_number);
        return;
    }
    
    
    compile_expression(tokenizer, output);
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, ")")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing ')' in if statement\n", token->line_number);
        return;
    }
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "{")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '{' in if statement\n", token->line_number);
        return;
    }
    
    
    compile_statements(tokenizer, output);
    
    
    token = tokenizer->current_token;
    if (string_equals(token->value, "}")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '}' in if statement\n", token->line_number);
        return;
    }
    
    
    Token *possible_else_token = token->next;
    if (string_equals(possible_else_token->value, "else")) {
        token = next_token(tokenizer);
        fprintf(output, "<keyword> %s </keyword>\n", token->value);
        
        
        token = next_token(tokenizer);
        if (string_equals(token->value, "{")) {
            fprintf(output, "<symbol> %s </symbol>\n", token->value);
        } else {
            fprintf(stderr, "Error(%lld): Missing '{' in if statement\n", token->line_number);
            return;
        }
        
        
        compile_statements(tokenizer, output);
        
        
        token = tokenizer->current_token;
        if (string_equals(token->value, "}")) {
            fprintf(output, "<symbol> %s </symbol>\n", token->value);
        } else {
            fprintf(stderr, "Error(%lld): Missing '}' in if statement\n", token->line_number);
            return;
        }
        
    }
    
    
    fprintf(output, "</ifStatement>\n");
}

inline
void compile_while(Tokenizer *tokenizer, FILE *output)
{
    fprintf(output, "<whileStatement>\n");
    
    Token *token = tokenizer->current_token;
    fprintf(output, "<keyword> %s </keyword>\n", token->value);
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "(")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '(' in while statement\n", token->line_number);
        return;
    }
    
    
    compile_expression(tokenizer, output);
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, ")")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing ')' in while statement\n", token->line_number);
        return;
    }
    
    
    token = next_token(tokenizer);
    if (string_equals(token->value, "{")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '{' in while statement\n", token->line_number);
        return;
    }
    
    
    compile_statements(tokenizer, output);
    
    
    token = tokenizer->current_token;
    if (string_equals(token->value, "}")) {
        fprintf(output, "<symbol> %s </symbol>\n", token->value);
    } else {
        fprintf(stderr, "Error(%lld): Missing '}' in while statement\n", token->line_number);
        return;
    }
    
    
    fprintf(output, "</whileStatement>\n");
}

inline
void process_file(FILE *file, FILE *output)
{
    Tokenizer *tokenizer = tokenize(file);
    if (tokenizer->tokens) {
        parse(tokenizer, output);
    }
}

int main()
{
    int status_code = 0;
    
    char *output_filename = "ExpressionLessSquare/Parsed.xml";
    //char *output_filename = "Square/Parsed.xml";
    //char *output_filename = "ArrayTest/Parsed.xml";
    FILE *output = fopen(output_filename, "w");
    if (!output) {
        printf("Could not open file %s to write on.", output_filename);
        status_code = 1;
        goto free_resources;
    }
    
    char *filename = "ExpressionLessSquare/Main.jack";
    //char *filename = "Square/Main.jack";
    //char *filename = "ArrayTest/Main.jack";
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file %s.\n", filename);
        status_code = 1;
        goto free_resources;
    }
    
    process_file(file, output);
    
    free_resources:
    if (file) fclose(file);
    if (output) fclose(output);
    
    
    return status_code;
}