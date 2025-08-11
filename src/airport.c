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

    log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Pista %d liberada (%d/%d disponíveis)", pista_idx, recursos->pistas_disponiveis, recursos->total_pistas);
    
    pthread_cond_signal(&recursos->cond_pistas);
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

    pthread_cond_signal(&recursos->cond_portoes);
    pthread_mutex_unlock(&recursos->mutex_portoes);
}

int solicitar_torre(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_torres);
    
    if (recursos->slots_torre_disponiveis <= 0) { 
        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_WARNING, "Aguardando torre de controle - Fila de espera");
    }

    while (recursos->slots_torre_disponiveis <= 0 && sim->ativa) {
        pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_torres);
    }
    
    if (!sim->ativa) {
        pthread_mutex_unlock(&recursos->mutex_torres);
        return -1;
    }
    
    recursos->slots_torre_disponiveis--;
    recursos->operacoes_ativas_torre++;
    
    // Encontra um slot livre para registrar o avião
    for (int i = 0; i < CAPACIDADE_TORRE; i++) {
        if (recursos->torre_ocupada_por[i] == -1) {
            recursos->torre_ocupada_por[i] = id_aviao;
            if (id_aviao > 0 && id_aviao <= sim->max_avioes) {
                sim->avioes[id_aviao - 1].torre_alocada = i + 1;
            }
            break;
        }
    }
    
    log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, 
                  "Torre alocada (%d/%d slots disponíveis)", 
                  recursos->slots_torre_disponiveis, recursos->capacidade_torre);
    
    pthread_mutex_unlock(&recursos->mutex_torres);
    return 0;
}

void liberar_torre(SimulacaoAeroporto* sim, int id_aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    pthread_mutex_lock(&recursos->mutex_torres);
    
    // Verifica se o avião realmente tem a torre alocada
    if (id_aviao > 0 && id_aviao <= sim->max_avioes) {
        if (sim->avioes[id_aviao - 1].torre_alocada == 0) {
            log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_WARNING, 
                         "Tentativa de liberar torre não alocada");
            pthread_mutex_unlock(&recursos->mutex_torres);
            return;
        }
        
        // Encontra e libera o slot específico do avião
        int slot_liberado = sim->avioes[id_aviao - 1].torre_alocada - 1; // -1 pois foi +1 na alocação
        if (slot_liberado >= 0 && slot_liberado < CAPACIDADE_TORRE) {
            recursos->torre_ocupada_por[slot_liberado] = -1;
        }
        
        sim->avioes[id_aviao - 1].torre_alocada = 0;
    }

    recursos->slots_torre_disponiveis++;
    recursos->operacoes_ativas_torre--;
    
    log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, 
                 "Torre liberada (%d/%d slots disponíveis)", 
                 recursos->slots_torre_disponiveis, recursos->capacidade_torre);
    
    pthread_cond_signal(&recursos->cond_torres);
    pthread_mutex_unlock(&recursos->mutex_torres);
}

// =============== FUNÇÕES COM SISTEMA DE PRIORIDADE ===============
int solicitar_pista_com_prioridade(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    
    time_t agora = time(NULL);
    int prioridade = calcular_prioridade_dinamica(aviao, agora);
    
    pthread_mutex_lock(&recursos->mutex_pistas);

    bool ja_na_fila = false; 
    for (int i = 0; i < recursos->fila_pistas.tamanho; i++) {
        if (recursos->fila_pistas.avioes_ids[i] == aviao->id) {
            ja_na_fila = true;
            break;
        }
    }
    
    if (!ja_na_fila) {
        inserir_na_fila_prioridade(&recursos->fila_pistas, aviao->id, prioridade);
        log_evento_ui(sim, aviao, LOG_INFO, "Aguardando pista - Fila de espera (prioridade: %d)", prioridade);
    }

    while ((recursos->pistas_disponiveis <= 0 || !eh_minha_vez_na_fila(&recursos->fila_pistas, aviao->id)) && sim->ativa) {
        int nova_prioridade = calcular_prioridade_dinamica(aviao, time(NULL));
        if (nova_prioridade != aviao->prioridade_dinamica) {
            aviao->prioridade_dinamica = nova_prioridade;
            atualizar_prioridade_na_fila(&recursos->fila_pistas, aviao->id, nova_prioridade);
        }
        pthread_cond_wait(&recursos->cond_pistas, &recursos->mutex_pistas);

        if (!sim->ativa) {
            pthread_mutex_unlock(&recursos->mutex_pistas);
            return -1;
        }

    }

    if (recursos->pistas_disponiveis > 0 && eh_minha_vez_na_fila(&recursos->fila_pistas, aviao->id)) {
        
        remover_da_fila_prioridade(&recursos->fila_pistas, aviao->id);

        for (int i = 0; i < recursos->total_pistas; i++) {
            if (recursos->pista_ocupada_por[i] == -1) {
                recursos->pista_ocupada_por[i] = aviao->id;
                aviao->pista_alocada = i;
                recursos->pistas_disponiveis--;
                
                log_evento_ui(sim, aviao, LOG_INFO, "Pista %d alocada (%d/%d disponíveis) [Prioridade: %d]", i, recursos->pistas_disponiveis, recursos->total_pistas, aviao->prioridade_dinamica);
                
                pthread_mutex_unlock(&recursos->mutex_pistas);
                return i;
            }
        }
    }
    pthread_mutex_unlock(&recursos->mutex_pistas);
    return -1;
}

