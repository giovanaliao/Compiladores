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
    // Palavras reservadas (ex: program, begin, end)
	AND,
	BEGIN,
	BOOLEAN,
	ELIF,
	END,
	FALSE,
	FOR,
	IF,
	INTEGER,
	NOT,
	OF,
	OR,
	PROGRAM,
	READ,
	SET,
	TO,
	TRUE,
	WRITE,
    PONTO_VIRGULA,      // ";"
    ABRE_PAR,           // "("
    FECHA_PAR,          // ")"
    COMENTARIO,         // Comentários (ex: # ou {- -})
    OP_RELACIONAL,      // Operadores relacionais (>, <, >=, <=)
    OP_SIMPLES,            // Operadores de soma e subtração (+, -)
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

typedef struct {
    char id[16];           // Nome do identificador
    int endereco;          // Endereço na memória
    TAtomo tipo;          // Tipo da variável (INTEGER ou BOOLEAN)
    int linha;            // Linha onde foi declarada
} SimboloTabela;

// Array de strings que mapeia os tipos de tokens para suas representações textuais
char *msgAtomo[] = {
    "erro", "identificador", "numero", "and", "begin", "boolean", "elif", "end", "false",
	"for", "if", "integer", "not", "of", "or", "program", "read", "set", "to", "true", "write",
	"ponto_virgula", "abre_par", "fecha_par", "comentario", "op_relacional", "op_simples",
    "op_mult", "virgula", "dois_pontos", "ponto_final", "EOS"
};

int contaLinha = 1;  // Contador de linhas para rastrear a linha atual do código sendo processada
int abreParentesesEsperado = 0;  // Variável para checar se um parêntese de fechamento é esperado

// Tabela de símbolos global
SimboloTabela tabela_simbolos[100];  // Limite de 100 símbolos
int num_simbolos = 0;                // Contador de símbolos na tabela
int proximo_endereco = 0;            // Próximo endereço de memória disponível
int rotulo_atual = 1;                // Contador para rótulos

// Declaração de funções para evitar erros de declaração implícita
int proximo_rotulo();
void expressao_logica();
void expressao_relacional();
void expressao_simples();
void termo();
void fator();
void lista_variavel();
void comando_atribuicao();
void comando_repeticao();
void comando_condicional();
void comando_entrada();
void comando_saida();
void tipo();

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

int main(int argc, char *argv[]) {
    // Desativa o buffer de saída para garantir que as mensagens sejam exibidas imediatamente
    setbuf(stdout, NULL);

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
    consome(EOS);

    // Imprime a tabela de símbolos ao final da execução
    printf("\nTabela de Simbolos:\n");
    printf("%-15s %-10s\n", "Identificador", "Endereco");
    printf("------------------------------------------------\n");
    for (int i = 0; i < num_simbolos; i++) {
        printf("%-15s %-10d\n", 
            tabela_simbolos[i].id,
            tabela_simbolos[i].endereco); 
    }

    printf("\n%d linhas analisadas, programa sintaticamente correto\n", contaLinha);
    return 0;
}

int proximo_rotulo() {
    return rotulo_atual++;
}

// Função para inserir símbolo na tabela
void inserir_simbolo(char *id, TAtomo tipo, int linha) {
    // Verifica se o símbolo já existe
    for (int i = 0; i < num_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].id, id) == 0) {
            printf("Erro semântico na linha %d: variável '%s' já declarada na linha %d\n",
                   linha, id, tabela_simbolos[i].linha);
            exit(1);
        }
    }
    
    // Insere novo símbolo
    strcpy(tabela_simbolos[num_simbolos].id, id);
    tabela_simbolos[num_simbolos].endereco = proximo_endereco++;
    tabela_simbolos[num_simbolos].tipo = tipo;
    tabela_simbolos[num_simbolos].linha = linha;
    num_simbolos++;
}

// Função para buscar símbolo na tabela
int busca_tabela_simbolos(char *id, int linha) {
    for (int i = 0; i < num_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].id, id) == 0) {
            return tabela_simbolos[i].endereco;
        }
    }
    printf("Erro semântico na linha %d: variável '%s' não declarada\n", linha, id);
    exit(1);
}


