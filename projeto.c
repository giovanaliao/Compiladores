//Giovana Liao
//RA: 10402264
//Maria Julia de Pádua
//RA: 10400630

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 1024  // Define o tamanho máximo do buffer para leitura

// Buffer para armazenar o conteúdo do arquivo de entrada
char buffer[MAX_BUFFER_SIZE];
// Ponteiro que percorre o conteúdo do buffer, processando o arquivo caractere por caractere
char *char_atual;

// Enumeração que define os diferentes tipos de tokens que podem ser encontrados no código-fonte
typedef enum {
    ERRO,               // Token inválido ou erro léxico
    IDENTIFICADOR,      // Identificadores (nomes de variáveis, funções, etc.)
    NUMERO,             // Números (constantes numéricas)
    PALAVRA_RESERVADA,  // Palavras reservadas (ex: program, begin, end)
    PONTO_VIRGULA,      // ";"
    ABRE_PAR,           // "("
    FECHA_PAR,          // ")"
    COMENTARIO,         // Comentários (ex: # ou {- -})
    OP_RELACIONAL,      // Operadores relacionais (>, <, >=, <=)
    OP_SOMA,            // Operadores de soma e subtração (+, -)
    OP_MULT,            // Operadores de multiplicação e divisão (*, /)
    VIRGULA,            // ","
    DOIS_PONTOS,        // ":"
    PONTO_FINAL,        // "."
    EOS                 // Fim do arquivo
} TAtomo;

// Estrutura para armazenar informações sobre o token lido, como seu tipo, a linha em que foi encontrado, etc.
typedef struct {
    TAtomo atomo;            // Tipo do token (um dos valores da enum TAtomo)
    int linha;               // Número da linha onde o token foi encontrado
    float atributo_numero;   // Se o token for um número, o valor é armazenado aqui
    char atributo_ID[16];    // Se for um identificador ou palavra reservada, o nome é armazenado aqui
} TInfoAtomo;

// Array de strings que mapeia os tipos de tokens para suas representações textuais
char *msgAtomo[] = {
    "erro", "identificador", "numero", "palavra_reservada", "ponto_virgula",
    "abre_par", "fecha_par", "comentario", "op_relacional", "op_soma",
    "op_mult", "virgula", "dois_pontos", "ponto_final", "EOS"
};

int contaLinha = 1;  // Contador de linhas para rastrear a linha atual do código sendo processada
int abreParentesesEsperado = 0;  // Variável para checar se um parêntese de fechamento é esperado

// Declaração de funções para evitar erros de declaração implícita
void expressao_logica();
void expressao_relacional();
void expressao_simples();
void termo();
void fator();

TInfoAtomo obter_atomo();  // Função que lê e retorna o próximo token do código-fonte
TInfoAtomo reconhece_id();  // Função que reconhece identificadores e palavras reservadas
TInfoAtomo reconhece_numero();  // Função que reconhece números binários
TInfoAtomo reconhece_comentario();  // Função que reconhece e ignora comentários
TInfoAtomo reconhece_simbolo();  // Função que reconhece símbolos como ;, (, ), :, etc.
TInfoAtomo reconhece_operador_relacional();  // Função que reconhece operadores relacionais (>, <, =, etc.)
void reportar_erro_sintatico(const char *esperado, const char *encontrado, int linha);  // Função que exibe erros sintáticos

// Funções do analisador sintático
void programa();  // Analisa o programa como um todo
void bloco();  // Analisa um bloco de código (declaracao de variáveis e comandos)
void declaracao_de_variaveis();  // Analisa a declaração de variáveis
void comando_composto();  // Analisa um comando composto (begin...end)
void comando();  // Analisa um comando individual
void expressao();  // Analisa uma expressão
void consome(TAtomo esperado);  // Consome o próximo token, verificando se é o esperado

// Variável global que armazena o próximo token a ser processado
TInfoAtomo lookahead;

