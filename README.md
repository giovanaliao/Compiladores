# README - Compilador fase 1

## Informações Gerais
 
- Giovana Liao: 10402264
- Maria Julia de Pádua: 10400630

**Descrição:**  
Este projeto implementa um compilador simples para uma linguagem inspirada em Pascal, chamada Pascal+. O programa é escrito em C e realiza a análise léxica e sintática de um código-fonte em Pascal+. O compilador reconhece identificadores, números binários, operadores, símbolos especiais e palavras reservadas da linguagem Pascal+ e verifica a conformidade do código com as regras da gramática.

## Estrutura do Código

### `buffer[MAX_BUFFER_SIZE]`  
Um buffer de tamanho fixo que armazena o conteúdo do arquivo de entrada, ou seja, o código-fonte a ser analisado. O ponteiro `char_atual` é usado para percorrer o buffer.

### Tipos de Tokens (`TAtomo`)  
Enumeração dos diferentes tipos de átomos (tokens) que podem ser encontrados durante a análise do código, tais como identificadores, números, operadores, e símbolos especiais (`PONTO_FINAL`, `PONTO_VIRGULA`, `ABRE_PAR`, etc.).

### Estrutura `TInfoAtomo`  
Uma estrutura que contém informações sobre o token reconhecido, incluindo:
- O tipo do token (`atomo`)
- A linha do arquivo onde o token foi encontrado (`linha`)
- Um atributo numérico para representar valores numéricos (`atributo_numero`)
- Um atributo de texto para armazenar identificadores ou palavras reservadas (`atributo_ID`)

### Função `main()`  
A função principal do programa que:
1. Abre o arquivo de entrada fornecido como argumento.
2. Lê o conteúdo do arquivo no buffer.
3. Inicia a análise sintática chamando a função `programa()`.

### Funções de Consumo (`consome()`)  
Função utilizada para consumir tokens do código-fonte e verificar se eles correspondem ao esperado. Caso o token consumido seja diferente do esperado, é gerado um erro sintático.

### Funções da Gramática  
Funções como `programa()`, `bloco()`, `comando_composto()`, e outras representam as regras da gramática da linguagem Pascal+:
- **`programa()`**: Inicia a análise verificando se o código começa com a palavra reservada "program", seguida de um identificador (nome do programa) e um ponto e vírgula.
- **`bloco()`**: Representa o bloco de código que contém a declaração de variáveis e comandos.
- **`comando_composto()`**: Analisa comandos compostos que começam com "begin" e terminam com "end".

### Funções de Expressão  
Estas funções analisam expressões lógicas e relacionais. Funções como `expressao()`, `expressao_logica()`, `expressao_relacional()`, e `expressao_simples()` verificam a conformidade das expressões no código-fonte.

### Funções de Análise Léxica  
Funções como `obter_atomo()`, `reconhece_id()`, `reconhece_numero()`, `reconhece_operador_relacional()` e `reconhece_simbolo()` são responsáveis por identificar e classificar os tokens encontrados no código-fonte.

### Funções de Erro  
A função `reportar_erro_sintatico()` exibe uma mensagem de erro indicando qual token era esperado e qual foi encontrado, além da linha onde o erro ocorreu.

## Palavras Reservadas Suportadas
As palavras reservadas atualmente suportadas pela linguagem Pascal+ são:
- `and`, `begin`, `boolean`, `elif`, `end`, `false`, `for`, `if`, `integer`, `not`, `of`, `or`, `program`, `read`, `set`, `to`, `true`, `write`.

## Como Executar o Programa

1. Compile o programa C usando um compilador de sua preferência, como `gcc`:
   ```bash
   gcc -g -Og -Wall projeto.c -o projeto
2. Execute o programa, passando o arquivo-fonte em Pascal+ como argumento:
   ```bash
   ./projeto exemplo.pas
3. O compilador irá exibir os tokens reconhecidos e, se houver algum erro sintático, ele será reportado com a linha onde o erro ocorreu.

## Estado Atual do Trabalho
O trabalho já implementa as seguintes funcionalidades:

- Análise léxica completa: o código reconhece identificadores, números binários, palavras reservadas, operadores e símbolos especiais.
- Análise sintática parcial: o compilador é capaz de verificar a conformidade do código com as regras básicas da gramática (programa, blocos, comandos compostos, expressões).
- Tratamento de erros sintáticos: o compilador exibe mensagens de erro sintático detalhadas, informando qual token era esperado e qual foi encontrado, além da linha em que o erro ocorreu.