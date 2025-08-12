#include "../../include/info_panel.h"
#include "../../include/terminal.h"

static void draw_alert_indicator(WINDOW* win, int linha) {
    if (!win) return;
    draw_window_text_full(win, linha, ALERT_POSITION, ALERT_TEXT, true, PAIR_ALERT);
}

static void draw_flight_line(WINDOW* win, int linha, FlightInfo* info, const char* estado) {
    if (!win || !info || !estado) return;
    
    char flight_line[100];
    snprintf(flight_line, sizeof(flight_line), " %-*s| %-*s | %-*s | %*ds | %*d", 
              FLIGHT_ID_WIDTH, info->id_str,
              STATE_WIDTH, estado,
              RESOURCES_WIDTH, info->recursos_str,
              WAIT_TIME_WIDTH - 1, info->tempo_espera,
              PRIORITY_WIDTH - 1, info->prioridade_dinamica);
    
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

static int calculate_wait_time_with_pause(Aviao* aviao, SimulacaoAeroporto* sim) {
    if (!aviao || !sim || aviao->chegada_na_fila <= 0) return 0;
    
    time_t agora = time(NULL);
    double tempo_total = difftime(agora, aviao->chegada_na_fila);
    
    // Desconta o tempo pausado desde que o avião entrou na fila
    double tempo_pausa_descontar = 0.0;
    
    // Se estamos pausados agora, inclui o tempo da pausa atual
    if (sim->pausado && sim->inicio_pausa > 0) {
        if (sim->inicio_pausa >= aviao->chegada_na_fila) {
            tempo_pausa_descontar += difftime(agora, sim->inicio_pausa);
        } else {
            tempo_pausa_descontar += difftime(agora, sim->inicio_pausa);
        }
    }
    
    // Desconta o tempo total já pausado (aproximação - consideramos que o avião estava presente durante todas as pausas)
    tempo_pausa_descontar += sim->tempo_pausado_total;
    
    int tempo_efetivo = (int)(tempo_total - tempo_pausa_descontar);
    return tempo_efetivo > 0 ? tempo_efetivo : 0;
}

static void format_flight_resources(char* recursos_str, size_t size, SimulacaoAeroporto* sim, Aviao* aviao) {
    if (!recursos_str || !sim || !aviao || size < 8) return;
    
    bool tem_pista  = flight_has_runway(sim, aviao->id);
    bool tem_portao = flight_has_gate(sim, aviao->id);
    bool tem_torre  = flight_has_tower(aviao);
    
    format_resource_status(recursos_str, size, tem_pista, tem_portao, tem_torre);
}

static FlightInfo collect_flight_info(SimulacaoAeroporto* sim, Aviao* aviao) {
    FlightInfo info = {0};
    
    if (!sim || !aviao) return info;
    
    format_flight_id(info.id_str, sizeof(info.id_str), aviao);
    format_flight_resources(info.recursos_str, sizeof(info.recursos_str), sim, aviao);
    info.tempo_espera = calculate_wait_time_with_pause(aviao, sim);
    info.em_alerta = is_flight_in_alert(aviao, info.tempo_espera, ALERT_THRESHOLD);
    info.color_pair = get_flight_color_pair(aviao);
    info.prioridade_dinamica = aviao->prioridade_dinamica;
    
    return info;
}

static void draw_filghts_info(WINDOW* win, SimulacaoAeroporto* sim) {
    if (!win || !sim) return;
    
    int linha_atual = FIDS_HEADER_LINES;
    int max_linhas = getmaxy(win) - 1;
    
    for (int i = 0; i < sim->metricas.total_avioes_criados && linha_atual < max_linhas; i++) {
        Aviao* aviao = &sim->avioes[i];
        
        if (is_flight_active(aviao)) {
            FlightInfo info = collect_flight_info(sim, aviao);
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

static void draw_info_header(WINDOW* win) {
    if (!win) return;
    
    draw_window_title(win, "[INFORMATION PANEL]");
    draw_window_section_title_full(win, 1, " Voo | Estado             | Recursos | Espera | Prioridade", true);
    draw_horizontal_separator(win, 3);
}

void manage_info_panel(SimulacaoAeroporto* sim, WINDOW* info_win) {
    if (!sim || !info_win) return;

    if (!validate_window_size(info_win)) {
        draw_insufficient_size_message(info_win);
        return;
    }

    clear_and_box_window(info_win);
    draw_info_header(info_win);
    draw_filghts_info(info_win, sim);
    finalize_window_display(info_win);
}