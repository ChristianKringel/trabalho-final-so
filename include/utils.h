#include "libs.h"
#include "metrics.h"

#ifndef UTILS_H
#define UTILS_H

int gerar_numero_aleatorio(int min, int max);

// Funções de prioridade e monitoramento
void* monitorar_avioes(void* arg);
void verificar_avioes_em_espera(SimulacaoAeroporto* sim);

// Funções de fila de prioridade
void inicializar_fila_prioridade(FilaPrioridade* fila);
void inserir_na_fila_prioridade(FilaPrioridade* fila, int aviao_id, int prioridade);
int remover_da_fila_prioridade(FilaPrioridade* fila, int aviao_id);
int obter_proximo_da_fila_prioridade(FilaPrioridade* fila);
bool eh_minha_vez_na_fila(FilaPrioridade* fila, int aviao_id);
void atualizar_prioridade_na_fila(FilaPrioridade* fila, int aviao_id, int nova_prioridade);
void destruir_fila_prioridade(FilaPrioridade* fila);

int verificar_starvation(Aviao* aviao, time_t tempo_atual);
int detectar_deadlock(SimulacaoAeroporto* sim);

void finalizar_simulacao(SimulacaoAeroporto* sim);
void destruir_recursos(RecursosAeroporto* recursos);
void liberar_memoria(SimulacaoAeroporto* sim);
double calcular_tempo_efetivo_simulacao(SimulacaoAeroporto* sim);
int calcular_tempo_espera_efetivo(SimulacaoAeroporto* sim, time_t inicio_espera);
void atualizar_tempo_pausa(SimulacaoAeroporto* sim, bool iniciando_pausa);

void recuperar_deadlock_por_sacrificio(SimulacaoAeroporto* sim);
void verificar_pausa(SimulacaoAeroporto* sim);
const char* estado_para_str(EstadoAviao estado);

void verificar_pausa_simulacao(SimulacaoAeroporto* sim);
void verificar_recursos_orfaos(SimulacaoAeroporto* sim);
bool verificar_avioes_em_alerta(SimulacaoAeroporto* sim);

void atualizar_estado_aviao(SimulacaoAeroporto* sim, Aviao* aviao, EstadoAviao novo_estado);

bool is_safe_state(Banqueiro* banco);
void banker_init_aviao(RecursosAeroporto* recursos, int aviao_id);
int banker_request_resources(RecursosAeroporto* recursos, int aviao_id, int request[]);
int banker_release_resources(RecursosAeroporto* recursos, int aviao_id, int release[]);
void definir_necessidade_operacao(EstadoAviao operacao, int necessidade[N_RESOURCES]);



#endif