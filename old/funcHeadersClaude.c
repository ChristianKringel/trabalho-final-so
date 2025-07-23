#ifndef AIRPORT_SIMULATION_H
#define AIRPORT_SIMULATION_H

#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libs.h"

// =============== FUNÇÕES DE INICIALIZAÇÃO ===============
SimulacaoAeroporto* inicializar_simulacao(int pistas, int portoes, int torres, int tempo_simulacao);
void inicializar_recursos(RecursosAeroporto* recursos, int pistas, int portoes, int torres);
void inicializar_metricas(MetricasSimulacao* metricas);

// =============== FUNÇÕES DE GERENCIAMENTO DE RECURSOS ===============
int solicitar_pista(RecursosAeroporto* recursos, int id_aviao, TipoVoo tipo);
void liberar_pista(RecursosAeroporto* recursos, int id_aviao);
int solicitar_portao(RecursosAeroporto* recursos, int id_aviao, TipoVoo tipo);
void liberar_portao(RecursosAeroporto* recursos, int id_aviao);
int solicitar_torre(RecursosAeroporto* recursos, int id_aviao, TipoVoo tipo);
void liberar_torre(RecursosAeroporto* recursos, int id_aviao);

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

// =============== FUNÇÃO MAIN ===============
int main(int argc, char* argv[]);

#endif // AIRPORT_SIMULATION_H