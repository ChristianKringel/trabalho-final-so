#include "airport.h"
#include "utils.h"

int solicitar_pista(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_pistas);
    
    if (recursos->pistas_disponiveis <= 0) { 
        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_WARNING, "Aguardando pista - Fila de espera");
    }

    while (recursos->pistas_disponiveis <= 0 && sim->ativa) {
        pthread_cond_wait(&recursos->cond_pistas, &recursos->mutex_pistas);
    }
    

    if (!sim->ativa) {
        pthread_mutex_unlock(&recursos->mutex_pistas);
        return -1;
    }
    
    int pista_idx = -1;
    for (int i = 0; i < recursos->total_pistas; i++) {
        if (recursos->pista_ocupada_por[i] == -1) {
            pista_idx = i;
            recursos->pista_ocupada_por[i] = id_aviao;
            break;
        }
    }

    if (pista_idx != -1) {
        recursos->pistas_disponiveis--;
        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, 
                      "Pista %d alocada (%d/%d disponíveis)", 
                      pista_idx, recursos->pistas_disponiveis, recursos->total_pistas);
    } else {
        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_ERROR, "ERRO: Falha na alocação de pista");
    }
    pthread_mutex_unlock(&recursos->mutex_pistas);
    return pista_idx;
}

void liberar_pista(SimulacaoAeroporto* sim, int id_aviao, int pista_idx) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_pistas);

    if (id_aviao > 0 && id_aviao <= sim->max_avioes) {
        sim->avioes[id_aviao - 1].pista_alocada = 0;
    }
    
    recursos->pista_ocupada_por[pista_idx] = -1;

    if (recursos->pistas_disponiveis < recursos->total_pistas) {
        recursos->pistas_disponiveis++;
    }

    log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, 
                  "Pista %d liberada (%d/%d disponíveis)", 
                  pista_idx, recursos->pistas_disponiveis, recursos->total_pistas);
    pthread_cond_broadcast(&recursos->cond_pistas);
    pthread_mutex_unlock(&recursos->mutex_pistas);
}

int solicitar_portao(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_portoes);

    if (recursos->portoes_disponiveis <= 0) {
        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_WARNING, "Aguardando portão - Fila de espera");
    }
    
    while (recursos->portoes_disponiveis <= 0 && sim->ativa) {
        pthread_cond_wait(&recursos->cond_portoes, &recursos->mutex_portoes);
    }
    
    if (!sim->ativa) {
        pthread_mutex_unlock(&recursos->mutex_portoes);
        return -1;
    }
    
    int portao_idx = -1;
    for (int i = 0; i < recursos->total_portoes; i++) {
        if (recursos->portao_ocupado_por[i] == -1) {
            portao_idx = i;
            recursos->portao_ocupado_por[i] = id_aviao;
            break;
        }
    }
    if (portao_idx != -1) {
        recursos->portoes_disponiveis--;
        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Portão %d alocado (%d/%d disponíveis)", portao_idx, recursos->portoes_disponiveis, recursos->total_portoes);
    }
    
    pthread_mutex_unlock(&recursos->mutex_portoes);
    return portao_idx;
}

void liberar_portao(SimulacaoAeroporto* sim, int id_aviao, int portao_idx) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_portoes);

    if (id_aviao > 0 && id_aviao <= sim->max_avioes) {
        sim->avioes[id_aviao - 1].portao_alocado = 0;
    }

    recursos->portao_ocupado_por[portao_idx] = -1;

    if (recursos->portoes_disponiveis < recursos->total_portoes) {
        recursos->portoes_disponiveis++;
    }

    log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Portão %d liberado (%d/%d disponíveis)", portao_idx, recursos->portoes_disponiveis, recursos->total_portoes);


    pthread_cond_broadcast(&recursos->cond_portoes);
    pthread_mutex_unlock(&recursos->mutex_portoes);
}

