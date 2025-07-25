#include "libs.h"

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
    int tempo_total_sim = 60; // 5 minutos
    int max_avioes = 30;

    bool pause_simulation = false; 

    //setlocale(LC_ALL, ""); 
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

        if (ch == 'p' || ch == 'P') {
            pthread_mutex_lock(&sim->mutex_pausado);
            sim->pausado = !sim->pausado;

            if (sim->pausado) {
                log_evento_ui(sim, "Simulação PAUSADA. Pressione 'p' para retomar.");
            } else {
                log_evento_ui(sim, "Simulação RETOMADA.");
                pthread_cond_broadcast(&sim->cond_pausado);
            }
            pthread_mutex_unlock(&sim->mutex_pausado);
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
    

    log_evento_ui(sim, "Aguardando finalização de todos os voos...");
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].thread_id != 0) {
            pthread_join(sim->avioes[i].thread_id, NULL);
        }
    }

    usleep(100000);
    
    pthread_mutex_destroy(&sim->mutex_pausado);
    pthread_cond_destroy(&sim->cond_pausado);

    close_terminal_ncurses();
    
    gerar_relatorio_final(sim);

    liberar_memoria(sim);
    pthread_mutex_destroy(&sim->mutex_ui_log);

    return 0;
}