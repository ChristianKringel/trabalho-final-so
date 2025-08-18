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
    aviao->sacrificado = false;

    return aviao;
}

TipoVoo gerar_tipo_voo_aleatorio() {
    return rand() % 2;
}

int pouso_internacional_atomico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando pouso internacional (SEQUENCIAL)");

    if (aviao->crash_iminente) {
        log_evento_ui(sim, aviao, LOG_ERROR, "ABORTOU: Avião já havia crashed");
        return 0;
    }

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando PISTA...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_PISTA) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar PISTA para pouso.");
        return 0;
    }

    log_evento_ui(sim, aviao, LOG_SUCCESS, "Pista %d alocada.", aviao->pista_alocada);

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando TORRE...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_TORRE) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar TORRE. Liberando pista...");
        liberar_pista(sim, aviao->id, aviao->pista_alocada);
        return 0;
    }

    log_evento_ui(sim, aviao, LOG_SUCCESS, "Torre (slot %d) alocada.", aviao->torre_alocada - 1);

    aviao->estado = POUSANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "POUSANDO - Pista %d, Torre slot %d", aviao->pista_alocada, aviao->torre_alocada - 1);
    usleep(1500000); // 1.5 segundos

    liberar_pista(sim, aviao->id, aviao->pista_alocada);
    liberar_torre(sim, aviao->id);

    aviao->estado = AGUARDANDO_DESEMBARQUE;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Pouso concluído - aguardando desembarque");
    return 1;
}

int pouso_domestico_atomico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando pouso doméstico (SEQUENCIAL)");

    if (aviao->crash_iminente) {
        log_evento_ui(sim, aviao, LOG_ERROR, "ABORTOU: Avião já havia crashed");
        return 0;
    }

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando TORRE...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_TORRE) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar TORRE para pouso.");
        return 0;
    }

    log_evento_ui(sim, aviao, LOG_SUCCESS, "Torre (slot %d) alocada.", aviao->torre_alocada - 1);

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando PISTA...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_PISTA) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar PISTA. Liberando torre...");
        liberar_torre(sim, aviao->id);
        return 0;
    }

    log_evento_ui(sim, aviao, LOG_SUCCESS, "Pista %d alocada.", aviao->pista_alocada);

    aviao->estado = POUSANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "POUSANDO - Pista %d, Torre slot %d", aviao->pista_alocada, aviao->torre_alocada - 1);
    usleep(1500000); 

    liberar_pista(sim, aviao->id, aviao->pista_alocada);
    liberar_torre(sim, aviao->id);

    aviao->estado = AGUARDANDO_DESEMBARQUE;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Pouso concluído - aguardando desembarque");
    return 1;
}

int desembarque_internacional_atomico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando desembarque internacional (SEQUENCIAL)");

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando PORTÃO...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_PORTAO) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar PORTÃO para desembarque.");
        return 0;
    }
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Portão %d alocado.", aviao->portao_alocado);

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando TORRE...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_TORRE) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar TORRE. Liberando portão...");
        liberar_portao(sim, aviao->id, aviao->portao_alocado);
        return 0;
    }
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Torre (slot %d) alocada.", aviao->torre_alocada - 1);

    aviao->estado = DESEMBARCANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "DESEMBARCANDO - Portão %d", aviao->portao_alocado);
    usleep(1500000); 

    liberar_torre(sim, aviao->id);
    liberar_portao(sim, aviao->id, aviao->portao_alocado);

    aviao->estado = AGUARDANDO_DECOLAGEM;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Desembarque concluído - aguardando decolagem");
    return 1;
}

int desembarque_domestico_atomico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando desembarque doméstico (SEQUENCIAL)");

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando TORRE...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_TORRE) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar TORRE para desembarque.");
        return 0;
    }
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Torre (slot %d) alocada.", aviao->torre_alocada - 1);

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando PORTÃO...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_PORTAO) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar PORTÃO. Liberando torre...");
        liberar_torre(sim, aviao->id);
        return 0;
    }
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Portão %d alocado.", aviao->portao_alocado);


    aviao->estado = DESEMBARCANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "DESEMBARCANDO - Portão %d", aviao->portao_alocado);
    usleep(2000000); // 2 segundos

    liberar_torre(sim, aviao->id);
    liberar_portao(sim, aviao->id, aviao->portao_alocado);

    aviao->estado = AGUARDANDO_DECOLAGEM;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Desembarque concluído - aguardando decolagem");
    return 1;
}

int decolagem_internacional_atomica(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando decolagem internacional (SEQUENCIAL)");

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando PORTÃO...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_PORTAO) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar PORTÃO para decolagem.");
        return 0;
    }
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Portão %d alocado.", aviao->portao_alocado);

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando PISTA...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_PISTA) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar PISTA. Liberando portão...");
        liberar_portao(sim, aviao->id, aviao->portao_alocado);
        return 0;
    }
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Pista %d alocada.", aviao->pista_alocada);

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando TORRE...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_TORRE) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar TORRE. Liberando pista e portão...");
        liberar_pista(sim, aviao->id, aviao->pista_alocada);
        liberar_portao(sim, aviao->id, aviao->portao_alocado);
        return 0;
    }
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Torre (slot %d) alocada.", aviao->torre_alocada - 1);


    aviao->estado = DECOLANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "DECOLANDO - Pista %d, Portão %d, Torre slot %d", aviao->pista_alocada, aviao->portao_alocado, aviao->torre_alocada - 1);
    usleep(2000000); // 1.5 segundos

    liberar_portao(sim, aviao->id, aviao->portao_alocado);
    liberar_pista(sim, aviao->id, aviao->pista_alocada);
    liberar_torre(sim, aviao->id);

    aviao->estado = FINALIZADO_SUCESSO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "VOO FINALIZADO COM SUCESSO!");
    return 1;
}

