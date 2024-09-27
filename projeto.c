#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 1024  // Define o tamanho máximo do buffer para leitura

char buffer[MAX_BUFFER_SIZE];  // Buffer para armazenar o conteúdo do arquivo
char *current_char;            // Ponteiro para o buffer atual

typedef enum {
    ERRO,
    IDENTIFICADOR,
    NUMERO,
    PALAVRA_RESERVADA,
    PONTO_VIRGULA,
    ABRE_PAR,
    FECHA_PAR,
    COMENTARIO,
    OP_RELACIONAL,  // Operador relacional (>, <, >=, <=, etc.)
    OP_SOMA,        // Operadores de soma e subtração (+, -)
    OP_MULT,        // Operadores de multiplicação e divisão (*, /)
    VIRGULA,        // Operador de vírgula (,)
    DOIS_PONTOS,    // Operador ":"
    PONTO_FINAL,    // Ponto final "."
    EOS             // Fim do arquivo
} TAtomo;

typedef struct {
    TAtomo atomo;
    int linha;
    float atributo_numero;
    char atributo_ID[16];
} TInfoAtomo;

char *msgAtomo[] = {
    "erro",
    "identificador",
    "numero",
    "palavra_reservada",
    "ponto_virgula",
    "abre_par",
    "fecha_par",
    "comentario",
    "op_relacional",
    "op_soma",
    "op_mult",
    "virgula",
    "dois_pontos",
    "ponto_final",
    "EOS"
};

int contaLinha = 1;
int abreParentesesEsperado = 0;  // Variável para verificar se o próximo token esperado é um `)`

// Declaração das funções para evitar erros de declaração implícita
void expressao_logica();
void expressao_relacional();
void expressao_simples();
void termo();
void fator();

TInfoAtomo obter_atomo();
TInfoAtomo reconhece_id();
TInfoAtomo reconhece_numero();
TInfoAtomo reconhece_comentario();
TInfoAtomo reconhece_simbolo();
TInfoAtomo reconhece_operador_relacional();  // Função para reconhecer operadores relacionais
void reportar_erro_sintatico(const char *esperado, const char *encontrado, int linha);

// Funções do analisador sintático
void programa();
void bloco();
void declaracao_de_variaveis();
void comando_composto();
void comando();
void expressao();
void consome(TAtomo esperado);

// Variável global para armazenar o próximo token (lookahead)
TInfoAtomo lookahead;

// Função principal
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte>\n", argv[0]);
        return 1;
    }

    // Abre o arquivo
    FILE *arquivo = fopen(argv[1], "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo: %s\n", argv[1]);
        return 1;
    }

    // Lê o conteúdo do arquivo para o buffer
    size_t bytesLidos = fread(buffer, 1, MAX_BUFFER_SIZE - 1, arquivo);
    buffer[bytesLidos] = '\0';  // Garante que o buffer seja uma string válida
    current_char = buffer;      // Inicializa o ponteiro para o buffer

    fclose(arquivo);  // Fecha o arquivo após a leitura

    lookahead = obter_atomo();  // Obtém o primeiro token
    programa();                 // Começa a análise sintática a partir do símbolo inicial

    printf("%d linhas analisadas, programa sintaticamente correto\n", contaLinha);
    return 0;
}