// Função consome modificada para forçar flush do buffer
void consome(TAtomo esperado) {
    // Ignorar comentários enquanto consome tokens
    while (lookahead.atomo == COMENTARIO) {
        lookahead = obter_atomo();  // Avança para o próximo token
    }

    // Verifica se o token atual corresponde ao esperado
    if (lookahead.atomo == esperado) {
        // Exibe o token consumido dependendo do seu tipo
        if (lookahead.atomo == IDENTIFICADOR) {
            fflush(stdout);  // Força a saída imediata
        } else if (lookahead.atomo == NUMERO) {
            fflush(stdout);  // Força a saída imediata
        } else {
            fflush(stdout);  // Força a saída imediata
        }
        lookahead = obter_atomo();  // Avança para o próximo token
    } else {
        // Se o token não for o esperado, exibe um erro
        reportar_erro_sintatico(msgAtomo[esperado], msgAtomo[lookahead.atomo], lookahead.linha);
        exit(1);  // Encerra o programa em caso de erro
    }

    while (lookahead.atomo == COMENTARIO) {
        lookahead = obter_atomo();  // Avança para o próximo token
    }
}

// Função que analisa a estrutura do programa (gramática do programa)
void programa() {
    printf("INPP\n");  // Início do programa
    consome(PROGRAM);
    consome(IDENTIFICADOR);
    consome(PONTO_VIRGULA);
    bloco();
    consome(PONTO_FINAL);
    printf("PARA\n");  // Fim do programa
}

// Função que analisa um bloco de código que contém declarações de variáveis e comandos
void bloco() {
    declaracao_de_variaveis();  // Processa declarações de variáveis (se houver)
	comando_composto();         // Processa os comandos dentro de begin...end
}

// Função que analisa a declaração de variáveis do programa
void declaracao_de_variaveis() {
    TAtomo tipo_atual;
    while (lookahead.atomo == INTEGER || lookahead.atomo == BOOLEAN) {
        tipo_atual = lookahead.atomo;
        tipo();
        // Guarda o identificador para inserir na tabela
        char id[16];
        strcpy(id, lookahead.atributo_ID);
        inserir_simbolo(id, tipo_atual, lookahead.linha);
        printf("AMEM 1\n");  // Aloca memória para a variável
        
        consome(IDENTIFICADOR);
        while (lookahead.atomo == VIRGULA) {
            consome(VIRGULA);
            strcpy(id, lookahead.atributo_ID);
            inserir_simbolo(id, tipo_atual, lookahead.linha);
            printf("AMEM 1\n");
            consome(IDENTIFICADOR);
        }
        consome(PONTO_VIRGULA);
    }
}

void tipo() {
	consome(lookahead.atomo);
}

void lista_variavel() {
	consome(IDENTIFICADOR);
	while(lookahead.atomo == VIRGULA) {
		consome(lookahead.atomo);
		consome(IDENTIFICADOR);
	}
}

// Função que analisa um bloco de comandos composto (begin...end)
void comando_composto() {
    consome(BEGIN);  // Espera pela palavra reservada "begin"
    comando();  // Analisa o primeiro comando
    while (lookahead.atomo == PONTO_VIRGULA) {
        consome(PONTO_VIRGULA);  // Consome o ponto e vírgula após o comando
        comando();               // Analisa o próximo comando
    }
    consome(END);  // Espera pela palavra reservada "end"
}

void comando_atribuicao() {
    consome(SET);
    char id[16];
    strcpy(id, lookahead.atributo_ID);
    int endereco = busca_tabela_simbolos(id, lookahead.linha);
    consome(IDENTIFICADOR);
    consome(TO);

    // Processa a expressão
    expressao();

    // Armazena o resultado da expressão na variável
    printf("ARMZ %d\n", endereco);
}