// Função principal do programa
int main(int argc, char *argv[]) {
    // Verifica se o arquivo fonte foi passado como argumento
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte>\n", argv[0]);
        return 1;
    }

    // Abre o arquivo passado como argumento para leitura
    FILE *arquivo = fopen(argv[1], "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo: %s\n", argv[1]);
        return 1;
    }

    // Lê o conteúdo do arquivo para o buffer
    size_t bytesLidos = fread(buffer, 1, MAX_BUFFER_SIZE - 1, arquivo);
    buffer[bytesLidos] = '\0';  // Garante que o buffer seja uma string válida
    char_atual = buffer;      // Inicializa o ponteiro para o início do buffer

    fclose(arquivo);  // Fecha o arquivo após a leitura

    // Inicia o processo de análise sintática
    lookahead = obter_atomo();  // Obtém o primeiro token
    programa();                 // Começa a análise sintática a partir do símbolo inicial "program"

    printf("%d linhas analisadas, programa sintaticamente correto\n", contaLinha);
    return 0;
}

// Função para consumir o próximo token e verificar se é o esperado
void consome(TAtomo esperado) {
    // Ignorar comentários enquanto consome tokens
    while (lookahead.atomo == COMENTARIO) {
        printf("# %d: comentario\n", lookahead.linha);  // Exibe o comentário
        lookahead = obter_atomo();  // Avança para o próximo token
    }

    // Verifica se o token atual corresponde ao esperado
    if (lookahead.atomo == esperado) {
        // Exibe o token consumido dependendo do seu tipo
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
        // Se o token não for o esperado, exibe um erro
        reportar_erro_sintatico(msgAtomo[esperado], msgAtomo[lookahead.atomo], lookahead.linha);
        exit(1);  // Encerra o programa em caso de erro
    }
}

// Função que analisa a estrutura do programa (gramática do programa)
void programa() {
    consome(PALAVRA_RESERVADA);  // Espera pela palavra reservada "program"
    consome(IDENTIFICADOR);      // Espera por um identificador (nome do programa)
    consome(PONTO_VIRGULA);      // Espera por ";"
    bloco();                     // Analisa o bloco de código principal
}

// Função que analisa um bloco de código que contém declarações de variáveis e comandos
void bloco() {
    declaracao_de_variaveis();  // Processa declarações de variáveis (se houver)
    comando_composto();         // Processa os comandos dentro de begin...end
}

// Função que analisa a declaração de variáveis do programa
void declaracao_de_variaveis() {
    // Verifica se existem declarações de variáveis (ex: integer, boolean)
    while (lookahead.atomo == PALAVRA_RESERVADA && 
          (strcmp(lookahead.atributo_ID, "integer") == 0 || strcmp(lookahead.atributo_ID, "boolean") == 0)) {
        consome(PALAVRA_RESERVADA);  // Consome a palavra reservada (integer ou boolean)
        consome(IDENTIFICADOR);      // Consome o nome da variável
        while (lookahead.atomo == VIRGULA) {
            consome(VIRGULA);        // Consome a vírgula (caso haja mais variáveis na mesma linha)
            consome(IDENTIFICADOR);  // Consome o próximo identificador
        }
        consome(PONTO_VIRGULA);      // Espera por ";" após a declaração
    }
}

// Função que analisa um bloco de comandos composto (begin...end)
void comando_composto() {
    consome(PALAVRA_RESERVADA);  // Espera pela palavra reservada "begin"
    comando();  // Analisa o primeiro comando
    while (lookahead.atomo == PONTO_VIRGULA) {
        consome(PONTO_VIRGULA);  // Consome o ponto e vírgula após o comando
        comando();               // Analisa o próximo comando
    }
    consome(PALAVRA_RESERVADA);  // Espera pela palavra reservada "end"

    // Verifica se o próximo token após "end" é o ponto final "."
    if (lookahead.atomo == PONTO_FINAL) {
        consome(PONTO_FINAL);  // Consome o ponto final
    } else {
        // Se o ponto final não for encontrado, reporta erro
        reportar_erro_sintatico("ponto_final", msgAtomo[lookahead.atomo], lookahead.linha);
        exit(1);  // Encerra o programa em caso de erro
    }
}

