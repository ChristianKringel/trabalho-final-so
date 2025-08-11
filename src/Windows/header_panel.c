#include "../../include/header_panel.h"
#include "../../include/terminal.h"

static const char* get_status(SimulacaoAeroporto* sim) {
    if (!sim) return "ERRO";
    return sim->ativa ? (sim->pausado ? "PAUSADO" : "ATIVA") : "FINALIZANDO";
}

static int get_lane_status(SimulacaoAeroporto* sim) {
    if (!sim) return 0;
    return sim->recursos.total_pistas - sim->recursos.pistas_disponiveis;
}

static int get_gate_status(SimulacaoAeroporto* sim) {
    if (!sim) return 0;
    return sim->recursos.total_portoes - sim->recursos.portoes_disponiveis;
}

static int get_tower_status(SimulacaoAeroporto* sim) {
    if (!sim) return 0;
    return sim->recursos.operacoes_ativas_torre;
}

static void draw_header_content(SimulacaoAeroporto* sim, int voos_ativos, WINDOW* win) {
    if (!sim || !win) return;

    int pistas_ocupadas      = get_lane_status(sim);
    int portoes_ocupados     = get_gate_status(sim);
    int torres_ocupadas      = get_tower_status(sim);
    const char* status_sim   = get_status(sim);

    mvwprintw(win, 0, 1,
              "SIMULACAO TRAFEGO AEROPORTO: %-9s | Tempo: %03d/%ds | Voos: %-2d | Pistas: %d/%d | Portoes: %d/%d | Torres: %d/%d",
              status_sim,
              (int)difftime(time(NULL), sim->tempo_inicio),
              sim->tempo_simulacao, 
              voos_ativos, 
              pistas_ocupadas, sim->recursos.total_pistas, 
              portoes_ocupados, sim->recursos.total_portoes, 
              torres_ocupadas, sim->recursos.capacidade_torre);
    
    mvwprintw(win, 0, COLS - 25, "[P] Pausar | [Q] Sair");
}

void manage_header_panel(SimulacaoAeroporto* sim, int voos_ativos, WINDOW* header_win) {
    if (!sim || !header_win) return;
    
    wbkgd(header_win, COLOR_PAIR(PAIR_HEADER));
    wclear(header_win);
    
    draw_header_content(sim, voos_ativos, header_win);
    
    wrefresh(header_win);
}