void comando_condicional() {
    int rotulo_falso = proximo_rotulo();
    int rotulo_fim = proximo_rotulo();
    
    consome(IF);
    expressao();
    consome(DOIS_PONTOS);
    printf("DSVF L%d\n", rotulo_falso);
    comando();
    printf("DSVS L%d\n", rotulo_fim);
    printf("L%d: NADA\n", rotulo_falso);
    
    if (lookahead.atomo == ELIF) {
        consome(ELIF);
        comando();
    }
    printf("L%d: NADA\n", rotulo_fim);
}

void comando_repeticao() {
    int rotulo_inicio = proximo_rotulo();
    int rotulo_fim = proximo_rotulo();
    
    consome(FOR);
    char id[16];
    strcpy(id, lookahead.atributo_ID);
    int endereco = busca_tabela_simbolos(id, lookahead.linha);
    consome(IDENTIFICADOR);
    consome(OF);

    expressao();
    printf("ARMZ %d\n", endereco);
    
    printf("L%d: NADA\n", rotulo_inicio);
    printf("CRVL %d\n", endereco);
    
    consome(TO);
    expressao();
    printf("CMEG\n");
    printf("DSVF L%d\n", rotulo_fim);
    
    consome(DOIS_PONTOS);
    comando();
    
    printf("CRVL %d\n", endereco);
    printf("CRCT 1\n");
    printf("SOMA\n");
    printf("ARMZ %d\n", endereco);
    printf("DSVS L%d\n", rotulo_inicio);
    printf("L%d: NADA\n", rotulo_fim);
}

void comando_entrada() {
    consome(READ);
    consome(ABRE_PAR);
    
    char id[16];
    strcpy(id, lookahead.atributo_ID);
    int endereco = busca_tabela_simbolos(id, lookahead.linha);
    printf("LEIT\n");
    printf("ARMZ %d\n", endereco);
    
    consome(IDENTIFICADOR);
    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        strcpy(id, lookahead.atributo_ID);
        endereco = busca_tabela_simbolos(id, lookahead.linha);
        printf("LEIT\n");
        printf("ARMZ %d\n", endereco);
        consome(IDENTIFICADOR);
    }
    consome(FECHA_PAR);
}

void comando_saida() {
    consome(WRITE);
    consome(ABRE_PAR);
    expressao();
    printf("IMPR\n");
    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        expressao();
        printf("IMPR\n");
    }
    consome(FECHA_PAR);
}

// Função que analisa um comando
void comando() {
    if(lookahead.atomo == SET) {
		comando_atribuicao();
	} else if(lookahead.atomo == IF) {
		comando_condicional();
	} else if(lookahead.atomo == FOR) {
		comando_repeticao();
	} else if(lookahead.atomo == READ) {
		comando_entrada();
	} else if(lookahead.atomo == WRITE) {
		comando_saida();
	} else {
		comando_composto();
	}
}

// Funções auxiliares para expressões
// Função que analisa uma expressão lógica
void expressao() {
    expressao_logica();  // Analisa a expressão lógica
	while(lookahead.atomo == OR) {
		consome(lookahead.atomo);
		expressao_logica();
        printf("DISJ\n");
	}
}

void expressao_logica() {
    expressao_relacional();
    while(lookahead.atomo == AND) {
        consome(lookahead.atomo);
        expressao_relacional();
        printf("CONJ\n");
    }
}

void expressao_relacional() {
    expressao_simples();  // Analisa a primeira parte da expressão aritmética

    // Verifica se existe um operador relacional
    if (lookahead.atomo == OP_RELACIONAL) {
        char op[3];
        strcpy(op, lookahead.atributo_ID);  // Copia o operador relacional
        consome(OP_RELACIONAL);  // Consome o operador relacional
        expressao_simples();  // Analisa a segunda parte da expressão aritmética

        // Gera o código MEPA correspondente ao operador relacional
        if (strcmp(op, ">") == 0) {
            printf("CMMA\n");
        } else if (strcmp(op, "<") == 0) {
            printf("CMME\n");
        } else if (strcmp(op, ">=") == 0) {
            printf("CMAG\n");
        } else if (strcmp(op, "<=") == 0) {
            printf("CMEG\n");
        } else if (strcmp(op, "=") == 0) {
            printf("CMIG\n");
        } else if (strcmp(op, "/=") == 0) {
            printf("CMIG\n");
            printf("NEGA\n");
        } else {
            // Caso não esperado
            printf("Erro: operador relacional inesperado '%s' na linha %d\n", op, lookahead.linha);
            exit(1);
        }
    }
}

