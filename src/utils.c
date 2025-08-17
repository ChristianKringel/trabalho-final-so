#include "utils.h"
#include "terminal.h"

void inicializar_fila_prioridade(FilaPrioridade* fila) {
    if (fila == NULL) return;
    
    fila->tamanho = 0;
    pthread_mutex_init(&fila->mutex, NULL);
    pthread_cond_init(&fila->cond, NULL);
    
    for (int i = 0; i < MAX_AVIOES; i++) {
        fila->avioes_ids[i] = -1;
        fila->prioridades[i] = 0;
    }
}

void inserir_na_fila_prioridade(FilaPrioridade* fila, int aviao_id, int prioridade) {
    if (fila == NULL || fila->tamanho >= MAX_AVIOES) return;
    
    pthread_mutex_lock(&fila->mutex);
    
    int pos = fila->tamanho;
    // CORREÇÃO: ordenação decrescente - maiores prioridades primeiro
    while(pos > 0 && fila->prioridades[pos-1] < prioridade){
        fila->avioes_ids[pos] = fila->avioes_ids[pos - 1];
        fila->prioridades[pos] = fila->prioridades[pos - 1];
        pos--;
    }
    
    fila->avioes_ids[pos] = aviao_id;
    fila->prioridades[pos] = prioridade;
    fila->tamanho++;
    
    pthread_cond_signal(&fila->cond);
    pthread_mutex_unlock(&fila->mutex);
}

int remover_da_fila_prioridade(FilaPrioridade* fila, int aviao_id) {
    if (fila == NULL) return -1;
    
    pthread_mutex_lock(&fila->mutex);
    
    if (fila->tamanho == 0) {
        pthread_mutex_unlock(&fila->mutex);
        return -1;
    }
    
    // Encontra o avião específico na fila
    int pos_encontrada = -1;
    for (int i = 0; i < fila->tamanho; i++) {
        if (fila->avioes_ids[i] == aviao_id) {
            pos_encontrada = i;
            break;
        }
    }
    
    // Se não encontrou o avião, retorna erro
    if (pos_encontrada == -1) {
        pthread_mutex_unlock(&fila->mutex);
        return -1;
    }
    
    // Move todos os elementos após a posição encontrada uma posição para trás
    for (int i = pos_encontrada; i < fila->tamanho - 1; i++) {
        fila->avioes_ids[i] = fila->avioes_ids[i + 1];
        fila->prioridades[i] = fila->prioridades[i + 1];
    }
    
    fila->tamanho--;
    fila->avioes_ids[fila->tamanho] = -1;
    fila->prioridades[fila->tamanho] = 0;
    
    // Sinalizar que a fila foi modificada
    pthread_cond_signal(&fila->cond);
    
    pthread_mutex_unlock(&fila->mutex);
    return aviao_id;
}

void remover_temporariamente_da_fila_prioridade(FilaPrioridade* fila, int posicao) {
    for (int i = posicao; i < fila->tamanho - 1; i++) {
        fila->avioes_ids[i] = fila->avioes_ids[i + 1];
        fila->prioridades[i] = fila->prioridades[i + 1];
    }
    fila->tamanho--;
}

void encontrar_posicao_por_prioridade(FilaPrioridade* fila, int nova_prioridade, int* nova_pos) {
    *nova_pos = fila->tamanho;
    while (*nova_pos > 0 && fila->prioridades[*nova_pos - 1] <= nova_prioridade) {
        fila->avioes_ids[*nova_pos] = fila->avioes_ids[*nova_pos - 1];
        fila->prioridades[*nova_pos] = fila->prioridades[*nova_pos - 1];
        (*nova_pos)--;
    }
}

void inserir_na_nova_posicao(FilaPrioridade* fila, int nova_pos, int aviao_id, int nova_prioridade) {
    fila->avioes_ids[nova_pos] = aviao_id;
    fila->prioridades[nova_pos] = nova_prioridade;
    fila->tamanho++;
}
    
void destruir_fila_prioridade(FilaPrioridade* fila) {
    if (fila == NULL) return;
    
    pthread_mutex_destroy(&fila->mutex);
    pthread_cond_destroy(&fila->cond);
}

// =============== FUNÇÕES DE PRIORIDADE E MONITORAMENTO ===============
int obter_proximo_da_fila_prioridade(FilaPrioridade* fila) {
    if (!fila || fila->tamanho == 0) return -1;

    return fila->tamanho > 0 ? fila->avioes_ids[0] : -1;
}

bool eh_minha_vez_na_fila(FilaPrioridade* fila, int aviao_id) {
    if (!fila) return true; // Se não há fila, todos podem tentar
    
    // Se a fila está vazia, qualquer avião pode tentar
    if (fila->tamanho == 0) return true;
    
    // Verifica se é o primeiro da fila
    return fila->avioes_ids[0] == aviao_id; 
}

int obter_posicao_na_fila(FilaPrioridade* fila, int aviao_id) {
    if (!fila || fila->tamanho == 0) return -1;
    
    for (int i = 0; i < fila->tamanho; i++) {
        if (fila->avioes_ids[i] == aviao_id) {
            return i;
        }
    }
}

