#include "terminal.h"

static WINDOW *header_win, *airspace_win, *status_panel_win, *fids_win, *log_win;

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
    if (header_win)         delwin(header_win);
    if (airspace_win)       delwin(airspace_win);
    if (status_panel_win)   delwin(status_panel_win);
    if (fids_win)           delwin(fids_win);
    if (log_win)            delwin(log_win);
    endwin();
}

static void init_windows(){
    header_win       = newwin(HEADER_HEIGHT, COLS, 0, 0);
    airspace_win     = newwin(AIRSPACE_HEIGHT, COLS, HEADER_HEIGHT, 0);
    status_panel_win = newwin(MAIN_HEIGHT, STATUS_WIDTH, HEADER_HEIGHT + AIRSPACE_HEIGHT, 0);
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
    draw_status_panel(sim);
    draw_fids_panel(sim, voos_ativos);

    doupdate();
}

static void draw_status_panel(SimulacaoAeroporto* sim) {
    wclear(status_panel_win);
    box(status_panel_win, 0, 0);
    mvwprintw(status_panel_win, 0, 2, "[AIRPORT STATUS]");
    
    // Seção Pistas
    mvwhline(status_panel_win, 1, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);
    mvwprintw(status_panel_win, 2, 2, "[PISTAS]");
    
    int linha_pistas = 3;
    for (int i = 0; i < sim->recursos.total_pistas; i++) {
        int aviao_id = sim->recursos.pista_ocupada_por[i];
        if (aviao_id != -1) {
            char id_str[5];
            char tipo_char = sim->avioes[aviao_id-1].tipo == VOO_DOMESTICO ? 'D' : 'I';
            snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_id);
            int color_pair = sim->avioes[aviao_id-1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
            wattron(status_panel_win, COLOR_PAIR(color_pair));
            mvwprintw(status_panel_win, linha_pistas + i, 2, "P%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_id - 1].estado));
            wattroff(status_panel_win, COLOR_PAIR(color_pair));
        } else {
            wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
            mvwprintw(status_panel_win, linha_pistas + i, 2, "P%d: [LIVRE]", i);
            wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
        }
    }

    mvwhline(status_panel_win, linha_pistas + sim->recursos.total_pistas, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);

    // Seção Portões
    int linha_portoes = linha_pistas + sim->recursos.total_pistas + 1;
    mvwprintw(status_panel_win, linha_portoes, 2, "[PORTOES]");
    for (int i = 0; i < sim->recursos.total_portoes; i++) {
        int aviao_id = sim->recursos.portao_ocupado_por[i];
        if (aviao_id != -1) {
            char id_str[5];
            char tipo_char = sim->avioes[aviao_id - 1].tipo == VOO_DOMESTICO ? 'D' : 'I';
            snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_id);
            int color_pair = sim->avioes[aviao_id - 1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
            wattron(status_panel_win, COLOR_PAIR(color_pair));
            mvwprintw(status_panel_win, linha_portoes + 1 + i, 2, "G%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_id - 1].estado));
            wattroff(status_panel_win, COLOR_PAIR(color_pair));
        } else {
            wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
            mvwprintw(status_panel_win, linha_portoes + 1 + i, 2, "G%d: [LIVRE]", i);
            wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
        }
    }

    mvwhline(status_panel_win, linha_portoes + 1 + sim->recursos.total_portoes, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);

    // Seção Torres
    int linha_torres = linha_portoes + 1 + sim->recursos.total_portoes + 1;
    mvwprintw(status_panel_win, linha_torres, 2, "[TORRES]");

    int torres_ocupadas = sim->recursos.total_torres - sim->recursos.torres_disponiveis;
    
    for (int i = 0; i < sim->recursos.total_torres; i++) {
        // Verificar se algum avião está usando uma torre
        bool torre_ocupada = false;
        int aviao_usando_torre = -1;
        
        // Procurar por aviões que estão em estados que usam torre
        for (int j = 0; j < sim->metricas.total_avioes_criados; j++) {
            if (sim->avioes[j].id > 0 && sim->avioes[j].torre_alocada && 
                (sim->avioes[j].estado == POUSANDO || sim->avioes[j].estado == DESEMBARCANDO || sim->avioes[j].estado == DECOLANDO)) {
                if (!torre_ocupada) { 
                    torre_ocupada = true;
                    aviao_usando_torre = sim->avioes[j].id;
                    break;
                }
            }
        }
        
        if (torre_ocupada && aviao_usando_torre != -1) {
            char id_str[5];
            char tipo_char = sim->avioes[aviao_usando_torre-1].tipo == VOO_DOMESTICO ? 'D' : 'I';
            snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_usando_torre);
            int color_pair = sim->avioes[aviao_usando_torre-1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
            wattron(status_panel_win, COLOR_PAIR(color_pair));
            mvwprintw(status_panel_win, linha_torres + 1 + i, 2, "T%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_usando_torre - 1].estado));
            wattroff(status_panel_win, COLOR_PAIR(color_pair));
        } else {
            wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
            mvwprintw(status_panel_win, linha_torres + 1 + i, 2, "T%d: [LIVRE]", i);
            wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
        }
    }

    mvwhline(status_panel_win, linha_torres + 1 + sim->recursos.total_torres, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);

    // Secao Legendas
    int linha_legendas = linha_torres + 1 + sim->recursos.total_torres + 1;
    mvwprintw(status_panel_win, linha_legendas, 2, "[LEGENDA]");
    mvwprintw(status_panel_win, linha_legendas + 1, 2, "DXX: Voo Domestico");
    mvwprintw(status_panel_win, linha_legendas + 2, 2, "IXX: Voo Internacional");
    mvwprintw(status_panel_win, linha_legendas + 3, 2, "P:   Pista");
    mvwprintw(status_panel_win, linha_legendas + 4, 2, "G:   Portão");
    mvwprintw(status_panel_win, linha_legendas + 5, 2, "T:   Torre");
    mvwprintw(status_panel_win, linha_legendas + 7, 2, "PLACEHOLDER");
    wattron(status_panel_win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
    mvwprintw(status_panel_win, linha_legendas + 8, 2, "ALERTA = PLACEHOLDER");
    wattroff(status_panel_win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
    wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS) | A_BOLD);
    mvwprintw(status_panel_win, linha_legendas + 9, 2, "ALERTA = PLACEHOLDER");
    wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS) | A_BOLD);

    wrefresh(status_panel_win);
    
}

static void draw_fids_panel(SimulacaoAeroporto* sim, int voos_ativos) {
    wclear(fids_win);
    box(fids_win, 0, 0);
    mvwprintw(fids_win, 0, 2, "[FLIGHT INFORMATION DISPLAY SYSTEM]");
    
    wattron(fids_win, A_BOLD);
    mvwhline(fids_win, 1, 2, ACS_HLINE, getmaxx(fids_win) - 4);
    mvwprintw(fids_win, 2, 2, " Voo | Estado             | Recursos | Espera");
    wattroff(fids_win, A_BOLD);
    mvwhline(fids_win, 3, 2, ACS_HLINE, getmaxx(fids_win) - 4);

    int linha_atual = 4;
    int max_linhas = getmaxy(fids_win) - 1;

    for (int i = 0; i < sim->metricas.total_avioes_criados && linha_atual < max_linhas; i++) {
        Aviao* aviao = &sim->avioes[i];
        if (aviao->id > 0 && aviao->estado != FINALIZADO_SUCESSO && aviao->estado < FALHA_DEADLOCK) {
            int color_pair = aviao->tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            wattron(fids_win, COLOR_PAIR(color_pair));

            char id_str[5];
            snprintf(id_str, sizeof(id_str), "%c%02d", aviao->tipo == VOO_DOMESTICO ? 'D' : 'I', aviao->id);
            
            char recursos_str[8];
            bool tem_pista = false; for(int p=0; p < sim->recursos.total_pistas; ++p) if(sim->recursos.pista_ocupada_por[p] == aviao->id) tem_pista = true;
            bool tem_portao = false; for(int g=0; g < sim->recursos.total_portoes; ++g) if(sim->recursos.portao_ocupado_por[g] == aviao->id) tem_portao = true;
            bool tem_torre = (aviao->estado == POUSANDO || aviao->estado == DESEMBARCANDO || aviao->estado == DECOLANDO);
            //SNPRINTF(FORMATA A STRING PRA SEGUIR UM PADRAO, SEMPRE - - - OU AS VARIAVIES P G T)
            snprintf(recursos_str, sizeof(recursos_str), "%c %c %c", tem_pista ? 'P' : '-', tem_portao ? 'G' : '-', tem_torre ? 'T' : '-');

            time_t agora = time(NULL);
            int espera = (aviao->chegada_na_fila > 0) ? difftime(agora, aviao->chegada_na_fila) : 0;
            
            mvwprintw(fids_win, linha_atual, 2, " %-4s| %-18s | %-8s | %3ds", 
                id_str, estado_para_str(aviao->estado), recursos_str, espera);

            if (espera > 60 && aviao->estado != DECOLANDO && aviao->estado != DESEMBARCANDO && aviao->estado != POUSANDO) {
                wattron(fids_win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
                mvwprintw(fids_win, linha_atual, 47, "[ALERTA]");
                wattroff(fids_win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
            }

            wattroff(fids_win, COLOR_PAIR(color_pair));
            linha_atual++;
        }
    }
    wrefresh(fids_win);
}


static void finalize_log_display(WINDOW* win) {
    if (!win) return;
    wclrtoeol(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, "[LOG]");
    wrefresh(win);
}

static int draw_flight_prefix(WINDOW* win, int y, int x, Aviao* aviao) {
    if (!win || !aviao || aviao->id <= 0) return x;

    int flight_color = (aviao->tipo == VOO_DOMESTICO) ? PAIR_DOM : PAIR_INTL;
    char prefix[PREFIX_SIZE];
    snprintf(prefix, sizeof(prefix), " %c%02d", 
             (aviao->tipo == VOO_DOMESTICO) ? 'D' : 'I', aviao->id);
    
    wattron(win, COLOR_PAIR(flight_color) | A_BOLD);
    mvwprintw(win, y, x, "%s", prefix);
    wattroff(win, COLOR_PAIR(flight_color) | A_BOLD);
    
    mvwprintw(win, y, x + strlen(prefix), "  ");
    return x + strlen(prefix) + 2;
}

static int draw_system_prefix(WINDOW* win, int y, int x) {
    if (!win) return x;
    wattron(win, COLOR_PAIR(LOG_SYSTEM) | A_BOLD);
    mvwprintw(win, y, x, "[SYSTEM]");
    wattroff(win, COLOR_PAIR(LOG_SYSTEM) | A_BOLD);
    return x + 8;
}

static void draw_message_with_color(WINDOW* win, int y, int x, const char* buffer, int cor) {
    if (!win || !buffer) return;

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    int max_len = max_x - x - 5;

    wattron(win, COLOR_PAIR(cor));
    if (max_len > 0) {
        mvwprintw(win, y, x, "  %.*s", max_len, buffer);
    }
    wattroff(win, COLOR_PAIR(cor));
}

static int draw_timestamp(WINDOW* win, int y, SimulacaoAeroporto* sim) {
    if (!sim || !win) return 1;

    int tempo_decorrido = difftime(time(NULL), sim->tempo_inicio);
    int minutos = tempo_decorrido / 60;
    int segundos = tempo_decorrido % 60;
    mvwprintw(win, y, 1, "[%02d:%02d] ", minutos, segundos);
    return 10;
}

static void display_log_in_window(SimulacaoAeroporto* sim, Aviao* aviao, const char* buffer, int cor) {
    if (!log_win || !buffer) return;

    int max_y, max_x;
    getmaxyx(log_win, max_y, max_x);
    if (max_y <= 2 || max_x <= 2) return;
    
    wscrl(log_win, 1);
    
    int y = max_y - 2;
    if (y < 1) y = 1;

    int x = draw_timestamp(log_win, y, sim);
    
    if (aviao && aviao->id > 0) {
        x = draw_flight_prefix(log_win, y, x, aviao);
        draw_message_with_color(log_win, y, x, buffer, cor);
    } else {
        x = draw_system_prefix(log_win, y, x);
        draw_message_with_color(log_win, y, x, buffer, cor);
    }
    
    finalize_log_display(log_win);
}

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
    display_log_in_window(sim, aviao, buffer, cor);
    
    pthread_mutex_unlock(&sim->mutex_ui_log);
}