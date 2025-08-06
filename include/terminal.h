#include "libs.h"
#include "utils.h"
#include "logger.h"

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

void update_terminal_display(SimulacaoAeroporto* sim);

void log_evento_ui(SimulacaoAeroporto* sim, Aviao* aviao, int cor, const char* formato, ...);
static bool validate_log_params(SimulacaoAeroporto* sim, const char* formato);
static void format_log_message(char* buffer, size_t size, const char* formato, va_list args);
static void display_log_in_window(SimulacaoAeroporto* sim, Aviao* aviao, const char* buffer, int cor);
static int draw_timestamp(WINDOW* win, int y, SimulacaoAeroporto* sim);
static int draw_flight_prefix(WINDOW* win, int y, int x, Aviao* aviao);
static int draw_system_prefix(WINDOW* win, int y, int x);
static void draw_message_with_color(WINDOW* win, int y, int x, const char* buffer, int cor);
static void finalize_log_display(WINDOW* win);


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

#define HEADER_HEIGHT 1
#define AIRSPACE_HEIGHT 3
#define STATUS_WIDTH 28
#define LOG_WIDTH 96
#define FIDS_WIDTH (COLS - STATUS_WIDTH - LOG_WIDTH)
#define MAIN_HEIGHT (LINES - HEADER_HEIGHT - AIRSPACE_HEIGHT)

#endif