void atualizar_prioridade_na_fila(FilaPrioridade* fila, int aviao_id, int nova_prioridade) {
    if (!fila || fila->tamanho == 0) return;
    
    pthread_mutex_lock(&fila->mutex);
    
    int posicao = obter_posicao_na_fila(fila, aviao_id);

    if (posicao == -1) {
        pthread_mutex_unlock(&fila->mutex);
        return; 
    }
    
    remover_temporariamente_da_fila_prioridade(fila, posicao);
    
    int nova_pos;
    encontrar_posicao_por_prioridade(fila, nova_prioridade, &nova_pos);
    
    inserir_na_nova_posicao(fila, nova_pos, aviao_id, nova_prioridade);
    
    pthread_mutex_unlock(&fila->mutex);
}

void atualizar_prioridade_nas_filas(SimulacaoAeroporto* sim, Aviao* aviao) {
    if (!sim || !aviao) return;
    
    RecursosAeroporto* recursos = &sim->recursos;
    
    if (aviao->estado == AGUARDANDO_POUSO || aviao->estado == AGUARDANDO_DECOLAGEM) {
        atualizar_prioridade_na_fila(&recursos->fila_pistas, aviao->id, aviao->prioridade_dinamica);
    }
    
    if (aviao->estado == AGUARDANDO_DESEMBARQUE || aviao->estado == AGUARDANDO_DECOLAGEM) {
        atualizar_prioridade_na_fila(&recursos->fila_portoes, aviao->id, aviao->prioridade_dinamica);
    }
    
    if (aviao->estado == AGUARDANDO_POUSO || 
        aviao->estado == AGUARDANDO_DESEMBARQUE || 
        aviao->estado == AGUARDANDO_DECOLAGEM) {
        atualizar_prioridade_na_fila(&recursos->fila_torres, aviao->id, aviao->prioridade_dinamica);
    }
}

int calcular_prioridade_dinamica(Aviao* aviao, time_t agora, SimulacaoAeroporto* sim) {
    if (aviao == NULL) return 0;
    
    // Prioridades base mais equilibradas
    int prioridade_base = (aviao->tipo == VOO_INTERNACIONAL) ? 25 : 15;
    int tempo_espera = 0;
    if (aviao->estado == AGUARDANDO_POUSO) {
        tempo_espera = sim ? calcular_tempo_espera_efetivo(sim, aviao->tempo_inicio_espera_ar) 
                          : (int)difftime(agora, aviao->tempo_inicio_espera_ar);
    } else if (aviao->estado == AGUARDANDO_DESEMBARQUE || aviao->estado == AGUARDANDO_DECOLAGEM) {
        tempo_espera = sim ? calcular_tempo_espera_efetivo(sim, aviao->chegada_na_fila)
                          : (int)difftime(agora, aviao->chegada_na_fila);
    }
    
    // AGING EXPONENCIAL - Prioridade aumenta drasticamente com o tempo
    int bonus_tempo = 0;
    if (tempo_espera >= 0) {
        bonus_tempo = (tempo_espera * tempo_espera * tempo_espera) / 20 + 
                      (tempo_espera * tempo_espera) / 5 + 
                      tempo_espera * 8;
    }
    
    // Bônus para aviões em situação crítica
    int bonus_emergencia = 0;
    if (aviao->crash_iminente) {
        // PRIORIDADE MÁXIMA - sempre será o primeiro da fila
        bonus_emergencia = 50000;
    } else if (aviao->em_alerta) {
        // Prioridade muito alta - baseada no tempo de alerta
        if (aviao->estado == AGUARDANDO_POUSO) {
            bonus_emergencia = 5000; // Pouso é crítico
        } else {
            bonus_emergencia = 3000; // Outras operações
        }
    }
    
    // Bônus adicional baseado no tempo total esperando (não só no ar)
    int tempo_total_espera = (int)difftime(agora, aviao->tempo_criacao);
    int bonus_persistencia = tempo_total_espera * 6; // 3 pontos por segundo de vida
    
    int bonus_domestico = 0;
    if (aviao->tipo == VOO_DOMESTICO && tempo_espera > 15) {
        // Voos domésticos ganham prioridade extra após 15 segundos
        bonus_domestico = (tempo_espera - 15) * 4;
    }
    
    int prioridade_final = prioridade_base + bonus_tempo + bonus_emergencia + 
                          bonus_persistencia + bonus_domestico;
    
    // Debug para prioridades extremas
    if (prioridade_final > 1000 && !aviao->em_alerta) {
        log_evento_ui(NULL, aviao, LOG_INFO, "Prioridade alta por aging: %d (tempo: %ds)", prioridade_final, tempo_espera);
    }
    
    return prioridade_final;
}

bool aviao_deve_ser_ignorado(Aviao* aviao) {
    return aviao->id <= 0 || aviao->estado == FINALIZADO_SUCESSO || aviao->estado == FALHA_STARVATION || aviao->estado == FALHA_DEADLOCK;
}

