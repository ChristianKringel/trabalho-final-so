#ifndef LIBS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#define LIBS_H

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
    AGUARDANDO_DECOLAGEM,
    DECOLANDO,
    FINALIZADO,
    FALHA_STARVATION,
    FALHA_DEADLOCK
} EstadoAviao;

// Estrutura para recursos do aeroporto
typedef struct {
    pthread_mutex_t mutex_pistas;
    pthread_cond_t cond_pistas;
    int pistas_disponiveis;
    int total_pistas;
    
    pthread_mutex_t mutex_portoes;
    pthread_cond_t cond_portoes;
    int portoes_disponiveis;
    int total_portoes;
    
    pthread_mutex_t mutex_torres;
    pthread_cond_t cond_torres;
    int torres_disponiveis;
    int total_torres;
} RecursosAeroporto;

// Estrutura para dados de um aviao
typedef struct {
    int id;
    TipoVoo tipo;
    EstadoAviao estado;
    time_t tempo_criacao;
    time_t tempo_inicio_espera;
    time_t tempo_fim_operacao;
    pthread_t thread_id;
    int pista_ocupada;
    int portao_ocupado;
    int usando_torre;
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
typedef struct {
    RecursosAeroporto recursos;
    MetricasSimulacao metricas;
    Aviao *avioes;
    int max_avioes;
    int tempo_simulacao;
    int ativa;
    pthread_mutex_t mutex_simulacao;
    time_t tempo_inicio;
} SimulacaoAeroporto;
#endif