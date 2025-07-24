#include "libs.h"

int solicitar_pista(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_pistas);
    
    while (recursos->pistas_disponiveis <= 0) {
        pthread_cond_wait(&recursos->cond_pistas, &recursos->mutex_pistas);
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
    printf("Avião %d do tipo %d solicitou uma pista. Pistas disponíveis: %d\n", id_aviao, tipo, recursos->pistas_disponiveis);
    
    pthread_mutex_unlock(&recursos->mutex_pistas);
    return pista_idx;
}

void liberar_pista(SimulacaoAeroporto* sim, int id_aviao, int pista_idx) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_pistas);
    
    recursos->pista_ocupada_por[pista_idx] = -1;
    recursos->pistas_disponiveis++;
    printf("Avião %d liberou uma pista. Pistas disponíveis: %d\n", id_aviao, recursos->pistas_disponiveis);
    
    pthread_cond_signal(&recursos->cond_pistas);
    pthread_mutex_unlock(&recursos->mutex_pistas);
}

int solicitar_portao(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_portoes);
    
    while (recursos->portoes_disponiveis <= 0) {
        pthread_cond_wait(&recursos->cond_portoes, &recursos->mutex_portoes);
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
    printf("Avião %d do tipo %d solicitou um portão. Portões disponíveis: %d\n", id_aviao, tipo, recursos->portoes_disponiveis);
    
    pthread_mutex_unlock(&recursos->mutex_portoes);
    return portao_idx;
}

void liberar_portao(SimulacaoAeroporto* sim, int id_aviao, int portao_idx) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_portoes);

    recursos->portao_ocupado_por[portao_idx] = -1;
    recursos->portoes_disponiveis++;
    printf("Avião %d liberou um portão. Portões disponíveis: %d\n", id_aviao, recursos->portoes_disponiveis);
    
    pthread_cond_signal(&recursos->cond_portoes);
    pthread_mutex_unlock(&recursos->mutex_portoes);
}

int solicitar_torre(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_torres);
    
    while (recursos->torres_disponiveis <= 0) {
        pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_torres);
    }
    
    recursos->torres_disponiveis--;
    printf("Avião %d do tipo %d solicitou uma torre. Torres disponíveis: %d\n", id_aviao, tipo, recursos->torres_disponiveis);
    
    pthread_mutex_unlock(&recursos->mutex_torres);
    return 0;
}

void liberar_torre(SimulacaoAeroporto* sim, int id_aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_torres);
    
    recursos->torres_disponiveis++;
    printf("Avião %d liberou uma torre. Torres disponíveis: %d\n", id_aviao, recursos->torres_disponiveis);
    
    pthread_cond_signal(&recursos->cond_torres);
    pthread_mutex_unlock(&recursos->mutex_torres);
}

// // =============== FUNÇÕES ESPECÍFICAS POR TIPO DE VOO ===============
// // =============== FUNÇÕES DE POUSO ===============

// int pouso_internacional(Aviao* aviao, SimulacaoAeroporto* sim) {
//     // Voo Internacional: Pista → Torre
//     imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "Solicitando pista");
    
//     // 1. Solicitar pista primeiro

//     if (!solicitar_pista(sim, aviao->id, aviao->tipo)) {
//         imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "FALHA - Não conseguiu pista");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "Pista adquirida, solicitando torre");
    
//     // 2. Solicitar torre depois
//     if (!solicitar_torre(sim, aviao->id, aviao->tipo)) {
//         // Se não conseguir torre, libera a pista
//         liberar_pista(sim, aviao->id, aviao->pista_alocada);
//         imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "FALHA - Não conseguiu torre");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "Recursos adquiridos - POUSANDO");
//     atualizar_estado_aviao(aviao, POUSANDO);
    
//     // Simular tempo de pouso (2-4 segundos)
//     dormir_operacao(2000, 4000);
    
//     // Liberar recursos após pouso
//     liberar_pista(sim, aviao->id, aviao->pista_alocada);
//     liberar_torre(sim, aviao->id);