// =============== FUNÇÕES DE AGING ===============
void processar_aging_pouso(SimulacaoAeroporto* sim, Aviao* aviao, int tempo_espera) {
    if (tempo_espera >= 90 && !aviao->crash_iminente) {
        aviao->crash_iminente = true;
        log_evento_ui(sim, aviao, LOG_ERROR, "CRASH! Avião %d caiu após 90s de espera no ar", aviao->id);
        atualizar_estado_aviao(aviao, FALHA_STARVATION);
        incrementar_aviao_falha_starvation(&sim->metricas);
        
    } else if (tempo_espera >= 60 && !aviao->em_alerta) {
        aviao->em_alerta = true;
        log_evento_ui(sim, aviao, LOG_WARNING, "ALERTA CRÍTICO! Avião %d há %ds no ar - PRIORIDADE EXTREMA!", aviao->id, tempo_espera);
        pthread_cond_broadcast(&sim->recursos.cond_torres);
        pthread_cond_broadcast(&sim->recursos.cond_pistas);
        pthread_cond_broadcast(&sim->recursos.cond_portoes);
    }
}

void processar_aging_desembarque(SimulacaoAeroporto* sim, Aviao* aviao, int tempo_espera) {
    if (tempo_espera >= 45 && !aviao->em_alerta) {
        aviao->em_alerta = true;
        log_evento_ui(sim, aviao, LOG_WARNING, "ALERTA! Avião %d aguardando desembarque há %ds", aviao->id, tempo_espera);
        
        pthread_cond_broadcast(&sim->recursos.cond_torres);
        pthread_cond_broadcast(&sim->recursos.cond_portoes);
    }
}

void processar_aging_decolagem(SimulacaoAeroporto* sim, Aviao* aviao, int tempo_espera) {
    if (tempo_espera >= 30 && !aviao->em_alerta) {
        aviao->em_alerta = true;
        log_evento_ui(sim, aviao, LOG_WARNING, "ALERTA! Avião %d aguardando decolagem há %ds", aviao->id, tempo_espera);
        
        pthread_cond_broadcast(&sim->recursos.cond_torres);
        pthread_cond_broadcast(&sim->recursos.cond_pistas);
        pthread_cond_broadcast(&sim->recursos.cond_portoes);
    }
}

void processar_aging_dinamico(SimulacaoAeroporto* sim, Aviao* aviao, time_t agora) {
    if (aviao->estado == AGUARDANDO_POUSO || 
        aviao->estado == AGUARDANDO_DESEMBARQUE || 
        aviao->estado == AGUARDANDO_DECOLAGEM) {
        
        int prioridade_antiga = aviao->prioridade_dinamica;
        aviao->prioridade_dinamica = calcular_prioridade_dinamica(aviao, agora, sim);
        
        atualizar_prioridade_nas_filas(sim, aviao);
        
        if (aviao->prioridade_dinamica > prioridade_antiga + 50) { 
            log_evento_ui(sim, aviao, LOG_INFO, "Prioridade atualizada: %d→%d (aging)", prioridade_antiga, aviao->prioridade_dinamica); 
        }
    }
}

void processar_aging_aviao(SimulacaoAeroporto* sim, Aviao* aviao, time_t agora) {
    if (aviao->estado == AGUARDANDO_POUSO) {
        int tempo_espera = calcular_tempo_espera_efetivo(sim, aviao->tempo_inicio_espera_ar);
        processar_aging_pouso(sim, aviao, tempo_espera);
    }
    
    if (aviao->estado == AGUARDANDO_DESEMBARQUE) {
        int tempo_espera = calcular_tempo_espera_efetivo(sim, aviao->chegada_na_fila);
        processar_aging_desembarque(sim, aviao, tempo_espera);
    }
    
    if (aviao->estado == AGUARDANDO_DECOLAGEM) {
        int tempo_espera = calcular_tempo_espera_efetivo(sim, aviao->tempo_inicio_espera);
        processar_aging_decolagem(sim, aviao, tempo_espera);
    }
    
    processar_aging_dinamico(sim, aviao, agora);
}

// =============== FUNÇÕES DE VERIFICAÇÃO E MONITORAMENTO ===============
void verificar_avioes_em_espera(SimulacaoAeroporto* sim) {
    if (sim == NULL) return;
    
    pthread_mutex_lock(&sim->mutex_pausado);
    if (sim->pausado) {
        pthread_mutex_unlock(&sim->mutex_pausado);
        return;
    }
    pthread_mutex_unlock(&sim->mutex_pausado);
    
    time_t agora = time(NULL);
    
    pthread_mutex_lock(&sim->mutex_simulacao);
    
    for (int i = 0; i < sim->metricas.total_avioes_criados && i < sim->max_avioes; i++) {
        Aviao* aviao = &sim->avioes[i];
        
        if (aviao_deve_ser_ignorado(aviao)) {
            continue;
        }
        
        processar_aging_aviao(sim, aviao, agora);
    }
    
    pthread_mutex_unlock(&sim->mutex_simulacao);
}

