#include "utils.h"
#include "terminal.h"

// =============== FUNÇÕES DE FILA DE PRIORIDADE ===============
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
    
    // Insere ordenado por prioridade (maior prioridade primeiro)
    int pos = fila->tamanho;
    while(pos > 0 && fila->prioridades[pos-1] <= prioridade){
        fila->avioes_ids[pos] = fila->avioes_ids[pos - 1];
        fila->prioridades[pos] = fila->prioridades[pos - 1];
        pos--;
    }
    // int pos = fila->tamanho;
    // while (pos > 0 && fila->prioridades[pos - 1] < prioridade) {
    //     fila->avioes_ids[pos] = fila->avioes_ids[pos - 1];
    //     fila->prioridades[pos] = fila->prioridades[pos - 1];
    //     pos--;
    // }
    
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
    
    for (int i = 0; i < fila->tamanho - 1; i++) {
        fila->avioes_ids[i] = fila->avioes_ids[i + 1];
        fila->prioridades[i] = fila->prioridades[i + 1];
    }
    
    fila->tamanho--;
    fila->avioes_ids[fila->tamanho] = -1;
    fila->prioridades[fila->tamanho] = 0;
    
    pthread_mutex_unlock(&fila->mutex);
    return aviao_id;
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
    if (!fila || fila->tamanho == 0) { return false; }
    
    return fila->avioes_ids[0] == aviao_id; 
}

void atualizar_prioridade_na_fila(FilaPrioridade* fila, int aviao_id, int nova_prioridade) {
    if (!fila || fila->tamanho == 0) return;
    
    pthread_mutex_lock(&fila->mutex);
    
    // Encontra o avião na fila
    int pos_atual = -1;
    for (int i = 0; i < fila->tamanho; i++) {
        if (fila->avioes_ids[i] == aviao_id) {
            pos_atual = i;
            break;
        }
    }
    
    if (pos_atual == -1) {
        pthread_mutex_unlock(&fila->mutex);
        return; // Avião não está na fila
    }
    
    // Remove temporariamente o avião da posição atual
    for (int i = pos_atual; i < fila->tamanho - 1; i++) {
        fila->avioes_ids[i] = fila->avioes_ids[i + 1];
        fila->prioridades[i] = fila->prioridades[i + 1];
    }
    fila->tamanho--;
    
    // Reinsere na posição correta com a nova prioridade
    int nova_pos = fila->tamanho;
    while (nova_pos > 0 && fila->prioridades[nova_pos - 1] <= nova_prioridade) {
        fila->avioes_ids[nova_pos] = fila->avioes_ids[nova_pos - 1];
        fila->prioridades[nova_pos] = fila->prioridades[nova_pos - 1];
        nova_pos--;
    }
    
    fila->avioes_ids[nova_pos] = aviao_id;
    fila->prioridades[nova_pos] = nova_prioridade;
    fila->tamanho++;
    
    pthread_mutex_unlock(&fila->mutex);
}

int calcular_prioridade_dinamica(Aviao* aviao, time_t agora, SimulacaoAeroporto* sim) {
    if (aviao == NULL) return 0;
    
    // Prioridades base mais equilibradas
    int prioridade_base = (aviao->tipo == VOO_INTERNACIONAL) ? 25 : 20;
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
        // Fórmula mais agressiva: tempo³ / 20 + tempo² / 5 + tempo * 8
        // Isso cria uma curva muito íngreme para aviões esperando há muito tempo
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
        log_evento_ui(NULL, aviao, LOG_INFO, "🔥 Prioridade alta por aging: %d (tempo: %ds)", 
                     prioridade_final, tempo_espera);
    }
    
    return prioridade_final;
}

void atualizar_prioridade_nas_filas(SimulacaoAeroporto* sim, Aviao* aviao) {
    if (!sim || !aviao) return;
    
    RecursosAeroporto* recursos = &sim->recursos;
    
    // Atualiza prioridade na fila de pistas (para pouso e decolagem)
    if (aviao->estado == AGUARDANDO_POUSO || aviao->estado == AGUARDANDO_DECOLAGEM) {
        atualizar_prioridade_na_fila(&recursos->fila_pistas, aviao->id, aviao->prioridade_dinamica);
    }
    
    // Atualiza prioridade na fila de portões (para desembarque e decolagem)
    if (aviao->estado == AGUARDANDO_DESEMBARQUE || aviao->estado == AGUARDANDO_DECOLAGEM) {
        atualizar_prioridade_na_fila(&recursos->fila_portoes, aviao->id, aviao->prioridade_dinamica);
    }
    
    // Atualiza prioridade na fila de torres (para todas as operações)
    if (aviao->estado == AGUARDANDO_POUSO || 
        aviao->estado == AGUARDANDO_DESEMBARQUE || 
        aviao->estado == AGUARDANDO_DECOLAGEM) {
        atualizar_prioridade_na_fila(&recursos->fila_torres, aviao->id, aviao->prioridade_dinamica);
    }
}

