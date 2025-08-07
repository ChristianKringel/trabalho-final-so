#include "../../include/status_panel.h"
#include "../../include/terminal.h"

#define LANE_SECTION_START 1
#define GATE_SECTION_START 3
#define TOWER_SECTION_START 5

#define LANE_LINE 3
#define GATE_LINE 5
#define TOWER_LINE 7

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


static void draw_pistas_section(SimulacaoAeroporto* sim, WINDOW* win, int start_line) {
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

//for (int i = 0; i < sim->recursos.total_pistas; i++) {
//         int aviao_id = sim->recursos.pista_ocupada_por[i];
//         if (aviao_id != -1) {
//             char id_str[5];
//             char tipo_char = sim->avioes[aviao_id-1].tipo == VOO_DOMESTICO ? 'D' : 'I';
//             snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_id);
//             int color_pair = sim->avioes[aviao_id-1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
//             wattron(status_panel_win, COLOR_PAIR(color_pair));
//             mvwprintw(status_panel_win, linha_pistas + i, 2, "P%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_id - 1].estado));
//             wattroff(status_panel_win, COLOR_PAIR(color_pair));
//         } else {
//             wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
//             mvwprintw(status_panel_win, linha_pistas + i, 2, "P%d: [LIVRE]", i);
//             wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
//         }
//     }

static void draw_portoes_section(SimulacaoAeroporto* sim, WINDOW* win, int start_line) {
    // Lógica para desenhar a seção de portões  
}

static void draw_torres_section(SimulacaoAeroporto* sim, WINDOW* win, int start_line) {
    // Lógica para desenhar a seção de torres
}

void manage_status_panel(SimulacaoAeroporto* sim, int voos_ativos, WINDOW* status_win){
    if (!sim || !status_win) return;

    wclear(status_win);
    draw_window_title(status_win, "[AIRPORT STATUS]");

    draw_pistas_section(sim, status_win, LANE_SECTION_START);
    //draw_portoes_section(sim, status_win, GATE_SECTION_START);
    //draw_torres_section(sim, status_win, TOWER_SECTION_START);
    
    wrefresh(status_win);
}