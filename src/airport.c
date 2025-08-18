#include "airport.h"
#include "utils.h"

void liberar_pista(SimulacaoAeroporto* sim, int id_aviao, int pista_idx) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};

    pthread_mutex_lock(&recursos->mutex_banco); // Trava o mutex principal do banqueiro

    // Verifica se o avião realmente possui esta pista
    if (id_aviao > 0 && id_aviao <= sim->max_avioes && sim->avioes[id_aviao - 1].pista_alocada == pista_idx) {
        
        // 1. Libera no sistema do banqueiro
        recurso_liberado[RECURSO_PISTA] = 1;
        banker_release_resources(recursos, id_aviao - 1, recurso_liberado);

        // 2. Libera fisicamente
        pthread_mutex_lock(&recursos->mutex_pistas);
        recursos->pista_ocupada_por[pista_idx] = -1;
        sim->avioes[id_aviao - 1].pista_alocada = -1; // Usa -1 para consistência
        recursos->pistas_disponiveis++;
        
        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Pista %d liberada (%d/%d disponíveis)", pista_idx, recursos->pistas_disponiveis, recursos->total_pistas);
        
        // Notifica threads esperando especificamente por pistas
        pthread_cond_broadcast(&recursos->cond_pistas);
        pthread_mutex_unlock(&recursos->mutex_pistas);
    }
    
    // Notifica todas as threads esperando por qualquer recurso
    pthread_cond_broadcast(&recursos->cond_banco); 
    pthread_mutex_unlock(&recursos->mutex_banco);
}

// Função corrigida para liberar o portão
void liberar_portao(SimulacaoAeroporto* sim, int id_aviao, int portao_idx) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};

    pthread_mutex_lock(&recursos->mutex_banco);

    if (id_aviao > 0 && id_aviao <= sim->max_avioes && sim->avioes[id_aviao - 1].portao_alocado == portao_idx) {
        recurso_liberado[RECURSO_PORTAO] = 1;
        banker_release_resources(recursos, id_aviao - 1, recurso_liberado);
        
        pthread_mutex_lock(&recursos->mutex_portoes);
        recursos->portao_ocupado_por[portao_idx] = -1;
        sim->avioes[id_aviao - 1].portao_alocado = -1; // Consistência
        recursos->portoes_disponiveis++;

        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Portão %d liberado (%d/%d disponíveis)", portao_idx, recursos->portoes_disponiveis, recursos->total_portoes);

        pthread_cond_broadcast(&recursos->cond_portoes);
        pthread_mutex_unlock(&recursos->mutex_portoes);
    }

    pthread_cond_broadcast(&recursos->cond_banco);
    pthread_mutex_unlock(&recursos->mutex_banco);
}

// *** FUNÇÃO CRÍTICA CORRIGIDA ***
// Garante que os contadores da torre só são modificados se o avião realmente possuir um slot.
void liberar_torre(SimulacaoAeroporto* sim, int id_aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};

    pthread_mutex_lock(&recursos->mutex_banco);

    // Verifica se o avião tem um ID válido e realmente possui uma torre alocada (> 0)
    if (id_aviao > 0 && id_aviao <= sim->max_avioes && sim->avioes[id_aviao - 1].torre_alocada > 0) {
        
        recurso_liberado[RECURSO_TORRE] = 1;
        banker_release_resources(recursos, id_aviao - 1, recurso_liberado);
        
        pthread_mutex_lock(&recursos->mutex_torres);
        
        int slot_liberado = sim->avioes[id_aviao - 1].torre_alocada - 1;
        
        // Liberação física
        recursos->torre_ocupada_por[slot_liberado] = -1;
        sim->avioes[id_aviao - 1].torre_alocada = -1;
        
        recursos->slots_torre_disponiveis++;
        recursos->operacoes_ativas_torre--;
        
        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Torre liberada (%d/%d slots disponíveis)", recursos->slots_torre_disponiveis, recursos->capacidade_torre);
        
        pthread_cond_broadcast(&recursos->cond_torres);
        pthread_mutex_unlock(&recursos->mutex_torres);
    }

    pthread_cond_broadcast(&recursos->cond_banco);
    pthread_mutex_unlock(&recursos->mutex_banco);
}


