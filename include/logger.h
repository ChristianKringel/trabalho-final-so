#include "libs.h"
#ifndef LOGGER_H
#define LOGGER_H

static void format_file_timestamp(char* timestamp, size_t size);
void append_to_log_file(SimulacaoAeroporto* sim, Aviao* aviao, const char* buffer); 
void create_log_file(); 

#endif