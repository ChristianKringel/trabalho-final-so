#include "libs.h"
#include "metrics.h"

#ifndef UTILS_H
#define UTILS_H

void dormir_operacao(int min_ms, int max_ms);
void dormir_operacao_com_pausa(SimulacaoAeroporto* sim, int min_ms, int max_ms);
int gerar_numero_aleatorio(int min, int max);

// Funções de prioridade e monitoramento
void* monitorar_avioes(void* arg);
int calcular_prioridade_dinamica(Aviao* aviao, time_t tempo_atual);
void verificar_avioes_em_espera(SimulacaoAeroporto* sim);

// Funções de fila de prioridade
void inicializar_fila_prioridade(FilaPrioridade* fila);
void inserir_na_fila_prioridade(FilaPrioridade* fila, int aviao_id, int prioridade);
int remover_da_fila_prioridade(FilaPrioridade* fila);
void destruir_fila_prioridade(FilaPrioridade* fila);

void* monitorar_starvation(void* arg);
int verificar_starvation(Aviao* aviao, time_t tempo_atual);
int detectar_deadlock(SimulacaoAeroporto* sim);
void atualizar_estado_aviao(Aviao* aviao, EstadoAviao novo_estado);

void finalizar_simulacao(SimulacaoAeroporto* sim);
void destruir_recursos(RecursosAeroporto* recursos);
void liberar_memoria(SimulacaoAeroporto* sim);

void imprimir_status_operacao(int id_aviao, TipoVoo tipo, const char* operacao, const char* status);
void imprimir_status_recursos(RecursosAeroporto* recursos);
void gerar_relatorio_final(SimulacaoAeroporto* sim);
void imprimir_resumo_aviao(Aviao* aviao);

void atualizar_estado_aviao(Aviao* aviao, EstadoAviao novo_estado);
void verificar_pausa(SimulacaoAeroporto* sim);
const char* estado_para_str(EstadoAviao estado);

#endif