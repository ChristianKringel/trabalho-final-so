#include "airport.h"
#include "utils.h"

void liberar_pista(SimulacaoAeroporto* sim, int id_aviao, int pista_idx) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};

    pthread_mutex_lock(&recursos->mutex_banco);
    // --- SEÇÃO DO BANQUEIRO ---
    recurso_liberado[RECURSO_PISTA] = 1;
    banker_release_resources(recursos, id_aviao - 1, recurso_liberado);
    // -------------------------

    //pthread_mutex_lock(&recursos->mutex_pistas);
    

    if (id_aviao > 0 && id_aviao <= sim->max_avioes) {
        sim->avioes[id_aviao - 1].pista_alocada = -1;
    }
    
    recursos->pista_ocupada_por[pista_idx] = -1;

    if (recursos->pistas_disponiveis < recursos->total_pistas) {
        recursos->pistas_disponiveis++;
    }

    log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Pista %d liberada (%d/%d disponíveis)", pista_idx, recursos->pistas_disponiveis, recursos->total_pistas);
    
    pthread_cond_broadcast(&recursos->cond_banco); 
    //pthread_mutex_unlock(&recursos->mutex_pistas);
    pthread_mutex_unlock(&recursos->mutex_banco);
}

void liberar_portao(SimulacaoAeroporto* sim, int id_aviao, int portao_idx) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};
    pthread_mutex_lock(&recursos->mutex_banco);
    // --- SEÇÃO DO BANQUEIRO ---
    recurso_liberado[RECURSO_PORTAO] = 1;
    banker_release_resources(recursos, id_aviao - 1, recurso_liberado);

    //pthread_mutex_lock(&recursos->mutex_portoes);
    

    if (id_aviao > 0 && id_aviao <= sim->max_avioes) {
        sim->avioes[id_aviao - 1].portao_alocado = 0;
    }

    recursos->portao_ocupado_por[portao_idx] = -1;

    if (recursos->portoes_disponiveis < recursos->total_portoes) {
        recursos->portoes_disponiveis++;
    }

    log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Portão %d liberado (%d/%d disponíveis)", portao_idx, recursos->portoes_disponiveis, recursos->total_portoes);

    pthread_cond_broadcast(&recursos->cond_banco);
    //pthread_mutex_unlock(&recursos->mutex_portoes);
    pthread_mutex_unlock(&recursos->mutex_banco);
}

void liberar_torre(SimulacaoAeroporto* sim, int id_aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};
    pthread_mutex_lock(&recursos->mutex_banco);
    // --- SEÇÃO DO BANQUEIRO ---
    recurso_liberado[RECURSO_TORRE] = 1;
    banker_release_resources(recursos, id_aviao - 1, recurso_liberado);
    //pthread_mutex_lock(&recursos->mutex_torres);
    
    
    if (id_aviao > 0 && id_aviao <= sim->max_avioes) {
        if (sim->avioes[id_aviao - 1].torre_alocada == 0) {
            log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_WARNING, "Tentativa de liberar torre não alocada");
            pthread_mutex_unlock(&recursos->mutex_torres);
            return;
        }
        
        // Encontra e libera o slot específico do avião
        int slot_liberado = sim->avioes[id_aviao - 1].torre_alocada - 1; // -1 pois foi +1 na alocação
        if (slot_liberado >= 0 && slot_liberado < recursos->capacidade_torre) {
            recursos->torre_ocupada_por[slot_liberado] = -1;
        }
        
        sim->avioes[id_aviao - 1].torre_alocada = 0;
    }

    recursos->slots_torre_disponiveis++;
    recursos->operacoes_ativas_torre--;
    
    log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, 
                 "Torre liberada (%d/%d slots disponíveis)", 
                 recursos->slots_torre_disponiveis, recursos->capacidade_torre);
    
    pthread_cond_broadcast(&recursos->cond_banco);
    //pthread_mutex_unlock(&recursos->mutex_torres);
    pthread_mutex_unlock(&recursos->mutex_banco);
}
// =============== FUNÇÕES AUXILIARES PARA USO DA TORRE ===============

void liberar_uso_torre(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    
    pthread_mutex_lock(&recursos->mutex_torres);
    
    if (aviao->torre_alocada > 0) {
        int slot = aviao->torre_alocada - 1;
        recursos->torre_ocupada_por[slot] = -1;
        aviao->torre_alocada = 0;
    }
    
    recursos->slots_torre_disponiveis++;
    recursos->operacoes_ativas_torre--;
    pthread_cond_signal(&recursos->cond_torres);
    pthread_mutex_unlock(&recursos->mutex_torres);
}

