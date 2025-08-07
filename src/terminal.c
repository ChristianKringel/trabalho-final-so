#include "terminal.h"

static WINDOW *header_win, *airspace_win, *status_win, *fids_win, *log_win;

#define MAX_LOG_BUFFER 256
#define MAX_ID_STR 10
#define MAX_RESOURCES_STR 10
#define TIMESTAMP_SIZE 10
#define PREFIX_SIZE 8
#define MAX_AIRSPACE_WIDTH (COLS - 15)

#define PISTA_FORMAT "P%d: [%s] %-14s"
#define PORTAO_FORMAT "G%d: [%s] %-14s"
#define TORRE_FORMAT "T%d: [%s] %-14s"
#define RECURSO_LIVRE_FORMAT "%c%d: [LIVRE]"
#define FLIGHT_INFO_FORMAT " %-4s| %-18s | %-8s | %3ds"

void init_terminal_ncurses() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    
    if (has_colors() == FALSE) {
        endwin();
        printf("Seu terminal nao suporta cores.\n");
        exit(1);
    }
    init_colors();

    if (LINES < 24 || COLS < 120) {
        endwin();
        printf("Terminal muito pequeno. Minimo recomendado: 120x24\n");
        exit(1);
    }

    init_windows();

    clear();
    refresh();
}

void close_terminal_ncurses() {
    if (header_win)     delwin(header_win);
    if (airspace_win)   delwin(airspace_win);
    if (status_win)     delwin(status_win);
    if (fids_win)       delwin(fids_win);
    if (log_win)        delwin(log_win);
    endwin();
}

static void init_windows(){
    header_win       = newwin(HEADER_HEIGHT, COLS, 0, 0);
    airspace_win     = newwin(AIRSPACE_HEIGHT, COLS, HEADER_HEIGHT, 0);
    status_win       = newwin(MAIN_HEIGHT, STATUS_WIDTH, HEADER_HEIGHT + AIRSPACE_HEIGHT, 0);
    fids_win         = newwin(MAIN_HEIGHT, FIDS_WIDTH, HEADER_HEIGHT + AIRSPACE_HEIGHT, STATUS_WIDTH);
    log_win          = newwin(MAIN_HEIGHT, LOG_WIDTH, HEADER_HEIGHT + AIRSPACE_HEIGHT, STATUS_WIDTH + FIDS_WIDTH);
    scrollok(log_win, TRUE);
    wbkgd(log_win, COLOR_PAIR(PAIR_DEFAULT));
    box(log_win, 0, 0);
    mvwprintw(log_win, 0, 2, "[LOG]");
    wrefresh(log_win);
}

static void init_colors() {
    start_color();
    
    init_pair(PAIR_DEFAULT, COLOR_WHITE, COLOR_BLACK);        // Branco padrão
    init_pair(PAIR_HEADER, COLOR_BLACK, COLOR_WHITE);         // Header invertido
    init_pair(PAIR_DOM, COLOR_BLUE, COLOR_BLACK);             // Azul para voos domésticos
    init_pair(PAIR_INTL, COLOR_MAGENTA, COLOR_BLACK);         // Magenta para voos internacionais
    init_pair(PAIR_SUCCESS, COLOR_GREEN, COLOR_BLACK);        // Verde para sucessos
    init_pair(PAIR_ALERT, COLOR_RED, COLOR_BLACK);            // Vermelho para erros
    init_pair(PAIR_WARNING, COLOR_YELLOW, COLOR_BLACK);       // Amarelo para avisos
    init_pair(PAIR_INFO, COLOR_CYAN, COLOR_BLACK);            // Ciano para informações
    init_pair(PAIR_SYSTEM, COLOR_WHITE, COLOR_BLACK);         // Branco para sistema
    init_pair(PAIR_RESOURCE, COLOR_MAGENTA, COLOR_BLACK);     // Magenta para recursos
    init_pair(PAIR_TIMING, COLOR_WHITE, COLOR_BLACK);         // Branco brilhante para timing
    init_pair(PAIR_DEBUG, COLOR_BLACK, COLOR_WHITE);          // Cinza para debug
}

void update_terminal_display(SimulacaoAeroporto* sim) {
    if (!sim) return;

    int voos_ativos = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado != FINALIZADO_SUCESSO && sim->avioes[i].estado < FALHA_DEADLOCK) {
            voos_ativos++;
        }
    }

    manage_header_panel(sim, voos_ativos, header_win);
    manage_queue_panel(sim, voos_ativos, airspace_win);
    manage_status_panel(sim, voos_ativos, status_win);
   //draw_status_panel(sim);
    manage_info_panel(sim, voos_ativos, fids_win);

    doupdate();
}

// static void draw_status_panel(SimulacaoAeroporto* sim) {
//     wclear(status_panel_win);
//     box(status_panel_win, 0, 0);
//     mvwprintw(status_panel_win, 0, 2, "[AIRPORT STATUS]");
    
//     // Seção Pistas
//     mvwhline(status_panel_win, 1, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);
//     mvwprintw(status_panel_win, 2, 2, "[PISTAS]");
    
//     int linha_pistas = 3;
//     for (int i = 0; i < sim->recursos.total_pistas; i++) {
//         int aviao_id = sim->recursos.pista_ocupada_por[i];
//         if (aviao_id != -1) {
//             char id_str[5];
//             char tipo_char = sim->avioes[aviao_id-1].tipo == VOO_DOMESTICO ? 'D' : 'I';
//             snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_id);
//             int color_pair = sim->avioes[aviao_id-1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
//             wattron(status_panel_win, COLOR_PAIR(color_pair));
//             mvwprintw(status_panel_win, linha_pistas + i, 2, "P%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_id - 1].estado));
//             wattroff(status_panel_win, COLOR_PAIR(color_pair));
//         } else {
//             wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
//             mvwprintw(status_panel_win, linha_pistas + i, 2, "P%d: [LIVRE]", i);
//             wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
//         }
//     }

