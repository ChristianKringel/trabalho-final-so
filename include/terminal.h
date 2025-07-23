#ifndef TERMINAL_H
#define TERMINAL_H

#include <ncurses.h>

// Declare as janelas como variáveis globais (para simplicidade inicial)
extern WINDOW *main_win;
extern WINDOW *runway_win;
extern WINDOW *gate_win;
extern WINDOW *tower_win;
extern WINDOW *airplane_list_win;
extern WINDOW *metrics_win;
extern WINDOW *log_win;

// Funções básicas da API da UI
void init_terminal_ncurses();
// update_terminal_display vai precisar de acesso aos dados do simulador
// Por enquanto, vamos passar o que for necessário.
// No futuro, podemos encapsular em structs mais elaboradas.
void update_terminal_display(
    int num_runways, int* runway_ids, int* runway_occupancy, // Exemplo para pistas
    int num_gates, int* gate_ids, int* gate_occupancy,       // Exemplo para portões
    int tower_current_occupancy, int tower_max_occupancy,    // Exemplo para torre
    // ... adicione mais parâmetros para aviões, métricas, etc.
    // Ou comece com apenas alguns parâmetros e adicione mais conforme precisar
    const char* log_message // Para um log simples de linha única
);
void close_terminal_ncurses();

#endif // TERMINAL_H