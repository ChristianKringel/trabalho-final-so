// terminal.c
#include "libs.h" // Ou simulador.h


// As janelas agora são 'static' para serem visíveis apenas neste arquivo.
// Isso é uma boa prática para evitar poluir o namespace global.
static WINDOW *runway_win, *gate_win, *tower_win, *airplane_list_win, *log_win, *header_win;

void init_terminal_ncurses() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    // ... (seu código de inicialização de janelas é ótimo e pode permanecer) ...
    // Exemplo:
    header_win = newwin(3, COLS, 0, 0);
    log_win = newwin(5, COLS, LINES - 5, 0);
    scrollok(log_win, TRUE); // Habilita rolagem para o log
    // ... criar as outras janelas
    refresh();
}

void close_terminal_ncurses() {
    // ... (seu código de destruição de janelas) ...
    endwin();
    printf("Terminal Ncurses Encerrado.\n");
}

// Função para converter o estado do avião em uma string legível
const char* estado_para_str(EstadoAviao estado) {
    switch (estado) {
        case AGUARDANDO_POUSO: return "Aguardando Pouso";
        case POUSANDO: return "Pousando";
        case AGUARDANDO_DESEMBARQUE: return "Aguard. Desembarque";
        case DESEMBARCANDO: return "Desembarcando";
        case AGUARDANDO_DECOLAGEM: return "Aguard. Decolagem";
        case DECOLANDO: return "Decolando";
        case FINALIZADO_SUCESSO: return "Finalizado";
        default: return "Falha";
    }
}

// A nova função de atualização, muito mais limpa e poderosa
void update_terminal_display(SimulacaoAeroporto* sim) {
    // ---- Cabeçalho ----
    wclear(header_win);
    box(header_win, 0, 0);
    time_t tempo_atual = time(NULL);
    int tempo_decorrido = difftime(tempo_atual, sim->tempo_inicio);
    mvwprintw(header_win, 1, 1, "Simulação de Tráfego Aéreo | Tempo: %d/%d s | Simulação: %s", 
              tempo_decorrido, sim->tempo_simulacao, sim->ativa ? "ATIVA" : "FINALIZANDO");
    wrefresh(header_win);
    
    // ---- Pistas ----
    wclear(runway_win);
    box(runway_win, 0, 0);
    mvwprintw(runway_win, 1, 2, "PISTAS (%d/%d)", sim->recursos.total_pistas - sim->recursos.pistas_disponiveis, sim->recursos.total_pistas);
    for (int i = 0; i < sim->recursos.total_pistas; i++) {
        int aviao_id = sim->recursos.pista_ocupada_por[i];
        if (aviao_id != -1) {
            mvwprintw(runway_win, 2 + i, 2, "Pista %d: [OCUPADA] - Avião %d", i, aviao_id);
        } else {
            mvwprintw(runway_win, 2 + i, 2, "Pista %d: [LIVRE]", i);
        }
    }
    wrefresh(runway_win);

    // ---- Portões (similar às pistas) ----
    wclear(gate_win);
    box(gate_win, 0, 0);
    mvwprintw(gate_win, 1, 2, "PORTÕES (%d/%d)", sim->recursos.total_portoes - sim->recursos.portoes_disponiveis, sim->recursos.total_portoes);
    for (int i = 0; i < sim->recursos.total_portoes; i++) {
        int aviao_id = sim->recursos.portao_ocupado_por[i];
        if (aviao_id != -1) {
            mvwprintw(gate_win, 2 + i, 2, "Portão %d: [OCUPADO] - Avião %d", i, aviao_id);
        } else {
            mvwprintw(gate_win, 2 + i, 2, "Portão %d: [LIVRE]", i);
        }
    }
    wrefresh(gate_win);

    // ---- Torre ----
    wclear(tower_win);
    box(tower_win, 0, 0);
    mvwprintw(tower_win, 1, 2, "TORRE DE CONTROLE");
    mvwprintw(tower_win, 2, 2, "Operações simultâneas: %d/%d", sim->recursos.total_torres - sim->recursos.torres_disponiveis, sim->recursos.total_torres);
    wrefresh(tower_win);

    // ---- Lista de Aviões ----
    wclear(airplane_list_win);
    box(airplane_list_win, 0, 0);
    mvwprintw(airplane_list_win, 1, 2, "ID | TIPO | ESTADO");
    int linha_aviao = 2;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].estado != FINALIZADO_SUCESSO && sim->avioes[i].estado != FALHA_DEADLOCK && sim->avioes[i].estado != FALHA_STARVATION) {
             mvwprintw(airplane_list_win, linha_aviao++, 2, "%-3d| %-4s | %s", 
                sim->avioes[i].id, 
                sim->avioes[i].tipo == VOO_DOMESTICO ? "DOM" : "INTL",
                estado_para_str(sim->avioes[i].estado)
            );
        }
    }
    wrefresh(airplane_list_win);

    // É importante chamar doupdate() uma vez no final para renderizar tudo de uma vez
    doupdate();
}

// Função de log segura para threads
void log_evento_ui(SimulacaoAeroporto* sim, const char* formato, ...) {
    char buffer[256];
    va_list args;
    va_start(args, formato);
    vsnprintf(buffer, sizeof(buffer), formato, args);
    va_end(args);

    // Trava o mutex específico para o log antes de interagir com a janela
    pthread_mutex_lock(&sim->mutex_ui_log);
    
    wscrl(log_win, 1); // Rola o conteúdo da janela de log
    mvwprintw(log_win, getmaxy(log_win) - 2, 1, ">> %s", buffer);
    box(log_win, 0, 0); // Redesenha a borda
    wrefresh(log_win);
    
    pthread_mutex_unlock(&sim->mutex_ui_log);
}