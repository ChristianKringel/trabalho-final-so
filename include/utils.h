#include "libs.h"
#include "metrics.h"

#ifndef UTILS_H
#define UTILS_H

void dormir_operacao(int min_ms, int max_ms);
void dormir_operacao_com_pausa(SimulacaoAeroporto* sim, int min_ms, int max_ms);
int gerar_numero_aleatorio(int min, int max);

// Funções de controle de tempo durante pausa
double calcular_tempo_efetivo_simulacao(SimulacaoAeroporto* sim);
void atualizar_tempo_pausa(SimulacaoAeroporto* sim, bool iniciando_pausa);
int calcular_tempo_espera_efetivo(SimulacaoAeroporto* sim, time_t inicio_espera);

// Funções de prioridade e monitoramento
void* monitorar_avioes(void* arg);
int calcular_prioridade_dinamica(Aviao* aviao, time_t tempo_atual, SimulacaoAeroporto* sim);
void verificar_avioes_em_espera(SimulacaoAeroporto* sim);

// Funções de fila de prioridade
void inicializar_fila_prioridade(FilaPrioridade* fila);
void inserir_na_fila_prioridade(FilaPrioridade* fila, int aviao_id, int prioridade);
int remover_da_fila_prioridade(FilaPrioridade* fila, int aviao_id);
int obter_proximo_da_fila_prioridade(FilaPrioridade* fila);
bool eh_minha_vez_na_fila(FilaPrioridade* fila, int aviao_id);
void atualizar_prioridade_na_fila(FilaPrioridade* fila, int aviao_id, int nova_prioridade);
void destruir_fila_prioridade(FilaPrioridade* fila);

bool pode_acessar_recurso_hibrido(FilaPrioridade* fila, int aviao_id, int recursos_disponiveis);
int obter_faixa_prioridade_maxima(FilaPrioridade* fila);
int contar_avioes_alta_prioridade(FilaPrioridade* fila, int prioridade_corte);
bool eh_elegivel_para_acesso_hibrido(FilaPrioridade* fila, int aviao_id, int recursos_disponiveis, int faixa_prioridade);

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

void verificar_pausa_simulacao(SimulacaoAeroporto* sim);
void verificar_recursos_orfaos(SimulacaoAeroporto* sim);
bool verificar_avioes_em_alerta(SimulacaoAeroporto* sim);

bool is_safe_state(Banqueiro* banco);
int banker_request_resources(RecursosAeroporto* recursos, int aviao_id, int request[]);
int banker_release_resources(RecursosAeroporto* recursos, int aviao_id, int release[]);
void definir_necessidade_operacao(EstadoAviao operacao, int necessidade[N_RESOURCES]);
void banker_init_aviao(RecursosAeroporto* recursos, int aviao_id);

#endif