int solicitar_portao_com_prioridade(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    
    time_t agora = time(NULL);
    aviao->prioridade_dinamica = calcular_prioridade_dinamica(aviao, agora);
    
    pthread_mutex_lock(&recursos->mutex_portoes);
    
    // Adiciona na fila se ainda não estiver
    bool ja_na_fila = false;
    for (int i = 0; i < recursos->fila_portoes.tamanho; i++) {
        if (recursos->fila_portoes.avioes_ids[i] == aviao->id) {
            ja_na_fila = true;
            break;
        }
    }
    
    if (!ja_na_fila) {
        inserir_na_fila_prioridade(&recursos->fila_portoes, aviao->id, aviao->prioridade_dinamica);
        log_evento_ui(sim, aviao, LOG_WARNING, "Aguardando portão - Fila de espera (prioridade: %d)", aviao->prioridade_dinamica);
    }
    
    // CONDIÇÃO FORTALECIDA: Verifica disponibilidade E se é o próximo na fila
    while ((recursos->portoes_disponiveis <= 0 || !eh_minha_vez_na_fila(&recursos->fila_portoes, aviao->id)) && sim->ativa) {
        int nova_prioridade = calcular_prioridade_dinamica(aviao, time(NULL));
        if (nova_prioridade != aviao->prioridade_dinamica) {
            aviao->prioridade_dinamica = nova_prioridade;
            atualizar_prioridade_na_fila(&recursos->fila_portoes, aviao->id, nova_prioridade);
        }
        pthread_cond_wait(&recursos->cond_portoes, &recursos->mutex_portoes);

        if (!sim->ativa) {
            pthread_mutex_unlock(&recursos->mutex_portoes);
            return -1;
        }
    }
    
    // Se chegou até aqui, tem recurso disponível E é o próximo na fila
    if (recursos->portoes_disponiveis > 0 && eh_minha_vez_na_fila(&recursos->fila_portoes, aviao->id)) {
        // Remove da fila de prioridade
        remover_da_fila_prioridade(&recursos->fila_portoes, aviao->id);
        
        // Aloca o portão e retorna o índice
        for (int i = 0; i < recursos->total_portoes; i++) {
            if (recursos->portao_ocupado_por[i] == -1) {
                recursos->portao_ocupado_por[i] = aviao->id;
                aviao->portao_alocado = i;
                recursos->portoes_disponiveis--;
                
                log_evento_ui(sim, aviao, LOG_RESOURCE, "Portão %d alocado (%d/%d disponíveis) [Prioridade: %d]", i, recursos->portoes_disponiveis, recursos->total_portoes, aviao->prioridade_dinamica);
                
                pthread_mutex_unlock(&recursos->mutex_portoes);
                return i; // Retorna o índice do portão alocado
            }
        }
    }
    
    pthread_mutex_unlock(&recursos->mutex_portoes);
    return -1; // Falha na alocação
}

