#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#ifndef LIBS_H
#define LIBS_H

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h> 
#include <ctype.h>
#include <locale.h>

#define MAX_PISTAS 3
#define MAX_PORTOES 5
#define MAX_TORRES 1
#define MAX_OP_TORRE 2
#define MAX_AVIOES 50
#define MAX_TEMPO_SIM 240
#define N_RESOURCES 3

// Enum para tipos de voo
typedef enum { 
    VOO_DOMESTICO, 
    VOO_INTERNACIONAL 
} TipoVoo;

// Enum para estados do aviao
typedef enum {
    AGUARDANDO_POUSO,
    POUSANDO,
    DESEMBARCANDO,
    AGUARDANDO_DESEMBARQUE,
    AGUARDANDO_DECOLAGEM,
    DECOLANDO,
    FINALIZADO,
    FALHA_STARVATION,
    FALHA_DEADLOCK,
    FINALIZADO_SUCESSO
    
} EstadoAviao;

typedef enum {
    RECURSO_PISTA = 0,
    RECURSO_PORTAO = 1, 
    RECURSO_TORRE = 2
} TipoRecurso;
typedef struct {
    int alocacao[MAX_AVIOES][N_RESOURCES];
    int necessidade[MAX_AVIOES][N_RESOURCES];
    int disponivel[N_RESOURCES];
    pthread_mutex_t mutex_banqueiro;
} Banqueiro;

// Estrutura para fila de prioridade
typedef struct {
    int avioes_ids[MAX_AVIOES];
    int prioridades[MAX_AVIOES];
    int tamanho;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} FilaPrioridade;

// Estrutura para recursos do aeroporto - VERSÃO CORRIGIDA
typedef struct {
    pthread_mutex_t mutex_pistas;
    pthread_cond_t cond_pistas;
    int pistas_disponiveis;
    int total_pistas;
    int pista_ocupada_por[MAX_PISTAS];
    FilaPrioridade fila_pistas;

    pthread_mutex_t mutex_portoes;
    pthread_cond_t cond_portoes;
    int portoes_disponiveis;
    int total_portoes;
    int portao_ocupado_por[MAX_PORTOES];
    FilaPrioridade fila_portoes;

    pthread_mutex_t mutex_torres;
    pthread_cond_t cond_torres;
    int slots_torre_disponiveis;
    int capacidade_torre;
    int operacoes_ativas_torre; 
    int torre_ocupada_por[MAX_OP_TORRE]; 
    FilaPrioridade fila_torres;

    Banqueiro banco;
    pthread_mutex_t mutex_banco;
    pthread_cond_t cond_banco;

} RecursosAeroporto;

// Estrutura para dados de um aviao
typedef struct {
    int id;
    TipoVoo tipo;
    EstadoAviao estado;
    time_t tempo_criacao;
    time_t tempo_inicio_espera;
    time_t tempo_fim_operacao;
    time_t chegada_na_fila;
    time_t tempo_inicio_espera_ar;
    int prioridade_dinamica;
    bool em_alerta;
    bool crash_iminente;
    pthread_t thread_id;
    int pista_alocada;
    int portao_alocado;
    int torre_alocada;
    bool sacrificado;
} Aviao;

// Estrutura para metricas da simulacao
typedef struct {
    int total_avioes_criados;
    int avioes_finalizados_sucesso;
    int avioes_falha_starvation;
    int avioes_falha_deadlock;
    int voos_domesticos_total;
    int voos_internacionais_total;
    int operacoes_pouso;
    int operacoes_desembarque;
    int operacoes_decolagem;
    pthread_mutex_t mutex_metricas;
} MetricasSimulacao;

// Estrutura principal da simulacao
typedef struct s_simulacao_aeroporto {
    RecursosAeroporto recursos;
    MetricasSimulacao metricas;
    Aviao *avioes;
    int max_avioes;
    int tempo_simulacao;
    volatile int ativa;
    pthread_mutex_t mutex_simulacao;
    pthread_mutex_t mutex_ui_log; 
    time_t tempo_inicio;
    time_t tempo_pausado_total;    
    time_t inicio_pausa;           
    //UIViewMode modo_ui;
    bool pausado;
    pthread_cond_t cond_pausado;
    pthread_mutex_t mutex_pausado;
    pthread_t monitor_thread;
    time_t ultimo_evento_global;      // NOVO: Guarda o tempo do último estado alterado
    pthread_mutex_t mutex_ultimo_evento; // NOVO: Protege a variável acima
} SimulacaoAeroporto;

typedef struct {
    Aviao* aviao;
    SimulacaoAeroporto* sim;
} ThreadArgs;

#endif
