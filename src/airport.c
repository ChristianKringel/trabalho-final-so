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

// =============== FUNÇÕES COM SISTEMA DE PRIORIDADE ===============
int solicitar_pista_com_prioridade(SimulacaoAeroporto* sim, Aviao* aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    
    time_t agora = time(NULL);
    int prioridade = calcular_prioridade_dinamica(aviao, agora, sim);
    
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
        int nova_prioridade = calcular_prioridade_dinamica(aviao, time(NULL), sim);
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

        // Checa se tem pista disponivel e se é a vez do avião na fila 
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
    aviao->prioridade_dinamica = calcular_prioridade_dinamica(aviao, agora, sim);
    
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
        int nova_prioridade = calcular_prioridade_dinamica(aviao, time(NULL), sim);
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
    aviao->prioridade_dinamica = calcular_prioridade_dinamica(aviao, agora, sim);
    
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
    
    // Loop de espera para recurso compartilhado
    while ((recursos->slots_torre_disponiveis <= 0 || !eh_minha_vez_na_fila(&recursos->fila_torres, aviao->id)) && sim->ativa) {
        // Atualiza prioridade dinâmica enquanto espera
        int nova_prioridade = calcular_prioridade_dinamica(aviao, time(NULL), sim);
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
        for (int i = 0; i < recursos->capacidade_torre; i++) {
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
    // 2. Loop de espera: continua tentando até o banqueiro aprovar o pedido
    while ((resultado_banqueiro = banker_request_resources(recursos, aviao->id - 1, pedido)) != 0) {
        // Se o banqueiro negou, esperamos por uma mudança no estado dos recursos.
        // A função pthread_cond_wait irá atomicamente desbloquear o mutex e esperar.
        // Quando acordar, ela irá travar o mutex novamente antes de continuar.
        pthread_cond_wait(&recursos->cond_banco, &recursos->mutex_banco);

        // Se a simulação foi desativada enquanto esperávamos, saímos.
        if (!sim->ativa) {
            pthread_mutex_unlock(&recursos->mutex_banco);
            return -1; // Falha por término da simulação
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