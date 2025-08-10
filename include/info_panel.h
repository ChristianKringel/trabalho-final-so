#ifndef INFO_PANEL_H
#define INFO_PANEL_H

#include "libs.h"
#include "utils.h"
#include "window_utils.h"

#define FIDS_HEADER_LINES 4
#define FLIGHT_ID_WIDTH 4
#define STATE_WIDTH 18
#define RESOURCES_WIDTH 8
#define WAIT_TIME_WIDTH 5
#define PRIORITY_WIDTH 5
#define ALERT_THRESHOLD 60
#define ALERT_TEXT "[ALERTA]"
#define ALERT_POSITION 47

typedef struct {
    char id_str[6];
    char recursos_str[10];
    bool em_alerta;
    int tempo_espera;
    int color_pair;
    int prioridade_dinamica;
} FlightInfo;

void manage_info_panel(SimulacaoAeroporto* sim, WINDOW* info_win);


#endif