int solicitar_torre(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_torres);
    
    if (recursos->torres_disponiveis <= 0) { 
        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_WARNING, "Aguardando torre de controle - Fila de espera");
    }

    while (recursos->torres_disponiveis <= 0 && sim->ativa) {
        pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_torres);
    }
    
    if (!sim->ativa) {
        pthread_mutex_unlock(&recursos->mutex_torres);
        return -1;
    }
    
    recursos->torres_disponiveis--;
    
    if (id_aviao > 0 && id_aviao <= sim->max_avioes) {
        sim->avioes[id_aviao - 1].torre_alocada = 1;
    }
    log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, 
                  "Torre alocada (%d/%d disponíveis)", 
                  recursos->torres_disponiveis, recursos->total_torres);
    
    //printf("Avião %d do tipo %d solicitou uma torre. Torres disponíveis: %d\n", id_aviao, tipo, recursos->torres_disponiveis);
    
    pthread_mutex_unlock(&recursos->mutex_torres);
    return 0;
}

void liberar_torre(SimulacaoAeroporto* sim, int id_aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_torres);
    
    if (id_aviao > 0 && id_aviao <= sim->max_avioes) {
        sim->avioes[id_aviao - 1].torre_alocada = 0;
    }

    if (recursos->torres_disponiveis < recursos->total_torres) {
        recursos->torres_disponiveis++;
    }
    log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Torre liberada (%d/%d disponíveis)", recursos->torres_disponiveis, recursos->total_torres);
    
    pthread_cond_broadcast(&recursos->cond_torres);
    
    if (recursos->fila_torres.tamanho > 0) {
        pthread_mutex_unlock(&recursos->mutex_torres);
        usleep(1000);
        pthread_mutex_lock(&recursos->mutex_torres);
        pthread_cond_broadcast(&recursos->cond_torres);
    }
    
    pthread_mutex_unlock(&recursos->mutex_torres);
}

// =============== FUNÇÕES COM SISTEMA DE PRIORIDADE ===============
int solicitar_pista_com_prioridade(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    
    time_t agora = time(NULL);
    int prioridade = calcular_prioridade_dinamica(aviao, agora);
    
    pthread_mutex_lock(&recursos->mutex_pistas);
    
    if (recursos->pistas_disponiveis <= 0) {
        log_evento_ui(sim, aviao, LOG_WARNING, "Aguardando pista - Fila de espera (prioridade: %d)", prioridade);
        inserir_na_fila_prioridade(&recursos->fila_pistas, aviao->id, prioridade);
        
        while (recursos->pistas_disponiveis <= 0 && sim->ativa) {
            pthread_cond_wait(&recursos->cond_pistas, &recursos->mutex_pistas);
            
            // Quando uma pista é liberada, verifica se este avião tem a maior prioridade
            if (recursos->pistas_disponiveis > 0) {
                int proximo_id = remover_da_fila_prioridade(&recursos->fila_pistas);
                if (proximo_id == aviao->id) {
                    break; // É a vez deste avião
                } else if (proximo_id != -1) {
                    // Reinsere o avião que foi removido mas não é este
                    inserir_na_fila_prioridade(&recursos->fila_pistas, proximo_id, 
                        calcular_prioridade_dinamica(&sim->avioes[proximo_id-1], time(NULL)));
                    
                    // Reinsere este avião com prioridade atualizada
                    inserir_na_fila_prioridade(&recursos->fila_pistas, aviao->id, 
                        calcular_prioridade_dinamica(aviao, time(NULL)));
                }
            }
        }
    }
    
    if (!sim->ativa) {
        pthread_mutex_unlock(&recursos->mutex_pistas);
        return -1;
    }
    
    int pista_idx = -1;
    for (int i = 0; i < recursos->total_pistas; i++) {
        if (recursos->pista_ocupada_por[i] == -1) {
            pista_idx = i;
            recursos->pista_ocupada_por[i] = aviao->id;
            break;
        }
    }

    if (pista_idx != -1) {
        recursos->pistas_disponiveis--;
        log_evento_ui(sim, aviao, LOG_RESOURCE, 
                      "Pista %d alocada (%d/%d disponíveis) [Prioridade: %d]", 
                      pista_idx, recursos->pistas_disponiveis, recursos->total_pistas, prioridade);
    }
    
    pthread_mutex_unlock(&recursos->mutex_pistas);
    return pista_idx;
}