// Função que analisa um comando
void comando() {
    if (strcmp(lookahead.atributo_ID, "set") == 0) {
        consome(PALAVRA_RESERVADA);  // Consome a palavra "set"
        consome(IDENTIFICADOR);      // Consome o nome da variável
        consome(PALAVRA_RESERVADA);  // Consome a palavra "to"
        expressao();                 // Analisa a expressão atribuída à variável
    } else if (strcmp(lookahead.atributo_ID, "if") == 0) {
        consome(PALAVRA_RESERVADA);  // Consome "if"
        expressao();                 // Analisa a expressão lógica
        consome(DOIS_PONTOS);        // Espera por ":"
        comando();                   // Analisa o comando dentro do if
        if (strcmp(lookahead.atributo_ID, "elif") == 0) {
            consome(PALAVRA_RESERVADA);  // Consome "elif"
            comando();                   // Analisa o comando do elif
        }
    } else if (strcmp(lookahead.atributo_ID, "for") == 0) {
        consome(PALAVRA_RESERVADA);  // Consome "for"
        consome(IDENTIFICADOR);      // Consome o identificador
        consome(PALAVRA_RESERVADA);  // Consome "of"
        expressao();                 // Analisa a expressão após "of"
        consome(PALAVRA_RESERVADA);  // Consome "to"
        expressao();                 // Analisa a expressão após "to"
        consome(DOIS_PONTOS);        // Consome ":"
        comando();                   // Analisa o comando do loop
    } else if (strcmp(lookahead.atributo_ID, "read") == 0) {
        consome(PALAVRA_RESERVADA);  // Consome "read"
        consome(ABRE_PAR);           // Consome "("
        consome(IDENTIFICADOR);      // Consome o identificador
        while (lookahead.atomo == VIRGULA) {
            consome(VIRGULA);        // Consome a vírgula
            consome(IDENTIFICADOR);  // Consome o próximo identificador
        }
        consome(FECHA_PAR);          // Consome ")"
    } else if (strcmp(lookahead.atributo_ID, "write") == 0) {
        consome(PALAVRA_RESERVADA);  // Consome "write"
        consome(ABRE_PAR);           // Consome "("
        expressao();                 // Analisa a expressão a ser escrita
        while (lookahead.atomo == VIRGULA) {
            consome(VIRGULA);        // Consome a vírgula
            expressao();             // Analisa mais expressões
        }
        consome(FECHA_PAR);          // Consome ")"
    } else {
        comando_composto();          // Analisa comandos compostos (begin...end)
    }
}

// Funções auxiliares para expressões
// Função que analisa uma expressão lógica
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
    expressao_simples();  // Analisa a expressão aritmética simples
    
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

// Função que analisa uma expressão aritmética simples
void expressao_simples() {
    termo();  // Analisa o primeiro termo
    // Verifica se há operadores aritméticos (+, -) e consome-os
    while (lookahead.atomo == OP_SOMA || 
          (lookahead.atomo == PALAVRA_RESERVADA && 
          (strcmp(lookahead.atributo_ID, "+") == 0 || strcmp(lookahead.atributo_ID, "-") == 0))) {
        consome(lookahead.atomo);  // Consome o operador "+" ou "-"
        termo();  // Analisa o próximo termo
    }
}

// Função que analisa um termo (parte de uma expressão)
void termo() {
    fator();  // Analisa o fator
    // Verifica se há operadores aritméticos (*, /) e consome-os
    while (lookahead.atomo == OP_MULT || 
          (lookahead.atomo == PALAVRA_RESERVADA && 
          (strcmp(lookahead.atributo_ID, "*") == 0 || strcmp(lookahead.atributo_ID, "/") == 0))) {
        consome(lookahead.atomo);  // Consome o operador "*" ou "/"
        fator();  // Analisa o próximo fator
    }
}

