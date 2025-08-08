#include "../../include/status_panel.h"
#include "../../include/terminal.h"

#define LANE_SECTION_START 1
#define GATE_SECTION_START 7
#define TOWER_SECTION_START 15
#define LEGEND_SECTION_START 23

#define LANE_LINE 3
#define GATE_LINE 9
#define TOWER_LINE 17

static void draw_resource_line_occupied(WINDOW* win, int line, const char* prefix, int index, Aviao* aviao) {
    if (!win || !prefix || !aviao) return;
    
    char id[10];
    format_flight_id(id, sizeof(id), aviao);
    int color_pair = get_flight_color_pair(aviao);
    
    char full_line[50];
    snprintf(full_line, sizeof(full_line), "%s%d: [%s] %-14s", prefix, index, id, estado_para_str(aviao->estado));
    
    draw_window_text(win, line, 2, full_line, color_pair);
}

static void draw_resource_line_free(WINDOW* win, int line, const char* prefix, int index) {
    if (!win || !prefix) return;
    
    char free_line[20];
    snprintf(free_line, sizeof(free_line), "%s%d: [LIVRE]", prefix, index);
    draw_window_text(win, line, 2, free_line, PAIR_SUCCESS);
}

static int find_tower_using_airplane(SimulacaoAeroporto* sim) {
    if (!sim) return -1;
    
    for (int j = 0; j < sim->metricas.total_avioes_criados; j++) {
        if (sim->avioes[j].id > 0 && sim->avioes[j].torre_alocada && 
            (sim->avioes[j].estado == POUSANDO || sim->avioes[j].estado == DESEMBARCANDO || sim->avioes[j].estado == DECOLANDO)) {
            return sim->avioes[j].id;
        }
    }
    return -1;
}

static void draw_lane_section(SimulacaoAeroporto* sim, WINDOW* win) {
    if (!sim || !win) return;

    draw_window_section_title(win, LANE_SECTION_START, "[PISTAS]");
    
    for (int i = 0; i < sim->recursos.total_pistas; i++) {
        int aviao_id = sim->recursos.pista_ocupada_por[i];
        int current_line = LANE_LINE + 1 + i;
        
        if (aviao_id != -1) {
            draw_resource_line_occupied(win, current_line, "P", i, &sim->avioes[aviao_id - 1]);
        } else {
            draw_resource_line_free(win, current_line, "P", i);
        }
    }
    draw_horizontal_separator(win, LANE_LINE + 1 + sim->recursos.total_pistas);
}

static void draw_gate_section(SimulacaoAeroporto* sim, WINDOW* win) {
    if (!sim || !win) return;

    draw_window_section_title(win, GATE_SECTION_START, "PORTOES");
    
    for (int i = 0; i < sim->recursos.total_portoes; i++) {
        int aviao_id = sim->recursos.portao_ocupado_por[i];
        int current_line = GATE_LINE + 1 + i;
        
        if (aviao_id != -1) {
            draw_resource_line_occupied(win, current_line, "G", i, &sim->avioes[aviao_id - 1]);
        } else {
            draw_resource_line_free(win, current_line, "G", i);
        } 
    }
    draw_horizontal_separator(win, GATE_LINE + 1 + sim->recursos.total_portoes);
}

static void draw_tower_section(SimulacaoAeroporto* sim, WINDOW* win) {
    if (!sim || !win) return;

    draw_window_section_title(win, TOWER_SECTION_START, "TORRES");
    
    for (int i = 0; i < sim->recursos.total_torres; i++) {
        int current_line = TOWER_LINE + 1 + i;
        int aviao_usando_torre = find_tower_using_airplane(sim);
        
        if (aviao_usando_torre != -1) {
            Aviao* aviao = &sim->avioes[aviao_usando_torre - 1];
            char id[10];
            format_flight_id(id, sizeof(id), aviao);
            int color_pair = get_flight_color_pair(aviao);
            
            char full_line[50];
            snprintf(full_line, sizeof(full_line), "T%d: [%s] %-14s", i, id, estado_para_str(aviao->estado));
            
            draw_window_text(win, current_line, 2, full_line, color_pair);
        } else {
            draw_resource_line_free(win, current_line, "T", i);
        }
    }

    draw_horizontal_separator(win, TOWER_LINE + 1 + sim->recursos.total_torres);
}

static void draw_legend_section(WINDOW* win, int start_line) {
    if (!win) return;

    draw_window_section_title(win, LEGEND_SECTION_START, "[LEGENDA]");
    
    draw_window_text(win, LEGEND_SECTION_START + 1, 2, "DXX: Voo Domestico", PAIR_DEFAULT);
    draw_window_text(win, LEGEND_SECTION_START + 2, 2, "IXX: Voo Internacional", PAIR_DEFAULT);
    draw_window_text(win, LEGEND_SECTION_START + 3, 2, "P:   Pista", PAIR_DEFAULT);
    draw_window_text(win, LEGEND_SECTION_START + 4, 2, "G:   Portão", PAIR_DEFAULT);
    draw_window_text(win, LEGEND_SECTION_START + 5, 2, "T:   Torre", PAIR_DEFAULT);
    
    draw_window_text(win, LEGEND_SECTION_START + 7, 2, "PLACEHOLDER", PAIR_DEFAULT);
    draw_window_text_full(win, LEGEND_SECTION_START + 8, 2, "ALERTA = PLACEHOLDER", true, PAIR_ALERT);
    draw_window_text_full(win, LEGEND_SECTION_START + 9, 2, "SUCESSO = PLACEHOLDER", true, PAIR_SUCCESS);
}

void manage_status_panel(SimulacaoAeroporto* sim, WINDOW* status_win){
    if (!sim || !status_win) return;

    clear_and_box_window(status_win);
    draw_window_title(status_win, "[AIRPORT STATUS]");

    draw_lane_section(sim, status_win);
    draw_gate_section(sim, status_win);
    draw_tower_section(sim, status_win);
    draw_legend_section(status_win, LEGEND_SECTION_START);
    
    finalize_window_display(status_win);
}