#include "libs.h"

int solicitar_pista(RecursosAeroporto* recursos, int id_aviao, TipoVoo tipo) {
    pthread_mutex_lock(&recursos->mutex_pistas);
    
    while (recursos->pistas_disponiveis <= 0) {
        pthread_cond_wait(&recursos->cond_pistas, &recursos->mutex_pistas);
    }
    
    recursos->pistas_disponiveis--;
    printf("Avião %d do tipo %d solicitou uma pista. Pistas disponíveis: %d\n", id_aviao, tipo, recursos->pistas_disponiveis);
    
    pthread_mutex_unlock(&recursos->mutex_pistas);
    return 0;
}

void liberar_pista(RecursosAeroporto* recursos, int id_aviao) {
    pthread_mutex_lock(&recursos->mutex_pistas);
    
    recursos->pistas_disponiveis++;
    printf("Avião %d liberou uma pista. Pistas disponíveis: %d\n", id_aviao, recursos->pistas_disponiveis);
    
    pthread_cond_signal(&recursos->cond_pistas);
    pthread_mutex_unlock(&recursos->mutex_pistas);
}

int solicitar_portao(RecursosAeroporto* recursos, int id_aviao, TipoVoo tipo) {
    pthread_mutex_lock(&recursos->mutex_portoes);
    
    while (recursos->portoes_disponiveis <= 0) {
        pthread_cond_wait(&recursos->cond_portoes, &recursos->mutex_portoes);
    }
    
    recursos->portoes_disponiveis--;
    printf("Avião %d do tipo %d solicitou um portão. Portões disponíveis: %d\n", id_aviao, tipo, recursos->portoes_disponiveis);
    
    pthread_mutex_unlock(&recursos->mutex_portoes);
    return 0;
}

void liberar_portao(RecursosAeroporto* recursos, int id_aviao) {
    pthread_mutex_lock(&recursos->mutex_portoes);
    
    recursos->portoes_disponiveis++;
    printf("Avião %d liberou um portão. Portões disponíveis: %d\n", id_aviao, recursos->portoes_disponiveis);
    
    pthread_cond_signal(&recursos->cond_portoes);
    pthread_mutex_unlock(&recursos->mutex_portoes);
}

int solicitar_torre(RecursosAeroporto* recursos, int id_aviao, TipoVoo tipo) {
    pthread_mutex_lock(&recursos->mutex_torres);
    
    while (recursos->torres_disponiveis <= 0) {
        pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_torres);
    }
    
    recursos->torres_disponiveis--;
    printf("Avião %d do tipo %d solicitou uma torre. Torres disponíveis: %d\n", id_aviao, tipo, recursos->torres_disponiveis);
    
    pthread_mutex_unlock(&recursos->mutex_torres);
    return 0;
}

void liberar_torre(RecursosAeroporto* recursos, int id_aviao) {
    pthread_mutex_lock(&recursos->mutex_torres);
    
    recursos->torres_disponiveis++;
    printf("Avião %d liberou uma torre. Torres disponíveis: %d\n", id_aviao, recursos->torres_disponiveis);
    
    pthread_cond_signal(&recursos->cond_torres);
    pthread_mutex_unlock(&recursos->mutex_torres);
}