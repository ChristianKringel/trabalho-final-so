#include "terminal.h"

static WINDOW *header_win, *airspace_win, *status_win, *queue_win, *log_win;

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

static void init_windows(){
    header_win       = newwin(HEADER_HEIGHT, COLS, 0, 0);
    airspace_win     = newwin(AIRSPACE_HEIGHT, COLS, HEADER_HEIGHT, 0);
    status_win       = newwin(MAIN_HEIGHT, STATUS_WIDTH, HEADER_HEIGHT + AIRSPACE_HEIGHT, 0);
    queue_win         = newwin(MAIN_HEIGHT, FIDS_WIDTH, HEADER_HEIGHT + AIRSPACE_HEIGHT, STATUS_WIDTH);
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

static bool validate_log_params(SimulacaoAeroporto* sim, const char* formato) {
    return sim && formato;
}

static void format_log_message(char* buffer, size_t size, const char* formato, va_list args) {
    if (size == 0 || !buffer || !formato) return;

    vsnprintf(buffer, size - 1, formato, args);
    buffer[size - 1] = '\0';
}

static void get_active_planes(SimulacaoAeroporto* sim, int* voos_ativos) {
    if (!sim || !voos_ativos) return;

    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado != FINALIZADO_SUCESSO && sim->avioes[i].estado < FALHA_DEADLOCK) {
            (*voos_ativos)++;
        }
    }
}

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
    if (queue_win)      delwin(queue_win);
    if (log_win)        delwin(log_win);
    endwin();
}

void update_terminal_display(SimulacaoAeroporto* sim) {
    if (!sim) return;

    int voos_ativos = 0;
    get_active_planes(sim, &voos_ativos);

    manage_header_panel(sim, voos_ativos, header_win);
    manage_queue_panel(sim, airspace_win);
    manage_status_panel(sim, status_win);
    manage_info_panel(sim, queue_win);

    doupdate();
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