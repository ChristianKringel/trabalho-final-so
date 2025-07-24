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
    
    // Verificar se o terminal é grande o suficiente
    if (LINES < 20 || COLS < 80) {
        endwin();
        printf("Terminal muito pequeno. Mínimo: 80x20\n");
        exit(1);
    }
    
    // Calcular dimensões das janelas de forma mais segura
    int header_height = 3;
    int log_height = 5;
    int remaining_height = LINES - header_height - log_height;
    int col_width = COLS / 3;
    
    // Garantir que as dimensões são válidas
    if (remaining_height < 6) remaining_height = 6;
    if (col_width < 20) col_width = 20;
    
    // Criar janelas com verificação de erro
    header_win = newwin(header_height, COLS, 0, 0);
    if (!header_win) { endwin(); printf("Erro criando header_win\n"); exit(1); }
    
    // Janelas principais em colunas
    runway_win = newwin(remaining_height, col_width, header_height, 0);
    if (!runway_win) { endwin(); printf("Erro criando runway_win\n"); exit(1); }
    
    gate_win = newwin(remaining_height, col_width, header_height, col_width);
    if (!gate_win) { endwin(); printf("Erro criando gate_win\n"); exit(1); }
    
    tower_win = newwin(remaining_height / 2, col_width, header_height, col_width * 2);
    if (!tower_win) { endwin(); printf("Erro criando tower_win\n"); exit(1); }
    
    airplane_list_win = newwin(remaining_height / 2, col_width, header_height + remaining_height / 2, col_width * 2);
    if (!airplane_list_win) { endwin(); printf("Erro criando airplane_list_win\n"); exit(1); }
    
    // Janela de log na parte inferior
    log_win = newwin(log_height, COLS, LINES - log_height, 0);
    if (!log_win) { endwin(); printf("Erro criando log_win\n"); exit(1); }
    
    scrollok(log_win, TRUE); // Habilita rolagem para o log
    
    // Limpar tela e atualizar
    clear();
    refresh();
}

void close_terminal_ncurses() {
    // Destruir todas as janelas criadas
    if (header_win) delwin(header_win);
    if (runway_win) delwin(runway_win);
    if (gate_win) delwin(gate_win);
    if (tower_win) delwin(tower_win);
    if (airplane_list_win) delwin(airplane_list_win);
    if (log_win) delwin(log_win);
    
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
    if (!sim) return;
    
    // ---- Cabeçalho ----
    if (header_win) {
        wclear(header_win);
        box(header_win, 0, 0);
        time_t tempo_atual = time(NULL);
        int tempo_decorrido = difftime(tempo_atual, sim->tempo_inicio);
        mvwprintw(header_win, 1, 1, "Simulacao de Trafego Aereo | Tempo: %d/%d s | Simulacao: %s", 
                  tempo_decorrido, sim->tempo_simulacao, sim->ativa ? "ATIVA" : "FINALIZANDO");
        wrefresh(header_win);
    }
    
    // ---- Pistas ----
    if (runway_win) {
        wclear(runway_win);
        box(runway_win, 0, 0);
        mvwprintw(runway_win, 1, 2, "PISTAS (%d/%d)", 
                  sim->recursos.total_pistas - sim->recursos.pistas_disponiveis, 
                  sim->recursos.total_pistas);
        for (int i = 0; i < sim->recursos.total_pistas && i < getmaxy(runway_win) - 3; i++) {
            int aviao_id = sim->recursos.pista_ocupada_por[i];
            if (aviao_id != -1) {
                mvwprintw(runway_win, 2 + i, 2, "Pista %d: [OCUPADA] - Aviao %d", i, aviao_id);
            } else {
                mvwprintw(runway_win, 2 + i, 2, "Pista %d: [LIVRE]", i);
            }
        }
        wrefresh(runway_win);
    }

    // ---- Portões ----
    if (gate_win) {
        wclear(gate_win);
        box(gate_win, 0, 0);
        mvwprintw(gate_win, 1, 2, "PORTOES (%d/%d)", 
                  sim->recursos.total_portoes - sim->recursos.portoes_disponiveis, 
                  sim->recursos.total_portoes);
        for (int i = 0; i < sim->recursos.total_portoes && i < getmaxy(gate_win) - 3; i++) {
            int aviao_id = sim->recursos.portao_ocupado_por[i];
            if (aviao_id != -1) {
                mvwprintw(gate_win, 2 + i, 2, "Portao %d: [OCUPADO] - Aviao %d", i, aviao_id);
            } else {
                mvwprintw(gate_win, 2 + i, 2, "Portao %d: [LIVRE]", i);
            }
        }
        wrefresh(gate_win);
    }

    // ---- Torre ----
    if (tower_win) {
        wclear(tower_win);
        box(tower_win, 0, 0);
        mvwprintw(tower_win, 1, 2, "TORRE DE CONTROLE");
        mvwprintw(tower_win, 2, 2, "Operacoes: %d/%d", 
                  sim->recursos.total_torres - sim->recursos.torres_disponiveis, 
                  sim->recursos.total_torres);
        wrefresh(tower_win);
    }

    // ---- Lista de Aviões ----
    if (airplane_list_win) {
        wclear(airplane_list_win);
        box(airplane_list_win, 0, 0);
        mvwprintw(airplane_list_win, 1, 2, "ID | TIPO | ESTADO");
        int linha_aviao = 2;
        int max_lines = getmaxy(airplane_list_win) - 3;
        
        for (int i = 0; i < sim->metricas.total_avioes_criados && linha_aviao < max_lines; i++) {
            if (sim->avioes[i].id > 0 && 
                sim->avioes[i].estado != FINALIZADO_SUCESSO && 
                sim->avioes[i].estado != FALHA_DEADLOCK && 
                sim->avioes[i].estado != FALHA_STARVATION) {
                 mvwprintw(airplane_list_win, linha_aviao++, 2, "%-3d| %-4s | %s", 
                    sim->avioes[i].id, 
                    sim->avioes[i].tipo == VOO_DOMESTICO ? "DOM" : "INTL",
                    estado_para_str(sim->avioes[i].estado)
                );
            }
        }
        wrefresh(airplane_list_win);
    }

    // É importante chamar doupdate() uma vez no final para renderizar tudo de uma vez
    doupdate();
}

// Função de log segura para threads
void log_evento_ui(SimulacaoAeroporto* sim, const char* formato, ...) {
    if (!sim || !log_win || !formato) return;
    
    char buffer[256];
    va_list args;
    va_start(args, formato);
    vsnprintf(buffer, sizeof(buffer), formato, args);
    va_end(args);

    // Trava o mutex específico para o log antes de interagir com a janela
    pthread_mutex_lock(&sim->mutex_ui_log);
    
    // Verificar se a janela ainda existe
    if (log_win) {
        wscrl(log_win, 1); // Rola o conteúdo da janela de log
        mvwprintw(log_win, getmaxy(log_win) - 2, 1, ">> %s", buffer);
        box(log_win, 0, 0); // Redesenha a borda
        wrefresh(log_win);
    }
    
    pthread_mutex_unlock(&sim->mutex_ui_log);
}