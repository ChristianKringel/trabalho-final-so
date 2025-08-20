#include "airport.h"
#include "utils.h"

void liberar_pista(SimulacaoAeroporto* sim, int id_aviao, int pista_idx) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};

    pthread_mutex_lock(&recursos->mutex_banco); 

    if (id_aviao > 0 && id_aviao <= sim->max_avioes && sim->avioes[id_aviao - 1].pista_alocada == pista_idx) {
        
        recurso_liberado[RECURSO_PISTA] = 1;
        banker_release_resources(recursos, id_aviao - 1, recurso_liberado);

        pthread_mutex_lock(&recursos->mutex_pistas);
        recursos->pista_ocupada_por[pista_idx] = -1;
        sim->avioes[id_aviao - 1].pista_alocada = -1;
        recursos->pistas_disponiveis++;
        
        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Pista %d liberada (%d/%d disponíveis)", pista_idx, recursos->pistas_disponiveis, recursos->total_pistas);
        
        pthread_cond_broadcast(&recursos->cond_pistas);
        pthread_mutex_unlock(&recursos->mutex_pistas);
    }
    
    pthread_cond_broadcast(&recursos->cond_banco); 
    pthread_mutex_unlock(&recursos->mutex_banco);
}

void liberar_portao(SimulacaoAeroporto* sim, int id_aviao, int portao_idx) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};

    pthread_mutex_lock(&recursos->mutex_banco);

    if (id_aviao > 0 && id_aviao <= sim->max_avioes && sim->avioes[id_aviao - 1].portao_alocado == portao_idx) {
        recurso_liberado[RECURSO_PORTAO] = 1;
        banker_release_resources(recursos, id_aviao - 1, recurso_liberado);
        
        pthread_mutex_lock(&recursos->mutex_portoes);
        recursos->portao_ocupado_por[portao_idx] = -1;
        sim->avioes[id_aviao - 1].portao_alocado = -1;
        recursos->portoes_disponiveis++;

        log_evento_ui(sim, &sim->avioes[id_aviao-1], LOG_RESOURCE, "Portão %d liberado (%d/%d disponíveis)", portao_idx, recursos->portoes_disponiveis, recursos->total_portoes);

        pthread_cond_broadcast(&recursos->cond_portoes);
        pthread_mutex_unlock(&recursos->mutex_portoes);
    }

    pthread_cond_broadcast(&recursos->cond_banco);
    pthread_mutex_unlock(&recursos->mutex_banco);
}

void liberar_torre(SimulacaoAeroporto* sim, int id_aviao) {
    RecursosAeroporto* recursos = &sim->recursos;
    int recurso_liberado[N_RESOURCES] = {0};

    pthread_mutex_lock(&recursos->mutex_banco);

    if (id_aviao > 0 && id_aviao <= sim->max_avioes && sim->avioes[id_aviao - 1].torre_alocada > 0) {
        
        recurso_liberado[RECURSO_TORRE] = 1;
        banker_release_resources(recursos, id_aviao - 1, recurso_liberado);
        
        pthread_mutex_lock(&recursos->mutex_torres);
        
        int slot_liberado = sim->avioes[id_aviao - 1].torre_alocada - 1;
        
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
        
        bool recurso_fisicamente_ocupado = false;
        if (resultado_banqueiro == -3) { 
             recurso_fisicamente_ocupado = true;
        }

        if (recurso_fisicamente_ocupado) {
            switch (tipo_recurso) {
                case RECURSO_PISTA:  pthread_cond_wait(&recursos->cond_pistas, &recursos->mutex_banco); break;
                case RECURSO_PORTAO: pthread_cond_wait(&recursos->cond_portoes, &recursos->mutex_banco); break;
                case RECURSO_TORRE:  pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_banco); break;
                default:             pthread_cond_wait(&recursos->cond_banco, &recursos->mutex_banco); break;
            }
        } else {
            pthread_mutex_unlock(&recursos->mutex_banco);
            usleep(10000 + (rand() % 5000));
            pthread_mutex_lock(&recursos->mutex_banco);
        }

        if (aviao->sacrificado) {
            log_evento_ui(sim, aviao, LOG_ERROR, "Fui sacrificado, abortando operação.");
            
            if (tipo_recurso == RECURSO_PISTA) remover_da_fila_prioridade(&recursos->fila_pistas, aviao->id);
            if (tipo_recurso == RECURSO_PORTAO) remover_da_fila_prioridade(&recursos->fila_portoes, aviao->id);
            if (tipo_recurso == RECURSO_TORRE) remover_da_fila_prioridade(&recursos->fila_torres, aviao->id);

            pthread_mutex_unlock(&recursos->mutex_banco);
            return -99;
        }

        if (!sim->ativa) {
            pthread_mutex_unlock(&recursos->mutex_banco);
            return -1;
        }
        
        int nova_prioridade = calcular_prioridade_dinamica(aviao, time(NULL), sim);
        if (tipo_recurso == RECURSO_PISTA) atualizar_prioridade_na_fila(&recursos->fila_pistas, aviao->id, nova_prioridade);
        if (tipo_recurso == RECURSO_PORTAO) atualizar_prioridade_na_fila(&recursos->fila_portoes, aviao->id, nova_prioridade);
        if (tipo_recurso == RECURSO_TORRE) atualizar_prioridade_na_fila(&recursos->fila_torres, aviao->id, nova_prioridade);
    }

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
                    aviao->torre_alocada = i + 1;
                    break;
                }
            }
            break;
    }
    
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Recursos alocados via Banqueiro.");
    pthread_mutex_unlock(&recursos->mutex_banco);
    return 0;
}

