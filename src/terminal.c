// terminal.c
#include "libs.h" // Ou simulador.h


// As janelas agora são 'static' para serem visíveis apenas neste arquivo.
// Isso é uma boa prática para evitar poluir o namespace global.
static WINDOW *runway_win, *gate_win, *tower_win, *airplane_list_win, *log_win, *header_win, *airspace_win;

void init_terminal_ncurses() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    
    // ISSO AQUI VIRA UMA FUNCAO DEPOis
    if (LINES < 24 || COLS < 120) {
        endwin();
        printf("Terminal muito pequeno. Mínimo recomendado: 120x24\n");
        exit(1);
    }

    // Define a largura dos painéis. O visual ocupa 65% do espaço.
    int visual_panel_width = (COLS * 65) / 100;
    int fids_panel_width = COLS - visual_panel_width;

    // --- Criação das Janelas ---
    int header_height = 3;
    int log_height = 5;
    int main_panel_height = LINES - header_height - log_height;
    header_win = newwin(header_height, COLS, 0, 0);
    log_win = newwin(log_height, COLS, LINES - log_height, 0);
    visual_win = newwin(main_panel_height, visual_panel_width, header_height, 0);
    fids_win = newwin(main_panel_height, fids_panel_width, header_height, visual_panel_width);

    
    // ESSAS DEFINICOES PODEM SER GLOBAIS
    // int header_height = 3;
    // int log_height = 5;
    // int remaining_height = LINES - header_height - log_height;
    // int col_width = COLS / 3;
    
    // // Transformar em uma funcao tbm
    // if (remaining_height < 6) remaining_height = 6;
    // if (col_width < 20) col_width = 20;
    
    // // Criar janelas com verificação de erro
    // header_win = newwin(header_height, COLS, 0, 0);
    // if (!header_win) { endwin(); printf("Erro criando header_win\n"); exit(1); }
    
    // // Janelas principais em colunas
    // runway_win = newwin(remaining_height, col_width, header_height, 0);
    // if (!runway_win) { endwin(); printf("Erro criando runway_win\n"); exit(1); }
    
    // gate_win = newwin(remaining_height, col_width, header_height, col_width);
    // if (!gate_win) { endwin(); printf("Erro criando gate_win\n"); exit(1); }
    
    // tower_win = newwin(remaining_height / 2, col_width, header_height, col_width * 2);
    // if (!tower_win) { endwin(); printf("Erro criando tower_win\n"); exit(1); }
    
    // airplane_list_win = newwin(remaining_height / 2, col_width, header_height + remaining_height / 2, col_width * 2);
    // if (!airplane_list_win) { endwin(); printf("Erro criando airplane_list_win\n"); exit(1); }
    
    // // Janela de log na parte inferior
    // log_win = newwin(log_height, COLS, LINES - log_height, 0);
    // if (!log_win) { endwin(); printf("Erro criando log_win\n"); exit(1); }

    // airspace_win = newwin(remaining_height, col_width, header_height, col_width * 3);
    // if (!airspace_win) { endwin(); printf("Erro criando airspace_win\n"); exit(1); }
    
    if (!header_win || !log_win || !visual_win || !fids_win) {
        endwin();
        printf("Erro ao criar uma das janelas principais.\n");
        exit(1);
    }

    scrollok(log_win, TRUE); // Habilita rolagem para o log
    clear();
    refresh();
    
    // Limpar tela e atualizar
    clear();
    refresh();
}

void close_terminal_ncurses() {
    // Destruir todas as janelas criadas
    // if (header_win) delwin(header_win);
    // if (runway_win) delwin(runway_win);
    // if (gate_win) delwin(gate_win);
    // if (tower_win) delwin(tower_win);
    // if (airplane_list_win) delwin(airplane_list_win);
    // if (log_win) delwin(log_win);
    // if (airspace_win) delwin(airspace_win);
    if (header_win) delwin(header_win);
    if (log_win) delwin(log_win);
    if (visual_win) delwin(visual_win);
    if (fids_win) delwin(fids_win);
    
    endwin();
    printf("Terminal Ncurses Encerrado.\n");
}

// Função para converter o estado do avião em uma string legível
const char* estado_para_str(EstadoAviao estado) {
    switch (estado) {
        case AGUARDANDO_POUSO: return "Aguardando Pouso";
        case POUSANDO: return "Pousando";
        case AGUARDANDO_DESEMBARQUE: return "Aguard. Desembarque";
        case DESEMBARCANDO: return "Desembarcando";
        case AGUARDANDO_DECOLAGEM: return "Aguard. Decolagem";
        case DECOLANDO: return "Decolando";
        case FINALIZADO_SUCESSO: return "Finalizado";
        default: return "Falha";
    }
}

