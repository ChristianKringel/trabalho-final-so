// terminal.c
#include "libs.h" // Ou simulador.h


// As janelas agora são 'static' para serem visíveis apenas neste arquivo.
// Isso é uma boa prática para evitar poluir o namespace global.
//static WINDOW *runway_win, *gate_win, *tower_win, *airplane_list_win, *log_win, *header_win, *airspace_win;

void init_terminal_ncurses() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    start_color();
    
    // ISSO AQUI VIRA UMA FUNCAO DEPOis
    if (LINES < 24 || COLS < 120) {
        endwin();
        printf("Terminal muito pequeno. Mínimo recomendado: 120x24\n");
        exit(1);
    }

    int header_height = 3;
    int status_height = 5; 
    int info_width = 50;   

    int main_height = LINES - header_height - status_height;
    int log_width = COLS - info_width;

    // --- Criação das Janelas ---
    header_win = newwin(header_height, COLS, 0, 0);
    status_win = newwin(status_height, COLS, LINES - status_height, 0);
    log_win = newwin(main_height, log_width, header_height, 0);
    info_win = newwin(main_height, info_width, header_height, log_width);

    scrollok(log_win, TRUE);
    clear();
    refresh();
}

void close_terminal_ncurses() {
    if (header_win) delwin(header_win);
    if (log_win) delwin(log_win);
    if (info_win) delwin(info_win);
    if (status_win) delwin(status_win);
    
    endwin();
}

const char* estado_para_str(EstadoAviao estado) {
    switch (estado) {
        case AGUARDANDO_POUSO:          return "Aguardando Pouso";
        case POUSANDO:                  return "Pousando";
        case AGUARDANDO_DESEMBARQUE:    return "Aguard. Desembarque";
        case DESEMBARCANDO:             return "Desembarcando";
        case AGUARDANDO_DECOLAGEM:      return "Aguard. Decolagem";
        case DECOLANDO:                 return "Decolando";
        case FINALIZADO_SUCESSO:        return "Finalizado";
        default:                        return "Falha";
    }
}

