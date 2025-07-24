#include "libs.h"
#include <unistd.h>

// ======================= FUNÇÕES DE CRIAÇÃO =======================

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
    aviao->tempo_inicio_espera = 0;
    aviao->thread_id = 0;
    aviao->pista_alocada = -1;
    aviao->portao_alocado = -1;
    aviao->usando_torre = 0;

    return aviao;
}

TipoVoo gerar_tipo_voo_aleatorio() {
    return rand() % 2;
}

// ======================= FUNÇÕES DE OPERAÇÃO DE VOO =======================

// --- POUSO ---
int pouso_internacional(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, "ID %d (INTL) | Pouso: Solicitando Pista.", aviao->id);
    aviao->pista_alocada = solicitar_pista(sim, aviao->id, aviao->tipo);
    if (aviao->pista_alocada == -1) { return 0; }
    log_evento_ui(sim, "ID %d (INTL) obteve Pista %d. Solicitando Torre.", aviao->id, aviao->pista_alocada);

    if (solicitar_torre(sim, aviao->id, aviao->tipo) != 0) {
        liberar_pista(sim, aviao->id, aviao->pista_alocada);
        aviao->pista_alocada = -1;
        return 0;
    }
    log_evento_ui(sim, "ID %d (INTL) obteve Torre. Pousando...", aviao->id);
    
    atualizar_estado_aviao(aviao, POUSANDO);
    dormir_operacao_com_pausa(sim, 2000, 4000);

    liberar_pista(sim, aviao->id, aviao->pista_alocada);
    liberar_torre(sim, aviao->id);
    log_evento_ui(sim, "ID %d (INTL) | Pouso concluído. Pista %d e Torre liberadas.", aviao->id, aviao->pista_alocada);
    
    aviao->pista_alocada = -1;
    incrementar_metrica_pouso(&sim->metricas);
    return 1;
}

int pouso_domestico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, "ID %d (DOM) | Pouso: Solicitando Torre.", aviao->id);
    if (solicitar_torre(sim, aviao->id, aviao->tipo) != 0) { return 0; }
    log_evento_ui(sim, "ID %d (DOM) obteve Torre. Solicitando Pista.", aviao->id);

    aviao->pista_alocada = solicitar_pista(sim, aviao->id, aviao->tipo);
    if (aviao->pista_alocada == -1) {
        liberar_torre(sim, aviao->id);
        return 0;
    }
    log_evento_ui(sim, "ID %d (DOM) obteve Pista %d. Pousando...", aviao->id, aviao->pista_alocada);
    
    atualizar_estado_aviao(aviao, POUSANDO);
    dormir_operacao_com_pausa(sim, 2000, 4000);

    liberar_torre(sim, aviao->id);
    liberar_pista(sim, aviao->id, aviao->pista_alocada);
    log_evento_ui(sim, "ID %d (DOM) | Pouso concluído. Torre e Pista %d liberadas.", aviao->id, aviao->pista_alocada);
    
    aviao->pista_alocada = -1;
    incrementar_metrica_pouso(&sim->metricas);
    return 1;
}

// --- DESEMBARQUE ---
int desembarque_internacional(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, "ID %d (INTL) | Desembarque: Solicitando Portão.", aviao->id);
    aviao->portao_alocado = solicitar_portao(sim, aviao->id, aviao->tipo);
    if (aviao->portao_alocado == -1) { return 0; }
    log_evento_ui(sim, "ID %d (INTL) obteve Portão %d. Solicitando Torre.", aviao->id, aviao->portao_alocado);

    if (solicitar_torre(sim, aviao->id, aviao->tipo) != 0) {
        liberar_portao(sim, aviao->id, aviao->portao_alocado);
        aviao->portao_alocado = -1;
        return 0;
    }
    log_evento_ui(sim, "ID %d (INTL) obteve Torre. Desembarcando...", aviao->id);

    atualizar_estado_aviao(aviao, DESEMBARCANDO);
    dormir_operacao_com_pausa(sim, 3000, 6000);

    liberar_torre(sim, aviao->id);
    log_evento_ui(sim, "ID %d (INTL) liberou Torre. Finalizando desembarque...", aviao->id);
    dormir_operacao_com_pausa(sim, 1000, 2000);

    liberar_portao(sim, aviao->id, aviao->portao_alocado);
    log_evento_ui(sim, "ID %d (INTL) | Desembarque concluído. Portão %d liberado.", aviao->id, aviao->portao_alocado);
    
    aviao->portao_alocado = -1;
    incrementar_metrica_desembarque(&sim->metricas);
    return 1;
}

