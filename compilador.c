#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Definindo tipos e palavras reservadas
#define MAX_ID_LEN 15

// Primeiro definimos o enum TokenType
typedef enum {
    T_PROGRAM, T_IDENTIFIER, T_INTEGER, T_BOOLEAN, T_IF, T_ELIF, T_FOR, T_READ, T_WRITE, T_SET,
    T_TO, T_BEGIN, T_END, T_OR, T_AND, T_NOT, T_TRUE, T_FALSE, T_NUMBER, T_ERROR, T_EOF,
    T_SEMICOLON, T_OPEN_PAREN, T_CLOSE_PAREN, T_DOT
} TokenType;

// Agora podemos definir a estrutura Token, que usa o TokenType
typedef struct {
    TokenType type;
    char lexeme[MAX_ID_LEN + 1]; // Identificadores limitados a 15 caracteres
    int line;
} Token;

// Declaração das funções antes de usá-las
void parseProgram();
void parseBlock();
void parseCommands();
void parseCommand();
void printToken(Token token); // Função para imprimir tokens

// Variáveis globais para armazenar estado
char *sourceCode;
int currentIndex = 0;
int currentLine = 1;

// Funções auxiliares
char nextChar() {
    return sourceCode[currentIndex++];
}

void ungetChar() {
    currentIndex--;
}

Token getToken();
void consume(TokenType expected);

int isReservedWord(const char *word) {
    const char *reservedWords[] = {
        "program", "integer", "boolean", "if", "elif", "for", "read", "write", "set",
        "to", "begin", "end", "or", "and", "not", "true", "false"
    };
    int numWords = sizeof(reservedWords) / sizeof(reservedWords[0]);
    for (int i = 0; i < numWords; i++) {
        if (strcmp(word, reservedWords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Função que obtém o próximo token (Analisador Léxico)
Token getToken() {
    Token token;
    token.line = currentLine;

    char ch = nextChar();
    
    // Ignorando delimitadores (espaços, tabulações, quebras de linha)
    while (isspace(ch)) {
        if (ch == '\n') currentLine++;
        ch = nextChar();
    }

    // Checando se é um identificador ou palavra reservada
    if (isalpha(ch)) {
        int i = 0;
        token.lexeme[i++] = ch;
        while (isalnum((ch = nextChar())) || ch == '_') {
            if (i < MAX_ID_LEN) {
                token.lexeme[i++] = ch;
            } else {
                token.type = T_ERROR;
                return token;
            }
        }
        ungetChar();  // Voltando um caractere após a identificação
        token.lexeme[i] = '\0';
        if (isReservedWord(token.lexeme)) {
            if (strcmp(token.lexeme, "program") == 0) {
                token.type = T_PROGRAM; // Exemplo de palavra reservada
            } else if (strcmp(token.lexeme, "begin") == 0) {
                token.type = T_BEGIN;
            } else if (strcmp(token.lexeme, "end") == 0) {
                token.type = T_END;
            } else if (strcmp(token.lexeme, "write") == 0) {
                token.type = T_WRITE;
            } // Adicione as outras palavras reservadas aqui...
        } else {
            token.type = T_IDENTIFIER;
        }
        return token;
    }

    // Checando números binários
    if (ch == '0' && nextChar() == 'b') {
        int i = 0;
        while ((ch = nextChar()) == '0' || ch == '1') {
            token.lexeme[i++] = ch;
        }
        ungetChar();
        token.lexeme[i] = '\0';
        token.type = T_NUMBER;
        return token;
    }

    // Checando operadores e símbolos especiais
    if (ch == ';') {
        token.type = T_SEMICOLON;
        return token;
    } else if (ch == '(') {
        token.type = T_OPEN_PAREN;
        return token;
    } else if (ch == ')') {
        token.type = T_CLOSE_PAREN;
        return token;
    } else if (ch == '.') {
        token.type = T_DOT;
        return token;
    }

    // Outros casos a serem implementados: operadores, símbolos, comentários, etc.
    token.type = T_EOF; // Fim de arquivo ou caractere inesperado
    return token;
}

const char* tokenToString(TokenType type) {
    switch (type) {
        case T_PROGRAM: return "program";
        case T_IDENTIFIER: return "identificador";
        case T_INTEGER: return "integer";
        case T_BOOLEAN: return "boolean";
        case T_IF: return "if";
        case T_ELIF: return "elif";
        case T_FOR: return "for";
        case T_READ: return "read";
        case T_WRITE: return "write";
        case T_SET: return "set";
        case T_TO: return "to";
        case T_BEGIN: return "begin";
        case T_END: return "end";
        case T_OR: return "or";
        case T_AND: return "and";
        case T_NOT: return "not";
        case T_TRUE: return "true";
        case T_FALSE: return "false";
        case T_NUMBER: return "numero";
        case T_ERROR: return "erro";
        case T_EOF: return "fim_do_arquivo";
        default:
            if (type == T_SEMICOLON) return "ponto_virgula";
            if (type == T_OPEN_PAREN) return "abre_par";
            if (type == T_CLOSE_PAREN) return "fecha_par";
            if (type == T_DOT) return "ponto";
            return "desconhecido";
    }
}

// Função para imprimir tokens
void printToken(Token token) {
    printf("# %d:%s | %s\n", token.line, tokenToString(token.type), token.lexeme);
}

// Função para consumir tokens (Analisador Sintático)
void consume(TokenType expected) {
    Token token = getToken();
    printToken(token); // Imprimir o token para depuração
    if (token.type != expected) {
        printf("Erro sintático na linha %d: esperado [%s] encontrado [%s]\n",
               token.line, tokenToString(expected), tokenToString(token.type));
        exit(1);
    }
}

void parseProgram() {
    consume(T_PROGRAM); // Verifica se o primeiro token é "program"
    consume(T_IDENTIFIER); // Verifica se há um identificador após "program"
    consume(T_SEMICOLON); // Verifica se há um ponto e vírgula após o identificador

    parseBlock(); // Procedimento para analisar o bloco de código principal
    consume(T_DOT); // Verifica se o programa termina com um ponto final
}

void parseBlock() {
    consume(T_BEGIN); // Espera o "begin"
    
    // Loop para permitir múltiplos comandos
    while (1) {
        Token token = getToken();
        printToken(token); // Imprimir o token para depuração
        if (token.type == T_END) {
            break; // Sai do loop quando encontrar o "end"
        }
        ungetChar(); // Recoloca o token para ser processado corretamente
        parseCommand(); // Processa um comando dentro do bloco
    }
}

void parseCommand() {
    Token token = getToken();
    printToken(token); // Imprimir o token para depuração
    
    // Exemplo de comando simples, como write
    if (token.type == T_WRITE) {
        consume(T_OPEN_PAREN);
        consume(T_IDENTIFIER); // Espera um identificador dentro de write
        consume(T_CLOSE_PAREN);
        consume(T_SEMICOLON);
    }
    // Adicione mais comandos aqui, como if, for, etc.
}


// Função principal
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s exemplo1\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Erro ao abrir o arquivo %s\n", argv[1]);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    sourceCode = malloc(fileSize + 1);
    fread(sourceCode, 1, fileSize, file);
    sourceCode[fileSize] = '\0';

    fclose(file);

    parseProgram(); // Inicia a análise sintática

    free(sourceCode);
    return 0;
} 