// Função que analisa fatores (como identificadores, números ou expressões entre parênteses)
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
        consome(ABRE_PAR);  // Consome "("
        expressao();  // Analisa a expressão dentro dos parênteses
        consome(FECHA_PAR);  // Consome ")"
    } else {
        // Reporta erro se o fator esperado não for encontrado
        reportar_erro_sintatico("fator esperado", msgAtomo[lookahead.atomo], lookahead.linha);
    }
}

// Função que reporta erros sintáticos quando o token encontrado não é o esperado
void reportar_erro_sintatico(const char *esperado, const char *encontrado, int linha) {
    const char* token_encontrado = (lookahead.atomo == PALAVRA_RESERVADA) ? lookahead.atributo_ID : msgAtomo[lookahead.atomo];
    printf("# %d: erro sintatico, esperado [%s] encontrado [%s]\n", linha, esperado, token_encontrado);
}

// Funções do analisador léxico (responsáveis por reconhecer os tokens no código fonte)

// Função que lê o próximo token do código-fonte
TInfoAtomo obter_atomo() {
    TInfoAtomo info_atomo;

    // Consome espaços em branco, quebras de linha e tabulações
    while (*char_atual == ' ' || *char_atual == '\n' || *char_atual == '\t' || *char_atual == '\r') {
        if (*char_atual == '\n')
            contaLinha++;  // Incrementa o contador de linhas quando encontra uma nova linha
        char_atual++;
    }

    // Verifica se o caractere atual é o ponto final "."
    if (*char_atual == '.') {
        info_atomo.atomo = PONTO_FINAL;
        info_atomo.linha = contaLinha;
        char_atual++;  // Avança para o próximo caractere
        return info_atomo;
    }

    // Verifica se o token atual é um comentário
    if (*char_atual == '#' || (*char_atual == '{' && *(char_atual + 1) == '-')) {
        return reconhece_comentario();  // Processa o comentário
    }

    // Verifica se o token atual é um operador relacional
    if (strchr("><=", *char_atual)) {
        return reconhece_operador_relacional();  // Processa operadores relacionais
    }

    // Verifica se o token atual é um símbolo especial (;, (), :, etc.)
    if (strchr(";(),:", *char_atual)) {
        return reconhece_simbolo();  // Processa símbolos
    }

    // Verifica se o token atual é um identificador ou palavra reservada
    if (islower(*char_atual)) {
        return reconhece_id();  // Processa identificadores ou palavras reservadas
    }

    // Verifica se o token atual é um número binário (ex: 0b101)
    if (*char_atual == '0' && *(char_atual + 1) == 'b') {
        return reconhece_numero();  // Processa números binários
    }

    // Verifica se chegamos ao fim do arquivo
    if (*char_atual == '\0') {
        info_atomo.atomo = EOS;
        info_atomo.linha = contaLinha;
        return info_atomo;
    }

    // Se nenhum padrão foi reconhecido, retorna um erro léxico
    info_atomo.atomo = ERRO;
    info_atomo.linha = contaLinha;
    return info_atomo;
}

// Função que reconhece operadores relacionais (>, <, >=, <=)
TInfoAtomo reconhece_operador_relacional() {
    TInfoAtomo info_atomo;
    info_atomo.atomo = OP_RELACIONAL;

    if (*char_atual == '>' || *char_atual == '<' || *char_atual == '=') {
        info_atomo.atributo_ID[0] = *char_atual;
        char_atual++;
        if ((*info_atomo.atributo_ID == '>' || *info_atomo.atributo_ID == '<') && *char_atual == '=') {
            strcat(info_atomo.atributo_ID, "=");  // Reconhece operadores >= ou <=
            char_atual++;
        }
    }

    info_atomo.atributo_ID[1] = '\0';
    info_atomo.linha = contaLinha;
    return info_atomo;
}