void verificar_avioes_em_espera(SimulacaoAeroporto* sim) {
    if (sim == NULL) return;
    
    // Não atualiza tempos durante pausa
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
        
        if (aviao->id <= 0 || aviao->estado == FINALIZADO_SUCESSO || 
            aviao->estado == FALHA_STARVATION || aviao->estado == FALHA_DEADLOCK) {
            continue;
        }
        
        // AGING PARA AVIÕES AGUARDANDO POUSO
        if (aviao->estado == AGUARDANDO_POUSO) {
            int tempo_espera = calcular_tempo_espera_efetivo(sim, aviao->tempo_inicio_espera_ar);
            
            if (tempo_espera >= 90 && !aviao->crash_iminente) {
                // Avião crashed após 90 segundos
                aviao->crash_iminente = true;
                log_evento_ui(sim, aviao, LOG_ERROR, "CRASH! Avião %d caiu após 90s de espera no ar", aviao->id);
                atualizar_estado_aviao(aviao, FALHA_STARVATION);
                incrementar_aviao_falha_starvation(&sim->metricas);
                
            } else if (tempo_espera >= 60 && !aviao->em_alerta) {
                // Alerta após 60 segundos - PRIORIDADE EXTREMA
                aviao->em_alerta = true;
                log_evento_ui(sim, aviao, LOG_WARNING, "ALERTA CRÍTICO! Avião %d há %ds no ar - PRIORIDADE EXTREMA!", aviao->id, tempo_espera);
                
                // Força um broadcast para que este avião seja processado imediatamente
                pthread_cond_broadcast(&sim->recursos.cond_torres);
                pthread_cond_broadcast(&sim->recursos.cond_pistas);
                pthread_cond_broadcast(&sim->recursos.cond_portoes);
            }
        }
        
        // AGING PARA AVIÕES AGUARDANDO DESEMBARQUE
        if (aviao->estado == AGUARDANDO_DESEMBARQUE) {
            int tempo_espera = calcular_tempo_espera_efetivo(sim, aviao->chegada_na_fila);
            
            if (tempo_espera >= 45 && !aviao->em_alerta) {
                aviao->em_alerta = true;
                log_evento_ui(sim, aviao, LOG_WARNING, "ALERTA! Avião %d aguardando desembarque há %ds", aviao->id, tempo_espera);
                
                // Força broadcasts para acelerar processamento
                pthread_cond_broadcast(&sim->recursos.cond_torres);
                pthread_cond_broadcast(&sim->recursos.cond_portoes);
            }
        }
        
        // AGING PARA AVIÕES AGUARDANDO DECOLAGEM
        if (aviao->estado == AGUARDANDO_DECOLAGEM) {
            int tempo_espera = calcular_tempo_espera_efetivo(sim, aviao->tempo_inicio_espera);
            
            if (tempo_espera >= 30 && !aviao->em_alerta) {
                aviao->em_alerta = true;
                log_evento_ui(sim, aviao, LOG_WARNING, "ALERTA! Avião %d aguardando decolagem há %ds", aviao->id, tempo_espera);
                
                // Força broadcasts para acelerar processamento
                pthread_cond_broadcast(&sim->recursos.cond_torres);
                pthread_cond_broadcast(&sim->recursos.cond_pistas);
                pthread_cond_broadcast(&sim->recursos.cond_portoes);
            }
        }
        
        // AGING DINÂMICO - Atualiza prioridade para todos os aviões em espera
        if (aviao->estado == AGUARDANDO_POUSO || 
            aviao->estado == AGUARDANDO_DESEMBARQUE || 
            aviao->estado == AGUARDANDO_DECOLAGEM) {
            
            // Atualiza prioridade dinâmica
            int prioridade_antiga = aviao->prioridade_dinamica;
            aviao->prioridade_dinamica = calcular_prioridade_dinamica(aviao, agora, sim);
            
            // Atualiza prioridade nas filas correspondentes
            atualizar_prioridade_nas_filas(sim, aviao);
            
            // Log quando a prioridade aumenta significativamente
            if (aviao->prioridade_dinamica > prioridade_antiga + 50) {
                log_evento_ui(sim, aviao, LOG_INFO, "⚡ Prioridade atualizada: %d→%d (aging)", 
                             prioridade_antiga, aviao->prioridade_dinamica);
            }
        }
    }
    
    pthread_mutex_unlock(&sim->mutex_simulacao);
}