// Função para consumir o próximo token e verificar se é o esperado
void consome(TAtomo esperado) {
    // Ignorar comentários ao consumir tokens
    while (lookahead.atomo == COMENTARIO) {
        printf("# %d: comentario\n", lookahead.linha);  // Exibe o comentário
        lookahead = obter_atomo();  // Avança para o próximo token
    }

    if (lookahead.atomo == esperado) {
        // Exibe o token consumido
        if (lookahead.atomo == IDENTIFICADOR) {
            printf("# %d: %s | %s\n", lookahead.linha, msgAtomo[lookahead.atomo], lookahead.atributo_ID);
        } else if (lookahead.atomo == PALAVRA_RESERVADA) {
            printf("# %d: %s\n", lookahead.linha, lookahead.atributo_ID);  // Exibe o nome da palavra reservada
        } else if (lookahead.atomo == NUMERO) {
            printf("# %d: %s | %f\n", lookahead.linha, msgAtomo[lookahead.atomo], lookahead.atributo_numero);
        } else {
            printf("# %d: %s\n", lookahead.linha, msgAtomo[lookahead.atomo]);
        }
        lookahead = obter_atomo();  // Avança para o próximo token
    } else {
        reportar_erro_sintatico(msgAtomo[esperado], msgAtomo[lookahead.atomo], lookahead.linha);
        exit(1);  // Encerra o programa em caso de erro
    }
}

// Implementação das funções da gramática

void programa() {
    consome(PALAVRA_RESERVADA);  // "program"
    consome(IDENTIFICADOR);      // Nome do programa
    consome(PONTO_VIRGULA);      // ;
    bloco();                     // <bloco>
    consome(PONTO_FINAL);        // Consome o ponto final (.)
}

void bloco() {
    declaracao_de_variaveis();
    comando_composto();
}

void declaracao_de_variaveis() {
    while (lookahead.atomo == PALAVRA_RESERVADA && 
          (strcmp(lookahead.atributo_ID, "integer") == 0 || strcmp(lookahead.atributo_ID, "boolean") == 0)) {
        consome(PALAVRA_RESERVADA);  // "integer" ou "boolean"
        consome(IDENTIFICADOR);      // Nome da variável
        while (lookahead.atomo == VIRGULA) {
            consome(VIRGULA);        // Consome vírgula
            consome(IDENTIFICADOR);  // Mais identificadores
        }
        consome(PONTO_VIRGULA);      // ;
    }
}

void comando_composto() {
    consome(PALAVRA_RESERVADA);  // "begin"
    comando();
    while (lookahead.atomo == PONTO_VIRGULA) {
        consome(PONTO_VIRGULA);  // ;
        comando();
    }
    consome(PALAVRA_RESERVADA);  // "end"
}

void comando() {
    if (strcmp(lookahead.atributo_ID, "set") == 0) {
        consome(PALAVRA_RESERVADA);  // "set"
        consome(IDENTIFICADOR);      // Nome da variável
        consome(PALAVRA_RESERVADA);  // "to"
        expressao();
    } else if (strcmp(lookahead.atributo_ID, "if") == 0) {
        consome(PALAVRA_RESERVADA);  // "if"
        expressao();                 // <expressao>
        consome(DOIS_PONTOS);        // ":"
        comando();                   // <comando>
        if (strcmp(lookahead.atributo_ID, "elif") == 0) {
            consome(PALAVRA_RESERVADA);  // "elif"
            comando();                   // <comando>
        }
    } else if (strcmp(lookahead.atributo_ID, "for") == 0) {
        consome(PALAVRA_RESERVADA);  // "for"
        consome(IDENTIFICADOR);      // Identificador
        consome(PALAVRA_RESERVADA);  // "of"
        expressao();                 // <expressao>
        consome(PALAVRA_RESERVADA);  // "to"
        expressao();                 // <expressao>
        consome(DOIS_PONTOS);        // ":"
        comando();                   // <comando>
    } else if (strcmp(lookahead.atributo_ID, "read") == 0) {
        consome(PALAVRA_RESERVADA);  // "read"
        consome(ABRE_PAR);           // (
        consome(IDENTIFICADOR);      // Identificador
        while (lookahead.atomo == VIRGULA) {
            consome(VIRGULA);        // Consome a vírgula
            consome(IDENTIFICADOR);  // Próximo identificador
        }
        consome(FECHA_PAR);          // )
    } else if (strcmp(lookahead.atributo_ID, "write") == 0) {
        consome(PALAVRA_RESERVADA);  // "write"
        consome(ABRE_PAR);           // (
        expressao();                 // <expressao>
        while (lookahead.atomo == VIRGULA) {
            consome(VIRGULA);        // Consome a vírgula
            expressao();             // Mais expressões
        }
        consome(FECHA_PAR);          // )
    } else {
        comando_composto();          // Comando composto (begin ... end)
    }
}

