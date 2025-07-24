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
    int num_pistas = MAX_PISTAS;
    int num_portoes = MAX_PORTOES;
    int num_torres = MAX_TORRES;
    int tempo_total_sim = 300; // 5 minutos
    int max_avioes = 200;
    //UIViewMode modo_ui = UI_VISUAL;

    srand(time(NULL));

    SimulacaoAeroporto* sim = inicializar_simulacao(num_pistas, num_portoes, num_torres, tempo_total_sim, max_avioes);
    if (!sim) {
        fprintf(stderr, "Falha ao inicializar a simulação.\n");
        return 1;
    }
    pthread_mutex_init(&sim->mutex_ui_log, NULL);

    init_terminal_ncurses();

    timeout(100); 
    
    pthread_t ui_thread_id;
    pthread_t criador_avioes_thread_id;

    log_evento_ui(sim, "Simulação iniciada. Pressione 'q' para finalizar.");

    pthread_create(&ui_thread_id, NULL, ui_thread_func, sim);
    pthread_create(&criador_avioes_thread_id, NULL, criador_avioes, sim);

    while(sim->ativa) {
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            log_evento_ui(sim, "Simulação finalizada pelo usuário.");
            finalizar_simulacao(sim); 
            break;
        }
        
        if (difftime(time(NULL), sim->tempo_inicio) >= tempo_total_sim) {
            log_evento_ui(sim, "Tempo de simulação esgotado. Finalizando...");
            finalizar_simulacao(sim); 
            break;
        }
        
        usleep(100000); 
    }

    pthread_join(criador_avioes_thread_id, NULL);
    pthread_join(ui_thread_id, NULL);

    close_terminal_ncurses();
    
    gerar_relatorio_final(sim);

    liberar_memoria(sim);
    pthread_mutex_destroy(&sim->mutex_ui_log);

    return 0;
}