void verificar_recursos_orfaos(SimulacaoAeroporto* sim) {
    if (sim == NULL) return;
    
    RecursosAeroporto* recursos = &sim->recursos;
    
    // Verifica torres órfãs com mais agressividade
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
            for (int i = 0; i < 2; i++) {
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
            for (int i = 0; i < 2; i++) {
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
    if (sim == NULL) return;
    
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

void imprimir_status_operacao(int id_aviao, TipoVoo tipo, const char* operacao, const char* status) {
    return; 
    //printf("Avião ID: %d, Tipo: %s, Operação: %s, Status: %s\n", id_aviao, tipo == VOO_DOMESTICO ? "Doméstico" : "Internacional", operacao, status);
}

void imprimir_status_recursos(RecursosAeroporto* recursos) {
    if (recursos == NULL) {
        return; 
    }

    // printf("Recursos do Aeroporto:\n");
    // printf("Pistas Disponíveis: %d/%d\n", recursos->pistas_disponiveis, recursos->total_pistas);
    // printf("Portões Disponíveis: %d/%d\n", recursos->portoes_disponiveis, recursos->total_portoes);
    // printf("Torres Disponíveis: slots %d/%d (operações ativas: %d)\n", recursos->slots_torre_disponiveis, recursos->capacidade_torre, recursos->operacoes_ativas_torre);
}

void gerar_relatorio_final(SimulacaoAeroporto* sim) {
    if (sim == NULL) {
        return; 
    }
    // IMPORTANTE
    // Imprime o relatório final da simulação SEM USAR PRINTF, talvez uma janela da UI antes de encerrar? Talvez criar um arquivo de log?
    ///////////////////////////////////////////////////////////////

    // printf("Relatório Final da Simulação:\n");
    // printf("Total de Aviões Criados: %d\n", sim->metricas.total_avioes_criados);
    // printf("Aviões Finalizados com Sucesso: %d\n", sim->metricas.avioes_finalizados_sucesso);
    // printf("Aviões com Falha por Starvation: %d\n", sim->metricas.avioes_falha_starvation);
    // printf("Aviões com Falha por Deadlock: %d\n", sim->metricas.avioes_falha_deadlock);
    // printf("Voos Domésticos: %d\n", sim->metricas.voos_domesticos_total);
    // printf("Voos Internacionais: %d\n", sim->metricas.voos_internacionais_total);
    // printf("Operações de Pouso: %d\n", sim->metricas.operacoes_pouso);
    // printf("Operações de Desembarque: %d\n", sim->metricas.operacoes_desembarque);
    // printf("Operações de Decolagem: %d\n", sim->metricas.operacoes_decolagem);
}

void imprimir_resumo_aviao(Aviao* aviao) {
    if (aviao == NULL) {
        return; 
    }

    printf("Avião ID: %d, Tipo: %s, Estado: %d, Tempo de Criação: %ld\n", 
           aviao->id, 
           aviao->tipo == VOO_DOMESTICO ? "Doméstico" : "Internacional", 
           aviao->estado, 
           aviao->tempo_criacao);
}

int gerar_numero_aleatorio(int min, int max) {
    if (min >= max) {
        return min; 
    }

    return rand() % (max - min + 1) + min; // Gera número aleatório entre min e max
}

void dormir_operacao(int min_ms, int max_ms) {
    if (min_ms < 0 || max_ms < min_ms) {
        return; 
    }

    int tempo = gerar_numero_aleatorio(min_ms, max_ms);
    usleep(tempo * 1000); // Converte milissegundos para microssegundos
}

// New pause-aware sleep function
void dormir_operacao_com_pausa(SimulacaoAeroporto* sim, int min_ms, int max_ms) {
    if (min_ms < 0 || max_ms < min_ms || !sim) {
        return; 
    }

    int tempo = gerar_numero_aleatorio(min_ms, max_ms);
    int sleep_chunks = tempo / 100; // Break sleep into 100ms chunks
    int remaining_ms = tempo % 100;
    
    for (int i = 0; i < sleep_chunks && sim->ativa; i++) {
        // Check for pause before each chunk
        pthread_mutex_lock(&sim->mutex_pausado);
        while (sim->pausado && sim->ativa) {
            pthread_cond_wait(&sim->cond_pausado, &sim->mutex_pausado);
        }
        pthread_mutex_unlock(&sim->mutex_pausado);
        
        if (!sim->ativa) break;
        usleep(100 * 1000); // 100ms
    }
    
    // Sleep remaining time if any
    if (remaining_ms > 0 && sim->ativa) {
        pthread_mutex_lock(&sim->mutex_pausado);
        while (sim->pausado && sim->ativa) {
            pthread_cond_wait(&sim->cond_pausado, &sim->mutex_pausado);
        }
        pthread_mutex_unlock(&sim->mutex_pausado);
        
        if (sim->ativa) {
            usleep(remaining_ms * 1000);
        }
    }
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