// Função que analisa uma expressão aritmética simples
void expressao_simples() {
    termo();
    while (lookahead.atomo == OP_SIMPLES) {
        char op = lookahead.atributo_ID[0];
        consome(OP_SIMPLES);
        termo();
        if (op == '+') {
            printf("SOMA\n");
        } else if (op == '-') {
            printf("SUBT\n");
        }
    }
}

// Função que analisa um termo (parte de uma expressão)
void termo() {
    fator();
    while (lookahead.atomo == OP_MULT) {
        char op = lookahead.atributo_ID[0];
        consome(OP_MULT);
        fator();
        if (op == '*') {
            printf("MULT\n");
        } else if (op == '/') {
            printf("DIVI\n");
        }
    }
}

// Função que analisa fatores (como identificadores, números ou expressões entre parênteses)
void fator() {
    if (lookahead.atomo == IDENTIFICADOR) {
        int endereco = busca_tabela_simbolos(lookahead.atributo_ID, lookahead.linha);
        printf("CRVL %d\n", endereco);
        consome(IDENTIFICADOR);
    } else if (lookahead.atomo == NUMERO) {
        printf("CRCT %d\n", (int)lookahead.atributo_numero);
        consome(NUMERO);
    } else if (lookahead.atomo == TRUE) {
        printf("CRCT 1\n");
        consome(TRUE);
    } else if (lookahead.atomo == FALSE) {
        printf("CRCT 0\n");
        consome(FALSE);
    } else if (lookahead.atomo == NOT) {
        consome(NOT);
        fator();
        printf("NEGA\n");
    } else if (lookahead.atomo == ABRE_PAR) {
        consome(ABRE_PAR);
        expressao();
        consome(FECHA_PAR);
    } else {
        reportar_erro_sintatico("fator", msgAtomo[lookahead.atomo], lookahead.linha);
    }
}

// Função que reporta erros sintáticos quando o token encontrado não é o esperado
void reportar_erro_sintatico(const char *esperado, const char *encontrado, int linha) {
    printf("# %d: erro sintatico, esperado [%s] encontrado [%s]\n", linha, esperado, encontrado);
	exit(0);
}

