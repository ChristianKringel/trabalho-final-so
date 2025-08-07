#include "../../include/log_panel.h"
#include "../../include/terminal.h"

static bool validate_log_params(SimulacaoAeroporto* sim, const char* formato) {
    return sim && formato;
}

static void format_log_message(char* buffer, size_t size, const char* formato, va_list args) {
    if (size == 0 || !buffer || !formato) return;

    vsnprintf(buffer, size - 1, formato, args);
    buffer[size - 1] = '\0';
}

static int calculate_max_message_length(WINDOW* win) {
    if (!win) return 0;
    
    int max_x = getmaxx(win);
    return max_x - LOG_MESSAGE_OFFSET - LOG_MARGIN;
}

static bool validate_log_window(WINDOW* win) {
    if (!win) return false;
    
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    return (max_y > 2 && max_x > 2);
}

static void finalize_log_display(WINDOW* win) {
    if (!win) return;
    
    wclrtoeol(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, "[LOG]");
    wrefresh(win);
}

static void scroll_log_window(WINDOW* win) {
    if (!win) return;
    
    wscrl(win, 1);
}

static void draw_log_message(WINDOW* win, int y, int x, const char* message, int color) {
    if (!win || !message) return;
    
    int max_len = calculate_max_message_length(win) - x;
    
    wattron(win, COLOR_PAIR(color));
    if (max_len > 0) {
        mvwprintw(win, y, x, "%.*s", max_len, message);
    }
    wattroff(win, COLOR_PAIR(color));
}

static void draw_log_prefix(WINDOW* win, int y, int x, LogLineInfo* info) {
    if (!win || !info) return;
    
    wattron(win, COLOR_PAIR(info->prefix_color) | A_BOLD);
    mvwprintw(win, y, x, "%s", info->prefix);
    wattroff(win, COLOR_PAIR(info->prefix_color) | A_BOLD);
    
    mvwprintw(win, y, x + strlen(info->prefix), "  ");
}

static void draw_log_timestamp(WINDOW* win, int y, const char* timestamp) {
    if (!win || !timestamp) return;
    
    mvwprintw(win, y, 1, "%s ", timestamp);
}

static void draw_log_line(WINDOW* win, int y, LogLineInfo* info, const char* message) {
    if (!win || !info || !message) return;
    
    int x = LOG_MARGIN;
    
    draw_log_timestamp(win, y, info->timestamp);
    x += LOG_TIMESTAMP_WIDTH + 1;

    draw_log_prefix(win, y, x, info);
    x += LOG_PREFIX_WIDTH + 2;

    draw_log_message(win, y, x, message, info->message_color);
}

static void format_log_prefix(char* prefix, size_t size, Aviao* aviao, int* prefix_color) {
    if (!prefix || !prefix_color || size < 8) return;
    
    if (aviao && aviao->id > 0) {
        char tipo_char = (aviao->tipo == VOO_DOMESTICO) ? 'D' : 'I';
        snprintf(prefix, size, " %c%02d", tipo_char, aviao->id);
        *prefix_color = (aviao->tipo == VOO_DOMESTICO) ? PAIR_DOM : PAIR_INTL;
    } else {
        snprintf(prefix, size, "[SYS]");
        *prefix_color = LOG_SYSTEM;
    }
}

static void format_log_timestamp(char* timestamp, size_t size, SimulacaoAeroporto* sim) {
    if (!timestamp || size < 10) return;
    
    int tempo_decorrido = 0;
    if (sim && sim->tempo_inicio > 0) {
        tempo_decorrido = (int)difftime(time(NULL), sim->tempo_inicio);
    }
    
    int minutos = tempo_decorrido / 60;
    int segundos = tempo_decorrido % 60;
    snprintf(timestamp, size, "[%02d:%02d]", minutos, segundos);
}

static LogLineInfo create_log_line_info(SimulacaoAeroporto* sim, Aviao* aviao, int cor) {
    LogLineInfo info = {0};
    
    format_log_timestamp(info.timestamp, sizeof(info.timestamp), sim);
    format_log_prefix(info.prefix, sizeof(info.prefix), aviao, &info.prefix_color);
    info.is_system_message = (aviao == NULL || aviao->id <= 0);
    info.message_color = cor;
    
    return info;
}

void manage_log_panel(SimulacaoAeroporto* sim, Aviao* aviao, const char* message, int cor, WINDOW* log_win) {
    if (!validate_log_window(log_win) || !message) return;
    
    LogLineInfo info = create_log_line_info(sim, aviao, cor);
    
    scroll_log_window(log_win);
    
    int max_y = getmaxy(log_win);
    int y = max_y - 2;
    if (y < 1) y = 1;
    
    draw_log_line(log_win, y, &info, message);
    finalize_log_display(log_win);
}