int decolagem_domestica_atomica(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, aviao, LOG_INFO, "Iniciando decolagem doméstica (SEQUENCIAL)");

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando TORRE...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_TORRE) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar TORRE para decolagem.");
        return 0;
    }
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Torre (slot %d) alocada.", aviao->torre_alocada - 1);

    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando PORTÃO...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_PORTAO) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar PORTÃO. Liberando torre...");
        liberar_torre(sim, aviao->id);
        return 0;
    }
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Portão %d alocado.", aviao->portao_alocado);
    
    log_evento_ui(sim, aviao, LOG_INFO, "Solicitando PISTA...");
    if (solicitar_recurso_individual(sim, aviao, RECURSO_PISTA) != 0) {
        log_evento_ui(sim, aviao, LOG_ERROR, "FALHA ao alocar PISTA. Liberando torre e portão...");
        liberar_torre(sim, aviao->id);
        liberar_portao(sim, aviao->id, aviao->portao_alocado);
        return 0;
    }
    log_evento_ui(sim, aviao, LOG_SUCCESS, "Pista %d alocada.", aviao->pista_alocada);

    aviao->estado = DECOLANDO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "DECOLANDO - Pista %d, Portão %d, Torre slot %d", aviao->pista_alocada, aviao->portao_alocado, aviao->torre_alocada - 1);
    usleep(1500000); // 1.5 segundos

    liberar_portao(sim, aviao->id, aviao->portao_alocado);
    liberar_pista(sim, aviao->id, aviao->pista_alocada);
    liberar_torre(sim, aviao->id);

    aviao->estado = FINALIZADO_SUCESSO;
    log_evento_ui(sim, aviao, LOG_SUCCESS, "VOO FINALIZADO COM SUCESSO!");
    return 1;
}

void* thread_aviao(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Aviao* aviao = args->aviao;
    SimulacaoAeroporto* sim = args->sim;
    int sucesso = 0;

    if (aviao->tipo == VOO_INTERNACIONAL) {
        sucesso = pouso_internacional_atomico(aviao, sim);
    } else {
        sucesso = pouso_domestico_atomico(aviao, sim);
    }
    
    // --- SUBSTITUA O BLOCO DE SACRIFÍCIO AQUI ---
    if (sucesso == -99) {
        atualizar_estado_aviao(sim, aviao, FALHA_DEADLOCK);
        incrementar_aviao_deadlock(&sim->metricas);
        
        // Libera os recursos que porventura já possuía
        if (aviao->pista_alocada != -1) liberar_pista(sim, aviao->id, aviao->pista_alocada);
        if (aviao->portao_alocado != -1) liberar_portao(sim, aviao->id, aviao->portao_alocado);
        if (aviao->torre_alocada > 0) liberar_torre(sim, aviao->id);
        
        free(args);
        return NULL;
    }
    if (!sucesso) { /* ... */ }

    // ... (lógica de desembarque) ...
    if (aviao->tipo == VOO_INTERNACIONAL) {
        sucesso = desembarque_internacional_atomico(aviao, sim);
    } else {
        sucesso = desembarque_domestico_atomico(aviao, sim);
    }

    // --- E AQUI TAMBÉM ---
    if (sucesso == -99) {
        atualizar_estado_aviao(sim, aviao, FALHA_DEADLOCK);
        incrementar_aviao_deadlock(&sim->metricas);

        if (aviao->pista_alocada != -1) liberar_pista(sim, aviao->id, aviao->pista_alocada);
        if (aviao->portao_alocado != -1) liberar_portao(sim, aviao->id, aviao->portao_alocado);
        if (aviao->torre_alocada > 0) liberar_torre(sim, aviao->id);

        free(args);
        return NULL;
    }
    if (!sucesso) { /* ... */ }

    // ... (lógica de decolagem) ...
    if (aviao->tipo == VOO_INTERNACIONAL) {
        sucesso = decolagem_internacional_atomica(aviao, sim);
    } else {
        sucesso = decolagem_domestica_atomica(aviao, sim);
    }
    
    // --- E AQUI FINALMENTE ---
    if (sucesso == -99) {
        atualizar_estado_aviao(sim, aviao, FALHA_DEADLOCK);
        incrementar_aviao_deadlock(&sim->metricas);

        if (aviao->pista_alocada != -1) liberar_pista(sim, aviao->id, aviao->pista_alocada);
        if (aviao->portao_alocado != -1) liberar_portao(sim, aviao->id, aviao->portao_alocado);
        if (aviao->torre_alocada > 0) liberar_torre(sim, aviao->id);

        free(args);
        return NULL;
    }
    if (!sucesso) {
        log_evento_ui(sim, aviao, LOG_WARNING, "ABORTOU na fase de decolagem.");
        free(args);
        return NULL;
    }

    log_evento_ui(sim, aviao, LOG_SUCCESS, "completou seu ciclo de vida com SUCESSO.");
    atualizar_estado_aviao(sim, aviao, FINALIZADO_SUCESSO);
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

        usleep(1000000 + (rand() % 2000000)); // 1 - 3 segundos

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
        
        banker_init_aviao(&sim->recursos, novo_aviao->id - 1);

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