void verificar_recursos_orfaos(SimulacaoAeroporto* sim) {
    if (sim == NULL) return;
    
    RecursosAeroporto* recursos = &sim->recursos;
    
    if (recursos->slots_torre_disponiveis > 0 && recursos->fila_torres.tamanho > 0) {
        pthread_mutex_lock(&recursos->mutex_torres);
        if (recursos->slots_torre_disponiveis > 0 && recursos->fila_torres.tamanho > 0) {
            log_evento_ui(sim, NULL, LOG_WARNING, "Detectado torre órfã - forçando broadcast múltiplo (%d slots, %d na fila)", 
                         recursos->slots_torre_disponiveis, recursos->fila_torres.tamanho);
            
            // Força múltiplos broadcasts para garantir que as threads acordem
            for (int i = 0; i < 3; i++) {
                pthread_cond_broadcast(&recursos->cond_torres);
                pthread_mutex_unlock(&recursos->mutex_torres);
                usleep(500); // 0.5ms entre broadcasts
                pthread_mutex_lock(&recursos->mutex_torres);
            }
        }
        pthread_mutex_unlock(&recursos->mutex_torres);
    }
    
    // Verifica pistas órfãs
    if (recursos->pistas_disponiveis > 0 && recursos->fila_pistas.tamanho > 0) {
        pthread_mutex_lock(&recursos->mutex_pistas);
        if (recursos->pistas_disponiveis > 0 && recursos->fila_pistas.tamanho > 0) {
            for (int i = 0; i < 3; i++) {
                pthread_cond_broadcast(&recursos->cond_pistas);
                pthread_mutex_unlock(&recursos->mutex_pistas);
                usleep(500);
                pthread_mutex_lock(&recursos->mutex_pistas);
            }
        }
        pthread_mutex_unlock(&recursos->mutex_pistas);
    }
    
    // Verifica portões órfãos
    if (recursos->portoes_disponiveis > 0 && recursos->fila_portoes.tamanho > 0) {
        pthread_mutex_lock(&recursos->mutex_portoes);
        if (recursos->portoes_disponiveis > 0 && recursos->fila_portoes.tamanho > 0) {
            for (int i = 0; i < 3; i++) {
                pthread_cond_broadcast(&recursos->cond_portoes);
                pthread_mutex_unlock(&recursos->mutex_portoes);
                usleep(500);
                pthread_mutex_lock(&recursos->mutex_portoes);
            }
        }
        pthread_mutex_unlock(&recursos->mutex_portoes);
    }
}

void verificar_pausa_simulacao(SimulacaoAeroporto* sim) {    
    pthread_mutex_lock(&sim->mutex_pausado);
    while (sim->pausado && sim->ativa) {
        pthread_cond_wait(&sim->cond_pausado, &sim->mutex_pausado);
    }
    pthread_mutex_unlock(&sim->mutex_pausado);
}

bool verificar_avioes_em_alerta(SimulacaoAeroporto* sim) {    
    pthread_mutex_lock(&sim->mutex_simulacao);
    
    for (int i = 0; i < sim->metricas.total_avioes_criados && i < sim->max_avioes; i++) {
        Aviao* aviao = &sim->avioes[i];
        
        if (aviao->id > 0 && aviao->em_alerta && aviao->estado == AGUARDANDO_POUSO) {
            pthread_mutex_unlock(&sim->mutex_simulacao);
            return true;
        }
    }   
    pthread_mutex_unlock(&sim->mutex_simulacao);
    return false;
}

void tratar_aviao_em_alerta(SimulacaoAeroporto* sim, Aviao* aviao) {
    pthread_cond_broadcast(&sim->recursos.cond_torres);
    pthread_cond_broadcast(&sim->recursos.cond_pistas);
    pthread_cond_broadcast(&sim->recursos.cond_portoes);
    usleep(100000); 
}

void* monitorar_avioes(void* arg) {
    SimulacaoAeroporto* sim = (SimulacaoAeroporto*)arg;
    int ciclo_contador = 0;
    
    while (sim->ativa) {
        
        verificar_pausa_simulacao(sim);

        if (!sim->ativa) break;
        
        verificar_avioes_em_espera(sim);
        
        if (ciclo_contador % 3 == 0) {
            verificar_recursos_orfaos(sim);
        }
        
        if (ciclo_contador % 5 == 0) {
            detectar_deadlock(sim);
        }

        bool critico = verificar_avioes_em_alerta(sim);
        if (critico) {
            tratar_aviao_em_alerta(sim, NULL);
        } else {
            sleep(1); 
        }
        
        ciclo_contador++;
    }
    
    return NULL;
}

void atualizar_estado_aviao(Aviao* aviao, EstadoAviao novo_estado) {
    if (aviao == NULL) {
        return; 
    }

    aviao->estado = novo_estado;
    if (novo_estado == AGUARDANDO_POUSO || novo_estado == AGUARDANDO_DECOLAGEM) {
        aviao->tempo_inicio_espera = time(NULL);
    } else if (novo_estado == FINALIZADO_SUCESSO || novo_estado == FALHA_STARVATION || novo_estado == FALHA_DEADLOCK) {
        aviao->tempo_fim_operacao = time(NULL);
    }
}

