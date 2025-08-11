/*
 * EXEMPLO DE IMPLEMENTAÇÃO DAS TORRES COMPARTILHADAS
 * ===================================================
 * 
 * Este arquivo demonstra como usar a nova implementação de torres
 * como recurso compartilhado com capacidade limitada.
 * 
 * MUDANÇAS IMPLEMENTADAS:
 * - Torres agora suportam até 2 operações simultâneas (CAPACIDADE_TORRE = 2)
 * - Cada torre tem slots individuais para controle
 * - Sistema de contagem de operações ativas
 * - Funções auxiliares para uso simples da torre
 */

#include "include/libs.h"
#include "include/airport.h"

// EXEMPLO 1: Uso das funções auxiliares simples
void exemplo_uso_simples_torre(SimulacaoAeroporto* sim, Aviao* aviao) {
    printf("=== EXEMPLO: Uso simples da torre ===\n");
    
    // Solicitar uso da torre
    printf("Avião %d solicitando torre...\n", aviao->id);
    
    // Esta função bloqueia até conseguir um slot na torre
    if (solicitar_uso_torre(sim, aviao) == 0) {
        printf("Avião %d obteve acesso à torre (slot %d)\n", 
               aviao->id, aviao->torre_alocada);
        
        // Simula uso da torre (pouso/decolagem)
        printf("Avião %d usando torre...\n", aviao->id);
        sleep(2); // Simula operação
        
        // Libera o uso da torre
        liberar_uso_torre(sim, aviao);
        printf("Avião %d liberou a torre\n", aviao->id);
    }
}

// EXEMPLO 2: Uso manual da lógica compartilhada
void exemplo_uso_manual_torre(SimulacaoAeroporto* sim, Aviao* aviao) {
    printf("=== EXEMPLO: Uso manual da torre compartilhada ===\n");
    
    RecursosAeroporto* recursos = &sim->recursos;
    
    // Solicitar uso da torre
    pthread_mutex_lock(&recursos->mutex_torres);
    while (recursos->slots_torre_disponiveis == 0) {
        printf("Avião %d aguardando slot na torre...\n", aviao->id);
        pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_torres);
    }
    
    // Aloca o slot
    recursos->slots_torre_disponiveis--;
    recursos->operacoes_ativas_torre++;
    
    // Encontra slot livre e registra o avião
    int slot_alocado = -1;
    for (int i = 0; i < CAPACIDADE_TORRE; i++) {
        if (recursos->torre_ocupada_por[i] == -1) {
            recursos->torre_ocupada_por[i] = aviao->id;
            aviao->torre_alocada = i + 1; // +1 para diferir de 0
            slot_alocado = i;
            break;
        }
    }
    
    printf("Avião %d alocou slot %d da torre (%d/%d slots disponíveis)\n", 
           aviao->id, slot_alocado, 
           recursos->slots_torre_disponiveis, recursos->capacidade_torre);
    
    pthread_mutex_unlock(&recursos->mutex_torres);
    
    // Simula uso da torre
    printf("Avião %d operando na torre...\n", aviao->id);
    sleep(3);
    
    // Liberar uso da torre
    pthread_mutex_lock(&recursos->mutex_torres);
    
    if (aviao->torre_alocada > 0) {
        int slot = aviao->torre_alocada - 1;
        recursos->torre_ocupada_por[slot] = -1;
        aviao->torre_alocada = 0;
    }
    
    recursos->slots_torre_disponiveis++;
    recursos->operacoes_ativas_torre--;
    
    printf("Avião %d liberou slot da torre (%d/%d slots disponíveis)\n", 
           aviao->id, recursos->slots_torre_disponiveis, recursos->capacidade_torre);
    
    pthread_cond_signal(&recursos->cond_torres);
    pthread_mutex_unlock(&recursos->mutex_torres);
}

// EXEMPLO 3: Demonstração de operações simultâneas
void* thread_aviao_exemplo(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Aviao* aviao = args->aviao;
    SimulacaoAeroporto* sim = args->sim;
    
    printf("Thread do Avião %d iniciada\n", aviao->id);
    
    // Usa as funções auxiliares simples
    exemplo_uso_simples_torre(sim, aviao);
    
    return NULL;
}

// EXEMPLO DE COMO TESTAR MÚLTIPLAS OPERAÇÕES SIMULTÂNEAS
void demonstrar_operacoes_simultaneas() {
    printf("=== DEMONSTRAÇÃO DE OPERAÇÕES SIMULTÂNEAS ===\n");
    printf("Torre suporta até %d operações simultâneas\n", CAPACIDADE_TORRE);
    printf("Criando %d aviões para testar...\n", 4);
    
    // Aqui você criaria múltiplas threads de aviões
    // Todas podem usar a torre simultaneamente até o limite de capacidade
    
    /*
    SimulacaoAeroporto* sim = inicializar_simulacao(3, 5, 1, 60, 10);
    
    pthread_t threads[4];
    ThreadArgs args[4];
    
    for (int i = 0; i < 4; i++) {
        args[i].aviao = &sim->avioes[i];
        args[i].aviao->id = i + 1;
        args[i].sim = sim;
        
        pthread_create(&threads[i], NULL, thread_aviao_exemplo, &args[i]);
    }
    
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    */
}

/*
 * RESUMO DAS MUDANÇAS IMPLEMENTADAS:
 * ==================================
 * 
 * 1. ESTRUTURA MODIFICADA (RecursosAeroporto):
 *    - slots_torre_disponiveis: slots atualmente disponíveis
 *    - capacidade_torre: capacidade total (2)
 *    - operacoes_ativas_torre: contador de operações ativas
 *    - torre_ocupada_por[]: array com IDs dos aviões usando cada slot
 * 
 * 2. CONSTANTES ADICIONADAS:
 *    - CAPACIDADE_TORRE = 2 (máximo de operações simultâneas)
 * 
 * 3. FUNÇÕES MODIFICADAS:
 *    - solicitar_torre_com_prioridade(): agora suporta múltiplos slots
 *    - liberar_torre(): libera slot específico do avião
 *    - solicitar_torre(): versão simples também atualizada
 * 
 * 4. NOVAS FUNÇÕES AUXILIARES:
 *    - solicitar_uso_torre(): versão simples para solicitar torre
 *    - liberar_uso_torre(): versão simples para liberar torre
 * 
 * 5. UI ATUALIZADA:
 *    - Header panel mostra operações ativas em vez de torres ocupadas
 *    - Status panel exibe slots individuais da torre
 * 
 * COMO USAR:
 * ==========
 * 
 * Opção 1 - Funções auxiliares simples:
 *   solicitar_uso_torre(sim, aviao);
 *   // ... usar torre ...
 *   liberar_uso_torre(sim, aviao);
 * 
 * Opção 2 - Controle manual (como no exemplo do usuário):
 *   pthread_mutex_lock(&recursos->mutex_torres);
 *   while (recursos->slots_torre_disponiveis == 0) {
 *       pthread_cond_wait(&recursos->cond_torres, &recursos->mutex_torres);
 *   }
 *   recursos->slots_torre_disponiveis--;
 *   recursos->operacoes_ativas_torre++;
 *   // registrar avião...
 *   pthread_mutex_unlock(&recursos->mutex_torres);
 *   
 *   // ... usar torre ...
 *   
 *   pthread_mutex_lock(&recursos->mutex_torres);
 *   recursos->slots_torre_disponiveis++;
 *   recursos->operacoes_ativas_torre--;
 *   pthread_cond_signal(&recursos->cond_torres);
 *   pthread_mutex_unlock(&recursos->mutex_torres);
 */
