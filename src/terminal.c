#include "terminal.h"
#include <string.h> // Para memset, etc.

// Definição das janelas (globais, visíveis apenas neste .c)
WINDOW *main_win;
WINDOW *runway_win;
WINDOW *gate_win;
WINDOW *tower_win;
WINDOW *airplane_list_win;
WINDOW *metrics_win;
WINDOW *log_win; // Janela para mensagens de log
WINDOW *header_win; // Janela para o cabeçalho (opcional, se quiser um título ou status geral)

void init_terminal_ncurses() {
    main_win = initscr(); 
    if (main_win == NULL) {
        // Trate o erro, talvez imprima para stderr e saia
        return;
    }
    noecho();             // Não exibe caracteres digitados
    cbreak();             // Leitura imediata de caracteres (Ctrl+C funciona)
    keypad(stdscr, TRUE); // Habilita teclas especiais (setas, F-keys)
    curs_set(0);          // Esconde o cursor
    start_color();        // Habilita cores (se quiser usar)
    // init_pair(1, COLOR_GREEN, COLOR_BLACK); // Exemplo de cor para livre
    // init_pair(2, COLOR_RED, COLOR_BLACK);   // Exemplo de cor para ocupado

    // --- Criação e Layout Básico das Janelas ---
    // Você vai ajustar esses tamanhos e posições conforme sua tela e o que deseja mostrar
    int current_y = 0;

    // Cabeçalho (ex: tempo de simulação, status geral)
    header_win = newwin(3, COLS, current_y, 0); box(header_win, 0, 0); wrefresh(header_win); current_y += 3;

    // Pistas (ex: 3 pistas, 1 linha por pista + título + bordas)
    runway_win = newwin(5, COLS / 2, current_y, 0);
    box(runway_win, 0, 0);
    mvwprintw(runway_win, 1, 1, "Pistas:");
    wrefresh(runway_win);
    current_y += 5;

    // Portões (ex: 5 portões)
    gate_win = newwin(7, COLS / 2, current_y, 0);
    box(gate_win, 0, 0);
    mvwprintw(gate_win, 1, 1, "Portões:");
    wrefresh(gate_win);
    current_y += 7;

    // Torre de Controle
    tower_win = newwin(5, COLS / 2, current_y, 0);
    box(tower_win, 0, 0);
    mvwprintw(tower_win, 1, 1, "Torre de Controle:");
    wrefresh(tower_win);
    current_y += 5;

    // Lista de Aviões (ocupará a maior parte da direita)
    airplane_list_win = newwin(LINES - (current_y + 3), COLS / 2, header_win ? header_win->_maxy + 1 : 0, COLS / 2); // Ajuste a posição Y e largura
    box(airplane_list_win, 0, 0);
    mvwprintw(airplane_list_win, 1, 1, "Aviões Ativos:");
    wrefresh(airplane_list_win);

    // Log de eventos na parte inferior
    log_win = newwin(3, COLS, LINES - 3, 0); // Últimas 3 linhas
    box(log_win, 0, 0);
    scrollok(log_win, TRUE); // Habilita rolagem para o log
    wrefresh(log_win);

    refresh(); // Atualiza a tela principal (stdscr) após criar as janelas
}

void update_terminal_display(
    int num_runways, int* runway_ids, int* runway_occupancy,
    int num_gates, int* gate_ids, int* gate_occupancy,
    int tower_current_occupancy, int tower_max_occupancy,
    const char* log_message // Log simples, uma mensagem por vez
) {
    // --- Pistas ---
    wclear(runway_win);
    box(runway_win, 0, 0);
    mvwprintw(runway_win, 1, 1, "Pistas:");
    for (int i = 0; i < num_runways; i++) {
        mvwprintw(runway_win, 2 + i, 1, "Pista %d: %s (Avião: %d)",
                  runway_ids[i],
                  runway_occupancy[i] == -1 ? "Livre" : "Ocupada",
                  runway_occupancy[i]); // -1 se livre
    }
    wrefresh(runway_win);

    // --- Portões ---
    wclear(gate_win);
    box(gate_win, 0, 0);
    mvwprintw(gate_win, 1, 1, "Portões:");
    for (int i = 0; i < num_gates; i++) {
        mvwprintw(gate_win, 2 + i, 1, "Portão %d: %s (Avião: %d)",
                  gate_ids[i],
                  gate_occupancy[i] == -1 ? "Livre" : "Ocupada",
                  gate_occupancy[i]); // -1 se livre
    }
    wrefresh(gate_win);

    // --- Torre de Controle ---
    wclear(tower_win);
    box(tower_win, 0, 0);
    mvwprintw(tower_win, 1, 1, "Torre de Controle:");
    mvwprintw(tower_win, 2, 1, "Ocupação: %d/%d", tower_current_occupancy, tower_max_occupancy);
    wrefresh(tower_win);

    // --- Log de Eventos Simples ---
    if (log_message != NULL && strlen(log_message) > 0) {
        wscrl(log_win, 1); // Rola o conteúdo para cima
        mvwprintw(log_win, getmaxy(log_win) - 2, 1, "%s", log_message); // Adiciona na penúltima linha
    }
    wrefresh(log_win);


    doupdate(); // Atualiza todas as janelas virtuais para a tela física
}

void close_terminal_ncurses() {
    // Destruir as janelas
    delwin(runway_win);
    delwin(gate_win);
    delwin(tower_win);
    delwin(airplane_list_win);
    // delwin(metrics_win); // Se existirem
    delwin(log_win);
    delwin(main_win); // stdscr, geralmente não é necessário, endwin() faz isso.

    endwin(); // Finaliza ncurses
    printf("Terminal Ncurses Encerrado.\n");
}