void finalizar_simulacao(SimulacaoAeroporto* sim) {
    if (sim == NULL) {
        return; 
    }

    sim->ativa = 0;
    
    pthread_mutex_lock(&sim->mutex_simulacao);
    pthread_cond_broadcast(&sim->recursos.cond_pistas);
    pthread_cond_broadcast(&sim->recursos.cond_portoes);
    pthread_cond_broadcast(&sim->recursos.cond_torres);
    pthread_mutex_unlock(&sim->mutex_simulacao);
    

    pthread_mutex_lock(&sim->mutex_pausado);
    sim->pausado = false;
    
    pthread_cond_broadcast(&sim->cond_pausado);
    pthread_mutex_unlock(&sim->mutex_pausado);
}

void destruir_recursos(RecursosAeroporto* recursos) {
    if (recursos == NULL) {
        return; 
    }

    pthread_mutex_destroy(&recursos->mutex_pistas);
    pthread_cond_destroy(&recursos->cond_pistas);
    destruir_fila_prioridade(&recursos->fila_pistas);
    
    pthread_mutex_destroy(&recursos->mutex_portoes);
    pthread_cond_destroy(&recursos->cond_portoes);
    destruir_fila_prioridade(&recursos->fila_portoes);
    
    pthread_mutex_destroy(&recursos->mutex_torres);
    pthread_cond_destroy(&recursos->cond_torres);
    destruir_fila_prioridade(&recursos->fila_torres);
}

void liberar_memoria(SimulacaoAeroporto* sim) {
    if (sim == NULL) {
        return; 
    }

    if (sim->avioes != NULL) {
        free(sim->avioes);
    }

    destruir_recursos(&sim->recursos);
    pthread_mutex_destroy(&sim->mutex_simulacao);
    pthread_mutex_destroy(&sim->metricas.mutex_metricas);
    
    free(sim);
}

// REVISAR // 
int verificar_starvation(Aviao* aviao, time_t tempo_atual) {
    if (aviao == NULL) {
        return 0; 
    }

    if (aviao->tempo_inicio_espera == 0) {
        aviao->tempo_inicio_espera = tempo_atual;
    }

    if (difftime(tempo_atual, aviao->tempo_inicio_espera) > 5) { // 5 segundos de espera
        return 1; 
    }
    
    return 0; 
}

int detectar_deadlock(SimulacaoAeroporto* sim) {
    if (sim == NULL) {
        return 0; 
    }

    pthread_mutex_lock(&sim->mutex_simulacao);
    
    // Contadores para análise de deadlock
    int avioes_esperando_torre = 0;
    int avioes_esperando_pista = 0; 
    int avioes_esperando_portao = 0;
    int total_avioes_ativos = 0;
    
    for (int i = 0; i < sim->max_avioes; i++) {
        Aviao* aviao = &sim->avioes[i];
        if (aviao->id <= 0 || aviao->estado == FINALIZADO_SUCESSO || 
            aviao->estado == FALHA_STARVATION || aviao->estado == FALHA_DEADLOCK) {
            continue;
        }
        
        total_avioes_ativos++;
        
        // Conta aviões em espera por recursos
        if (aviao->estado == AGUARDANDO_POUSO || aviao->estado == AGUARDANDO_DECOLAGEM) {
            // Verifica se está na fila de torres (com lock)
            pthread_mutex_lock(&sim->recursos.fila_torres.mutex);
            for (int j = 0; j < sim->recursos.fila_torres.tamanho; j++) {
                if (sim->recursos.fila_torres.avioes_ids[j] == aviao->id) {
                    avioes_esperando_torre++;
                    break;
                }
            }
            pthread_mutex_unlock(&sim->recursos.fila_torres.mutex);
            
            // Verifica se está na fila de pistas (com lock)
            pthread_mutex_lock(&sim->recursos.fila_pistas.mutex);
            for (int j = 0; j < sim->recursos.fila_pistas.tamanho; j++) {
                if (sim->recursos.fila_pistas.avioes_ids[j] == aviao->id) {
                    avioes_esperando_pista++;
                    break;
                }
            }
            pthread_mutex_unlock(&sim->recursos.fila_pistas.mutex);
        }
        
        if (aviao->estado == AGUARDANDO_DESEMBARQUE || aviao->estado == AGUARDANDO_DECOLAGEM) {
            // Verifica se está na fila de portões (com lock)
            pthread_mutex_lock(&sim->recursos.fila_portoes.mutex);
            for (int j = 0; j < sim->recursos.fila_portoes.tamanho; j++) {
                if (sim->recursos.fila_portoes.avioes_ids[j] == aviao->id) {
                    avioes_esperando_portao++;
                    break;
                }
            }
            pthread_mutex_unlock(&sim->recursos.fila_portoes.mutex);
        }
        
        // Log de starvation sem marcar como falha - avião continua tentando
        if (aviao->estado == AGUARDANDO_POUSO || aviao->estado == AGUARDANDO_DECOLAGEM || 
            aviao->estado == AGUARDANDO_DESEMBARQUE) {
            if (verificar_starvation(aviao, time(NULL))) {
                log_evento_ui(sim, aviao, LOG_WARNING, "Avião em starvation - continuará tentando alocar recursos");
                // NÃO marca como falha - avião deve continuar tentando
            }
        }
    }
    
    // Detecção de padrão de deadlock circular
    // Se muitos aviões estão esperando por recursos e os recursos estão ocupados
    bool possivel_deadlock = false;
    
    if (total_avioes_ativos > 3) { // Só considera deadlock com múltiplos aviões
        // Deadlock com torres: muitos aviões esperando torre, mas torre disponível
        if (avioes_esperando_torre > sim->recursos.capacidade_torre && 
            sim->recursos.slots_torre_disponiveis == 0) {
            possivel_deadlock = true;
        }
        
        // Deadlock circular entre pistas e torres
        if (avioes_esperando_pista > 1 && avioes_esperando_torre > 1 &&
            sim->recursos.pistas_disponiveis == 0 && sim->recursos.slots_torre_disponiveis == 0) {
            possivel_deadlock = true;
        }
        
        // Deadlock com portões e torres
        if (avioes_esperando_portao > sim->recursos.total_portoes && 
            avioes_esperando_torre > 0 && sim->recursos.portoes_disponiveis == 0) {
            possivel_deadlock = true;
        }
    }
    
    if (possivel_deadlock) {
        log_evento_ui(sim, NULL, LOG_ERROR, "Possível DEADLOCK detectado - forçando broadcasts de emergência");
        
        // Força broadcasts múltiplos em todos os recursos
        pthread_cond_broadcast(&sim->recursos.cond_torres);
        pthread_cond_broadcast(&sim->recursos.cond_pistas);  
        pthread_cond_broadcast(&sim->recursos.cond_portoes);
        
        pthread_mutex_unlock(&sim->mutex_simulacao);
        return 1;
    }

    pthread_mutex_unlock(&sim->mutex_simulacao);
    return 0; 
}

