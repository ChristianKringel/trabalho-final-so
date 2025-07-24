#ifndef LIBS_H

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdarg.h> 


//#include "structs.h" // RECOMENDAÇÃO: Mova todas as suas structs para um arquivo separado

#define LIBS_H


#define MAX_PISTAS 3
#define MAX_PORTOES 5
#define MAX_TORRES 2
#define MAX_AVIOES 200

// Enum para tipos de voo
typedef enum { VOO_DOMESTICO, VOO_INTERNACIONAL } TipoVoo;

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

// Estrutura para recursos do aeroporto
typedef struct {
    pthread_mutex_t mutex_pistas;
    pthread_cond_t cond_pistas;
    int pistas_disponiveis;
    int total_pistas;
    int pista_ocupada_por[MAX_PISTAS];

    pthread_mutex_t mutex_portoes;
    pthread_cond_t cond_portoes;
    int portoes_disponiveis;
    int total_portoes;
    int portao_ocupado_por[MAX_PORTOES];

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
    int pista_alocada;
    int portao_alocado;
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
} SimulacaoAeroporto;

typedef struct {
    Aviao* aviao;
    SimulacaoAeroporto* sim;
} ThreadArgs;

// =============== FUNÇÕES DE INICIALIZAÇÃO ===============
SimulacaoAeroporto* inicializar_simulacao(int pistas, int portoes, int torres, int tempo_simulacao, int max_avioes);
void inicializar_recursos(RecursosAeroporto* recursos, int pistas, int portoes, int torres);
void inicializar_metricas(MetricasSimulacao* metricas);

// =============== FUNÇÕES DE GERENCIAMENTO DE RECURSOS ===============
int solicitar_pista(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo);
void liberar_pista(SimulacaoAeroporto* sim, int id_aviao, int pista_idx);
int solicitar_portao(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo);
void liberar_portao(SimulacaoAeroporto* sim, int id_aviao, int portao_idx);
int solicitar_torre(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo);
void liberar_torre(SimulacaoAeroporto* sim, int id_aviao);

// =============== FUNÇÕES DE OPERAÇÕES DE AVIÕES ===============
void* thread_aviao(void* arg);
int realizar_pouso(Aviao* aviao, SimulacaoAeroporto* sim);
int realizar_desembarque(Aviao* aviao, SimulacaoAeroporto* sim);
int realizar_decolagem(Aviao* aviao, SimulacaoAeroporto* sim);

// =============== FUNÇÕES ESPECÍFICAS POR TIPO DE VOO ===============
int pouso_internacional(Aviao* aviao, SimulacaoAeroporto* sim);
int pouso_domestico(Aviao* aviao, SimulacaoAeroporto* sim);
int desembarque_internacional(Aviao* aviao, SimulacaoAeroporto* sim);
int desembarque_domestico(Aviao* aviao, SimulacaoAeroporto* sim);
int decolagem_internacional(Aviao* aviao, SimulacaoAeroporto* sim);
int decolagem_domestico(Aviao* aviao, SimulacaoAeroporto* sim);

// =============== FUNÇÕES DE MONITORAMENTO ===============
void* monitor_starvation(void* arg);
int verificar_starvation(Aviao* aviao, time_t tempo_atual);
int detectar_deadlock(SimulacaoAeroporto* sim);
void atualizar_estado_aviao(Aviao* aviao, EstadoAviao novo_estado);

// =============== FUNÇÕES DE MÉTRICAS ===============
void incrementar_metrica_pouso(MetricasSimulacao* metricas);
void incrementar_metrica_desembarque(MetricasSimulacao* metricas);
void incrementar_metrica_decolagem(MetricasSimulacao* metricas);
void incrementar_aviao_sucesso(MetricasSimulacao* metricas);
void incrementar_aviao_starvation(MetricasSimulacao* metricas);
void incrementar_aviao_deadlock(MetricasSimulacao* metricas);

// =============== FUNÇÕES DE CRIAÇÃO DE AVIÕES ===============
Aviao* criar_aviao(int id, TipoVoo tipo);
void* criador_avioes(void* arg);
TipoVoo gerar_tipo_voo_aleatorio();

// =============== FUNÇÕES DE SIMULAÇÃO PRINCIPAL ===============
void iniciar_simulacao(SimulacaoAeroporto* sim);
void parar_simulacao(SimulacaoAeroporto* sim);
int simulacao_ativa(SimulacaoAeroporto* sim);

// =============== FUNÇÕES DE RELATÓRIO ===============
void imprimir_status_operacao(int id_aviao, TipoVoo tipo, const char* operacao, const char* status);
void imprimir_status_recursos(RecursosAeroporto* recursos);
void gerar_relatorio_final(SimulacaoAeroporto* sim);
void imprimir_resumo_aviao(Aviao* aviao);

// =============== FUNÇÕES DE LIMPEZA ===============
void finalizar_simulacao(SimulacaoAeroporto* sim);
void destruir_recursos(RecursosAeroporto* recursos);
void liberar_memoria(SimulacaoAeroporto* sim);

// =============== FUNÇÕES AUXILIARES ===============
const char* estado_para_string(EstadoAviao estado);
const char* tipo_voo_para_string(TipoVoo tipo);
double calcular_tempo_decorrido(time_t inicio, time_t fim);
void dormir_operacao(int min_ms, int max_ms);
int gerar_numero_aleatorio(int min, int max);

void init_terminal_ncurses();
void close_terminal_ncurses();
void update_terminal_display(SimulacaoAeroporto* sim);

void log_evento_ui(SimulacaoAeroporto* sim, const char* formato, ...);
#endif



// // ======================= PROTÓTIPOS DE FUNÇÕES =======================

// // --- De initialize.c ---
// SimulacaoAeroporto* inicializar_simulacao(int pistas, int portoes, int torres, int tempo_simulacao, int max_avioes);

// // --- De airport.c (Assinaturas Simplificadas e Corrigidas) ---
// int solicitar_pista(SimulacaoAeroporto* sim, int id_aviao);
// void liberar_pista(SimulacaoAeroporto* sim, int pista_idx);
// int solicitar_portao(SimulacaoAeroporto* sim, int id_aviao);
// void liberar_portao(SimulacaoAeroporto* sim, int portao_idx);
// int solicitar_torre(SimulacaoAeroporto* sim);
// void liberar_torre(SimulacaoAeroporto* sim);

// // --- De airplane.c ---
// void* criador_avioes(void* arg);
// void* thread_aviao(void* arg);

// // --- De metrics.c ---
// void inicializar_metricas(MetricasSimulacao* metricas);
// void incrementar_metrica_pouso(MetricasSimulacao* metricas);
// void incrementar_metrica_desembarque(MetricasSimulacao* metricas);
// void incrementar_metrica_decolagem(MetricasSimulacao* metricas);
// void incrementar_aviao_sucesso(MetricasSimulacao* metricas);
// void incrementar_aviao_criado(MetricasSimulacao* metricas);
// // Adicione outros protótipos de métricas aqui...

// // --- De terminal.c ---
// void init_terminal_ncurses();
// void close_terminal_ncurses();
// void update_terminal_display(SimulacaoAeroporto* sim);
// void log_evento_ui(SimulacaoAeroporto* sim, const char* formato, ...);

// // --- De utils.c ---
// void atualizar_estado_aviao(Aviao* aviao, EstadoAviao novo_estado);
// void dormir_operacao(int min_ms, int max_ms);
// void gerar_relatorio_final(SimulacaoAeroporto* sim);
// void liberar_memoria(SimulacaoAeroporto* sim);
// // Adicione outros protótipos de utils aqui...

// #endif // LIBS_H