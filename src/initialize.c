#include "initialize.h"
#include "utils.h"

void inicializar_metricas(MetricasSimulacao* metricas)
{
    if (metricas == NULL) {
        return; 
    }

    metricas->total_avioes_criados = 0;
    metricas->avioes_finalizados_sucesso = 0;
    metricas->avioes_falha_starvation = 0;
    metricas->avioes_falha_deadlock = 0;
    metricas->voos_domesticos_total = 0;
    metricas->voos_internacionais_total = 0;
    metricas->operacoes_pouso = 0;
    metricas->operacoes_desembarque = 0;
    metricas->operacoes_decolagem = 0;

    pthread_mutex_init(&metricas->mutex_metricas, NULL);
}

void inicializar_recursos(RecursosAeroporto* recursos, int pistas, int portoes, int torres)
{
    if (recursos == NULL) {
        return; 
    }

    recursos->total_pistas = pistas;
    recursos->pistas_disponiveis = pistas;
    for (int i = 0; i < MAX_PISTAS; i++) {
        recursos->pista_ocupada_por[i] = -1; 
    }
    pthread_mutex_init(&recursos->mutex_pistas, NULL);
    pthread_cond_init(&recursos->cond_pistas, NULL);
    inicializar_fila_prioridade(&recursos->fila_pistas);

    recursos->total_portoes = portoes;
    recursos->portoes_disponiveis = portoes;
    for (int i = 0; i < MAX_PORTOES; i++) {
        recursos->portao_ocupado_por[i] = -1; 
    }
    pthread_mutex_init(&recursos->mutex_portoes, NULL);
    pthread_cond_init(&recursos->cond_portoes, NULL);
    inicializar_fila_prioridade(&recursos->fila_portoes);

    recursos->capacidade_torre = CAPACIDADE_TORRE;
    recursos->slots_torre_disponiveis = CAPACIDADE_TORRE;
    recursos->operacoes_ativas_torre = 0;
    for (int i = 0; i < CAPACIDADE_TORRE; i++) {
        recursos->torre_ocupada_por[i] = -1;
    }
    pthread_mutex_init(&recursos->mutex_torres, NULL);
    pthread_cond_init(&recursos->cond_torres, NULL);
    inicializar_fila_prioridade(&recursos->fila_torres);
}

SimulacaoAeroporto* inicializar_simulacao(int pistas, int portoes, int torres, int tempo_simulacao, int max_avioes)
{
    SimulacaoAeroporto* sim = (SimulacaoAeroporto*)malloc(sizeof(SimulacaoAeroporto));
    if (sim == NULL) {
        return NULL; 
    }
    
    sim->max_avioes = max_avioes; 
    sim->avioes = (Aviao*)malloc(sim->max_avioes * sizeof(Aviao));
    if (sim->avioes == NULL) {
        free(sim);
        return NULL; 
    }

    for (int i = 0; i < sim->max_avioes; i++) {
        sim->avioes[i].id = 0; 
    }

    inicializar_recursos(&sim->recursos, pistas, portoes, torres);
    inicializar_metricas(&sim->metricas);

    sim->tempo_simulacao = tempo_simulacao;
    sim->ativa = 1;

    pthread_mutex_init(&sim->mutex_simulacao, NULL);
    pthread_mutex_init(&sim->mutex_ui_log, NULL);

    sim->tempo_inicio = time(NULL);
    sim->tempo_pausado_total = 0;
    sim->inicio_pausa = 0;

    
    sim->pausado = false;
    pthread_mutex_init(&sim->mutex_pausado, NULL);
    pthread_cond_init(&sim->cond_pausado, NULL);

    return sim;
}