int gerar_numero_aleatorio(int min, int max) {
    if (min >= max) {
        return min; 
    }

    return rand() % (max - min + 1) + min;
}

void verificar_pausa(SimulacaoAeroporto* sim) {
    pthread_mutex_lock(&sim->mutex_pausado);
    while (sim->pausado && sim->ativa) {
        pthread_cond_wait(&sim->cond_pausado, &sim->mutex_pausado);
    }
    pthread_mutex_unlock(&sim->mutex_pausado);
}

const char* estado_para_str(EstadoAviao estado) {
    switch (estado) {
        case AGUARDANDO_POUSO:          return "Aguard. Pouso";
        case POUSANDO:                  return "Pousando";
        case AGUARDANDO_DESEMBARQUE:    return "Aguard. Desemb.";
        case DESEMBARCANDO:             return "Desembarcando";
        case AGUARDANDO_DECOLAGEM:      return "Aguard. Decol.";
        case DECOLANDO:                 return "Decolando";
        case FINALIZADO_SUCESSO:        return "Finalizado";
        default:                        return "Falha";
    }
}

// Calcula o tempo efetivo da simulação descontando pausas
double calcular_tempo_efetivo_simulacao(SimulacaoAeroporto* sim) {
    if (!sim) return 0.0;
    
    time_t agora = time(NULL);
    double tempo_total = difftime(agora, sim->tempo_inicio);
    double tempo_pausa_atual = 0.0;
    
    // Se estamos em pausa, inclui o tempo da pausa atual
    if (sim->pausado && sim->inicio_pausa > 0) {
        tempo_pausa_atual = difftime(agora, sim->inicio_pausa);
    }
    
    return tempo_total - sim->tempo_pausado_total - tempo_pausa_atual;
}

// Atualiza o controle de tempo de pausa
void atualizar_tempo_pausa(SimulacaoAeroporto* sim, bool iniciando_pausa) {
    if (!sim) return;
    
    if (iniciando_pausa) {
        sim->inicio_pausa = time(NULL);
    } else {
        // Finalizando pausa - acumula o tempo pausado
        if (sim->inicio_pausa > 0) {
            time_t agora = time(NULL);
            sim->tempo_pausado_total += difftime(agora, sim->inicio_pausa);
            sim->inicio_pausa = 0;
        }
    }
}

// Calcula o tempo efetivo de espera de um avião descontando pausas
int calcular_tempo_espera_efetivo(SimulacaoAeroporto* sim, time_t inicio_espera) {
    if (!sim || inicio_espera <= 0) return 0;
    
    time_t agora = time(NULL);
    double tempo_total = difftime(agora, inicio_espera);
    double tempo_pausa_descontar = 0.0;
    
    // Se estamos pausados agora e a pausa começou depois do início da espera
    if (sim->pausado && sim->inicio_pausa > 0 && sim->inicio_pausa >= inicio_espera) {
        tempo_pausa_descontar += difftime(agora, sim->inicio_pausa);
    }
    
    // Desconta o tempo total já pausado (aproximação - considera que o avião estava presente durante todas as pausas anteriores)
    if (sim->tempo_pausado_total > 0) {
        tempo_pausa_descontar += sim->tempo_pausado_total;
    }
    
    int tempo_efetivo = (int)(tempo_total - tempo_pausa_descontar);
    return tempo_efetivo > 0 ? tempo_efetivo : 0;
}

