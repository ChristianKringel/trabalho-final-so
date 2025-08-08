#ifndef WINDOW_UTILS_H
#define WINDOW_UTILS_H

#include "libs.h"
#include "utils.h"
#include "terminal.h"

void draw_window_title(WINDOW* win, const char* title);
void draw_window_title_full(WINDOW* win, const char* title, bool bold);

void draw_window_section_title(WINDOW* win, int linha, const char* title);
void draw_window_section_title_full(WINDOW* win, int linha, const char* title, bool bold);

void draw_horizontal_separator(WINDOW* win, int linha);
void finalize_window_display(WINDOW* win);
void clear_and_box_window(WINDOW* win);

// bool validate_window_params(WINDOW* win, void* data);
// bool validate_window_bounds(WINDOW* win, int linha, int coluna);
// bool is_window_usable(WINDOW* win);

// void format_flight_id(char* buffer, size_t size, Aviao* aviao);
// int get_flight_color_pair(Aviao* aviao);
// void format_elapsed_time(char* buffer, size_t size, time_t inicio);
// void format_resource_status(char* buffer, size_t size, bool has_runway, bool has_gate, bool has_tower);

#endif