int solicitar_recurso_individual(SimulacaoAeroporto* sim, Aviao* aviao, TipoRecurso tipo_recurso) {
    RecursosAeroporto* recursos = &sim->recursos;
    int pedido[N_RESOURCES] = {0};
    pedido[tipo_recurso] = 1;

    pthread_mutex_lock(&recursos->mutex_banco);

    bool adicionado_na_fila = false;
    int resultado_banqueiro = -1;

    while ((resultado_banqueiro = banker_request_resources(recursos, aviao->id - 1, pedido)) != 0) {
        if (!adicionado_na_fila) {
            int prioridade = calcular_prioridade_dinamica(aviao, time(NULL), sim);
            
            if (tipo_recurso == RECURSO_PISTA) inserir_na_fila_prioridade(&recursos->fila_pistas, aviao->id, prioridade);
            if (tipo_recurso == RECURSO_PORTAO) inserir_na_fila_prioridade(&recursos->fila_portoes, aviao->id, prioridade);
            if (tipo_recurso == RECURSO_TORRE) inserir_na_fila_prioridade(&recursos->fila_torres, aviao->id, prioridade);
            
            adicionado_na_fila = true;
        }

        if (resultado_banqueiro == -4) { // -4 = Estado Inseguro detectado pelo Banqueiro
            log_evento_ui(sim, aviao, LOG_WARNING, "Banqueiro negou. Cedendo a vez para evitar live-lock.");

            // Pega a prioridade atual para reinserir o avião na fila
            int prioridade_atual = aviao->prioridade_dinamica;

            // Remove da fila correspondente
            if (tipo_recurso == RECURSO_PISTA) remover_da_fila_prioridade(&recursos->fila_pistas, aviao->id);
            if (tipo_recurso == RECURSO_PORTAO) remover_da_fila_prioridade(&recursos->fila_portoes, aviao->id);
            if (tipo_recurso == RECURSO_TORRE) remover_da_fila_prioridade(&recursos->fila_torres, aviao->id);

            // Pequena pausa para dar chance a outras threads
            pthread_mutex_unlock(&recursos->mutex_banco);
            usleep(5000); // 50ms
            pthread_mutex_lock(&recursos->mutex_banco);

            // Reinsere na fila (irá para o fim do seu grupo de prioridade)
            if (tipo_recurso == RECURSO_PISTA) inserir_na_fila_prioridade(&recursos->fila_pistas, aviao->id, prioridade_atual);
            if (tipo_recurso == RECURSO_PORTAO) inserir_na_fila_prioridade(&recursos->fila_portoes, aviao->id, prioridade_atual);
            if (tipo_recurso == RECURSO_TORRE) inserir_na_fila_prioridade(&recursos->fila_torres, aviao->id, prioridade_atual);
        }

        switch (tipo_recurso) {
            case RECURSO_PISTA:
                pthread_cond_wait(&recursos->cond_pistas, &recursos->mutex_banco);
                break;
            case RECURSO_PORTAO:
                pthread_cond_wait(&recursos->cond_portoes, &recursos->mutex_banco);
                break;
            case RECURSO_TORRE:
                pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_banco);
                break;
            default:
                // Fallback para o comportamento antigo caso algo dê errado
                pthread_cond_wait(&recursos->cond_banco, &recursos->mutex_banco);
                break;
        }


        if (aviao->sacrificado) {
            log_evento_ui(sim, aviao, LOG_ERROR, "Fui sacrificado, abortando operação.");
            
            // Remove da fila atual para não ser considerado por outros
            if (tipo_recurso == RECURSO_PISTA) remover_da_fila_prioridade(&recursos->fila_pistas, aviao->id);
            if (tipo_recurso == RECURSO_PORTAO) remover_da_fila_prioridade(&recursos->fila_portoes, aviao->id);
            if (tipo_recurso == RECURSO_TORRE) remover_da_fila_prioridade(&recursos->fila_torres, aviao->id);

            pthread_mutex_unlock(&recursos->mutex_banco);
            return -99; // Retorna um código de erro especial para "sacrifício"
        }

        if (!sim->ativa) {
            pthread_mutex_unlock(&recursos->mutex_banco);
            return -1;
        }
        
        // Atualiza a prioridade para evitar starvation
        int nova_prioridade = calcular_prioridade_dinamica(aviao, time(NULL), sim);
        if (tipo_recurso == RECURSO_PISTA) atualizar_prioridade_na_fila(&recursos->fila_pistas, aviao->id, nova_prioridade);
        if (tipo_recurso == RECURSO_PORTAO) atualizar_prioridade_na_fila(&recursos->fila_portoes, aviao->id, nova_prioridade);
        if (tipo_recurso == RECURSO_TORRE) atualizar_prioridade_na_fila(&recursos->fila_torres, aviao->id, nova_prioridade);
    }

    // Se o recurso foi concedido, o banqueiro já removeu da fila.
    // Agora, encontramos qual slot físico foi alocado e o atribuímos ao avião.
    switch (tipo_recurso) {
        case RECURSO_PISTA:
            for (int i = 0; i < recursos->total_pistas; i++) {
                if (recursos->pista_ocupada_por[i] == aviao->id) {
                    aviao->pista_alocada = i;
                    break;
                }
            }
            break;
        case RECURSO_PORTAO:
            for (int i = 0; i < recursos->total_portoes; i++) {
                if (recursos->portao_ocupado_por[i] == aviao->id) {
                    aviao->portao_alocado = i;
                    break;
                }
            }
            break;
        case RECURSO_TORRE:
            for (int i = 0; i < recursos->capacidade_torre; i++) {
                if (recursos->torre_ocupada_por[i] == aviao->id) {
                    aviao->torre_alocada = i + 1; // +1 para diferenciar de 0
                    break;
                }
            }
            break;
    }
    
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Recursos alocados via Banqueiro.");
    pthread_mutex_unlock(&recursos->mutex_banco);
    return 0; // Sucesso
}

