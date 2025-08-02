#include "airport.h"

int solicitar_pista(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_pistas);
    
    while (recursos->pistas_disponiveis <= 0 && sim->ativa) {
        pthread_cond_wait(&recursos->cond_pistas, &recursos->mutex_pistas);
    }
    
    // PODE VIRAR FUNCAO
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
    recursos->pistas_disponiveis--;
    //log_evento_ui(sim, NULL, "Avião %d do tipo %d solicitou uma pista. Pistas disponíveis: %d", id_aviao, tipo, recursos->pistas_disponiveis);
    //printf("Avião %d do tipo %d solicitou uma pista. Pistas disponíveis: %d\n", id_aviao, tipo, recursos->pistas_disponiveis);
    
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
    //recursos->pistas_disponiveis++;
    if (recursos->pistas_disponiveis < recursos->total_pistas) {
        recursos->pistas_disponiveis++;
    }
    //printf("Avião %d liberou uma pista. Pistas disponíveis: %d\n", id_aviao, recursos->pistas_disponiveis);
    
    pthread_cond_signal(&recursos->cond_pistas);
    pthread_mutex_unlock(&recursos->mutex_pistas);
}

int solicitar_portao(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_portoes);
    
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
    recursos->portoes_disponiveis--;
    //printf("Avião %d do tipo %d solicitou um portão. Portões disponíveis: %d\n", id_aviao, tipo, recursos->portoes_disponiveis);
    
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
    //recursos->portoes_disponiveis++;
    //printf("Avião %d liberou um portão. Portões disponíveis: %d\n", id_aviao, recursos->portoes_disponiveis);
    
    pthread_cond_signal(&recursos->cond_portoes);
    pthread_mutex_unlock(&recursos->mutex_portoes);
}

int solicitar_torre(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_torres);
    
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
    recursos->torres_disponiveis++;
    //printf("Avião %d liberou uma torre. Torres disponíveis: %d\n", id_aviao, recursos->torres_disponiveis);
    
    pthread_cond_signal(&recursos->cond_torres);
    pthread_mutex_unlock(&recursos->mutex_torres);
}