//     imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "CONCLUÍDO");
//     incrementar_metrica_pouso(&sim->metricas);
    
//     return 1;
// }

// int pouso_domestico(Aviao* aviao, SimulacaoAeroporto* sim) {
//     // Voo Doméstico: Torre → Pista
//     imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "Solicitando torre");
    
//     // 1. Solicitar torre primeiro
//     if (!solicitar_torre(sim, aviao->id, aviao->tipo)) {
//         imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "FALHA - Não conseguiu torre");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "Torre adquirida, solicitando pista");
    
//     // 2. Solicitar pista depois
//     if (!solicitar_pista(sim, aviao->id, aviao->tipo)) {
//         // Se não conseguir pista, libera a torre
//         liberar_torre(sim, aviao->id);
//         imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "FALHA - Não conseguiu pista");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "Recursos adquiridos - POUSANDO");
//     atualizar_estado_aviao(aviao, POUSANDO);
    
//     // Simular tempo de pouso (2-4 segundos)
//     dormir_operacao(2000, 4000);
    
//     // Liberar recursos após pouso
//     liberar_torre(sim, aviao->id);
//     liberar_pista(sim, aviao->id, aviao->pista_alocada);

//     imprimir_status_operacao(aviao->id, aviao->tipo, "POUSO", "CONCLUÍDO");
//     incrementar_metrica_pouso(&sim->metricas);
    
//     return 1;
// }

// // =============== FUNÇÕES DE DESEMBARQUE ===============

// int desembarque_internacional(Aviao* aviao, SimulacaoAeroporto* sim) {
//     // Voo Internacional: Portão → Torre
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "Solicitando portão");
    
//     // 1. Solicitar portão primeiro
//     if (!solicitar_portao(sim, aviao->id, aviao->tipo)) {
//         imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "FALHA - Não conseguiu portão");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "Portão adquirido, solicitando torre");
    
//     // 2. Solicitar torre depois
//     if (!solicitar_torre(sim, aviao->id, aviao->tipo)) {
//         // Se não conseguir torre, libera o portão
//         liberar_portao(sim, aviao->id, aviao->portao_alocado);
//         imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "FALHA - Não conseguiu torre");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "Recursos adquiridos - DESEMBARCANDO");
//     atualizar_estado_aviao(aviao, DESEMBARCANDO);
    
//     // Simular tempo de desembarque (3-6 segundos)
//     dormir_operacao(3000, 6000);
    
//     // Liberar torre primeiro, mantém portão por mais tempo
//     liberar_torre(sim, aviao->id);
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "Torre liberada, finalizando desembarque");
    
//     // Tempo adicional no portão (1-2 segundos)
//     dormir_operacao(1000, 2000);
    
//     // Liberar portão
//     liberar_portao(sim, aviao->id, aviao->portao_alocado);
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "CONCLUÍDO");
//     incrementar_metrica_desembarque(&sim->metricas);
    
//     return 1;
// }

// int desembarque_domestico(Aviao* aviao, SimulacaoAeroporto* sim) {
//     // Voo Doméstico: Torre → Portão
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "Solicitando torre");
    
//     // 1. Solicitar torre primeiro
//     if (!solicitar_torre(sim, aviao->id, aviao->tipo)) {
//         imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "FALHA - Não conseguiu torre");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "Torre adquirida, solicitando portão");
    
//     // 2. Solicitar portão depois
//     if (!solicitar_portao(sim, aviao->id, aviao->tipo)) {
//         // Se não conseguir portão, libera a torre
//         liberar_torre(sim, aviao->id);
//         imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "FALHA - Não conseguiu portão");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "Recursos adquiridos - DESEMBARCANDO");
//     atualizar_estado_aviao(aviao, DESEMBARCANDO);
    
//     // Simular tempo de desembarque (3-6 segundos)
//     dormir_operacao(3000, 6000);
    