// Libera TODOS os recursos de um avião
void liberar_todos_recursos(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};
    bool recursos_para_liberar = false;
    
    // ✓ CORREÇÃO: Proteger TUDO com mutex do banqueiro
    pthread_mutex_lock(&recursos->mutex_banco);
    
    remover_da_fila_prioridade(&recursos->fila_pistas, aviao->id);
    remover_da_fila_prioridade(&recursos->fila_portoes, aviao->id);
    remover_da_fila_prioridade(&recursos->fila_torres, aviao->id);

    if (aviao->pista_alocada >= 0) {
        recurso_liberado[RECURSO_PISTA] = 1;
        recursos_para_liberar = true;
    }
    if (aviao->portao_alocado >= 0) {
        recurso_liberado[RECURSO_PORTAO] = 1;
        recursos_para_liberar = true;
    }
    if (aviao->torre_alocada > 0) {
        recurso_liberado[RECURSO_TORRE] = 1;
        recursos_para_liberar = true;
    }
    
    // Liberar no banqueiro se há recursos alocados
    if (recursos_para_liberar) {
        int resultado = banker_release_resources(recursos, aviao->id - 1, recurso_liberado);
        if (resultado != 0) {
            log_evento_ui(sim, aviao, LOG_WARNING, "Falha ao liberar recursos no banqueiro: %d", resultado);
            pthread_mutex_unlock(&recursos->mutex_banco);
            return;
        }
    }
    
    if (aviao->torre_alocada > 0) {
        pthread_mutex_lock(&recursos->mutex_torres);
        int slot = aviao->torre_alocada - 1;
        if (slot >= 0 && slot < recursos->capacidade_torre && 
            recursos->torre_ocupada_por[slot] == aviao->id) {
            recursos->torre_ocupada_por[slot] = -1;
            recursos->slots_torre_disponiveis++;
            recursos->operacoes_ativas_torre--;
            aviao->torre_alocada = -1;
            log_evento_ui(sim, aviao, LOG_RESOURCE, "Torre slot %d liberado fisicamente", slot);
        }
        pthread_cond_broadcast(&recursos->cond_torres);
        pthread_mutex_unlock(&recursos->mutex_torres);
    }
    
    // Liberar pista se ainda alocada
    if (aviao->pista_alocada >= 0) {
        pthread_mutex_lock(&recursos->mutex_pistas);
        int pista = aviao->pista_alocada;
        if (pista >= 0 && pista < recursos->total_pistas && 
            recursos->pista_ocupada_por[pista] == aviao->id) {
            recursos->pista_ocupada_por[pista] = -1;
            recursos->pistas_disponiveis++;
            aviao->pista_alocada = -1;
            log_evento_ui(sim, aviao, LOG_RESOURCE, "Pista %d liberada fisicamente", pista);
        }
        pthread_cond_broadcast(&recursos->cond_pistas);
        pthread_mutex_unlock(&recursos->mutex_pistas);
    }
    
    // Liberar portão se ainda alocado
    if (aviao->portao_alocado >= 0) {
        pthread_mutex_lock(&recursos->mutex_portoes);
        int portao = aviao->portao_alocado;
        if (portao >= 0 && portao < recursos->total_portoes && 
            recursos->portao_ocupado_por[portao] == aviao->id) {
            recursos->portao_ocupado_por[portao] = -1;
            recursos->portoes_disponiveis++;
            aviao->portao_alocado = -1;
            log_evento_ui(sim, aviao, LOG_RESOURCE, "Portão %d liberado fisicamente", portao);
        }
        pthread_cond_broadcast(&recursos->cond_portoes);
        pthread_mutex_unlock(&recursos->mutex_portoes);
    }
    
    // Broadcast final do banqueiro
    if (recursos_para_liberar) {
        pthread_cond_broadcast(&recursos->cond_banco);
        log_evento_ui(sim, aviao, LOG_RESOURCE, "TODOS os recursos liberados com sucesso");
    }
    
    pthread_mutex_unlock(&recursos->mutex_banco);
}