// Funções auxiliares para expressões

void expressao() {
    expressao_logica();  // Analisa a expressão lógica
}

void expressao_logica() {
    expressao_relacional();  // Analisa a expressão relacional
    while (lookahead.atomo == PALAVRA_RESERVADA && strcmp(lookahead.atributo_ID, "or") == 0) {
        consome(PALAVRA_RESERVADA);  // Consome o operador "or"
        expressao_relacional();  // Analisa outra expressão relacional
    }
}

void expressao_relacional() {
    expressao_simples();  // Analisa a expressão simples (aritmética)
    
    // Verifica se existe um operador relacional e consome-o
    if (lookahead.atomo == OP_RELACIONAL || 
       (lookahead.atomo == PALAVRA_RESERVADA && 
       (strcmp(lookahead.atributo_ID, "<") == 0 || strcmp(lookahead.atributo_ID, ">") == 0 ||
        strcmp(lookahead.atributo_ID, "<=") == 0 || strcmp(lookahead.atributo_ID, ">=") == 0 ||
        strcmp(lookahead.atributo_ID, "=") == 0 || strcmp(lookahead.atributo_ID, "/=") == 0))) {
        consome(lookahead.atomo);  // Consome o operador relacional
        expressao_simples();  // Analisa a segunda parte da expressão relacional
    }
}

void expressao_simples() {
    termo();  // Analisa o primeiro termo
    // Verifica se existe um operador aritmético de soma ou subtração
    while (lookahead.atomo == OP_SOMA || 
          (lookahead.atomo == PALAVRA_RESERVADA && 
          (strcmp(lookahead.atributo_ID, "+") == 0 || strcmp(lookahead.atributo_ID, "-") == 0))) {
        consome(lookahead.atomo);  // Consome o operador "+" ou "-"
        termo();  // Analisa o próximo termo
    }
}

void termo() {
    fator();  // Analisa o primeiro fator
    // Verifica se existe um operador aritmético de multiplicação ou divisão
    while (lookahead.atomo == OP_MULT || 
          (lookahead.atomo == PALAVRA_RESERVADA && 
          (strcmp(lookahead.atributo_ID, "*") == 0 || strcmp(lookahead.atributo_ID, "/") == 0))) {
        consome(lookahead.atomo);  // Consome o operador "*" ou "/"
        fator();  // Analisa o próximo fator
    }
}

void fator() {
    if (lookahead.atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);  // Consome o identificador
    } else if (lookahead.atomo == NUMERO) {
        consome(NUMERO);  // Consome o número
    } else if (lookahead.atomo == PALAVRA_RESERVADA && 
              (strcmp(lookahead.atributo_ID, "true") == 0 || strcmp(lookahead.atributo_ID, "false") == 0)) {
        consome(PALAVRA_RESERVADA);  // Consome o valor booleano "true" ou "false"
    } else if (lookahead.atomo == PALAVRA_RESERVADA && strcmp(lookahead.atributo_ID, "not") == 0) {
        consome(PALAVRA_RESERVADA);  // Consome o operador "not"
        fator();  // Analisa o fator seguinte (negação)
    } else if (lookahead.atomo == ABRE_PAR) {
        consome(ABRE_PAR);  // Consome o '('
        expressao();  // Analisa a expressão dentro dos parênteses
        consome(FECHA_PAR);  // Consome o ')'
    } else {
        reportar_erro_sintatico("fator esperado", msgAtomo[lookahead.atomo], lookahead.linha);
    }
}

void reportar_erro_sintatico(const char *esperado, const char *encontrado, int linha) {
    // Corrige a exibição do nome da palavra reservada ou símbolo no erro sintático
    const char* token_encontrado = (lookahead.atomo == PALAVRA_RESERVADA) ? lookahead.atributo_ID : msgAtomo[lookahead.atomo];
    printf("# %d: erro sintatico, esperado [%s] encontrado [%s]\n", linha, esperado, token_encontrado);
}

