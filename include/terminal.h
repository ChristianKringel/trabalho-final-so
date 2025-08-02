#include "libs.h"
#include "utils.h"

#ifndef TERMINAL_H
#define TERMINAL_H

void init_terminal_ncurses();
void close_terminal_ncurses();
void log_evento_ui(SimulacaoAeroporto* sim, Aviao* aviao, int cor, const char* formato, ...);
void update_terminal_display(SimulacaoAeroporto* sim);
const char* estado_para_str(EstadoAviao estado);

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