int solicitar_torre_com_prioridade(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    
    time_t agora = time(NULL);
    aviao->prioridade_dinamica = calcular_prioridade_dinamica(aviao, agora);
    
    pthread_mutex_lock(&recursos->mutex_torres);
    
    // Adiciona na fila se ainda não estiver
    bool ja_na_fila = false;
    for (int i = 0; i < recursos->fila_torres.tamanho; i++) {
        if (recursos->fila_torres.avioes_ids[i] == aviao->id) {
            ja_na_fila = true;
            break;
        }
    }
    
    if (!ja_na_fila) {
        inserir_na_fila_prioridade(&recursos->fila_torres, aviao->id, aviao->prioridade_dinamica);
        log_evento_ui(sim, aviao, LOG_WARNING, "Aguardando torre de controle - Fila de espera (prioridade: %d)", aviao->prioridade_dinamica);
    }
    
    // Loop de espera - nova lógica para recurso compartilhado
    while ((recursos->slots_torre_disponiveis <= 0 || !eh_minha_vez_na_fila(&recursos->fila_torres, aviao->id)) && sim->ativa) {
        // Atualiza prioridade dinâmica enquanto espera
        int nova_prioridade = calcular_prioridade_dinamica(aviao, time(NULL));
        if (nova_prioridade != aviao->prioridade_dinamica) {
            aviao->prioridade_dinamica = nova_prioridade;
            atualizar_prioridade_na_fila(&recursos->fila_torres, aviao->id, nova_prioridade);
        }
        
        pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_torres);
        
        if (!sim->ativa) {
            remover_da_fila_prioridade(&recursos->fila_torres, aviao->id);
            pthread_mutex_unlock(&recursos->mutex_torres);
            return -1;
        }
    }
    
    // Se chegou até aqui, tem slot disponível E é o próximo na fila
    if (recursos->slots_torre_disponiveis > 0 && eh_minha_vez_na_fila(&recursos->fila_torres, aviao->id)) {
        remover_da_fila_prioridade(&recursos->fila_torres, aviao->id);
        
        // Aloca um slot da torre
        recursos->slots_torre_disponiveis--;
        recursos->operacoes_ativas_torre++;
        
        // Encontra um slot livre para registrar o avião
        int slot_alocado = -1;
        for (int i = 0; i < CAPACIDADE_TORRE; i++) {
            if (recursos->torre_ocupada_por[i] == -1) {
                recursos->torre_ocupada_por[i] = aviao->id;
                slot_alocado = i;
                break;
            }
        }
        
        aviao->torre_alocada = slot_alocado + 1; // +1 para diferir de 0 (não alocado)

        log_evento_ui(sim, aviao, LOG_RESOURCE, "Torre alocada - Slot %d (%d/%d slots disponíveis) [Prioridade: %d]", 
                     slot_alocado, recursos->slots_torre_disponiveis, recursos->capacidade_torre, aviao->prioridade_dinamica);        
        
        pthread_mutex_unlock(&recursos->mutex_torres);
        return 0; 
    }
    
    pthread_mutex_unlock(&recursos->mutex_torres);
    return -1;
}

// =============== FUNÇÕES AUXILIARES PARA USO DA TORRE ===============

// Solicitar uso da torre (exemplo simples)
int solicitar_uso_torre(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    
    pthread_mutex_lock(&recursos->mutex_torres);
    while (recursos->slots_torre_disponiveis == 0) {
        pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_torres);
    }
    recursos->slots_torre_disponiveis--;
    recursos->operacoes_ativas_torre++;
    
    // Encontra um slot livre
    for (int i = 0; i < CAPACIDADE_TORRE; i++) {
        if (recursos->torre_ocupada_por[i] == -1) {
            recursos->torre_ocupada_por[i] = aviao->id;
            aviao->torre_alocada = i + 1;
            break;
        }
    }
    
    pthread_mutex_unlock(&recursos->mutex_torres);
    return 0;
}

// Liberar uso da torre (exemplo simples)
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

// =============== FUNÇÕES DE ALOCAÇÃO ATÔMICA (ANTI-DEADLOCK) ===============

// Aloca TODOS os recursos necessários para pouso de uma vez (torre + pista)
int alocar_recursos_pouso_atomico(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    bool recursos_alocados = false;
    
    while (!recursos_alocados && sim->ativa) {
        // Tenta pegar TODOS os mutexes em ordem fixa (evita deadlock circular)
        pthread_mutex_lock(&recursos->mutex_torres);
        pthread_mutex_lock(&recursos->mutex_pistas);
        
        // Verifica se TODOS os recursos estão disponíveis
        bool torre_disponivel = recursos->slots_torre_disponiveis > 0;
        bool pista_disponivel = recursos->pistas_disponiveis > 0;
        
        if (torre_disponivel && pista_disponivel) {
            // Aloca torre
            recursos->slots_torre_disponiveis--;
            recursos->operacoes_ativas_torre++;
            for (int i = 0; i < CAPACIDADE_TORRE; i++) {
                if (recursos->torre_ocupada_por[i] == -1) {
                    recursos->torre_ocupada_por[i] = aviao->id;
                    aviao->torre_alocada = i + 1;
                    break;
                }
            }
            
            // Aloca pista
            recursos->pistas_disponiveis--;
            for (int i = 0; i < MAX_PISTAS; i++) {
                if (recursos->pista_ocupada_por[i] == -1) {
                    recursos->pista_ocupada_por[i] = aviao->id;
                    aviao->pista_alocada = i;
                    break;
                }
            }
            
            recursos_alocados = true;
            log_evento_ui(sim, aviao, LOG_RESOURCE, 
                         "Recursos POUSO alocados atomicamente (Torre: slot %d, Pista: %d)", 
                         aviao->torre_alocada - 1, aviao->pista_alocada);
        }
        
        pthread_mutex_unlock(&recursos->mutex_pistas);
        pthread_mutex_unlock(&recursos->mutex_torres);
        
        // Se não conseguiu alocar, espera um pouco antes de tentar novamente
        if (!recursos_alocados) {
            usleep(1000 + (rand() % 5000)); // 1-6ms de espera aleatória
        }
    }
    
    return recursos_alocados ? 0 : -1;
}

