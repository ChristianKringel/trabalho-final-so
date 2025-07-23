#include "libs.h"

// Inicializacao das metricas
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
    pthread_mutex_init(&recursos->mutex_pistas, NULL);
    pthread_cond_init(&recursos->cond_pistas, NULL);

    recursos->total_portoes = portoes;
    recursos->portoes_disponiveis = portoes;
    pthread_mutex_init(&recursos->mutex_portoes, NULL);
    pthread_cond_init(&recursos->cond_portoes, NULL);

    recursos->total_torres = torres;
    recursos->torres_disponiveis = torres;
    pthread_mutex_init(&recursos->mutex_torres, NULL);
    pthread_cond_init(&recursos->cond_torres, NULL);
}