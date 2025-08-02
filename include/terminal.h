#include "libs.h"
#include "utils.h"

#ifndef TERMINAL_H
#define TERMINAL_H

static void init_colors();
static void draw_header(SimulacaoAeroporto* sim, int voos_ativos);
static void draw_airspace_panel(SimulacaoAeroporto* sim);
static void draw_status_panel(SimulacaoAeroporto* sim);
static void draw_fids_panel(SimulacaoAeroporto* sim, int voos_ativos);
const char* estado_para_str(EstadoAviao estado);
void initialize_windows();

#define PAIR_DEFAULT 1
#define PAIR_HEADER  2
#define PAIR_INTL    3
#define PAIR_DOM     4
#define PAIR_ALERT   5
#define PAIR_SUCCESS 6
#define PAIR_WARNING 6 

#define LOG_SUCCESS  PAIR_SUCCESS   
#define LOG_ERROR    PAIR_ALERT     
#define LOG_WARNING  PAIR_WARNING   
#define LOG_INFO     PAIR_DEFAULT   
#define LOG_SYSTEM   PAIR_HEADER 

#endif