// Aloca TODOS os recursos necessários para desembarque (portão)
int alocar_recursos_desembarque_atomico(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    bool recursos_alocados = false;
    
    while (!recursos_alocados && sim->ativa) {
        pthread_mutex_lock(&recursos->mutex_portoes);
        
        if (recursos->portoes_disponiveis > 0) {
            // Aloca portão
            recursos->portoes_disponiveis--;
            for (int i = 0; i < MAX_PORTOES; i++) {
                if (recursos->portao_ocupado_por[i] == -1) {
                    recursos->portao_ocupado_por[i] = aviao->id;
                    aviao->portao_alocado = i;
                    break;
                }
            }
            
            recursos_alocados = true;
            log_evento_ui(sim, aviao, LOG_RESOURCE, 
                         "Recursos DESEMBARQUE alocados atomicamente (Portão: %d)", 
                         aviao->portao_alocado);
        }
        
        pthread_mutex_unlock(&recursos->mutex_portoes);
        
        if (!recursos_alocados) {
            usleep(1000 + (rand() % 5000)); // 1-6ms de espera aleatória
        }
    }
    
    return recursos_alocados ? 0 : -1;
}

// Aloca TODOS os recursos necessários para decolagem (torre + pista)
int alocar_recursos_decolagem_atomico(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    bool recursos_alocados = false;
    
    while (!recursos_alocados && sim->ativa) {
        // Ordem fixa de mutexes para evitar deadlock
        pthread_mutex_lock(&recursos->mutex_torres);
        pthread_mutex_lock(&recursos->mutex_pistas);
        
        bool torre_disponivel = recursos->slots_torre_disponiveis > 0;
        bool pista_disponivel = recursos->pistas_disponiveis > 0;
        
        if (torre_disponivel && pista_disponivel) {
            // Aloca torre
            recursos->slots_torre_disponiveis--;
            recursos->operacoes_ativas_torre++;
            for (int i = 0; i < CAPACIDADE_TORRE; i++) {
                if (recursos->torre_ocupada_por[i] == -1) {
                    recursos->torre_ocupada_por[i] = aviao->id;
                    aviao->torre_alocada = i + 1;
                    break;
                }
            }
            
            // Aloca pista
            recursos->pistas_disponiveis--;
            for (int i = 0; i < MAX_PISTAS; i++) {
                if (recursos->pista_ocupada_por[i] == -1) {
                    recursos->pista_ocupada_por[i] = aviao->id;
                    aviao->pista_alocada = i;
                    break;
                }
            }
            
            recursos_alocados = true;
            log_evento_ui(sim, aviao, LOG_RESOURCE, 
                         "Recursos DECOLAGEM alocados atomicamente (Torre: slot %d, Pista: %d)", 
                         aviao->torre_alocada - 1, aviao->pista_alocada);
        }
        
        pthread_mutex_unlock(&recursos->mutex_pistas);
        pthread_mutex_unlock(&recursos->mutex_torres);
        
        if (!recursos_alocados) {
            usleep(1000 + (rand() % 5000)); // 1-6ms de espera aleatória
        }
    }
    
    return recursos_alocados ? 0 : -1;
}

// Libera TODOS os recursos de um avião
void liberar_todos_recursos(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    
    // Libera torre se alocada
    if (aviao->torre_alocada > 0) {
        pthread_mutex_lock(&recursos->mutex_torres);
        int slot = aviao->torre_alocada - 1;
        if (slot >= 0 && slot < CAPACIDADE_TORRE) {
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
    
    log_evento_ui(sim, aviao, LOG_RESOURCE, "TODOS os recursos liberados");
}