#ifndef LOG_PANEL_H
#define LOG_PANEL_H

#include "libs.h"
#include "utils.h"

#define LOG_TIMESTAMP_WIDTH 8
#define LOG_PREFIX_WIDTH 6
#define LOG_MARGIN 2
#define LOG_MESSAGE_OFFSET 16

typedef struct {
    char timestamp[16];
    char prefix[10];
    int prefix_color;
    bool is_system_message;
    int message_color;
} LogLineInfo;

void manage_log_panel(SimulacaoAeroporto* sim, Aviao* aviao, const char* message, int cor, WINDOW* log_win);


#endif