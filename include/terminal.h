#include "libs.h"
#include "utils.h"

#ifndef TERMINAL_H
#define TERMINAL_H

static void init_colors();
static void init_windows();
static void draw_header(SimulacaoAeroporto* sim, int voos_ativos);
static void draw_airspace_panel(SimulacaoAeroporto* sim);
static void draw_status_panel(SimulacaoAeroporto* sim);
static void draw_fids_panel(SimulacaoAeroporto* sim, int voos_ativos);
const char* estado_para_str(EstadoAviao estado);
void init_terminal_ncurses();
void close_terminal_ncurses();
void log_evento_ui(SimulacaoAeroporto* sim, Aviao* aviao, int cor, const char* formato, ...);
void update_terminal_display(SimulacaoAeroporto* sim);

#define PAIR_DEFAULT 1
#define PAIR_HEADER  2
#define PAIR_INTL    3
#define PAIR_DOM     4
#define PAIR_ALERT   5
#define PAIR_SUCCESS 6
#define PAIR_WARNING 7
#define PAIR_INFO    8
#define PAIR_SYSTEM  9
#define PAIR_RESOURCE 10
#define PAIR_TIMING  11
#define PAIR_DEBUG   12

#define LOG_SUCCESS   PAIR_SUCCESS   
#define LOG_ERROR     PAIR_ALERT     
#define LOG_WARNING   PAIR_WARNING   
#define LOG_INFO      PAIR_INFO      
#define LOG_SYSTEM    PAIR_SYSTEM    
#define LOG_RESOURCE  PAIR_RESOURCE  
#define LOG_TIMING    PAIR_TIMING    
#define LOG_DEBUG     PAIR_DEBUG 

#endif