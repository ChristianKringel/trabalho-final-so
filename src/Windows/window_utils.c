#include "../../include/window_utils.h"

void draw_window_title_full(WINDOW* win, const char* title, bool bold) {
    if (!win || !title) return;
    if (bold) { wattron(win, A_BOLD); }

    box(win, 0, 0);
    mvwprintw(win, 0, 2, "%s", title);

    if (bold) { wattroff(win, A_BOLD); }
}

void draw_window_title(WINDOW* win, const char* title) {
    draw_window_title_full(win, title, false);
}

void draw_window_section_title_full(WINDOW* win, int line, const char* title, bool bold) {
    if (!win || !title) return;
    if (bold) { wattron(win, A_BOLD); }

    mvwhline(win, line, 2, ACS_HLINE, getmaxx(win) - 4);
    mvwprintw(win, line + 1, 2, "%s", title);

    if (bold) { wattroff(win, A_BOLD); }
}

void draw_window_section_title(WINDOW* win, int line, const char* title) {
    draw_window_section_title_full(win, line, title, false);
}

void draw_window_text_full(WINDOW* win, int line, int column, const char* text, bool bold, int cor) {
    if (!win || !text) return;
    
    if (cor != 0) wattron(win, COLOR_PAIR(cor));
    if (bold) wattron(win, A_BOLD);

    mvwprintw(win, line, column, "%s", text);

    if (bold) wattroff(win, A_BOLD);
    if (cor != 0) wattroff(win, COLOR_PAIR(cor));
}

void draw_window_text(WINDOW* win, int line, int column, const char* text, int cor) {
    draw_window_text_full(win, line, column, text, false, cor);
}

void draw_horizontal_separator(WINDOW* win, int line) {
    if (!win) return;
    
    mvwhline(win, line, 2, ACS_HLINE, getmaxx(win) - 4);
}

void finalize_window_display(WINDOW* win) {
    if (!win) return;
    
    wrefresh(win);
}

void clear_and_box_window(WINDOW* win) {
    if (!win) return;
    
    wclear(win);
    box(win, 0, 0);
}

int get_waiting_airplanes(SimulacaoAeroporto* sim) {
    if (!sim) return 0;
    int count = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado == AGUARDANDO_POUSO) {
            count++;
        }
    }
    return count;
}

bool validate_window_params(WINDOW* win, void* data) {
    return win && data;
}

bool validate_window_bounds(WINDOW* win, int linha, int coluna) {
    if (!win) return false;
    
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    return (linha >= 0 && linha < max_y && coluna >= 0 && coluna < max_x);
}

bool is_window_usable(WINDOW* win) {
    if (!win) return false;
    
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    return (max_y > 2 && max_x > 2);
}

void format_flight_id(char* buffer, size_t size, Aviao* aviao) {
    if (!buffer || !aviao || size < 5) return;
    
    char tipo_char = (aviao->tipo == VOO_DOMESTICO) ? 'D' : 'I';
    snprintf(buffer, size, "%c%02d", tipo_char, aviao->id);
}

void format_flight_id_dot(char* buffer, size_t size, Aviao* aviao) {
    if (!buffer || !aviao || size < 7) return;
    
    char tipo_char = (aviao->tipo == VOO_DOMESTICO) ? 'D' : 'I';
    snprintf(buffer, size, "%c%02d · ", tipo_char, aviao->id);
}

int get_flight_color_pair(Aviao* aviao) {
    if (!aviao) return PAIR_DEFAULT;
    
    return (aviao->tipo == VOO_DOMESTICO) ? PAIR_DOM : PAIR_INTL;
}

void format_elapsed_time(char* buffer, size_t size, time_t inicio) {
    if (!buffer || size < 8) return;
    
    int tempo_decorrido = (inicio > 0) ? difftime(time(NULL), inicio) : 0;
    int minutos = tempo_decorrido / 60;
    int segundos = tempo_decorrido % 60;
    snprintf(buffer, size, "[%02d:%02d]", minutos, segundos);
}

void format_elapsed_time_with_pause(char* buffer, size_t size, SimulacaoAeroporto* sim) {
    if (!buffer || size < 8 || !sim) return;
    
    int tempo_decorrido = (int)calcular_tempo_efetivo_simulacao(sim);
    int minutos = tempo_decorrido / 60;
    int segundos = tempo_decorrido % 60;
    snprintf(buffer, size, "[%02d:%02d]", minutos, segundos);
}

void format_resource_status(char* buffer, size_t size, bool has_runway, bool has_gate, bool has_tower) {
    if (!buffer || size < 8) return;
    
    snprintf(buffer, size, "%c %c %c", 
             has_runway ? 'P' : '-', 
             has_gate ? 'G' : '-', 
             has_tower ? 'T' : '-');
}

bool is_flight_active(Aviao* aviao) {
    return aviao && 
           aviao->id > 0 && 
           aviao->estado != FINALIZADO_SUCESSO && 
           aviao->estado < FALHA_DEADLOCK;
}

bool is_flight_waiting_for_landing(Aviao* aviao) {
    return aviao && aviao->estado == AGUARDANDO_POUSO;
}

bool is_flight_in_alert(Aviao* aviao, int tempo_espera, int threshold) {
    if (!aviao) return false;
    
    return tempo_espera > threshold && 
           aviao->estado != DECOLANDO && 
           aviao->estado != DESEMBARCANDO && 
           aviao->estado != POUSANDO;
}

bool flight_has_runway(SimulacaoAeroporto* sim, int aviao_id) {
    if (!sim || aviao_id <= 0) return false;
    
    for (int i = 0; i < sim->recursos.total_pistas; i++) {
        if (sim->recursos.pista_ocupada_por[i] == aviao_id) {
            return true;
        }
    }
    return false;
}

bool flight_has_gate(SimulacaoAeroporto* sim, int aviao_id) {
    if (!sim || aviao_id <= 0) return false;
    
    for (int i = 0; i < sim->recursos.total_portoes; i++) {
        if (sim->recursos.portao_ocupado_por[i] == aviao_id) {
            return true;
        }
    }
    return false;
}

bool flight_has_tower(Aviao* aviao) {
    if (!aviao) return false;
    return (aviao->torre_alocada > 0);
    //return (aviao->estado == POUSANDO || aviao->estado == DESEMBARCANDO || aviao->estado == DECOLANDO);
}