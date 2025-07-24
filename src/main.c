// main.c
#include "libs.h" // Ou simulador.h

// Função que será executada pela thread da UI
void* ui_thread_func(void* arg) {
    SimulacaoAeroporto* sim = (SimulacaoAeroporto*)arg;
    
    while (sim->ativa) {
        pthread_mutex_lock(&sim->mutex_simulacao); // Trava para ler o estado
        update_terminal_display(sim);
        pthread_mutex_unlock(&sim->mutex_simulacao); // Libera
        
        sleep(1); // Update every 1 second instead of 33ms

    }
    
    // Última atualização para mostrar o estado final
    pthread_mutex_lock(&sim->mutex_simulacao);
    update_terminal_display(sim);
    pthread_mutex_unlock(&sim->mutex_simulacao);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    // Parâmetros da simulação (poderiam vir de argv)
    int num_pistas = MAX_PISTAS;
    int num_portoes = MAX_PORTOES;
    int num_torres = MAX_TORRES;
    int tempo_total_sim = 300; // 5 minutos
    int max_avioes = 200;
    UIViewMode modo_ui = UI_VISUAL;

    srand(time(NULL));

    // 1. Inicializa a simulação
    SimulacaoAeroporto* sim = inicializar_simulacao(num_pistas, num_portoes, num_torres, tempo_total_sim, max_avioes, modo_ui);
    if (!sim) {
        fprintf(stderr, "Falha ao inicializar a simulação.\n");
        return 1;
    }
    pthread_mutex_init(&sim->mutex_ui_log, NULL);

    // 2. Inicializa a UI
    init_terminal_ncurses();

    // 3. Cria e lança as threads principais
    pthread_t ui_thread_id;
    pthread_t criador_avioes_thread_id;

    pthread_create(&ui_thread_id, NULL, ui_thread_func, sim);
    pthread_create(&criador_avioes_thread_id, NULL, criador_avioes, sim);

    // 4. Aguarda o tempo de simulação
    log_evento_ui(sim, "Simulação iniciada. Pressione qualquer tecla para finalizar antes.");
    sleep(tempo_total_sim);
    
    // 5. Finaliza a simulação
    log_evento_ui(sim, "Tempo de simulação esgotado. Finalizando...");
    sim->ativa = 0; // Sinaliza para as threads terminarem

    // 6. Aguarda as threads terminarem
    pthread_join(criador_avioes_thread_id, NULL);
    // Aqui você também precisaria de um loop para dar join em todas as threads de avião
    // (A ser implementado no criador_avioes)
    
    pthread_join(ui_thread_id, NULL); // Aguarda a thread da UI

    // 7. Limpeza
    close_terminal_ncurses();
    
    // 8. Gera relatório final no console
    gerar_relatorio_final(sim);

    // 9. Libera memória
    liberar_memoria(sim);
    pthread_mutex_destroy(&sim->mutex_ui_log);

    return 0;
}