int solicitar_portao_com_prioridade(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    
    time_t agora = time(NULL);
    int prioridade = calcular_prioridade_dinamica(aviao, agora);
    
    pthread_mutex_lock(&recursos->mutex_portoes);
    
    if (recursos->portoes_disponiveis <= 0) {
        log_evento_ui(sim, aviao, LOG_WARNING, "Aguardando portão - Fila de espera (prioridade: %d)", prioridade);
        inserir_na_fila_prioridade(&recursos->fila_portoes, aviao->id, prioridade);
        
        while (recursos->portoes_disponiveis <= 0 && sim->ativa) {
            pthread_cond_wait(&recursos->cond_portoes, &recursos->mutex_portoes);
            
            // Quando um portão é liberado, verifica se este avião tem a maior prioridade
            if (recursos->portoes_disponiveis > 0) {
                int proximo_id = remover_da_fila_prioridade(&recursos->fila_portoes);
                if (proximo_id == aviao->id) {
                    break; // É a vez deste avião
                } else if (proximo_id != -1) {
                    // Reinsere o avião que foi removido mas não é este
                    inserir_na_fila_prioridade(&recursos->fila_portoes, proximo_id, 
                        calcular_prioridade_dinamica(&sim->avioes[proximo_id-1], time(NULL)));
                    
                    // Reinsere este avião com prioridade atualizada
                    inserir_na_fila_prioridade(&recursos->fila_portoes, aviao->id, 
                        calcular_prioridade_dinamica(aviao, time(NULL)));
                }
            }
        }
    }
    
    if (!sim->ativa) {
        pthread_mutex_unlock(&recursos->mutex_portoes);
        return -1;
    }
    
    int portao_idx = -1;
    for (int i = 0; i < recursos->total_portoes; i++) {
        if (recursos->portao_ocupado_por[i] == -1) {
            portao_idx = i;
            recursos->portao_ocupado_por[i] = aviao->id;
            break;
        }
    }
    
    if (portao_idx != -1) {
        recursos->portoes_disponiveis--;
        log_evento_ui(sim, aviao, LOG_RESOURCE, 
                      "Portão %d alocado (%d/%d disponíveis) [Prioridade: %d]", 
                      portao_idx, recursos->portoes_disponiveis, recursos->total_portoes, prioridade);
    }
    
    pthread_mutex_unlock(&recursos->mutex_portoes);
    return portao_idx;
}

int solicitar_torre_com_prioridade(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    
    time_t agora = time(NULL);
    int prioridade = calcular_prioridade_dinamica(aviao, agora);
    
    pthread_mutex_lock(&recursos->mutex_torres);
    
    if (recursos->torres_disponiveis <= 0) {
        log_evento_ui(sim, aviao, LOG_WARNING, "Aguardando torre de controle - Fila de espera (prioridade: %d)", prioridade);
        inserir_na_fila_prioridade(&recursos->fila_torres, aviao->id, prioridade);
        
        while (recursos->torres_disponiveis <= 0 && sim->ativa) {
            pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_torres);
            
            // Quando uma torre é liberada, verifica se este avião tem a maior prioridade
            if (recursos->torres_disponiveis > 0) {
                int proximo_id = remover_da_fila_prioridade(&recursos->fila_torres);
                if (proximo_id == aviao->id) {
                    break; // É a vez deste avião
                } else if (proximo_id != -1) {
                    // Reinsere o avião que foi removido mas não é este
                    inserir_na_fila_prioridade(&recursos->fila_torres, proximo_id, 
                        calcular_prioridade_dinamica(&sim->avioes[proximo_id-1], time(NULL)));
                    
                    // Reinsere este avião com prioridade atualizada
                    inserir_na_fila_prioridade(&recursos->fila_torres, aviao->id, 
                        calcular_prioridade_dinamica(aviao, time(NULL)));
                }
            }
        }
    }
    
    if (!sim->ativa) {
        pthread_mutex_unlock(&recursos->mutex_torres);
        return -1;
    }
    
    recursos->torres_disponiveis--;
    
    if (aviao->id > 0 && aviao->id <= sim->max_avioes) {
        sim->avioes[aviao->id - 1].torre_alocada = 1;
    }
    
    log_evento_ui(sim, aviao, LOG_RESOURCE, 
                  "Torre alocada (%d/%d disponíveis) [Prioridade: %d]", 
                  recursos->torres_disponiveis, recursos->total_torres, prioridade);
    
    pthread_mutex_unlock(&recursos->mutex_torres);
    return 0;
}