int desembarque_domestico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, "ID %d (DOM) | Desembarque: Solicitando Torre.", aviao->id);
    if (solicitar_torre(sim, aviao->id, aviao->tipo) != 0) { return 0; }
    log_evento_ui(sim, "ID %d (DOM) obteve Torre. Solicitando Portão.", aviao->id);

    aviao->portao_alocado = solicitar_portao(sim, aviao->id, aviao->tipo);
    if (aviao->portao_alocado == -1) {
        liberar_torre(sim, aviao->id);
        return 0;
    }
    log_evento_ui(sim, "ID %d (DOM) obteve Portão %d. Desembarcando...", aviao->id, aviao->portao_alocado);
    
    atualizar_estado_aviao(aviao, DESEMBARCANDO);
    dormir_operacao(3000, 6000);

    liberar_torre(sim, aviao->id);
    log_evento_ui(sim, "ID %d (DOM) liberou Torre. Finalizando desembarque...", aviao->id);
    dormir_operacao(1000, 2000);

    liberar_portao(sim, aviao->id, aviao->portao_alocado);
    log_evento_ui(sim, "ID %d (DOM) | Desembarque concluído. Portão %d liberado.", aviao->id, aviao->portao_alocado);
    
    aviao->portao_alocado = -1;
    incrementar_metrica_desembarque(&sim->metricas);
    return 1;
}

// --- DECOLAGEM ---
int decolagem_internacional(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, "ID %d (INTL) | Decolagem: Solicitando Portão.", aviao->id);
    aviao->portao_alocado = solicitar_portao(sim, aviao->id, aviao->tipo);
    if (aviao->portao_alocado == -1) { return 0; }
    log_evento_ui(sim, "ID %d (INTL) obteve Portão %d. Solicitando Pista.", aviao->id, aviao->portao_alocado);

    aviao->pista_alocada = solicitar_pista(sim, aviao->id, aviao->tipo);
    if (aviao->pista_alocada == -1) {
        liberar_portao(sim, aviao->id, aviao->portao_alocado);
        aviao->portao_alocado = -1;
        return 0;
    }
    log_evento_ui(sim, "ID %d (INTL) obteve Pista %d. Solicitando Torre.", aviao->id, aviao->pista_alocada);

    if (solicitar_torre(sim, aviao->id, aviao->tipo) != 0) {
        liberar_portao(sim, aviao->id, aviao->portao_alocado);
        liberar_pista(sim, aviao->id, aviao->pista_alocada);
        aviao->portao_alocado = -1;
        aviao->pista_alocada = -1;
        return 0;
    }
    log_evento_ui(sim, "ID %d (INTL) obteve todos os recursos. Decolando...", aviao->id);

    atualizar_estado_aviao(aviao, DECOLANDO);
    dormir_operacao_com_pausa(sim, 2000, 4000);

    liberar_portao(sim, aviao->id, aviao->portao_alocado);
    liberar_pista(sim, aviao->id, aviao->pista_alocada);
    liberar_torre(sim, aviao->id);
    log_evento_ui(sim, "ID %d (INTL) | Decolagem concluída. Recursos liberados.", aviao->id);
    
    aviao->portao_alocado = -1;
    aviao->pista_alocada = -1;
    incrementar_metrica_decolagem(&sim->metricas);
    return 1;
}

int decolagem_domestico(Aviao* aviao, SimulacaoAeroporto* sim) {
    log_evento_ui(sim, "ID %d (DOM) | Decolagem: Solicitando Torre.", aviao->id);
    if (solicitar_torre(sim, aviao->id, aviao->tipo) != 0) { return 0; }
    log_evento_ui(sim, "ID %d (DOM) obteve Torre. Solicitando Portão.", aviao->id);

    aviao->portao_alocado = solicitar_portao(sim, aviao->id, aviao->tipo);
    if (aviao->portao_alocado == -1) {
        liberar_torre(sim, aviao->id);
        return 0;
    }
    log_evento_ui(sim, "ID %d (DOM) obteve Portão %d. Solicitando Pista.", aviao->id, aviao->portao_alocado);

    aviao->pista_alocada = solicitar_pista(sim, aviao->id, aviao->tipo);
    if (aviao->pista_alocada == -1) {
        liberar_torre(sim, aviao->id);
        liberar_portao(sim,aviao->id, aviao->portao_alocado);
        aviao->portao_alocado = -1;
        return 0;
    }
    log_evento_ui(sim, "ID %d (DOM) obteve todos os recursos. Decolando...", aviao->id);

    atualizar_estado_aviao(aviao, DECOLANDO);
    dormir_operacao(2000, 4000);

    liberar_torre(sim, aviao->id);
    liberar_portao(sim, aviao->id, aviao->portao_alocado);
    liberar_pista(sim, aviao->id, aviao->pista_alocada);
    log_evento_ui(sim, "ID %d (DOM) | Decolagem concluída. Recursos liberados.", aviao->id);
    
    aviao->portao_alocado = -1;
    aviao->pista_alocada = -1;
    incrementar_metrica_decolagem(&sim->metricas);
    return 1;
}