//     mvwhline(status_panel_win, linha_pistas + sim->recursos.total_pistas, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);

//     // Seção Portões
//     int linha_portoes = linha_pistas + sim->recursos.total_pistas + 1;
//     mvwprintw(status_panel_win, linha_portoes, 2, "[PORTOES]");
//     for (int i = 0; i < sim->recursos.total_portoes; i++) {
//         int aviao_id = sim->recursos.portao_ocupado_por[i];
//         if (aviao_id != -1) {
//             char id_str[5];
//             char tipo_char = sim->avioes[aviao_id - 1].tipo == VOO_DOMESTICO ? 'D' : 'I';
//             snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_id);
//             int color_pair = sim->avioes[aviao_id - 1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
//             wattron(status_panel_win, COLOR_PAIR(color_pair));
//             mvwprintw(status_panel_win, linha_portoes + 1 + i, 2, "G%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_id - 1].estado));
//             wattroff(status_panel_win, COLOR_PAIR(color_pair));
//         } else {
//             wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
//             mvwprintw(status_panel_win, linha_portoes + 1 + i, 2, "G%d: [LIVRE]", i);
//             wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
//         }
//     }

//     mvwhline(status_panel_win, linha_portoes + 1 + sim->recursos.total_portoes, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);

//     // Seção Torres
//     int linha_torres = linha_portoes + 1 + sim->recursos.total_portoes + 1;
//     mvwprintw(status_panel_win, linha_torres, 2, "[TORRES]");

//     int torres_ocupadas = sim->recursos.total_torres - sim->recursos.torres_disponiveis;
    
//     for (int i = 0; i < sim->recursos.total_torres; i++) {
//         // Verificar se algum avião está usando uma torre
//         bool torre_ocupada = false;
//         int aviao_usando_torre = -1;
        
//         // Procurar por aviões que estão em estados que usam torre
//         for (int j = 0; j < sim->metricas.total_avioes_criados; j++) {
//             if (sim->avioes[j].id > 0 && sim->avioes[j].torre_alocada && 
//                 (sim->avioes[j].estado == POUSANDO || sim->avioes[j].estado == DESEMBARCANDO || sim->avioes[j].estado == DECOLANDO)) {
//                 if (!torre_ocupada) { 
//                     torre_ocupada = true;
//                     aviao_usando_torre = sim->avioes[j].id;
//                     break;
//                 }
//             }
//         }
        
//         if (torre_ocupada && aviao_usando_torre != -1) {
//             char id_str[5];
//             char tipo_char = sim->avioes[aviao_usando_torre-1].tipo == VOO_DOMESTICO ? 'D' : 'I';
//             snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_usando_torre);
//             int color_pair = sim->avioes[aviao_usando_torre-1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
//             wattron(status_panel_win, COLOR_PAIR(color_pair));
//             mvwprintw(status_panel_win, linha_torres + 1 + i, 2, "T%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_usando_torre - 1].estado));
//             wattroff(status_panel_win, COLOR_PAIR(color_pair));
//         } else {
//             wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
//             mvwprintw(status_panel_win, linha_torres + 1 + i, 2, "T%d: [LIVRE]", i);
//             wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
//         }
//     }

//     mvwhline(status_panel_win, linha_torres + 1 + sim->recursos.total_torres, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);

//     // Secao Legendas
//     int linha_legendas = linha_torres + 1 + sim->recursos.total_torres + 1;
//     mvwprintw(status_panel_win, linha_legendas, 2, "[LEGENDA]");
//     mvwprintw(status_panel_win, linha_legendas + 1, 2, "DXX: Voo Domestico");
//     mvwprintw(status_panel_win, linha_legendas + 2, 2, "IXX: Voo Internacional");
//     mvwprintw(status_panel_win, linha_legendas + 3, 2, "P:   Pista");
//     mvwprintw(status_panel_win, linha_legendas + 4, 2, "G:   Portão");
//     mvwprintw(status_panel_win, linha_legendas + 5, 2, "T:   Torre");
//     mvwprintw(status_panel_win, linha_legendas + 7, 2, "PLACEHOLDER");
//     wattron(status_panel_win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
//     mvwprintw(status_panel_win, linha_legendas + 8, 2, "ALERTA = PLACEHOLDER");
//     wattroff(status_panel_win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
//     wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS) | A_BOLD);
//     mvwprintw(status_panel_win, linha_legendas + 9, 2, "ALERTA = PLACEHOLDER");
//     wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS) | A_BOLD);

//     wrefresh(status_panel_win);
    
// }

static bool validate_log_params(SimulacaoAeroporto* sim, const char* formato) {
    return sim && formato;
}

static void format_log_message(char* buffer, size_t size, const char* formato, va_list args) {
    if (size == 0 || !buffer || !formato) return;

    vsnprintf(buffer, size - 1, formato, args);
    buffer[size - 1] = '\0';
}

void log_evento_ui(SimulacaoAeroporto* sim, Aviao* aviao, int cor, const char* formato, ...) {
    if (!validate_log_params(sim, formato)) return;
    
    char buffer[MAX_LOG_BUFFER];
    va_list args;
    va_start(args, formato);
    format_log_message(buffer, sizeof(buffer), formato, args);
    va_end(args);

    
    pthread_mutex_lock(&sim->mutex_ui_log);
    
    write_to_log_file(sim, aviao, buffer);
    manage_log_panel(sim, aviao, buffer, cor, log_win);

    pthread_mutex_unlock(&sim->mutex_ui_log);
}