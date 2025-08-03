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
    while (pos > 0 && fila->prioridades[pos - 1] < prioridade) {
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

int remover_da_fila_prioridade(FilaPrioridade* fila) {
    if (fila == NULL) return -1;
    
    pthread_mutex_lock(&fila->mutex);
    
    if (fila->tamanho == 0) {
        pthread_mutex_unlock(&fila->mutex);
        return -1;
    }
    
    int aviao_id = fila->avioes_ids[0];
    
    // Move todos os elementos uma posição para frente
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
int calcular_prioridade_dinamica(Aviao* aviao, time_t tempo_atual) {
    if (aviao == NULL) return 0;
    
    int prioridade_base = (aviao->tipo == VOO_INTERNACIONAL) ? 100 : 50;
    int tempo_espera = (int)difftime(tempo_atual, aviao->tempo_inicio_espera_ar);
    
    // Prioridade aumenta com o tempo de espera
    int bonus_tempo = tempo_espera * 2;
    
    // Bônus para aviões em alerta ou com crash iminente
    int bonus_emergencia = 0;
    if (aviao->crash_iminente) {
        bonus_emergencia = 1000; // Prioridade máxima
    } else if (aviao->em_alerta) {
        bonus_emergencia = 500;
    }
    
    return prioridade_base + bonus_tempo + bonus_emergencia;
}

void verificar_avioes_em_espera(SimulacaoAeroporto* sim) {
    if (sim == NULL) return;
    
    time_t agora = time(NULL);
    
    pthread_mutex_lock(&sim->mutex_simulacao);
    
    for (int i = 0; i < sim->metricas.total_avioes_criados && i < sim->max_avioes; i++) {
        Aviao* aviao = &sim->avioes[i];
        
        if (aviao->id <= 0 || aviao->estado == FINALIZADO_SUCESSO || 
            aviao->estado == FALHA_STARVATION || aviao->estado == FALHA_DEADLOCK) {
            continue;
        }
        
        if (aviao->estado == AGUARDANDO_POUSO) {
            int tempo_espera = (int)difftime(agora, aviao->tempo_inicio_espera_ar);
            
            if (tempo_espera >= 90 && !aviao->crash_iminente) {
                // Avião crashed após 90 segundos
                aviao->crash_iminente = true;
                log_evento_ui(sim, aviao, LOG_ERROR, "CRASH! Avião %d caiu após 90s de espera no ar", aviao->id);
                atualizar_estado_aviao(aviao, FALHA_STARVATION);
                incrementar_aviao_falha_starvation(&sim->metricas);
                
            } else if (tempo_espera >= 60 && !aviao->em_alerta) {
                // Alerta após 60 segundos
                aviao->em_alerta = true;
                log_evento_ui(sim, aviao, LOG_WARNING, "ALERTA! Avião %d há %ds no ar - Situação crítica!", aviao->id, tempo_espera);
            }
            
            // Atualiza prioridade dinâmica
            aviao->prioridade_dinamica = calcular_prioridade_dinamica(aviao, agora);
        }
    }
    
    pthread_mutex_unlock(&sim->mutex_simulacao);
}

void* monitorar_avioes(void* arg) {
    SimulacaoAeroporto* sim = (SimulacaoAeroporto*)arg;
    
    while (sim->ativa) {
        verificar_avioes_em_espera(sim);
        
        // Verifica a cada 2 segundos
        sleep(2);
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
    
    for (int i = 0; i < sim->max_avioes; i++) {
        Aviao* aviao = &sim->avioes[i];
        if (aviao->estado == AGUARDANDO_POUSO || aviao->estado == AGUARDANDO_DECOLAGEM) {
            if (verificar_starvation(aviao, time(NULL))) {
                atualizar_estado_aviao(aviao, FALHA_STARVATION);
                incrementar_aviao_starvation(&sim->metricas);
                pthread_mutex_unlock(&sim->mutex_simulacao);
                return 1; 
            }
        }
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
    // printf("Torres Disponíveis: %d/%d\n", recursos->torres_disponiveis, recursos->total_torres);
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