// Funções do analisador léxico

TInfoAtomo obter_atomo() {
    TInfoAtomo info_atomo;

    // Consumir espaços em branco, quebras de linha e tabulações
    while (*current_char == ' ' || *current_char == '\n' || *current_char == '\t' || *current_char == '\r') {
        if (*current_char == '\n')
            contaLinha++;  // Incrementa o contador de linhas
        current_char++;
    }

    // Verificar se é um comentário antes de prosseguir
    if (*current_char == '#' || (*current_char == '{' && *(current_char + 1) == '-')) {
        return reconhece_comentario();  // Reconhece um comentário
    }

    // Reconhecer operadores relacionais
    if (strchr("><=", *current_char)) {
        return reconhece_operador_relacional();  // Reconhece operadores como >, <, >=, <=, etc.
    }

    // Reconhecer símbolos como ; ( ) etc.
    if (strchr(";(),:", *current_char)) {
        return reconhece_simbolo();  // Reconhece símbolos específicos
    }

    // Reconhecer identificador ou palavra reservada
    if (islower(*current_char)) {
        return reconhece_id();  // Reconhece um identificador ou palavra reservada
    }

    // Reconhecer número binário (começando com "0b")
    if (*current_char == '0' && *(current_char + 1) == 'b') {
        return reconhece_numero();  // Reconhece um número binário
    }

    // Fim do arquivo
    if (*current_char == '\0') {
        info_atomo.atomo = EOS;
        info_atomo.linha = contaLinha;
        return info_atomo;
    }

    // Se nenhum padrão foi reconhecido, é um erro léxico
    info_atomo.atomo = ERRO;
    info_atomo.linha = contaLinha;
    return info_atomo;
}

// Função para reconhecer operadores relacionais
TInfoAtomo reconhece_operador_relacional() {
    TInfoAtomo info_atomo;
    info_atomo.atomo = OP_RELACIONAL;

    if (*current_char == '>' || *current_char == '<' || *current_char == '=') {
        info_atomo.atributo_ID[0] = *current_char;
        current_char++;
        if ((*info_atomo.atributo_ID == '>' || *info_atomo.atributo_ID == '<') && *current_char == '=') {
            strcat(info_atomo.atributo_ID, "=");  // Reconhece operadores >= ou <=
            current_char++;
        }
    }

    info_atomo.atributo_ID[1] = '\0';
    info_atomo.linha = contaLinha;
    return info_atomo;
}

TInfoAtomo reconhece_id() {
    TInfoAtomo info_atomo;
    info_atomo.atomo = ERRO;
    char *iniID = current_char;  // Marca o início do identificador
    int tamanho = 0;

    // Avança até o final do identificador
    while (islower(*current_char) || isdigit(*current_char) || *current_char == '_') {
        current_char++;
        tamanho++;
        if (tamanho > 15) {  // Verifica o tamanho do identificador
            info_atomo.atomo = ERRO;
            return info_atomo;
        }
    }

    strncpy(info_atomo.atributo_ID, iniID, current_char - iniID);  // Copia o identificador
    info_atomo.atributo_ID[current_char - iniID] = '\0';

    // Verificar se é uma palavra reservada
    if (strcmp(info_atomo.atributo_ID, "program") == 0 ||
        strcmp(info_atomo.atributo_ID, "if") == 0 ||
        strcmp(info_atomo.atributo_ID, "true") == 0 ||
        strcmp(info_atomo.atributo_ID, "false") == 0 ||
        strcmp(info_atomo.atributo_ID, "begin") == 0 ||
        strcmp(info_atomo.atributo_ID, "write") == 0 ||
        strcmp(info_atomo.atributo_ID, "end") == 0 ||
        strcmp(info_atomo.atributo_ID, "and") == 0 ||
        strcmp(info_atomo.atributo_ID, "boolean") == 0 ||
        strcmp(info_atomo.atributo_ID, "elif") == 0 ||
        strcmp(info_atomo.atributo_ID, "for") == 0 ||
        strcmp(info_atomo.atributo_ID, "integer") == 0 ||
        strcmp(info_atomo.atributo_ID, "not") == 0 ||
        strcmp(info_atomo.atributo_ID, "of") == 0 ||
        strcmp(info_atomo.atributo_ID, "or") == 0 ||
        strcmp(info_atomo.atributo_ID, "read") == 0 ||
        strcmp(info_atomo.atributo_ID, "set") == 0 ||
        strcmp(info_atomo.atributo_ID, "to") == 0) {
        info_atomo.atomo = PALAVRA_RESERVADA;  // É uma palavra reservada
    } else {
        info_atomo.atomo = IDENTIFICADOR;  // É um identificador
    }

    info_atomo.linha = contaLinha;
    return info_atomo;
}