// Função que reconhece identificadores ou palavras reservadas
TInfoAtomo reconhece_id() {
    TInfoAtomo info_atomo;
    info_atomo.atomo = ERRO;
    char *iniID = char_atual;  // Marca o início do identificador
    int tamanho = 0;

    // Avança até o final do identificador
    while (islower(*char_atual) || isdigit(*char_atual) || *char_atual == '_') {
        char_atual++;
        tamanho++;
        if (tamanho > 15) {  // Verifica o tamanho do identificador
            info_atomo.atomo = ERRO;
            return info_atomo;
        }
    }

    // Copia o identificador encontrado para a estrutura info_atomo
    strncpy(info_atomo.atributo_ID, iniID, char_atual - iniID);
    info_atomo.atributo_ID[char_atual - iniID] = '\0';

    // Verifica se o identificador é uma palavra reservada
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
        info_atomo.atomo = PALAVRA_RESERVADA;  // Se for uma palavra reservada
    } else {
        info_atomo.atomo = IDENTIFICADOR;  // Se for um identificador comum
    }

    info_atomo.linha = contaLinha;
    return info_atomo;
}

// Função que reconhece números binários
TInfoAtomo reconhece_numero() {
    TInfoAtomo info_atomo;
    info_atomo.atomo = NUMERO;
    char_atual += 2;  // Ignora o prefixo "0b"
    int valor = 0;

    // Converte o número binário para decimal
    while (*char_atual == '0' || *char_atual == '1') {
        valor = (valor << 1) + (*char_atual - '0');  // Faz o shift e adiciona o bit
        char_atual++;
    }

    // Verifica se o próximo caractere é válido
    if (*char_atual != ' ' && *char_atual != '\n' && *char_atual != '\t' && *char_atual != '\r' && 
        *char_atual != ';' && *char_atual != '(' && *char_atual != ')' && *char_atual != '\0') {
        // Retorna erro léxico se for um caractere inválido
        info_atomo.atomo = ERRO;
        info_atomo.linha = contaLinha;
        return info_atomo;
    }

    // Atribui o valor numérico convertido
    info_atomo.atributo_numero = valor;
    info_atomo.linha = contaLinha;
    return info_atomo;
}

// Função que reconhece símbolos como ";", "(", ")", ",", ":", "."
TInfoAtomo reconhece_simbolo() {
    TInfoAtomo info_atomo;
    switch (*char_atual) {
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
        case '.': 
            info_atomo.atomo = PONTO_FINAL;
            break;
        default:
            info_atomo.atomo = ERRO;
            break;
    }
    char_atual++;  // Avança para o próximo caractere
    info_atomo.linha = contaLinha;
    return info_atomo;
}

// Função que reconhece comentários e os ignora
TInfoAtomo reconhece_comentario() {
    TInfoAtomo info_atomo;
    info_atomo.atomo = COMENTARIO;

    // Reconhece comentários de linha iniciados com "#"
    if (*char_atual == '#') {
        while (*char_atual != '\n' && *char_atual != '\0') {
            char_atual++;  // Avança até o fim da linha
        }
        // Incrementa o número da linha se encontrar uma quebra de linha
        if (*char_atual == '\n') {
            contaLinha++;
            char_atual++;
        }
    }
    // Reconhece comentários de bloco iniciados com "{-" e terminados com "-}"
    else if (*char_atual == '{' && *(char_atual + 1) == '-') {
        char_atual += 2;  // Avança para além de "{-"
        while (!(*char_atual == '-' && *(char_atual + 1) == '}') && *char_atual != '\0') {
            if (*char_atual == '\n') contaLinha++;  // Conta as quebras de linha
            char_atual++;
        }
        if (*char_atual == '-') char_atual += 2;  // Avança para além de "-}"
    } else {
        info_atomo.atomo = ERRO;  // Se não for um comentário válido, retorna erro
    }

    info_atomo.linha = contaLinha;
    return info_atomo;
}
