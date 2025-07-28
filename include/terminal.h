#include "libs.h"
#include "utils.h"

#ifndef TERMINAL_H
#define TERMINAL_H

void init_terminal_ncurses();
void close_terminal_ncurses();
void log_evento_ui(SimulacaoAeroporto* sim, Aviao* aviao, const char* formato, ...);
void update_terminal_display(SimulacaoAeroporto* sim);
const char* estado_para_str(EstadoAviao estado);
void init_windows();

#endif