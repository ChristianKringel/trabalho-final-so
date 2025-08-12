#ifndef WINDOW_UTILS_H
#define WINDOW_UTILS_H

#include "libs.h"
#include "utils.h"
#include "terminal.h"

void draw_window_title(WINDOW* win, const char* title);
void draw_window_title_full(WINDOW* win, const char* title, bool bold);

void draw_window_section_title(WINDOW* win, int linha, const char* title);
void draw_window_section_title_full(WINDOW* win, int linha, const char* title, bool bold);

void draw_window_text(WINDOW* win, int linha, int coluna, const char* text, int cor);
void draw_window_text_full(WINDOW* win, int linha, int coluna, const char* text, bool bold, int cor);

void draw_horizontal_separator(WINDOW* win, int linha);
void finalize_window_display(WINDOW* win);
void clear_and_box_window(WINDOW* win);

int get_waiting_airplanes(SimulacaoAeroporto* sim);

bool validate_window_params(WINDOW* win, void* data);
bool validate_window_bounds(WINDOW* win, int linha, int coluna);
bool is_window_usable(WINDOW* win);

void format_flight_id(char* buffer, size_t size, Aviao* aviao);
void format_flight_id_dot(char* buffer, size_t size, Aviao* aviao);
int get_flight_color_pair(Aviao* aviao);
void format_elapsed_time(char* buffer, size_t size, time_t inicio);
void format_elapsed_time_with_pause(char* buffer, size_t size, SimulacaoAeroporto* sim);
void format_resource_status(char* buffer, size_t size, bool has_runway, bool has_gate, bool has_tower);

bool is_flight_active(Aviao* aviao);
bool is_flight_waiting_for_landing(Aviao* aviao);
bool is_flight_in_alert(Aviao* aviao, int tempo_espera, int threshold);
bool flight_has_runway(SimulacaoAeroporto* sim, int aviao_id);
bool flight_has_gate(SimulacaoAeroporto* sim, int aviao_id);
bool flight_has_tower(Aviao* aviao);

#endif