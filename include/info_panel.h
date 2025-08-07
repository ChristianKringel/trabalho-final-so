#ifndef INFO_PANEL_H
#define INFO_PANEL_H

#include "libs.h"
#include "utils.h"

#define FIDS_HEADER_LINES 4
#define FLIGHT_ID_WIDTH 4
#define STATE_WIDTH 18
#define RESOURCES_WIDTH 8
#define WAIT_TIME_WIDTH 5
#define ALERT_THRESHOLD 60
#define ALERT_TEXT "[ALERTA]"
#define ALERT_POSITION 47

typedef struct {
    char id_str[6];
    char recursos_str[10];
    int tempo_espera;
    bool em_alerta;
    int color_pair;
} FidsFlightInfo;

void manage_info_panel(SimulacaoAeroporto* sim, int voos_ativos, WINDOW* fids_win);


#endif