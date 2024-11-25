Segue o README atualizado com a inclusão da implementação do código MEPA (Máquina de Execução Para o Aprendizado):

```markdown
# README - Compilador Fase 1

## Informações Gerais

- **Autores:**
  - Giovana Liao: 10402264  
  - Maria Julia de Pádua: 10400630

**Descrição:**  
Este projeto implementa um compilador simples para uma linguagem inspirada em Pascal, chamada Pascal+. O programa, escrito em C, realiza as análises léxica e sintática de um código-fonte em Pascal+. Ele reconhece identificadores, números binários, operadores, símbolos especiais e palavras reservadas, além de verificar a conformidade do código com as regras da gramática.

---

## Alterações Recentes

### **Implementação do MEPA**  
- Foi implementado o suporte à geração de código para a Máquina de Execução Para o Aprendizado (MEPA).  
- **Principais Instruções Suportadas:**  
  - **INPP:** Inicialização do programa.  
  - **AMEM:** Alocação de memória para variáveis.  
  - **ARMZ:** Armazena o valor de uma expressão em um endereço de memória.  
  - **CRVL:** Carrega o valor de uma variável para o topo da pilha.  
  - **CRCT:** Carrega uma constante para o topo da pilha.  
  - **SOMA, SUBT, MULT, DIVI:** Operações aritméticas.  
  - **CMIG, CMME, CMMA, CMEG, CMAG:** Comparações entre valores.  
  - **DSVF, DSVS:** Desvios condicionais e incondicionais.  
  - **LEIT:** Operação de entrada.  
  - **IMPR:** Operação de saída.  
  - **PARA:** Finalização do programa.

Exemplo de código MEPA gerado para um programa básico:
```mepa
INPP
AMEM 1      ; Aloca memória para a variável
LEIT        ; Lê um valor de entrada
ARMZ 0      ; Armazena o valor na variável no endereço 0
CRCT 2      ; Carrega a constante 2
CRVL 0      ; Carrega o valor da variável no topo da pilha
MULT        ; Multiplica os dois valores
IMPR        ; Imprime o resultado
PARA        ; Finaliza o programa
```

---

## Estrutura do Código

### `buffer[MAX_BUFFER_SIZE]`  
Buffer que armazena o conteúdo do arquivo de entrada (código-fonte). O ponteiro `char_atual` percorre o buffer.

### Tokens Suportados  
A enumeração `TAtomo` lista os tipos de tokens reconhecidos pelo analisador léxico, incluindo palavras reservadas, operadores, identificadores, números e símbolos.

### Estrutura `TInfoAtomo`  
Armazena as informações do token reconhecido:
- Tipo (`atomo`)  
- Linha (`linha`)  
- Atributo numérico (`atributo_numero`)  
- Atributo textual (`atributo_ID`)

---

## Palavras Reservadas Suportadas  
- `and`, `begin`, `boolean`, `elif`, `end`, `false`, `for`, `if`, `integer`, `not`, `of`, `or`, `program`, `read`, `set`, `to`, `true`, `write`.

---

## Funcionalidades Implementadas

### Análise Léxica  
O analisador léxico reconhece:
1. **Identificadores:** Sequências alfanuméricas (case-insensitive).  
2. **Números:** Incluindo números binários prefixados por `0b`.  
3. **Operadores:** Aritméticos, relacionais e lógicos.  
4. **Símbolos Especiais:** Como `;`, `:`, `(`, `)`, `,`, `.`.  
5. **Palavras Reservadas.**

### Análise Sintática  
O analisador sintático verifica a conformidade do código com as regras da gramática:
- **Estrutura do programa:** `program nome; begin ... end.`  
- **Declarações de variáveis:** Tipos `integer` e `boolean`.  
- **Comandos:** Atribuições, laços, condicionais, entrada e saída.

### Geração de Código MEPA  
A implementação MEPA permite executar o código gerado em uma máquina virtual, sendo útil para fins acadêmicos e aprendizado.

### Mensagens de Erro  
Mensagens de erro detalhadas são exibidas para tokens inesperados ou caracteres inválidos, indicando o tipo de erro e a linha correspondente.

---

## Como Executar

1. Compile o programa com o GCC:
   ```bash
   gcc -g -Og -Wall projeto.c -o projeto
   ```
2. Execute o compilador, fornecendo o arquivo-fonte como argumento:
   ```bash
   ./projeto exemplo.pas
   ```

3. **Saída esperada:**
   - Código MEPA gerado.
   - Tabela de símbolos.
   - Mensagens de erro (se houver).

---

## Estado Atual

- **Análise Léxica:** Completa e funcional, com suporte para case-insensitive em identificadores.  
- **Análise Sintática:** Implementação básica das regras da gramática.  
- **Geração de Código MEPA:** Suporte a operações aritméticas, condicionais, loops, entrada e saída.  
- **Tratamento de Erros:** Abrangente para erros léxicos e sintáticos.

### **Próximos Passos**
- Implementação de uma análise semântica detalhada.  
- Expansão do suporte para funções e escopos.  
- Otimização do código intermediário gerado.  
```