int solicitar_recursos_com_espera(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    int pedido[N_RESOURCES];

    // 1. Define o que o avião precisa com base no seu estado atual
    definir_necessidade_operacao(aviao->estado, pedido);

    pthread_mutex_lock(&recursos->mutex_banco);

    int resultado_banqueiro = -1;
    bool adicionado_nas_filas = false;
    
    // 2. Loop de espera: continua tentando até o banqueiro aprovar o pedido
    while ((resultado_banqueiro = banker_request_resources(recursos, aviao->id - 1, pedido)) != 0) {
        // Adicionar às filas de prioridade na primeira tentativa que falhar
        if (!adicionado_nas_filas) {
            int prioridade = calcular_prioridade_dinamica(aviao, time(NULL), sim);
            
            if (pedido[RECURSO_PISTA] > 0) {
                inserir_na_fila_prioridade(&recursos->fila_pistas, aviao->id, prioridade);
            }
            if (pedido[RECURSO_PORTAO] > 0) {
                inserir_na_fila_prioridade(&recursos->fila_portoes, aviao->id, prioridade);
            }
            if (pedido[RECURSO_TORRE] > 0) {
                inserir_na_fila_prioridade(&recursos->fila_torres, aviao->id, prioridade);
            }
            adicionado_nas_filas = true;
        }
        
        // Aguardar nas filas de prioridade específicas
        if (resultado_banqueiro == -5) { // Não é sua vez na fila de pistas
            pthread_cond_wait(&recursos->fila_pistas.cond, &recursos->mutex_banco);
        } else if (resultado_banqueiro == -6) { // Não é sua vez na fila de portões
            pthread_cond_wait(&recursos->fila_portoes.cond, &recursos->mutex_banco);
        } else if (resultado_banqueiro == -7) { // Não é sua vez na fila de torres
            pthread_cond_wait(&recursos->fila_torres.cond, &recursos->mutex_banco);
        } else {
            // Para outros tipos de erro (recursos insuficientes, estado inseguro, etc.)
            pthread_cond_wait(&recursos->cond_banco, &recursos->mutex_banco);
        }

        // Se a simulação foi desativada enquanto esperávamos, saímos.
        if (!sim->ativa) {
            pthread_mutex_unlock(&recursos->mutex_banco);
            return -1; // Falha por término da simulação
        }
        
        // Atualizar prioridade dinamicamente a cada tentativa para evitar starvation
        if (adicionado_nas_filas) {
            int nova_prioridade = calcular_prioridade_dinamica(aviao, time(NULL), sim);
            
            if (pedido[RECURSO_PISTA] > 0) {
                atualizar_prioridade_na_fila(&recursos->fila_pistas, aviao->id, nova_prioridade);
            }
            if (pedido[RECURSO_PORTAO] > 0) {
                atualizar_prioridade_na_fila(&recursos->fila_portoes, aviao->id, nova_prioridade);
            }
            if (pedido[RECURSO_TORRE] > 0) {
                atualizar_prioridade_na_fila(&recursos->fila_torres, aviao->id, nova_prioridade);
            }
        }
    }
    
    // 3. Garantir que o avião seja removido das filas após conseguir recursos
    // (redundante com a remoção feita em banker_request_resources, mas por segurança)
    if (adicionado_nas_filas) {
        if (pedido[RECURSO_PISTA] > 0) {
            remover_da_fila_prioridade(&recursos->fila_pistas, aviao->id);
        }
        if (pedido[RECURSO_PORTAO] > 0) {
            remover_da_fila_prioridade(&recursos->fila_portoes, aviao->id);
        }
        if (pedido[RECURSO_TORRE] > 0) {
            remover_da_fila_prioridade(&recursos->fila_torres, aviao->id);
        }
    }
    
    //pthread_mutex_unlock(&recursos->mutex_banco);
    
    // Aloca Pista, se pedido (com mutex específico)
    if (pedido[RECURSO_PISTA] > 0) {
        //pthread_mutex_lock(&recursos->mutex_pistas);
        for (int i = 0; i < recursos->total_pistas; i++) {
            if (recursos->pista_ocupada_por[i] == -1) {
                recursos->pista_ocupada_por[i] = aviao->id;
                aviao->pista_alocada = i;
                recursos->pistas_disponiveis--;
                break;
            }
        }
        //pthread_mutex_unlock(&recursos->mutex_pistas);
    }
    
    // Aloca Portão, se pedido (com mutex específico)
    if (pedido[RECURSO_PORTAO] > 0) {
        //pthread_mutex_lock(&recursos->mutex_portoes);
        for (int i = 0; i < recursos->total_portoes; i++) {
            if (recursos->portao_ocupado_por[i] == -1) {
                recursos->portao_ocupado_por[i] = aviao->id;
                aviao->portao_alocado = i;
                recursos->portoes_disponiveis--;
                break;
            }
        }
        //pthread_mutex_unlock(&recursos->mutex_portoes);
    }
    
    // Aloca Torre, se pedido (com mutex específico)
    if (pedido[RECURSO_TORRE] > 0) {
        //pthread_mutex_lock(&recursos->mutex_torres);
        for (int i = 0; i < recursos->capacidade_torre; i++) {
            if (recursos->torre_ocupada_por[i] == -1) {
                recursos->torre_ocupada_por[i] = aviao->id;
                aviao->torre_alocada = i + 1; // +1 para diferenciar de 0
                recursos->slots_torre_disponiveis--;
                break;
            }
        }
        //pthread_mutex_unlock(&recursos->mutex_torres);
    }
    
    pthread_mutex_unlock(&recursos->mutex_banco);
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Recursos alocados via Banqueiro.");
    return 0; // Sucesso
}