TInfoAtomo obter_atomo() {
    TInfoAtomo info_atomo;

    // Consome espaços em branco, quebras de linha e tabulações
    while (*char_atual == ' ' || *char_atual == '\n' || *char_atual == '\t' || *char_atual == '\r') {
        if (*char_atual == '\n') contaLinha++;
        char_atual++;
    }

    // Verifica se chegamos ao fim do arquivo
    if (*char_atual == '\0') {
        info_atomo.atomo = EOS;
        info_atomo.linha = contaLinha;
        return info_atomo;
    }

    // Trata ponto final
    if (*char_atual == '.') {
        info_atomo.atomo = PONTO_FINAL;
        info_atomo.linha = contaLinha;
        char_atual++;
        return info_atomo;
    }

    // Verifica se é um comentário
    if (*char_atual == '#' || (*char_atual == '{' && *(char_atual + 1) == '-')) {
        return reconhece_comentario();
    }

    // Verifica operadores relacionais ou símbolos
    if (strchr("><=", *char_atual)) {
        return reconhece_operador_relacional();
    }

    // Verifica símbolos simples ou operadores
    if (strchr(";(),:", *char_atual)) {
        return reconhece_simbolo();
    }

    // Verifica operadores aritméticos (*, /)
    if (strchr("+-*/", *char_atual)) {
        info_atomo.atributo_ID[0] = *char_atual;
        info_atomo.atributo_ID[1] = '\0';
        info_atomo.linha = contaLinha;

        if (*char_atual == '*' || *char_atual == '/') {
            info_atomo.atomo = OP_MULT;
        } else {
            info_atomo.atomo = OP_SIMPLES;
        }

        char_atual++;
        return info_atomo;
    }

    // Verifica identificadores ou palavras reservadas
    if (islower(*char_atual)) {
        return reconhece_id();
    }

    // Verifica números binários
    if (*char_atual == '0' && *(char_atual + 1) == 'b') {
        return reconhece_numero();
    }

    // Token inválido
    printf("Erro léxico na linha %d: caractere inválido '%c'\n", contaLinha, *char_atual);
    exit(1);
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
    if (strcmp(info_atomo.atributo_ID, "and") == 0) {
		info_atomo.atomo = AND;
	} else if (strcmp(info_atomo.atributo_ID, "begin") == 0) {
		info_atomo.atomo = BEGIN;
	} else if (strcmp(info_atomo.atributo_ID, "boolean") == 0) {
		info_atomo.atomo = BOOLEAN;
	} else if (strcmp(info_atomo.atributo_ID, "elif") == 0) {
		info_atomo.atomo = ELIF;
	} else if (strcmp(info_atomo.atributo_ID, "end") == 0) {
		info_atomo.atomo = END;
	} else if (strcmp(info_atomo.atributo_ID, "false") == 0) {
		info_atomo.atomo = FALSE;
	} else if (strcmp(info_atomo.atributo_ID, "for") == 0) {
		info_atomo.atomo = FOR;
	} else if (strcmp(info_atomo.atributo_ID, "if") == 0) {
		info_atomo.atomo = IF;
	} else if (strcmp(info_atomo.atributo_ID, "integer") == 0) {
		info_atomo.atomo = INTEGER;
	} else if (strcmp(info_atomo.atributo_ID, "not") == 0) {
		info_atomo.atomo = NOT;
	} else if (strcmp(info_atomo.atributo_ID, "of") == 0) {
		info_atomo.atomo = OF;
	} else if (strcmp(info_atomo.atributo_ID, "or") == 0) {
		info_atomo.atomo = OR;
	} else if (strcmp(info_atomo.atributo_ID, "program") == 0) {
		info_atomo.atomo = PROGRAM;
	} else if (strcmp(info_atomo.atributo_ID, "read") == 0) {
		info_atomo.atomo = READ;
	} else if (strcmp(info_atomo.atributo_ID, "set") == 0) {
		info_atomo.atomo = SET;
	} else if (strcmp(info_atomo.atributo_ID, "to") == 0) {
		info_atomo.atomo = TO;
	} else if (strcmp(info_atomo.atributo_ID, "true") == 0) {
		info_atomo.atomo = TRUE;
	} else if (strcmp(info_atomo.atributo_ID, "write") == 0) {
		info_atomo.atomo = WRITE;
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
    
    // Verifica se é número binário
    if (*char_atual == '0' && *(char_atual + 1) == 'b') {
        char_atual += 2;
        int valor = 0;
        while (*char_atual == '0' || *char_atual == '1') {
            valor = (valor << 1) + (*char_atual - '0');
            char_atual++;
        }
        info_atomo.atributo_numero = valor;
    }
    // Número decimal
    else {
        int valor = 0;
        while (isdigit(*char_atual)) {
            valor = valor * 10 + (*char_atual - '0');
            char_atual++;
        }
        info_atomo.atributo_numero = valor;
    }

    // Verifica se o próximo caractere é válido
    if (!strchr(" \n\t\r;():,.", *char_atual)) {
        info_atomo.atomo = ERRO;
    }

    info_atomo.linha = contaLinha;
    return info_atomo;
}


// Função que reconhece símbolos como ";", "(", ")", ",", ":", "."
TInfoAtomo reconhece_simbolo() {
    TInfoAtomo info_atomo;
    info_atomo.atributo_ID[0] = *char_atual;
    info_atomo.atributo_ID[1] = '\0';
    info_atomo.linha = contaLinha;
    
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
        case '+':
        case '-':
            info_atomo.atomo = OP_SIMPLES;
            break;
        case '*':
        case '/':
            info_atomo.atomo = OP_MULT;
            info_atomo.atributo_ID[0] = *char_atual;
            info_atomo.atributo_ID[1] = '\0';
            break;
        default:
            info_atomo.atomo = ERRO;
            break;
    }

    char_atual++;
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