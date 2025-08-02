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

void calcular_metricas_timing(SimulacaoAeroporto* sim, float* tempo_medio_espera, 
                             float* tempo_medio_operacao, int* avioes_ativos) {
    if (sim == NULL || tempo_medio_espera == NULL || 
        tempo_medio_operacao == NULL || avioes_ativos == NULL) {
        return;
    }
    
    float total_tempo_espera = 0.0f;
    float total_tempo_operacao = 0.0f;
    int count_finalizados = 0;
    int count_ativos = 0;
    time_t agora = time(NULL);
    
    for (int i = 0; i < sim->max_avioes; i++) {
        Aviao* aviao = &sim->avioes[i];
        if (aviao->id > 0) {
            // Contabilizar aviões ativos
            if (aviao->estado != FINALIZADO_SUCESSO && 
                aviao->estado != FALHA_STARVATION && 
                aviao->estado != FALHA_DEADLOCK) {
                count_ativos++;
            }
            
            // Calcular tempos para aviões finalizados
            if (aviao->estado == FINALIZADO_SUCESSO) {
                count_finalizados++;
                
                // Tempo de espera (do início até começar operação)
                if (aviao->tempo_inicio_espera > 0) {
                    total_tempo_espera += (float)(aviao->tempo_fim_operacao - aviao->tempo_inicio_espera);
                }
                
                // Tempo total de operação (do início ao fim)
                if (aviao->tempo_criacao > 0 && aviao->tempo_fim_operacao > 0) {
                    total_tempo_operacao += (float)(aviao->tempo_fim_operacao - aviao->tempo_criacao);
                }
            }
        }
    }
    
    *avioes_ativos = count_ativos;
    *tempo_medio_espera = count_finalizados > 0 ? total_tempo_espera / count_finalizados : 0.0f;
    *tempo_medio_operacao = count_finalizados > 0 ? total_tempo_operacao / count_finalizados : 0.0f;
}