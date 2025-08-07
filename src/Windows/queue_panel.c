#include "../../include/queue_panel.h"
#include "../../include/terminal.h"

static bool can_draw_on_queue(WINDOW* win, int current_column, const char* id_str) {
    if (!win || !id_str) return false;
    return (current_column + strlen(id_str) < COLS - 15);
}

static bool verify_space(WINDOW* win, int y, int x, const char* str) {
    if (!win || !str) return false;
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    return (y >= 0 && y < max_y && x >= 0 && x + strlen(str) < max_x);
}

static int get_waiting_airplanes(SimulacaoAeroporto* sim) {
    if (!sim) return 0;
    int count = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado == AGUARDANDO_POUSO) {
            count++;
        }
    }
    return count;
}

static void draw_window_title(WINDOW* win, const char* title) {
    if (!win || !title) return;
    box(win, 0, 0);
    mvwprintw(win, 0, 2, "%s", title);
}

static void draw_airplane_id(WINDOW* win, int y, int x, Aviao* aviao) {
    if (!win || !aviao || aviao->id <= 0) return;
    int current_column = x;
    int flight_color = (aviao->tipo == VOO_DOMESTICO) ? PAIR_DOM : PAIR_INTL;
    int flight_type  = (aviao->tipo == VOO_DOMESTICO) ? 'D' : 'I';
    int flight_id    = aviao->id;
    char prefix[10];
    snprintf(prefix, sizeof(prefix), "%c%02d · ", flight_type, flight_id);

    if (!verify_space(win, y, current_column, prefix)) return;
    
    wattron(win, COLOR_PAIR(flight_color) | A_BOLD);
    mvwprintw(win, y, x, "%s", prefix);
    wattroff(win, COLOR_PAIR(flight_color) | A_BOLD);
    
    //mvwprintw(win, y, x + strlen(prefix), "  ");
}

static int draw_airplane_queue(SimulacaoAeroporto* sim, WINDOW* win, int* col_atual) {
    if (!sim || !win || !col_atual) return 0;
    
    int avioes_mostrados = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado == AGUARDANDO_POUSO) {
            char id_str[10];
            char tipo_char = sim->avioes[i].tipo == VOO_DOMESTICO ? 'D' : 'I';
            snprintf(id_str, sizeof(id_str), "%c%02d · ", tipo_char, sim->avioes[i].id);

            if (can_draw_on_queue(win, *col_atual, id_str)) {
                draw_airplane_id(win, 1, *col_atual, &sim->avioes[i]);
                *col_atual += strlen(id_str);
                avioes_mostrados++;
            }
        }
    }
    return avioes_mostrados;
}

static void draw_queue_indicators(WINDOW* win, int col_atual, int avioes_mostrados, int total_aguardando) {
    if (!win) return;
    
    if (avioes_mostrados < total_aguardando) {
        mvwprintw(win, 1, col_atual, "... ");
    }
    mvwprintw(win, 0, 19, "(Total: %d)", total_aguardando);
}

void manage_queue_panel(SimulacaoAeroporto* sim, int voos_ativos, WINDOW* queue_win) {
    if (!sim || !queue_win) return;

    wclear(queue_win);
    draw_window_title(queue_win, "[AIRSPACE QUEUE]");
    
    int col_atual = 2;
    int total_aguardando = get_waiting_airplanes(sim);
    int avioes_mostrados = draw_airplane_queue(sim, queue_win, &col_atual);
    
    draw_queue_indicators(queue_win, col_atual, avioes_mostrados, total_aguardando);
    
    wrefresh(queue_win);
}