#include "airplane.h"

Aviao* criar_aviao(int id, TipoVoo tipo) {
    Aviao* aviao = (Aviao*)malloc(sizeof(Aviao));
    if (aviao == NULL) {
        perror("Falha ao alocar memória para o avião");
        return NULL;
    }

    aviao->id = id;
    aviao->tipo = tipo;
    aviao->estado = AGUARDANDO_POUSO;
    aviao->tempo_criacao = time(NULL);
    aviao->chegada_na_fila = time(NULL); 
    aviao->tempo_inicio_espera_ar = time(NULL); 
    aviao->prioridade_dinamica = 0;
    aviao->em_alerta = false;
    aviao->crash_iminente = false;
    aviao->thread_id = 0;
    aviao->pista_alocada = -1;
    aviao->portao_alocado = -1;
    aviao->torre_alocada = -1;

    return aviao;
}

TipoVoo gerar_tipo_voo_aleatorio() {
    return rand() % 2;
}

int pouso_internacional_atomico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando procedimento de pouso internacional (ATÔMICO)");
    
    if (aviao->crash_iminente) {
        log_evento_ui(sim, aviao, LOG_ERROR, "ABORTOU: Avião já havia crashed");
        return 0;
    }
    
    if (alocar_recursos_pouso_atomico(sim, aviao) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA: Não foi possível alocar recursos para pouso");
        return 0;
    }
    
    aviao->estado = POUSANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "POUSANDO - Pista %d, Torre slot %d", aviao->pista_alocada, aviao->torre_alocada - 1);
    
    sleep(2 + rand() % 3);
    
    if (aviao->pista_alocada >= 0) {
        pthread_mutex_lock(&sim->recursos.mutex_pistas);
        sim->recursos.pista_ocupada_por[aviao->pista_alocada] = -1;
        sim->recursos.pistas_disponiveis++;
        pthread_cond_broadcast(&sim->recursos.cond_pistas);
        pthread_mutex_unlock(&sim->recursos.mutex_pistas);
        aviao->pista_alocada = -1;
    }
    
    aviao->estado = AGUARDANDO_DESEMBARQUE;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Pouso concluído - aguardando desembarque");
    
    return 1;
}

int pouso_domestico_atomico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando procedimento de pouso doméstico (ATÔMICO)");
    
    if (aviao->crash_iminente) {
        log_evento_ui(sim, aviao, LOG_ERROR, "ABORTOU: Avião já havia crashed");
        return 0;
    }
    
    if (alocar_recursos_pouso_atomico(sim, aviao) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA: Não foi possível alocar recursos para pouso");
        return 0;
    }
    
    aviao->estado = POUSANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "POUSANDO - Pista %d, Torre slot %d", 
                  aviao->pista_alocada, aviao->torre_alocada - 1);
    
    sleep(1 + rand() % 2);
    
    if (aviao->pista_alocada >= 0) {
        pthread_mutex_lock(&sim->recursos.mutex_pistas);
        sim->recursos.pista_ocupada_por[aviao->pista_alocada] = -1;
        sim->recursos.pistas_disponiveis++;
        pthread_cond_broadcast(&sim->recursos.cond_pistas);
        pthread_mutex_unlock(&sim->recursos.mutex_pistas);
        aviao->pista_alocada = -1;
    }
    
    aviao->estado = AGUARDANDO_DESEMBARQUE;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Pouso concluído - aguardando desembarque");
    
    return 1;
}

int desembarque_internacional_atomico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando desembarque internacional (ATÔMICO)");
    
    // Libera torre primeiro (não precisa mais dela)
    if (aviao->torre_alocada > 0) {
        pthread_mutex_lock(&sim->recursos.mutex_torres);
        int slot = aviao->torre_alocada - 1;
        if (slot >= 0 && slot < sim->recursos.capacidade_torre) {
            sim->recursos.torre_ocupada_por[slot] = -1;
        }
        sim->recursos.slots_torre_disponiveis++;
        sim->recursos.operacoes_ativas_torre--;
        aviao->torre_alocada = 0;
        pthread_cond_broadcast(&sim->recursos.cond_torres);
        pthread_mutex_unlock(&sim->recursos.mutex_torres);
    }
    
    // Aloca portão
    if (alocar_recursos_desembarque_atomico(sim, aviao) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA: Não foi possível alocar portão para desembarque");
        return 0;
    }
    
    aviao->estado = DESEMBARCANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "DESEMBARCANDO - Portão %d", aviao->portao_alocado);
    
    sleep(3 + rand() % 4); // Desembarque internacional demora mais
    
    aviao->estado = AGUARDANDO_DECOLAGEM;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Desembarque concluído - aguardando decolagem");
    
    return 1;
}

