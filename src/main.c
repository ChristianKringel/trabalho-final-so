#include "libs.h"

#include <stdio.h>
#include <unistd.h> // Para sleep ou usleep
#include "terminal.h" // Inclua sua interface ncurses

// Você precisará de variáveis para armazenar o estado do aeroporto
// Por enquanto, arrays simples para testar a UI
int runway_ids[] = {0, 1, 2};
int runway_occupancy[] = {-1, -1, -1}; // -1 para livre, ou ID do avião
int gate_ids[] = {0, 1, 2, 3, 4};
int gate_occupancy[] = {-1, -1, -1, -1, -1};
int tower_current = 0;
int tower_max = 2;

int main() {
    init_terminal_ncurses();

    // Loop de simulação (simplificado para testar a UI)
    for (int i = 0; i < 10; ++i) {
        // Simular alguma mudança de estado
        if (i % 2 == 0) {
            runway_occupancy[0] = 101; // Avião 101 ocupa pista 0
            gate_occupancy[2] = 203;   // Avião 203 ocupa portão 2
            tower_current = 1;         // Torre ocupada por 1
            update_terminal_display(
                3, runway_ids, runway_occupancy,
                5, gate_ids, gate_occupancy,
                tower_current, tower_max,
                "Avião 101 ocupou Pista 0."
            );
        } else {
            runway_occupancy[0] = -1; // Pista 0 livre
            gate_occupancy[2] = -1;   // Portão 2 livre
            tower_current = 0;         // Torre livre
            update_terminal_display(
                3, runway_ids, runway_occupancy,
                5, gate_ids, gate_occupancy,
                tower_current, tower_max,
                "Pista 0 e Portão 2 liberados."
            );
        }
        sleep(1); // Espera 1 segundo para visualização
    }

    close_terminal_ncurses();
    return 0;
}