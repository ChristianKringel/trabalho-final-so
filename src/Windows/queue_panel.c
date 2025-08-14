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

static void draw_airplane_id(WINDOW* win, int y, int x, Aviao* aviao) {
    if (!win || !aviao || aviao->id <= 0) return;

    char id[10];
    format_flight_id_dot(id, sizeof(id), aviao);
    if (!verify_space(win, y, x, id)) return;
    
    draw_window_text(win, y, x, id, get_flight_color_pair(aviao));
}

static int draw_airplane_queue(SimulacaoAeroporto* sim, WINDOW* win, int* col_atual) {
    if (!sim || !win || !col_atual) return 0;
    
    int avioes_mostrados = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado == AGUARDANDO_POUSO) {
            char id_str[10];
            format_flight_id_dot(id_str, sizeof(id_str), &sim->avioes[i]);

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
    
    if (avioes_mostrados < total_aguardando) { mvwprintw(win, 1, col_atual, "... "); }
    mvwprintw(win, 0, 19, "(Total: %d)", total_aguardando);
}

void manage_queue_panel(SimulacaoAeroporto* sim, WINDOW* queue_win) {
    if (!sim || !queue_win) return;

    clear_and_box_window(queue_win);
    draw_window_title(queue_win, "[AIRSPACE QUEUE]");
    
    int col_atual = 2;
    int total_aguardando = get_waiting_airplanes(sim);
    int avioes_mostrados = draw_airplane_queue(sim, queue_win, &col_atual);
    
    draw_queue_indicators(queue_win, col_atual, avioes_mostrados, total_aguardando);
    
    finalize_window_display(queue_win);
}