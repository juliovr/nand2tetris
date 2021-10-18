#include <stdio.h>
#include <stdlib.h>
#include <string>

#define BUFFER_SIZE 1024

#define global static

global bool in_multiline_comment = false;

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

bool string_equals(const char *s1, const char *s2)
{
    bool result = strcmp(s1, s2) == 0;
    return result;
}

bool start_with(const char *str, const char *pre)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

bool end_with(const char *str, const char *suffix)
{
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (str_len < suffix_len) return false;
    
    return memcmp(str + (str_len - suffix_len), suffix, suffix_len) == 0;
}

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
void insert_token(Tokens *tokens, char value[], int len, Token_Type token_type)
{
    for (int i = 0; i < len; ++i) {
        char current_char = value[i];
        if (current_char == '\n' || current_char == EOF) break;
    }
    
    Token *new_token = (Token *) malloc(sizeof(Token));
    new_token->value = (char *) malloc(len + 1);
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

Tokens* tokenize(char *original_line)
{
    int len = strlen(original_line);
    
    if (len == 0) {
        Tokens *empty = {};
        return empty;
    }
    
    char *line = (char *) malloc(len + 1);
    strcpy(line, original_line);
    
    Tokens *token_list = create_token_list();
    
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
                    insert_token(token_list, current_token, current_token_length, TOKEN_TYPE_UNKNOWN);
                }
                
                current_token_length = 0;
                state = TOKENIZER_STATE_START_TOKEN;
            } break;
            
            case TOKENIZER_STATE_SINGLE_CHAR_TOKEN: {
                // Process current accumulated token without the current char
                if (current_token_length > 0) {
                    insert_token(token_list, current_token, current_token_length, TOKEN_TYPE_UNKNOWN);
                }
                
                // Add current char
                current_token_length = 0;
                
                ++index;
                if (current_char == '<') {
                    insert_token(token_list, "&lt;", 4, TOKEN_TYPE_UNKNOWN);
                } else if (current_char == '>') {
                    insert_token(token_list, "&gt;", 4, TOKEN_TYPE_UNKNOWN);
                } else {
                    add_char_to_token(current_char);
                    insert_token(token_list, current_token, current_token_length, TOKEN_TYPE_UNKNOWN);
                }
                
                
                // Reset state
                current_token_length = 0;
                state = TOKENIZER_STATE_START_TOKEN;
            } break;
            
            case TOKENIZER_STATE_STRING_TOKEN: {
                ++index;
                
                // End string
                if (current_char == '"') {
                    insert_token(token_list, current_token, current_token_length, TOKEN_TYPE_STRING_CONSTANT);
                    
                    current_token_length = 0;
                    state = TOKENIZER_STATE_START_TOKEN;
                    
                    break;
                }
                
                add_char_to_token(current_char);
            } break;
        }
        
    } end_while:
    
    free(line);
    
    return token_list;
}

void process_file(FILE *file, FILE *output)
{
    fprintf(output, "<tokens>\n");
    
    char *line;
    while ((line = read_line(file)) != NULL) {
        Tokens *tokens = tokenize(line);
        if (tokens) {
            Token *token = tokens->head;
            while (token) {
                
                switch (token->type) {
                    case TOKEN_TYPE_KEYWORD:
                    fprintf(output, "<keyword> %s </keyword>\n", token->value);
                    break;
                    case TOKEN_TYPE_SYMBOL:
                    fprintf(output, "<symbol> %s </symbol>\n", token->value);
                    break;
                    case TOKEN_TYPE_INT_CONSTANT:
                    fprintf(output, "<integerConstant> %s </integerConstant>\n", token->value);
                    break;
                    case TOKEN_TYPE_STRING_CONSTANT:
                    fprintf(output, "<stringConstant> %s </stringConstant>\n", token->value);
                    break;
                    case TOKEN_TYPE_IDENTIFIER:
                    fprintf(output, "<identifier> %s </identifier>\n", token->value);
                    break;
                }
                
                token = token->next;
            }
            
            free_tokens(tokens);
            free(line);
        }
    }
    
    fprintf(output, "</tokens>\n");
}

int main()
{
    int status_code = 0;
    
    char *output_filename = "ExpressionLessSquare/Tokens.xml";
    FILE *output = fopen(output_filename, "w");
    if (!output) {
        printf("Could not open file %s to write on.", output_filename);
        status_code = 1;
        goto free_resources;
    }
    
    char *filename = "ExpressionLessSquare/Main.jack";
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