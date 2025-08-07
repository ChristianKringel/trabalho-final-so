#include "../../include/status_panel.h"
#include "../../include/terminal.h"

#define LANE_SECTION_START 1
#define GATE_SECTION_START 7
#define TOWER_SECTION_START 15
#define LEGEND_SECTION_START 23

#define LANE_LINE 3
#define GATE_LINE 9
#define TOWER_LINE 17

static void draw_window_title(WINDOW* win, const char* title) {
    if (!win || !title) return;
    box(win, 0, 0);
    mvwprintw(win, 0, 2, "%s", title);
}

static void draw_window_section_tittle(WINDOW* win, int linha, const char* title) {
    if (!win || !title) return;
    mvwhline(win, linha, 2, ACS_HLINE, getmaxx(win) - 4);
    mvwprintw(win, linha + 1, 2, "[%s]", title);
}

static void draw_window_section_separator(WINDOW* win, int linha) {
    if (!win) return;
    mvwhline(win, linha, 2, ACS_HLINE, getmaxx(win) - 4);
}

// USAR VARIAVEL LOCAL AO INVES DE VARIAVEL GLOBAL
// USAR start_line variavel para definir a linha inicial
static void draw_lane_section(SimulacaoAeroporto* sim, WINDOW* win, int start_line) {
    if (!sim || !win) return;


    draw_window_section_tittle(win, LANE_SECTION_START, "PISTAS");
    
    for (int i = 0; i < sim->recursos.total_pistas; i++) {
        int aviao_id = sim->recursos.pista_ocupada_por[i];
        if (aviao_id != -1) {
            char id_str[5];
            char tipo_char = sim->avioes[aviao_id-1].tipo == VOO_DOMESTICO ? 'D' : 'I';
            snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_id);
            int color_pair = sim->avioes[aviao_id-1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
            wattron(win, COLOR_PAIR(color_pair));
            mvwprintw(win, LANE_LINE + 1 + i, 2, "P%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_id - 1].estado));
            wattroff(win, COLOR_PAIR(color_pair));
        } else {
            wattron(win, COLOR_PAIR(PAIR_SUCCESS));
            mvwprintw(win, LANE_LINE + 1 + i, 2, "P%d: [LIVRE]", i);
            wattroff(win, COLOR_PAIR(PAIR_SUCCESS));
        }
    }

    draw_window_section_separator(win, LANE_LINE + 1 + sim->recursos.total_pistas);
}

static void draw_gate_section(SimulacaoAeroporto* sim, WINDOW* win, int start_line) {
    if (!sim || !win) return;

    draw_window_section_tittle(win, GATE_SECTION_START, "PORTOES");
    
    for (int i = 0; i < sim->recursos.total_portoes; i++) {
        int aviao_id = sim->recursos.portao_ocupado_por[i];
        if (aviao_id != -1) {
            char id_str[5];
            char tipo_char = sim->avioes[aviao_id - 1].tipo == VOO_DOMESTICO ? 'D' : 'I';
            snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_id);
            int color_pair = sim->avioes[aviao_id - 1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
            wattron(win, COLOR_PAIR(color_pair));
            mvwprintw(win, GATE_LINE + 1 + i, 2, "G%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_id - 1].estado));
            wattroff(win, COLOR_PAIR(color_pair));
        } else {
            wattron(win, COLOR_PAIR(PAIR_SUCCESS));
            mvwprintw(win, GATE_LINE + 1 + i, 2, "G%d: [LIVRE]", i);
            wattroff(win, COLOR_PAIR(PAIR_SUCCESS));
        }
    }

    draw_window_section_separator(win, GATE_LINE + 1 + sim->recursos.total_portoes);
}

static void draw_tower_section(SimulacaoAeroporto* sim, WINDOW* win, int start_line) {
    if (!sim || !win) return;

    draw_window_section_tittle(win, TOWER_SECTION_START, "TORRES");
    
    for (int i = 0; i < sim->recursos.total_torres; i++) {
        bool torre_ocupada = false;
        int aviao_usando_torre = -1;
        
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
            
            wattron(win, COLOR_PAIR(color_pair));
            mvwprintw(win, TOWER_LINE + 1 + i, 2, "T%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_usando_torre - 1].estado));
            wattroff(win, COLOR_PAIR(color_pair));
        } else {
            wattron(win, COLOR_PAIR(PAIR_SUCCESS));
            mvwprintw(win, TOWER_LINE + 1 + i, 2, "T%d: [LIVRE]", i);
            wattroff(win, COLOR_PAIR(PAIR_SUCCESS));
        }
    }

    draw_window_section_separator(win, TOWER_LINE + 1 + sim->recursos.total_torres);
}

static void draw_legend_section(WINDOW* win, int start_line) {
    if (!win) return;

    mvwprintw(win, LEGEND_SECTION_START, 2, "[LEGENDA]");
    mvwprintw(win, LEGEND_SECTION_START + 1, 2, "DXX: Voo Domestico");
    mvwprintw(win, LEGEND_SECTION_START + 2, 2, "IXX: Voo Internacional");
    mvwprintw(win, LEGEND_SECTION_START + 3, 2, "P:   Pista");
    mvwprintw(win, LEGEND_SECTION_START + 4, 2, "G:   Portão");
    mvwprintw(win, LEGEND_SECTION_START + 5, 2, "T:   Torre");
    mvwprintw(win, LEGEND_SECTION_START + 7, 2, "PLACEHOLDER");
    wattron(win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
    mvwprintw(win, LEGEND_SECTION_START + 8, 2, "ALERTA = PLACEHOLDER");
    wattroff(win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
    wattron(win, COLOR_PAIR(PAIR_SUCCESS) | A_BOLD);
    mvwprintw(win, LEGEND_SECTION_START + 9, 2, "ALERTA = PLACEHOLDER");
    wattroff(win, COLOR_PAIR(PAIR_SUCCESS) | A_BOLD);
}

void manage_status_panel(SimulacaoAeroporto* sim, WINDOW* status_win){
    if (!sim || !status_win) return;

    wclear(status_win);
    draw_window_title(status_win, "[AIRPORT STATUS]");

    draw_lane_section(sim, status_win, LANE_SECTION_START);
    draw_gate_section(sim, status_win, GATE_SECTION_START);
    draw_tower_section(sim, status_win, TOWER_SECTION_START);

    draw_legend_section(status_win, LEGEND_SECTION_START);
    wrefresh(status_win);
}