int desembarque_domestico_atomico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando desembarque doméstico (ATÔMICO)");
    
    // Libera torre primeiro (não precisa mais dela)
    if (aviao->torre_alocada > 0) {
        pthread_mutex_lock(&sim->recursos.mutex_torres);
        int slot = aviao->torre_alocada - 1;
        if (slot >= 0 && slot < sim->recursos.capacidade_torre) {
            sim->recursos.torre_ocupada_por[slot] = -1;
        }
        sim->recursos.slots_torre_disponiveis++;
        sim->recursos.operacoes_ativas_torre--;
        aviao->torre_alocada = 0;
        pthread_cond_broadcast(&sim->recursos.cond_torres);
        pthread_mutex_unlock(&sim->recursos.mutex_torres);
    }
    
    // Aloca portão
    if (alocar_recursos_desembarque_atomico(sim, aviao) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA: Não foi possível alocar portão para desembarque");
        return 0;
    }
    
    aviao->estado = DESEMBARCANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "DESEMBARCANDO - Portão %d", aviao->portao_alocado);
    
    sleep(2 + rand() % 3); // Desembarque doméstico é mais rápido
    
    aviao->estado = AGUARDANDO_DECOLAGEM;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Desembarque concluído - aguardando decolagem");
    
    return 1;
}

int decolagem_internacional_atomica(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando procedimento de decolagem internacional (ATÔMICO)");
    
    // Libera portão primeiro
    if (aviao->portao_alocado >= 0) {
        pthread_mutex_lock(&sim->recursos.mutex_portoes);
        sim->recursos.portao_ocupado_por[aviao->portao_alocado] = -1;
        sim->recursos.portoes_disponiveis++;
        pthread_cond_broadcast(&sim->recursos.cond_portoes);
        pthread_mutex_unlock(&sim->recursos.mutex_portoes);
        aviao->portao_alocado = -1;
    }
    
    // Aloca recursos para decolagem (torre + pista)
    if (alocar_recursos_decolagem_atomico(sim, aviao) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA: Não foi possível alocar recursos para decolagem");
        return 0;
    }
    
    aviao->estado = DECOLANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "DECOLANDO - Pista %d, Torre slot %d", 
                  aviao->pista_alocada, aviao->torre_alocada - 1);
    
    sleep(3 + rand() % 4); // Decolagem internacional demora mais
    
    // Libera TODOS os recursos
    liberar_todos_recursos(sim, aviao);
    
    aviao->estado = FINALIZADO_SUCESSO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "VOO FINALIZADO COM SUCESSO!");
    
    return 1;
}

int decolagem_domestica_atomica(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando procedimento de decolagem doméstica (ATÔMICO)");
    
    // Libera portão primeiro
    if (aviao->portao_alocado >= 0) {
        pthread_mutex_lock(&sim->recursos.mutex_portoes);
        sim->recursos.portao_ocupado_por[aviao->portao_alocado] = -1;
        sim->recursos.portoes_disponiveis++;
        pthread_cond_broadcast(&sim->recursos.cond_portoes);
        pthread_mutex_unlock(&sim->recursos.mutex_portoes);
        aviao->portao_alocado = -1;
    }
    
    // Aloca recursos para decolagem (torre + pista)
    if (alocar_recursos_decolagem_atomico(sim, aviao) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA: Não foi possível alocar recursos para decolagem");
        return 0;
    }
    
    aviao->estado = DECOLANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "DECOLANDO - Pista %d, Torre slot %d", 
                  aviao->pista_alocada, aviao->torre_alocada - 1);
    
    sleep(2 + rand() % 3); // Decolagem doméstica é mais rápida
    
    // Libera TODOS os recursos
    liberar_todos_recursos(sim, aviao);
    
    aviao->estado = FINALIZADO_SUCESSO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "VOO FINALIZADO COM SUCESSO!");
    
    return 1;
}

