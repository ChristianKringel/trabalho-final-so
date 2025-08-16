#include "metrics.h"

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

void incrementar_aviao_falha_starvation(MetricasSimulacao* metricas) {
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

void incrementar_contador_avioes_criados(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->total_avioes_criados++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_voos_domesticos(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->voos_domesticos_total++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_voos_internacionais(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->voos_internacionais_total++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}