TInfoAtomo reconhece_numero() {
    TInfoAtomo info_atomo;
    info_atomo.atomo = NUMERO;
    current_char += 2;  // Pula '0b'
    int valor = 0;

    // Avança enquanto houver 0s e 1s
    while (*current_char == '0' || *current_char == '1') {
        valor = (valor << 1) + (*current_char - '0');  // Converte o binário para decimal
        current_char++;
    }

    // Verifica se o próximo caractere é válido
    if (*current_char != ' ' && *current_char != '\n' && *current_char != '\t' && *current_char != '\r' && 
        *current_char != ';' && *current_char != '(' && *current_char != ')' && *current_char != '\0') {
        // Retorna erro léxico se for um caractere inválido
        info_atomo.atomo = ERRO;
        info_atomo.linha = contaLinha;
        return info_atomo;
    }

    info_atomo.atributo_numero = valor;  // Atribui o valor numérico decimal
    info_atomo.linha = contaLinha;
    return info_atomo;
}

TInfoAtomo reconhece_simbolo() {
    TInfoAtomo info_atomo;
    switch (*current_char) {
        case ';':
            info_atomo.atomo = PONTO_VIRGULA;
            break;
        case '(':
            info_atomo.atomo = ABRE_PAR;
            break;
        case ')':
            info_atomo.atomo = FECHA_PAR;
            break;
        case ',':
            info_atomo.atomo = VIRGULA;
            break;
        case ':':
            info_atomo.atomo = DOIS_PONTOS;
            break;
        case '.':  // Novo caso para o ponto final
            info_atomo.atomo = PONTO_FINAL;
            break;
        default:
            info_atomo.atomo = ERRO;
            break;
    }
    current_char++;  // Avança para o próximo caractere
    info_atomo.linha = contaLinha;
    return info_atomo;
}

TInfoAtomo reconhece_comentario() {
    TInfoAtomo info_atomo;
    info_atomo.atomo = COMENTARIO;

    // Comentário de linha iniciado com '#'
    if (*current_char == '#') {
        while (*current_char != '\n' && *current_char != '\0') {
            current_char++;  // Avança até o final da linha
        }
        // Consome a quebra de linha, se existir
        if (*current_char == '\n') {
            contaLinha++;
            current_char++;
        }
    }
    // Comentário de bloco iniciado com '{-' e terminado com '-}'
    else if (*current_char == '{' && *(current_char + 1) == '-') {
        current_char += 2;  // Avança '{-'
        while (!(*current_char == '-' && *(current_char + 1) == '}') && *current_char != '\0') {
            if (*current_char == '\n') contaLinha++;  // Conta as quebras de linha
            current_char++;
        }
        if (*current_char == '-') current_char += 2;  // Avança '-}'
    } else {
        info_atomo.atomo = ERRO;  // Se não for comentário válido, retorna erro
    }

    info_atomo.linha = contaLinha;
    return info_atomo;
}