void update_terminal_display(SimulacaoAeroporto* sim) {
    if (!sim) return;

    // --- Contagem de Voos Ativos ---
    int voos_ativos = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado != FINALIZADO_SUCESSO && sim->avioes[i].estado < FALHA_DEADLOCK) {
            voos_ativos++;
        }
    }

    // ---- Cabeçalho (Header) ----
    wclear(header_win);
    box(header_win, 0, 0);
    time_t tempo_atual = time(NULL);
    int tempo_decorrido = difftime(tempo_atual, sim->tempo_inicio);
    
    // Show pause status in header
    const char* status_sim = sim->ativa ? (sim->pausado ? "PAUSADA" : "ATIVA") : "FINALIZANDO";
    mvwprintw(header_win, 1, 2, "SIMULACAO DE TRAFEGO AEREO | Tempo: %d/%d s | Voos Ativos: %d | Status: %s", 
              tempo_decorrido, sim->tempo_simulacao, voos_ativos, status_sim);
    wrefresh(header_win);

    // ---- Painel de Status (Inferior) para Pistas e Portões ----
    wclear(status_win);
    box(status_win, 0, 0);
    int col_width = getmaxx(status_win) / 2;
    // Desenha Pistas na metade esquerda
    mvwprintw(status_win, 1, 2, "[PISTAS (%d/%d)]", sim->recursos.total_pistas - sim->recursos.pistas_disponiveis, sim->recursos.total_pistas);
    for (int i = 0; i < sim->recursos.total_pistas; i++) {
        int aviao_id = sim->recursos.pista_ocupada_por[i];
        if (aviao_id != -1) {
            mvwprintw(status_win, 2 + i, 2, "Pista %d: Aviao %d", i, aviao_id);
        } else {
            mvwprintw(status_win, 2 + i, 2, "Pista %d: [LIVRE]", i);
        }
    }
    // Desenha Portões na metade direita
    mvwprintw(status_win, 1, col_width, "[PORTOES (%d/%d)]", sim->recursos.total_portoes - sim->recursos.portoes_disponiveis, sim->recursos.total_portoes);
    for (int i = 0; i < sim->recursos.total_portoes; i++) {
        int aviao_id = sim->recursos.portao_ocupado_por[i];
        if (aviao_id != -1) {
             mvwprintw(status_win, 2 + i, col_width, "Portao %d: Aviao %d", i, aviao_id);
        } else {
             mvwprintw(status_win, 2 + i, col_width, "Portao %d: [LIVRE]", i);
        }
    }
    wrefresh(status_win);


    // ---- Painel de Informação (Direita) ----
    wclear(info_win);
    box(info_win, 0, 0);
    int info_height = getmaxy(info_win);
    // Dividimos o painel: 40% para Airspace, 60% para Flight Info
    int airspace_section_height = (info_height * 40) / 100;
    int fids_section_y_start = airspace_section_height;
    int linha_info = 1;

    // --- Seção ESPAÇO AÉREO (agora no painel direito) ---
    int total_aguardando = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado == AGUARDANDO_POUSO) total_aguardando++;
    }
    mvwprintw(info_win, linha_info++, 2, "[ESPACO AEREO (Total: %d)]", total_aguardando);

    int max_linhas_airspace = airspace_section_height - 2; // -2 para título e borda
    int impressos_airspace = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados && impressos_airspace < max_linhas_airspace; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado == AGUARDANDO_POUSO) {
            if (total_aguardando > max_linhas_airspace && impressos_airspace == max_linhas_airspace - 1) {
                mvwprintw(info_win, linha_info++, 3, "... e mais %d", total_aguardando - impressos_airspace);
            } else {
                mvwprintw(info_win, linha_info++, 3, "Aviao %d", sim->avioes[i].id);
            }
            impressos_airspace++;
        }
    }
    
    // --- Seção FLIGHT INFORMATION (com paginação) ---
    linha_info = fids_section_y_start; // Pula para a segunda metade do painel
    mvwprintw(info_win, linha_info++, 1, "------------------------------------------------");
    mvwprintw(info_win, linha_info++, 2, "[FLIGHT INFORMATION (Total: %d)]", voos_ativos);
    mvwprintw(info_win, linha_info++, 2, "ID|TIPO|ESTADO            |RECURSOS");

    int max_linhas_fids = info_height - fids_section_y_start - 3; // Espaço restante
    int impressos_fids = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados && impressos_fids < max_linhas_fids; i++) {
        Aviao* aviao = &sim->avioes[i];
        if (aviao->id > 0 && aviao->estado != FINALIZADO_SUCESSO && aviao->estado < FALHA_DEADLOCK) {
            char recursos_str[20] = ""; 
            bool tem_pista = false; for(int p=0; p < sim->recursos.total_pistas; ++p) if(sim->recursos.pista_ocupada_por[p] == aviao->id) tem_pista = true;
            bool tem_portao = false; for(int g=0; g < sim->recursos.total_portoes; ++g) if(sim->recursos.portao_ocupado_por[g] == aviao->id) tem_portao = true;
            bool tem_torre = (aviao->estado == POUSANDO || aviao->estado == DESEMBARCANDO || aviao->estado == DECOLANDO);
            sprintf(recursos_str, "%s%s%s", tem_pista ? "P " : "", tem_portao ? "G " : "", tem_torre ? "T" : "");

            if (voos_ativos > max_linhas_fids && impressos_fids == max_linhas_fids - 1) {
                mvwprintw(info_win, linha_info++, 2, "... e mais %d voos", voos_ativos - impressos_fids);
            } else {
                 mvwprintw(info_win, linha_info++, 2, "%-2d|%-4s|%-18s|%s",
                    aviao->id, aviao->tipo == VOO_DOMESTICO ? "DOM" : "INTL",
                    estado_para_str(aviao->estado), recursos_str);
            }
            impressos_fids++;
        }
    }
    wrefresh(info_win);


    // ---- Painel de Log (Principal Esquerdo) ----
    // Nenhuma lógica de escrita aqui. Apenas desenhamos a caixa.
    // A função log_evento_ui fará todo o trabalho de escrita.
    box(log_win, 0, 0);
    wrefresh(log_win);

    // Renderiza tudo de uma vez
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