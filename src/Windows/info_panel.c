#include "../../include/info_panel.h"
#include "../../include/terminal.h"

static void finalize_fids_display(WINDOW* win) {
    if (!win) return;
    
    wrefresh(win);
}

static void draw_alert_indicator(WINDOW* win, int linha) {
    if (!win) return;
    
    wattron(win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
    mvwprintw(win, linha, ALERT_POSITION, ALERT_TEXT);
    wattroff(win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
}

static void draw_flight_line(WINDOW* win, int linha, FidsFlightInfo* info, const char* estado) {
    if (!win || !info || !estado) return;
    
    wattron(win, COLOR_PAIR(info->color_pair));
    
    mvwprintw(win, linha, 2, " %-*s| %-*s | %-*s | %*ds", 
              FLIGHT_ID_WIDTH, info->id_str,
              STATE_WIDTH, estado,
              RESOURCES_WIDTH, info->recursos_str,
              WAIT_TIME_WIDTH - 1, info->tempo_espera);
    
    wattroff(win, COLOR_PAIR(info->color_pair));
    
    if (info->em_alerta) {
        draw_alert_indicator(win, linha);
    }
}

static bool is_flight_in_alert(Aviao* aviao, int tempo_espera) {
    if (!aviao) return false;
    
    return tempo_espera > ALERT_THRESHOLD && 
           aviao->estado != DECOLANDO && 
           aviao->estado != DESEMBARCANDO && 
           aviao->estado != POUSANDO;
}

static int calculate_wait_time(Aviao* aviao) {
    if (!aviao || aviao->chegada_na_fila <= 0) return 0;
    
    time_t agora = time(NULL);
    return (int)difftime(agora, aviao->chegada_na_fila);
}

static bool check_flight_has_tower(Aviao* aviao) {
    if (!aviao) return false;
    
    return (aviao->estado == POUSANDO || 
            aviao->estado == DESEMBARCANDO || 
            aviao->estado == DECOLANDO);
}

static bool check_flight_has_gate(SimulacaoAeroporto* sim, int aviao_id) {
    if (!sim || aviao_id <= 0) return false;
    
    for (int g = 0; g < sim->recursos.total_portoes; g++) {
        if (sim->recursos.portao_ocupado_por[g] == aviao_id) {
            return true;
        }
    }
    return false;
}

static bool check_flight_has_runway(SimulacaoAeroporto* sim, int aviao_id) {
    if (!sim || aviao_id <= 0) return false;
    
    for (int p = 0; p < sim->recursos.total_pistas; p++) {
        if (sim->recursos.pista_ocupada_por[p] == aviao_id) {
            return true;
        }
    }
    return false;
}

static void format_flight_resources(char* recursos_str, size_t size, SimulacaoAeroporto* sim, Aviao* aviao) {
    if (!recursos_str || !sim || !aviao || size < 8) return;
    
    bool tem_pista = check_flight_has_runway(sim, aviao->id);
    bool tem_portao = check_flight_has_gate(sim, aviao->id);
    bool tem_torre = check_flight_has_tower(aviao);
    
    snprintf(recursos_str, size, "%c %c %c", 
             tem_pista ? 'P' : '-', 
             tem_portao ? 'G' : '-', 
             tem_torre ? 'T' : '-');
}

static void format_flight_id(char* id_str, size_t size, Aviao* aviao) {
    if (!id_str || !aviao || size < 5) return;
    
    char tipo_char = (aviao->tipo == VOO_DOMESTICO) ? 'D' : 'I';
    snprintf(id_str, size, "%c%02d", tipo_char, aviao->id);
}

static FidsFlightInfo collect_flight_info(SimulacaoAeroporto* sim, Aviao* aviao) {
    FidsFlightInfo info = {0};
    
    if (!sim || !aviao) return info;
    
    format_flight_id(info.id_str, sizeof(info.id_str), aviao);
    format_flight_resources(info.recursos_str, sizeof(info.recursos_str), sim, aviao);
    info.tempo_espera = calculate_wait_time(aviao);
    info.em_alerta = is_flight_in_alert(aviao, info.tempo_espera);
    info.color_pair = (aviao->tipo == VOO_DOMESTICO) ? PAIR_DOM : PAIR_INTL;
    
    return info;
}

static bool is_flight_displayable(Aviao* aviao) {
    return aviao && 
           aviao->id > 0 && 
           aviao->estado != FINALIZADO_SUCESSO && 
           aviao->estado < FALHA_DEADLOCK;
}

static void draw_fids_flights(WINDOW* win, SimulacaoAeroporto* sim) {
    if (!win || !sim) return;
    
    int linha_atual = FIDS_HEADER_LINES;
    int max_linhas = getmaxy(win) - 1;
    
    for (int i = 0; i < sim->metricas.total_avioes_criados && linha_atual < max_linhas; i++) {
        Aviao* aviao = &sim->avioes[i];
        
        if (is_flight_displayable(aviao)) {
            FidsFlightInfo info = collect_flight_info(sim, aviao);
            draw_flight_line(win, linha_atual, &info, estado_para_str(aviao->estado));
            linha_atual++;
        }
    }
}

static void draw_flight_info_tittle(WINDOW* win) {
    if (!win) return;
    wclear(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, "[FLIGHT INFORMATION DISPLAY SYSTEM]");
}

static void draw_flight_info_headers(WINDOW* win) {
    if (!win) return;
    wattron(win, A_BOLD);
    mvwhline(win, 1, 2, ACS_HLINE, getmaxx(win) - 4);
    mvwprintw(win, 2, 2, " Voo | Estado             | Recursos | Espera");
    wattroff(win, A_BOLD);
    mvwhline(win, 3, 2, ACS_HLINE, getmaxx(win) - 4);
}



void manage_info_panel(SimulacaoAeroporto* sim, int voos_ativos, WINDOW* info_win) {
    if (!sim || !info_win) return;

    int max_y, max_x;
    getmaxyx(info_win, max_y, max_x);
    if (max_y < 5 || max_x < 50) {
        wclear(info_win);
        mvwprintw(info_win, 1, 1, "Tamanho insuficiente para o painel de informações.");
        wrefresh(info_win);
        return;
    }

    draw_flight_info_tittle(info_win);
    draw_flight_info_headers(info_win);
    draw_fids_flights(info_win, sim);
    finalize_fids_display(info_win);
}