bool is_safe_state(Banqueiro* banco) {
    int work[N_RESOURCES];
    bool finish[MAX_AVIOES];
    
    // 1. Inicializar recursos de trabalho
    for (int i = 0; i < N_RESOURCES; i++) {
        work[i] = banco->disponivel[i];
    }
    
    // 2. Identificar aviões ativos (mais agressivo)
    int avioes_ativos = 0;
    for (int i = 0; i < MAX_AVIOES; i++) {
        finish[i] = true;
        
        // Avião ativo se tem alocação OU necessidade válida (1 por recurso)
        bool ativo = false;
        for (int j = 0; j < N_RESOURCES; j++) {
            if (banco->alocacao[i][j] > 0 || banco->necessidade[i][j] == 1) {
                ativo = true;
                break;
            }
        }
        
        if (ativo) {
            finish[i] = false;
            avioes_ativos++;
        }
    }
    
    // 3. OTIMIZAÇÃO: Se há poucos aviões ativos, ser mais permissivo
    if (avioes_ativos <= 3) {
        // Com poucos aviões, quase sempre é seguro
        return true;
    }
    
    // 4. OTIMIZAÇÃO: Verificação rápida de recursos abundantes
    // Se há recursos suficientes para todos os aviões ativos, é seguro
    int recursos_necessarios[N_RESOURCES] = {0};
    for (int i = 0; i < MAX_AVIOES; i++) {
        if (!finish[i]) {
            for (int j = 0; j < N_RESOURCES; j++) {
                recursos_necessarios[j] += banco->necessidade[i][j];
            }
        }
    }
    
    bool recursos_abundantes = true;
    for (int i = 0; i < N_RESOURCES; i++) {
        if (work[i] < recursos_necessarios[i]) {
            recursos_abundantes = false;
            break;
        }
    }
    
    if (recursos_abundantes) {
        return true; // Recursos suficientes para todos
    }
    
    // 5. Algoritmo tradicional do banqueiro (mais rigoroso)
    bool progresso = true;
    int iteracoes = 0;
    const int MAX_ITERACOES = avioes_ativos * 2; // Evita loops infinitos
    
    while (progresso && iteracoes < MAX_ITERACOES) {
        progresso = false;
        iteracoes++;
        
        for (int i = 0; i < MAX_AVIOES; i++) {
            if (!finish[i]) {
                // Verificar se pode satisfazer necessidade atual
                bool pode_terminar = true;
                for (int j = 0; j < N_RESOURCES; j++) {
                    if (banco->necessidade[i][j] > work[j]) {
                        pode_terminar = false;
                        break;
                    }
                }
                
                if (pode_terminar) {
                    // Simular liberação dos recursos
                    for (int j = 0; j < N_RESOURCES; j++) {
                        work[j] += banco->alocacao[i][j];
                    }
                    finish[i] = true;
                    progresso = true;
                }
            }
        }
    }
    
    // 6. Verificar se todos os aviões ativos podem terminar
    for (int i = 0; i < MAX_AVIOES; i++) {
        if (!finish[i]) {
            return false; // Estado potencialmente inseguro
        }
    }
    
    return true; // Estado seguro
}

int banker_request_resources(RecursosAeroporto* recursos, int aviao_id, int request[]) {
    if (aviao_id < 0 || aviao_id >= MAX_AVIOES) { return -1; }
    
    int aviao_id_original = aviao_id + 1;
    
    // 1. Verificar se o pedido é válido
    for (int i = 0; i < N_RESOURCES; i++) {
        if (request[i] < 0 || request[i] > recursos->banco.necessidade[aviao_id][i]) {
            return -2;
        }
    }
    
    // 2. Verificar filas de prioridade
    if (request[RECURSO_PISTA] > 0 && !eh_minha_vez_na_fila(&recursos->fila_pistas, aviao_id_original)) {
        return -5;
    }
    if (request[RECURSO_PORTAO] > 0 && !eh_minha_vez_na_fila(&recursos->fila_portoes, aviao_id_original)) {
        return -6;
    }
    if (request[RECURSO_TORRE] > 0 && !eh_minha_vez_na_fila(&recursos->fila_torres, aviao_id_original)) {
        return -7;
    }
    
    // 3. Verificar disponibilidade física ANTES da simulação
    if (request[RECURSO_PISTA] > 0 && recursos->pistas_disponiveis == 0) {
        return -3;
    }
    if (request[RECURSO_PORTAO] > 0 && recursos->portoes_disponiveis == 0) {
        return -3;
    }
    if (request[RECURSO_TORRE] > 0 && recursos->slots_torre_disponiveis == 0) {
        return -3;
    }
    
    // 4. Simular alocação temporária
    for (int i = 0; i < N_RESOURCES; i++) {
        recursos->banco.disponivel[i] -= request[i];
        recursos->banco.alocacao[aviao_id][i] += request[i];
        recursos->banco.necessidade[aviao_id][i] -= request[i];
    }
    
    // 5. Verificar se o estado permanece seguro
    if (!is_safe_state(&recursos->banco)) {
        // Reverter simulação
        for (int i = 0; i < N_RESOURCES; i++) {
            recursos->banco.disponivel[i] += request[i];
            recursos->banco.alocacao[aviao_id][i] -= request[i];
            recursos->banco.necessidade[aviao_id][i] += request[i];
        }
        return -4;
    }
    
    // 6. ALOCAÇÃO FÍSICA (fazer aqui, não depois)
    if (request[RECURSO_PISTA] > 0) {
        pthread_mutex_lock(&recursos->mutex_pistas);
        for (int i = 0; i < recursos->total_pistas; i++) {
            if (recursos->pista_ocupada_por[i] == -1) {
                recursos->pista_ocupada_por[i] = aviao_id_original;
                recursos->pistas_disponiveis--;
                break;
            }
        }
        pthread_mutex_unlock(&recursos->mutex_pistas);
    }
    
    if (request[RECURSO_PORTAO] > 0) {
        pthread_mutex_lock(&recursos->mutex_portoes);
        for (int i = 0; i < recursos->total_portoes; i++) {
            if (recursos->portao_ocupado_por[i] == -1) {
                recursos->portao_ocupado_por[i] = aviao_id_original;
                recursos->portoes_disponiveis--;
                break;
            }
        }
        pthread_mutex_unlock(&recursos->mutex_portoes);
    }
    
    if (request[RECURSO_TORRE] > 0) {
        pthread_mutex_lock(&recursos->mutex_torres);
        for (int i = 0; i < recursos->capacidade_torre; i++) {
            if (recursos->torre_ocupada_por[i] == -1) {
                recursos->torre_ocupada_por[i] = aviao_id_original;
                recursos->slots_torre_disponiveis--;
                recursos->operacoes_ativas_torre++;
                break;
            }
        }
        pthread_mutex_unlock(&recursos->mutex_torres);
    }
    
    // 7. Remover das filas
    if (request[RECURSO_PISTA] > 0) {
        remover_da_fila_prioridade(&recursos->fila_pistas, aviao_id_original);
    }
    if (request[RECURSO_PORTAO] > 0) {
        remover_da_fila_prioridade(&recursos->fila_portoes, aviao_id_original);
    }
    if (request[RECURSO_TORRE] > 0) {
        remover_da_fila_prioridade(&recursos->fila_torres, aviao_id_original);
    }

    return 0; // Sucesso
}

