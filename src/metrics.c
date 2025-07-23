// VAI COLETAR OS DADOS DE MÉTRICAS DA SIMULAÇÃO
#include "libs.h"

// =============== FUNÇÕES DE MÉTRICAS ===============
void incrementar_metrica_pouso(MetricasSimulacao* metricas);
void incrementar_metrica_desembarque(MetricasSimulacao* metricas);
void incrementar_metrica_decolagem(MetricasSimulacao* metricas);
void incrementar_aviao_sucesso(MetricasSimulacao* metricas);
void incrementar_aviao_starvation(MetricasSimulacao* metricas);
void incrementar_aviao_deadlock(MetricasSimulacao* metricas);


void incrementar_metrica_pouso(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->operacoes_pouso++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_metrica_desembarque(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->operacoes_desembarque++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_metrica_decolagem(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->operacoes_decolagem++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_aviao_sucesso(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->avioes_finalizados_sucesso++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_aviao_starvation(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->avioes_falha_starvation++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_aviao_deadlock(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->avioes_falha_deadlock++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

