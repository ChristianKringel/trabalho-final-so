#include "libs.h"

static void init_colors();
static void draw_header(SimulacaoAeroporto* sim, int voos_ativos);
static void draw_airspace_panel(SimulacaoAeroporto* sim);
static void draw_status_panel(SimulacaoAeroporto* sim);
static void draw_fids_panel(SimulacaoAeroporto* sim, int voos_ativos);
const char* estado_para_str(EstadoAviao estado);
void initialize_windows();