// Libera TODOS os recursos de um avião
void liberar_todos_recursos(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};
    
    // =============== SEÇÃO DO BANQUEIRO ===============
    // Determina quais recursos estão sendo liberados baseado no que o avião tinha alocado
    if (aviao->pista_alocada >= 0) {
        recurso_liberado[RECURSO_PISTA] = 1;
    }
    if (aviao->portao_alocado >= 0) {
        recurso_liberado[RECURSO_PORTAO] = 1;
    }
    if (aviao->torre_alocada > 0) {
        recurso_liberado[RECURSO_TORRE] = 1;
    }
    
    // Libera recursos no banqueiro se algum estava alocado
    bool algum_recurso_liberado = false;
    for (int i = 0; i < N_RESOURCES; i++) {
        if (recurso_liberado[i] > 0) {
            algum_recurso_liberado = true;
            break;
        }
    }
    
    if (algum_recurso_liberado) {
        banker_release_resources(recursos, aviao->id - 1, recurso_liberado);
    }
    
    // Libera torre se alocada
    if (aviao->torre_alocada > 0) {
        pthread_mutex_lock(&recursos->mutex_torres);
        int slot = aviao->torre_alocada - 1;
        if (slot >= 0 && slot < recursos->capacidade_torre) {
            recursos->torre_ocupada_por[slot] = -1;
        }
        recursos->slots_torre_disponiveis++;
        recursos->operacoes_ativas_torre--;
        aviao->torre_alocada = 0;
        pthread_cond_broadcast(&recursos->cond_torres);
        pthread_mutex_unlock(&recursos->mutex_torres);
    }
    
    // Libera pista se alocada
    if (aviao->pista_alocada >= 0) {
        pthread_mutex_lock(&recursos->mutex_pistas);
        recursos->pista_ocupada_por[aviao->pista_alocada] = -1;
        recursos->pistas_disponiveis++;
        aviao->pista_alocada = -1;
        pthread_cond_broadcast(&recursos->cond_pistas);
        pthread_mutex_unlock(&recursos->mutex_pistas);
    }
    
    // Libera portão se alocado
    if (aviao->portao_alocado >= 0) {
        pthread_mutex_lock(&recursos->mutex_portoes);
        recursos->portao_ocupado_por[aviao->portao_alocado] = -1;
        recursos->portoes_disponiveis++;
        aviao->portao_alocado = -1;
        pthread_cond_broadcast(&recursos->cond_portoes);
        pthread_mutex_unlock(&recursos->mutex_portoes);
    }
    
    // =============== BROADCAST DO BANQUEIRO ===============
    if (algum_recurso_liberado) {
        pthread_mutex_lock(&recursos->mutex_banco);
        pthread_cond_broadcast(&recursos->cond_banco);
        pthread_mutex_unlock(&recursos->mutex_banco);
    }
    
    log_evento_ui(sim, aviao, LOG_RESOURCE, "TODOS os recursos liberados");
}