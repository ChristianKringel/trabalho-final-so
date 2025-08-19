# Simulador de Controle de Tráfego Aéreo
https://docs.github.com/copilot/concepts/coding-agent/coding-agent
Este é um projeto de simulação em C de um sistema de controle de tráfego aéreo, utilizando threads POSIX (pthreads) para gerenciar o fluxo de aviões em um aeroporto internacional de alta demanda. O principal objetivo é demonstrar e resolver os problemas de deadlock e *starvation* (inanição) que podem ocorrer na alocação de recursos limitados.

## Recursos do Aeroporto

O simulador modela os seguintes recursos limitados:

* **Pistas:** 3 pistas para pouso e decolagem.
* **Portões:** 5 portões de embarque para entrada e saída de passageiros.
* **Torres de Controle:** 1 torre de controle que gerencia a comunicação e liberação das operações, atendendo no máximo duas operações simultaneamente.

## Regras e Comportamento

A simulação apresenta diferentes ordens de alocação de recursos para voos domésticos e internacionais, que são a principal causa dos deadlocks. O projeto utiliza o Algoritmo do Banqueiro para a alocação segura de recursos, além de um sistema de prioridade dinâmica e detecção de inatividade para mitigar deadlocks e *starvation*.

As threads de aviões passam pelas seguintes fases, solicitando recursos em ordens específicas:

* **Voos Internacionais:**
    * **Pouso:** Requer Pista → Torre
    * **Desembarque:** Requer Portão → Torre
    * **Decolagem:** Requer Torre → Portão → Pista

* **Voos Domésticos:**
    * **Pouso:** Requer Torre → Pista
    * **Desembarque:** Requer Torre → Portão
    * **Decolagem:** Requer Portão → Pista → Torre

## Como Compilar e Executar
Para obter uma cópia local do projeto, clone o repositório utilizando o comando abaixo:

```bash
git clone [https://github.com/ChristianKringel/trabalho-final-so.git](https://github.com/ChristianKringel/trabalho-final-so.git)
```

O projeto pode ser compilado e executado usando o `Makefile` fornecido.

**Pré-requisitos:**

* Compilador `gcc` com suporte a C11.
* Biblioteca `pthreads` para threads POSIX.
* Biblioteca `ncurses` para a interface de usuário no terminal.

**Passo a passo:**

1.  Navegue até o diretório raiz do projeto.
2.  Certifique-se de que o seu terminal tenha no mínimo 120 colunas e 24 linhas para a correta exibição da interface.
3.  Execute o seguinte comando para compilar e linkar o executável:

    ```bash
    make all
    ```

    Isso criará um executável chamado `air_traffic_simulator` no diretório raiz.

4.  Opcionalmente, você pode compilar e executar em um único passo com o comando:

    ```bash
    make run
    ```

    Isso compilará o projeto e o executará imediatamente.

## Limpeza

Para remover os arquivos objeto e o executável gerados, execute o comando:

```bash
make clean
