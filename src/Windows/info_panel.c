#include "../../include/info_panel.h"
#include "../../include/terminal.h"

static void draw_alert_indicator(WINDOW* win, int linha) {
    if (!win) return;
    draw_window_text_full(win, linha, ALERT_POSITION, ALERT_TEXT, true, PAIR_ALERT);
}

static void draw_flight_line(WINDOW* win, int linha, FidsFlightInfo* info, const char* estado) {
    if (!win || !info || !estado) return;
    
    char flight_line[100];
    snprintf(flight_line, sizeof(flight_line), " %-*s| %-*s | %-*s | %*ds", 
              FLIGHT_ID_WIDTH, info->id_str,
              STATE_WIDTH, estado,
              RESOURCES_WIDTH, info->recursos_str,
              WAIT_TIME_WIDTH - 1, info->tempo_espera);
    
    draw_window_text(win, linha, 2, flight_line, info->color_pair);
    
    if (info->em_alerta) {
        draw_alert_indicator(win, linha);
    }
}

static int calculate_wait_time(Aviao* aviao) {
    if (!aviao || aviao->chegada_na_fila <= 0) return 0;
    
    time_t agora = time(NULL);
    return (int)difftime(agora, aviao->chegada_na_fila);
}

static void format_flight_resources(char* recursos_str, size_t size, SimulacaoAeroporto* sim, Aviao* aviao) {
    if (!recursos_str || !sim || !aviao || size < 8) return;
    
    bool tem_pista  = flight_has_runway(sim, aviao->id);
    bool tem_portao = flight_has_gate(sim, aviao->id);
    bool tem_torre  = flight_has_tower(aviao);
    
    snprintf(recursos_str, size, "%c %c %c", 
             tem_pista ? 'P' : '-', 
             tem_portao ? 'G' : '-', 
             tem_torre ? 'T' : '-');
}

static FidsFlightInfo collect_flight_info(SimulacaoAeroporto* sim, Aviao* aviao) {
    FidsFlightInfo info = {0};
    
    if (!sim || !aviao) return info;
    
    format_flight_id(info.id_str, sizeof(info.id_str), aviao);
    format_flight_resources(info.recursos_str, sizeof(info.recursos_str), sim, aviao);
    info.tempo_espera = calculate_wait_time(aviao);
    info.em_alerta = is_flight_in_alert(aviao, info.tempo_espera, ALERT_THRESHOLD);
    info.color_pair = get_flight_color_pair(aviao);
    
    return info;
}

static void draw_fids_flights(WINDOW* win, SimulacaoAeroporto* sim) {
    if (!win || !sim) return;
    
    int linha_atual = FIDS_HEADER_LINES;
    int max_linhas = getmaxy(win) - 1;
    
    for (int i = 0; i < sim->metricas.total_avioes_criados && linha_atual < max_linhas; i++) {
        Aviao* aviao = &sim->avioes[i];
        
        if (is_flight_active(aviao)) {
            FidsFlightInfo info = collect_flight_info(sim, aviao);
            draw_flight_line(win, linha_atual, &info, estado_para_str(aviao->estado));
            linha_atual++;
        }
    }
}

static bool validate_window_size(WINDOW* win) {
    if (!win) return false;
    
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    return (max_y >= 5 && max_x >= 50);
}

static void draw_insufficient_size_message(WINDOW* win) {
    if (!win) return;
    
    wclear(win);
    draw_window_text(win, 1, 1, "Tamanho insuficiente para o painel de informações.", PAIR_ALERT);
    finalize_window_display(win);
}

static void draw_fids_header(WINDOW* win) {
    if (!win) return;
    
    draw_window_title(win, "[INFORMATION PANEL]");
    draw_window_section_title_full(win, 1, " Voo | Estado             | Recursos | Espera", true);
    draw_horizontal_separator(win, 3);
}

void manage_info_panel(SimulacaoAeroporto* sim, WINDOW* info_win) {
    if (!sim || !info_win) return;

    if (!validate_window_size(info_win)) {
        draw_insufficient_size_message(info_win);
        return;
    }

    clear_and_box_window(info_win);
    draw_fids_header(info_win);
    draw_fids_flights(info_win, sim);
    finalize_window_display(info_win);
}