// ======================= THREADS PRINCIPAIS =======================

void* thread_aviao(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Aviao* aviao = args->aviao;
    SimulacaoAeroporto* sim = args->sim;
    int sucesso = 0;

    // ----- 1. FASE DE POUSO -----
    verificar_pausa(sim); // Check for pause before starting
    if (!sim->ativa) { free(args); return NULL; } // Exit if simulation stopped
    
    atualizar_estado_aviao(aviao, AGUARDANDO_POUSO);
    if (aviao->tipo == VOO_INTERNACIONAL) {
        sucesso = pouso_internacional(aviao, sim);
    } else {
        sucesso = pouso_domestico(aviao, sim);
    }
    if (!sucesso) {
        log_evento_ui(sim, "ID %d ABORTOU na fase de pouso.", aviao->id);
        free(args);
        return NULL;
    }

    // ----- 2. FASE DE DESEMBARQUE -----
    verificar_pausa(sim); // Check for pause before desembarque
    if (!sim->ativa) { free(args); return NULL; } // Exit if simulation stopped
    
    atualizar_estado_aviao(aviao, AGUARDANDO_DESEMBARQUE);
    if (aviao->tipo == VOO_INTERNACIONAL) {
        sucesso = desembarque_internacional(aviao, sim);
    } else {
        sucesso = desembarque_domestico(aviao, sim);
    }
    if (!sucesso) {
        log_evento_ui(sim, "ID %d ABORTOU na fase de desembarque.", aviao->id);
        free(args);
        return NULL;
    }

    // ----- 3. FASE DE DECOLAGEM -----
    verificar_pausa(sim); // Check for pause before decolagem
    if (!sim->ativa) { free(args); return NULL; } // Exit if simulation stopped
    
    atualizar_estado_aviao(aviao, AGUARDANDO_DECOLAGEM);
    if (aviao->tipo == VOO_INTERNACIONAL) {
        sucesso = decolagem_internacional(aviao, sim);
    } else {
        sucesso = decolagem_domestico(aviao, sim);
    }
    if (!sucesso) {
        log_evento_ui(sim, "ID %d ABORTOU na fase de decolagem.", aviao->id);
        free(args);
        return NULL;
    }

    // ----- 4. FINALIZAÇÃO -----
    log_evento_ui(sim, "ID %d completou seu ciclo de vida com SUCESSO.", aviao->id);
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
        
        usleep((rand() % 2000 + 500) * 1000); // Cria um avião a cada 0.5-2.5 segundos

        pthread_mutex_lock(&sim->mutex_simulacao);

        if (proximo_id > sim->max_avioes) {
            log_evento_ui(sim, "Capacidade máxima de aviões atingida.");
            pthread_mutex_unlock(&sim->mutex_simulacao);
            continue;
        }

        // Encontra um slot de avião vazio (o primeiro, já que o ID é sequencial)
        Aviao* novo_aviao = &sim->avioes[proximo_id - 1];
        
        // Cria o avião
        TipoVoo tipo = gerar_tipo_voo_aleatorio();
        Aviao* temp_aviao = criar_aviao(proximo_id, tipo);
        if (temp_aviao) {
            *novo_aviao = *temp_aviao; // Copia os dados
            free(temp_aviao); // Libera o temporário
        }
        
        // Prepara os argumentos para a thread
        ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
        if (!args) {
            perror("Falha ao alocar ThreadArgs");
            pthread_mutex_unlock(&sim->mutex_simulacao);
            continue;
        }
        args->aviao = novo_aviao;
        args->sim = sim;

        // Cria a thread do avião
        if (pthread_create(&novo_aviao->thread_id, NULL, thread_aviao, args) != 0) {
            perror("Falha ao criar a thread do avião");
            free(args);
        } else {
            log_evento_ui(sim, "CRIADO: Avião %d (%s)", novo_aviao->id, tipo == VOO_DOMESTICO ? "DOM" : "INTL");
            // Incrementa o contador de aviões criados
            pthread_mutex_lock(&sim->metricas.mutex_metricas);
            sim->metricas.total_avioes_criados++;
            if (tipo == VOO_DOMESTICO) {
                sim->metricas.voos_domesticos_total++;
            } else {
                sim->metricas.voos_internacionais_total++;
            }
            pthread_mutex_unlock(&sim->metricas.mutex_metricas);
            proximo_id++;
        }

        pthread_mutex_unlock(&sim->mutex_simulacao);
    }

    return NULL;
}

// ISSO AQUI PODE VIRAR UTILS?
void verificar_pausa(SimulacaoAeroporto* sim) {
    pthread_mutex_lock(&sim->mutex_pausado);
    while (sim->pausado && sim->ativa) {
        pthread_cond_wait(&sim->cond_pausado, &sim->mutex_pausado);
    }
    pthread_mutex_unlock(&sim->mutex_pausado);
}