//     // Liberar torre primeiro, mantém portão por mais tempo
//     liberar_torre(sim, aviao->id);
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "Torre liberada, finalizando desembarque");
    
//     // Tempo adicional no portão (1-2 segundos)
//     dormir_operacao(1000, 2000);
    
//     // Liberar portão
//     liberar_portao(sim, aviao->id, aviao->portao_alocado);
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DESEMBARQUE", "CONCLUÍDO");
//     incrementar_metrica_desembarque(&sim->metricas);
    
//     return 1;
// }

// // =============== FUNÇÕES DE DECOLAGEM ===============

// int decolagem_internacional(Aviao* aviao, SimulacaoAeroporto* sim) {
//     // Voo Internacional: Portão → Pista → Torre
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "Solicitando portão");
    
//     // 1. Solicitar portão primeiro
//     if (!solicitar_portao(sim, aviao->id, aviao->tipo)) {
//         imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "FALHA - Não conseguiu portão");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "Portão adquirido, solicitando pista");
    
//     // 2. Solicitar pista
//     if (!solicitar_pista(sim, aviao->id, aviao->tipo)) {
//         liberar_portao(sim, aviao->id, aviao->portao_alocado);
//         imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "FALHA - Não conseguiu pista");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "Pista adquirida, solicitando torre");
    
//     // 3. Solicitar torre por último
//     if (!solicitar_torre(sim, aviao->id, aviao->tipo)) {
//         liberar_pista(sim, aviao->id, aviao->pista_alocada);
//         liberar_portao(sim, aviao->id, aviao->portao_alocado);
//         imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "FALHA - Não conseguiu torre");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "Todos recursos adquiridos - DECOLANDO");
//     atualizar_estado_aviao(aviao, DECOLANDO);
    
//     // Simular tempo de decolagem (2-4 segundos)
//     dormir_operacao(2000, 4000);
    
//     // Liberar todos os recursos após decolagem
//     liberar_portao(sim, aviao->id, aviao->portao_alocado);
//     liberar_pista(sim, aviao->id, aviao->pista_alocada);
//     liberar_torre(sim, aviao->id);
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "CONCLUÍDO");
//     incrementar_metrica_decolagem(&sim->metricas);
    
//     return 1;
// }

// int decolagem_domestico(Aviao* aviao, SimulacaoAeroporto* sim) {
//     // Voo Doméstico: Torre → Portão → Pista
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "Solicitando torre");
    
//     // 1. Solicitar torre primeiro
//     if (!solicitar_torre(sim, aviao->id, aviao->tipo)) {
//         imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "FALHA - Não conseguiu torre");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "Torre adquirida, solicitando portão");
    
//     // 2. Solicitar portão
//     if (!solicitar_portao(sim, aviao->id, aviao->tipo)) {
//         liberar_torre(sim, aviao->id);
//         imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "FALHA - Não conseguiu portão");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "Portão adquirido, solicitando pista");
    
//     // 3. Solicitar pista por último
//     if (!solicitar_pista(sim, aviao->id, aviao->tipo)) {
//         liberar_portao(sim, aviao->id, aviao->portao_alocado);
//         liberar_torre(sim, aviao->id);
//         imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "FALHA - Não conseguiu pista");
//         return 0;
//     }
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "Todos recursos adquiridos - DECOLANDO");
//     atualizar_estado_aviao(aviao, DECOLANDO);
    
//     // Simular tempo de decolagem (2-4 segundos)
//     dormir_operacao(2000, 4000);
    
//     // Liberar todos os recursos após decolagem
//     liberar_torre(sim, aviao->id);
//     liberar_portao(sim, aviao->id, aviao->portao_alocado);
//     liberar_pista(sim, aviao->id, aviao->pista_alocada);
    
//     imprimir_status_operacao(aviao->id, aviao->tipo, "DECOLAGEM", "CONCLUÍDO");
//     incrementar_metrica_decolagem(&sim->metricas);
    
//     return 1;
// }