void* thread_aviao(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Aviao* aviao = args->aviao;
    SimulacaoAeroporto* sim = args->sim;
    int sucesso = 0;

    verificar_pausa(sim);
    if (!sim->ativa) { free(args); return NULL; } 

    log_evento_ui(sim, aviao, LOG_SYSTEM, "Avião chegou ao espaço aéreo");
    atualizar_estado_aviao(aviao, AGUARDANDO_POUSO);
    
    // Inicializa o tempo de início no ar
    aviao->tempo_inicio_espera_ar = time(NULL);
    aviao->tempo_inicio_espera = time(NULL);
    log_evento_ui(sim, aviao, LOG_INFO, "Está aguardando pouso.");
    
    if (aviao->tipo == VOO_INTERNACIONAL) {
        sucesso = pouso_internacional_atomico(aviao, sim);
    } else {
        sucesso = pouso_domestico_atomico(aviao, sim);
    }
    if (!sucesso) {
        log_evento_ui(sim, aviao, LOG_WARNING, "ABORTOU na fase de pouso.");
        free(args);
        return NULL;
    }

    verificar_pausa(sim);
    if (!sim->ativa) { free(args); return NULL; } 
    
    atualizar_estado_aviao(aviao, AGUARDANDO_DESEMBARQUE);
    aviao->chegada_na_fila = time(NULL);
    if (aviao->tipo == VOO_INTERNACIONAL) {
        sucesso = desembarque_internacional_atomico(aviao, sim);
    } else {
        sucesso = desembarque_domestico_atomico(aviao, sim);
    }
    if (!sucesso) {
        log_evento_ui(sim, aviao, LOG_WARNING, "ABORTOU na fase de desembarque.");
        free(args);
        return NULL;
    }

    verificar_pausa(sim);
    if (!sim->ativa) { free(args); return NULL; }
    
    atualizar_estado_aviao(aviao, AGUARDANDO_DECOLAGEM);
    aviao->tempo_inicio_espera = time(NULL);
    aviao->chegada_na_fila = time(NULL); 
    if (aviao->tipo == VOO_INTERNACIONAL) {
        sucesso = decolagem_internacional_atomica(aviao, sim);
    } else {
        sucesso = decolagem_domestica_atomica(aviao, sim);
    }
    if (!sucesso) {
        log_evento_ui(sim, aviao, LOG_WARNING, "ABORTOU na fase de decolagem.");
        free(args);
        return NULL;
    }

    log_evento_ui(sim, aviao, LOG_SUCCESS, "completou seu ciclo de vida com SUCESSO.");
    atualizar_estado_aviao(aviao, FINALIZADO_SUCESSO);
    incrementar_aviao_sucesso(&sim->metricas);
    
    free(args);
    return NULL;
}

void* criador_avioes(void* arg) {
    SimulacaoAeroporto* sim = (SimulacaoAeroporto*)arg;
    int proximo_id = 1;

    while (sim->ativa) {
        
        verificar_pausa(sim);
        
        if (!sim->ativa) break;
        
        usleep((rand() % 2000 + 500) * 1000); // A cada 0.5-2.5 segundos
        //usleep(100000);
        pthread_mutex_lock(&sim->mutex_simulacao);

        if (proximo_id > sim->max_avioes) {
            log_evento_ui(sim, NULL, LOG_SYSTEM, "Capacidade máxima de aviões atingida.");
            pthread_mutex_unlock(&sim->mutex_simulacao);
            continue;
        }

        Aviao* novo_aviao = &sim->avioes[proximo_id - 1];
        
        TipoVoo tipo = gerar_tipo_voo_aleatorio();
        Aviao* temp_aviao = criar_aviao(proximo_id, tipo);
        if (temp_aviao) {
            *novo_aviao = *temp_aviao; 
            free(temp_aviao);
        }
        
        ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
        if (!args) {
            perror("Falha ao alocar ThreadArgs");
            pthread_mutex_unlock(&sim->mutex_simulacao);
            continue;
        }
        args->aviao = novo_aviao;
        args->sim = sim;

        if (pthread_create(&novo_aviao->thread_id, NULL, thread_aviao, args) != 0) {
            perror("Falha ao criar a thread do avião");
            free(args);
        } else {
            log_evento_ui(sim, novo_aviao, tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL, "CRIADO: Avião %d (%s)", novo_aviao->id, tipo == VOO_DOMESTICO ? "DOM" : "INTL");
            incrementar_contador_avioes_criados(&sim->metricas);
            if (tipo == VOO_DOMESTICO) {
                incrementar_voos_domesticos(&sim->metricas);
            } else {
                incrementar_voos_internacionais(&sim->metricas);
            }
            proximo_id++;
        }

        pthread_mutex_unlock(&sim->mutex_simulacao);
    }

    return NULL;
}