// A nova função de atualização, muito mais limpa e poderosa
void update_terminal_display(SimulacaoAeroporto* sim) {
    if (!sim) return;
    
    if (!sim) return;

    // --- Contagem de Voos Ativos ---
    int voos_ativos = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado != FINALIZADO_SUCESSO && sim->avioes[i].estado < FALHA_DEADLOCK) {
            voos_ativos++;
        }
    }

    // ---- Cabeçalho (Header) ----
    wclear(header_win);
    box(header_win, 0, 0);
    time_t tempo_atual = time(NULL);
    int tempo_decorrido = difftime(tempo_atual, sim->tempo_inicio);
    // Adicionamos o modo da UI como placeholder!
    const char* modo_ui_str = (sim->modo_ui == UI_TEXTO) ? "TEXTO" : "VISUAL";
    mvwprintw(header_win, 1, 2, "SIMULACAO DE TRAFEGO AEREO | Tempo: %d/%d s | Voos Ativos: %d | UI: %s", 
              tempo_decorrido, sim->tempo_simulacao, voos_ativos, modo_ui_str);
    wrefresh(header_win);

    // ---- Painel Visual (versão texto) ----
    wclear(visual_win);
    box(visual_win, 0, 0);
    int linha_atual = 1;

    // Seção ESPAÇO AÉREO
    mvwprintw(visual_win, linha_atual++, 2, "[ESPACO AEREO (AGUARDANDO POUSO)]");
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado == AGUARDANDO_POUSO) {
            mvwprintw(visual_win, linha_atual++, 3, "Aviao %d", sim->avioes[i].id);
        }
    }

    // Seção PISTAS
    linha_atual++;
    mvwprintw(visual_win, linha_atual++, 2, "[PISTAS (%d/%d)]", sim->recursos.total_pistas - sim->recursos.pistas_disponiveis, sim->recursos.total_pistas);
    for (int i = 0; i < sim->recursos.total_pistas; i++) {
        int aviao_id = sim->recursos.pista_ocupada_por[i];
        if (aviao_id != -1) {
            mvwprintw(visual_win, linha_atual++, 3, "Pista %d: [OCUPADA] por Aviao %d", i, aviao_id);
        } else {
            mvwprintw(visual_win, linha_atual++, 3, "Pista %d: [LIVRE]", i);
        }
    }

    // Seção PORTÕES
    linha_atual++;
    mvwprintw(visual_win, linha_atual++, 2, "[PORTOES (%d/%d)]", sim->recursos.total_portoes - sim->recursos.portoes_disponiveis, sim->recursos.total_portoes);
    for (int i = 0; i < sim->recursos.total_portoes; i++) {
        int aviao_id = sim->recursos.portao_ocupado_por[i];
        if (aviao_id != -1) {
            mvwprintw(visual_win, linha_atual++, 3, "Portao %d: [OCUPADO] por Aviao %d", i, aviao_id);
        } else {
            mvwprintw(visual_win, linha_atual++, 3, "Portao %d: [LIVRE]", i);
        }
    }
    wrefresh(visual_win);

    // ---- Painel de Informações de Voo (FIDS) ----
    wclear(fids_win);
    box(fids_win, 0, 0);
    mvwprintw(fids_win, 1, 2, "[FLIGHT INFORMATION]");
    mvwprintw(fids_win, 2, 2, "ID | TIPO | ESTADO            | RECURSOS");
    mvwprintw(fids_win, 3, 2, "---|------|-------------------|---------");
    
    int linha_fids = 4;
    int max_linhas_fids = getmaxy(fids_win) - 2;

    for (int i = 0; i < sim->metricas.total_avioes_criados && linha_fids < max_linhas_fids; i++) {
        Aviao* aviao = &sim->avioes[i];
        if (aviao->id > 0 && aviao->estado != FINALIZADO_SUCESSO && aviao->estado < FALHA_DEADLOCK) {
            char recursos_str[20] = ""; 
            // Lógica para preencher os recursos
            bool tem_pista = false;
            for(int p=0; p < sim->recursos.total_pistas; ++p) if(sim->recursos.pista_ocupada_por[p] == aviao->id) tem_pista = true;
            
            bool tem_portao = false;
            for(int g=0; g < sim->recursos.total_portoes; ++g) if(sim->recursos.portao_ocupado_por[g] == aviao->id) tem_portao = true;

            // Inferimos o uso da torre pelo estado do avião
            bool tem_torre = (aviao->estado == POUSANDO || aviao->estado == DESEMBARCANDO || aviao->estado == DECOLANDO);
            
            sprintf(recursos_str, "%s%s%s", tem_pista ? "P " : "", tem_portao ? "G " : "", tem_torre ? "T" : "");

            mvwprintw(fids_win, linha_fids++, 2, "%-3d| %-4s | %-17s | %s",
                aviao->id,
                aviao->tipo == VOO_DOMESTICO ? "DOM" : "INTL",
                estado_para_str(aviao->estado),
                recursos_str);
        }
    }
    wrefresh(fids_win);

    // ---- Log de Eventos ----
    // A sua função log_evento_ui já lida com a atualização e o refresh do log_win,
    // então não precisamos mexer nela aqui. Apenas redesenhamos a borda caso outro painel sobreponha.
    box(log_win, 0, 0);
    wrefresh(log_win);

    // Renderiza todas as atualizações na tela de uma vez
    doupdate();
}

// Função de log segura para threads
void log_evento_ui(SimulacaoAeroporto* sim, const char* formato, ...) {
    if (!sim || !log_win || !formato) return;
    
    char buffer[256];
    va_list args;
    va_start(args, formato);
    vsnprintf(buffer, sizeof(buffer), formato, args);
    va_end(args);

    // Trava o mutex específico para o log antes de interagir com a janela
    pthread_mutex_lock(&sim->mutex_ui_log);
    
    // Verificar se a janela ainda existe
    if (log_win) {
        wscrl(log_win, 1); // Rola o conteúdo da janela de log
        mvwprintw(log_win, getmaxy(log_win) - 2, 1, ">> %s", buffer);
        box(log_win, 0, 0); // Redesenha a borda
        wrefresh(log_win);
    }
    
    pthread_mutex_unlock(&sim->mutex_ui_log);
}