int banker_request_single_resource(SimulacaoAeroporto* sim, Aviao* aviao, TipoRecurso tipo_recurso) {
    return solicitar_recurso_individual(sim, aviao, tipo_recurso);
}

int banker_release_resources(RecursosAeroporto* recursos, int aviao_id, int release[]) {
    if (aviao_id < 0 || aviao_id >= MAX_AVIOES) {
        return; // ID inválido
    }
    
    // 1. Verificar se o release é válido
    for (int i = 0; i < N_RESOURCES; i++) {
        if (release[i] < 0 || release[i] > recursos->banco.alocacao[aviao_id][i]) {

            return -1; // Release inválido
        }
    }
    
    // 2. Liberar os recursos
    for (int i = 0; i < N_RESOURCES; i++) {
        recursos->banco.disponivel[i] += release[i];
        recursos->banco.alocacao[aviao_id][i] -= release[i];
        recursos->banco.necessidade[aviao_id][i] += release[i];
    }
    
    // 3. Sinalizar as filas de prioridade dos recursos liberados
    if (release[RECURSO_PISTA] > 0) {
        pthread_cond_signal(&recursos->fila_pistas.cond);
    }
    if (release[RECURSO_PORTAO] > 0) {
        pthread_cond_signal(&recursos->fila_portoes.cond);
    }
    if (release[RECURSO_TORRE] > 0) {
        pthread_cond_signal(&recursos->fila_torres.cond);
    }
    
    // 4. Sinalizar também o condition variable geral do banco
    pthread_cond_broadcast(&recursos->cond_banco);
    return 0;
}

void definir_necessidade_operacao(EstadoAviao operacao, int necessidade[N_RESOURCES]) {
    for (int i = 0; i < N_RESOURCES; i++) {
        necessidade[i] = 0;
    }
    
    switch (operacao) {
        case AGUARDANDO_POUSO:
            necessidade[RECURSO_PISTA] = 1;
            necessidade[RECURSO_TORRE] = 1;
            break;
            
        case AGUARDANDO_DESEMBARQUE:
            necessidade[RECURSO_PORTAO] = 1;
            necessidade[RECURSO_TORRE] = 1;
            break;
            
        case AGUARDANDO_DECOLAGEM:
            necessidade[RECURSO_PISTA] = 1;
            necessidade[RECURSO_PORTAO] = 1;
            necessidade[RECURSO_TORRE] = 1;
            break;
            
        default:
            break;
    }
}

void banker_init_aviao(RecursosAeroporto* recursos, int aviao_id) {
    if (aviao_id < 0 || aviao_id >= MAX_AVIOES) {
        return; // ID inválido
    }

    pthread_mutex_lock(&recursos->mutex_banco);

    // A necessidade máxima é para a decolagem (pior caso)
    recursos->banco.necessidade[aviao_id][RECURSO_PISTA] = 1;
    recursos->banco.necessidade[aviao_id][RECURSO_PORTAO] = 1;
    recursos->banco.necessidade[aviao_id][RECURSO_TORRE] = 1;

    // Garante que a alocação inicial é zero para este avião
    for (int i = 0; i < N_RESOURCES; i++) {
        recursos->banco.alocacao[aviao_id][i] = 0;
    }

    pthread_mutex_unlock(&recursos->mutex_banco);
}