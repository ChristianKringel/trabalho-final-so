#include "terminal.h"

static WINDOW *header_win, *airspace_win, *status_panel_win, *fids_win, *log_win;

// #define PAIR_DEFAULT 1
// #define PAIR_HEADER  2
// #define PAIR_INTL    3
// #define PAIR_DOM     4
// #define PAIR_ALERT   5
// #define PAIR_SUCCESS 6
// #define PAIR_WARNING 6 

// #define LOG_SUCCESS  PAIR_SUCCESS   
// #define LOG_ERROR    PAIR_ALERT     
// #define LOG_WARNING  PAIR_WARNING   
// #define LOG_INFO     PAIR_DEFAULT   
// #define LOG_SYSTEM   PAIR_HEADER 

#define HEADER_HEIGHT 1
#define AIRSPACE_HEIGHT 3
#define STATUS_WIDTH 28
#define LOG_WIDTH 96
#define FIDS_WIDTH (COLS - STATUS_WIDTH - LOG_WIDTH)
#define MAIN_HEIGHT (LINES - HEADER_HEIGHT - AIRSPACE_HEIGHT)

static void init_colors();
static void init_windows();
static void draw_header(SimulacaoAeroporto* sim, int voos_ativos);
static void draw_airspace_panel(SimulacaoAeroporto* sim);
static void draw_status_panel(SimulacaoAeroporto* sim);
static void draw_fids_panel(SimulacaoAeroporto* sim, int voos_ativos);


void init_terminal_ncurses() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    
    if (has_colors() == FALSE) {
        endwin();
        printf("Seu terminal nao suporta cores.\n");
        exit(1);
    }
    init_colors();

    if (LINES < 24 || COLS < 120) {
        endwin();
        printf("Terminal muito pequeno. Minimo recomendado: 120x24\n");
        exit(1);
    }

    // int header_height = 1;
    // int airspace_height = 2;
    // int status_width = 28;
    // int log_width = 96;
    // int fids_width = COLS - status_width - log_width;
    // int main_height = LINES - header_height - airspace_height;

    init_windows();

    clear();
    refresh();
}

void close_terminal_ncurses() {
    if (header_win)         delwin(header_win);
    if (airspace_win)       delwin(airspace_win);
    if (status_panel_win)   delwin(status_panel_win);
    if (fids_win)           delwin(fids_win);
    if (log_win)            delwin(log_win);
    endwin();
}

static void init_windows(){
    header_win = newwin(HEADER_HEIGHT, COLS, 0, 0);
    airspace_win = newwin(AIRSPACE_HEIGHT, COLS, HEADER_HEIGHT, 0);
    status_panel_win = newwin(MAIN_HEIGHT, STATUS_WIDTH, HEADER_HEIGHT + AIRSPACE_HEIGHT, 0);
    fids_win = newwin(MAIN_HEIGHT, FIDS_WIDTH, HEADER_HEIGHT + AIRSPACE_HEIGHT, STATUS_WIDTH);
    log_win = newwin(MAIN_HEIGHT, LOG_WIDTH, HEADER_HEIGHT + AIRSPACE_HEIGHT, STATUS_WIDTH + FIDS_WIDTH);
    scrollok(log_win, TRUE);
    wbkgd(log_win, COLOR_PAIR(PAIR_DEFAULT));
    box(log_win, 0, 0);
    mvwprintw(log_win, 0, 2, "[LOG]");
    wrefresh(log_win);
}

static void init_colors() {
    start_color();
    
    init_pair(PAIR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(PAIR_HEADER, COLOR_BLACK, COLOR_WHITE);
    init_pair(PAIR_DOM, COLOR_BLUE, COLOR_BLACK);        // Azul para voos domésticos
    init_pair(PAIR_INTL, COLOR_MAGENTA, COLOR_BLACK);    // Magenta para voos internacionais
    init_pair(PAIR_SUCCESS, COLOR_GREEN, COLOR_BLACK);   // Verde para sucessos
    init_pair(PAIR_ALERT, COLOR_RED, COLOR_BLACK);       // Vermelho para falhas
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);             // Amarelo para aguardando
}

void update_terminal_display(SimulacaoAeroporto* sim) {
    if (!sim) return;

    int voos_ativos = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado != FINALIZADO_SUCESSO && sim->avioes[i].estado < FALHA_DEADLOCK) {
            voos_ativos++;
        }
    }

    draw_header(sim, voos_ativos);
    draw_airspace_panel(sim);
    draw_status_panel(sim);
    draw_fids_panel(sim, voos_ativos);

    doupdate();
}

static void draw_header(SimulacaoAeroporto* sim, int voos_ativos) {
    wbkgd(header_win, COLOR_PAIR(PAIR_HEADER));
    wclear(header_win);
    
    int pistas_ocupadas = sim->recursos.total_pistas - sim->recursos.pistas_disponiveis;
    int portoes_ocupados = sim->recursos.total_portoes - sim->recursos.portoes_disponiveis;
    int torres_ocupadas = sim->recursos.total_torres - sim->recursos.torres_disponiveis;
    const char* status_sim = sim->ativa ? (sim->pausado ? "PAUSADO" : "ATIVA") : "FINALIZANDO";

    //ARRUMAR CONTAGEM DE TEMPO 
    mvwprintw(header_win, 0, 1, "SIMULACAO TRAFEGO AEROPORTO: %-9s | Tempo: %03d/%ds | Voos: %-2d | Pistas: %d/%d | Portoes: %d/%d | Torres: %d/%d",
              status_sim, (int)difftime(time(NULL), sim->tempo_inicio), sim->tempo_simulacao,
              voos_ativos, pistas_ocupadas, sim->recursos.total_pistas, portoes_ocupados, sim->recursos.total_portoes, torres_ocupadas, sim->recursos.total_torres);
              
    mvwprintw(header_win, 0, COLS - 25, "[P] Pausar | [Q] Sair");
    wrefresh(header_win);
}

static void draw_airspace_panel(SimulacaoAeroporto* sim) {
    wclear(airspace_win);
    
    box(airspace_win, 0, 0);
    mvwprintw(airspace_win, 0, 2, "[AIRSPACE QUEUE]");
    
    int col_atual = 2;

    int total_aguardando = 0;
    int avioes_mostrados = 0;
    for (int i = 0; i < sim->metricas.total_avioes_criados; i++) {
        if (sim->avioes[i].id > 0 && sim->avioes[i].estado == AGUARDANDO_POUSO) {
            total_aguardando++;
            
            char id_str[10];
            char tipo_char = sim->avioes[i].tipo == VOO_DOMESTICO ? 'D' : 'I';
            snprintf(id_str, sizeof(id_str), "%c%02d · ", tipo_char, sim->avioes[i].id);

            if (col_atual + strlen(id_str) < COLS - 15) {
                int color_pair = sim->avioes[i].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
                wattron(airspace_win, COLOR_PAIR(color_pair));
                mvwprintw(airspace_win, 1, col_atual, "%s", id_str);
                wattroff(airspace_win, COLOR_PAIR(color_pair));
                col_atual += strlen(id_str);
                avioes_mostrados++;
            }
        }
    }
    
    if (avioes_mostrados < total_aguardando) {
        mvwprintw(airspace_win, 1, col_atual, "... ");
    }
    mvwprintw(airspace_win, 0, 19, "(Total: %d)", total_aguardando);
    wrefresh(airspace_win);
}

static void draw_status_panel(SimulacaoAeroporto* sim) {
    wclear(status_panel_win);
    box(status_panel_win, 0, 0);
    mvwprintw(status_panel_win, 0, 2, "[AIRPORT STATUS]");
    
    // Seção Pistas
    mvwhline(status_panel_win, 1, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);
    mvwprintw(status_panel_win, 2, 2, "[PISTAS]");
    
    int linha_pistas = 3;
    for (int i = 0; i < sim->recursos.total_pistas; i++) {
        int aviao_id = sim->recursos.pista_ocupada_por[i];
        if (aviao_id != -1) {
            char id_str[5];
            char tipo_char = sim->avioes[aviao_id-1].tipo == VOO_DOMESTICO ? 'D' : 'I';
            snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_id);
            int color_pair = sim->avioes[aviao_id-1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
            wattron(status_panel_win, COLOR_PAIR(color_pair));
            mvwprintw(status_panel_win, linha_pistas + i, 2, "P%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_id - 1].estado));
            wattroff(status_panel_win, COLOR_PAIR(color_pair));
        } else {
            wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
            mvwprintw(status_panel_win, linha_pistas + i, 2, "P%d: [LIVRE]", i);
            wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
        }
    }

    mvwhline(status_panel_win, linha_pistas + sim->recursos.total_pistas, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);

    // Seção Portões
    int linha_portoes = linha_pistas + sim->recursos.total_pistas + 1;
    mvwprintw(status_panel_win, linha_portoes, 2, "[PORTOES]");
    for (int i = 0; i < sim->recursos.total_portoes; i++) {
        int aviao_id = sim->recursos.portao_ocupado_por[i];
        if (aviao_id != -1) {
            char id_str[5];
            char tipo_char = sim->avioes[aviao_id - 1].tipo == VOO_DOMESTICO ? 'D' : 'I';
            snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_id);
            int color_pair = sim->avioes[aviao_id - 1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
            wattron(status_panel_win, COLOR_PAIR(color_pair));
            mvwprintw(status_panel_win, linha_portoes + 1 + i, 2, "G%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_id - 1].estado));
            wattroff(status_panel_win, COLOR_PAIR(color_pair));
        } else {
            wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
            mvwprintw(status_panel_win, linha_portoes + 1 + i, 2, "G%d: [LIVRE]", i);
            wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
        }
    }

    mvwhline(status_panel_win, linha_portoes + 1 + sim->recursos.total_portoes, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);

    // Seção Torres
    int linha_torres = linha_portoes + 1 + sim->recursos.total_portoes + 1;
    mvwprintw(status_panel_win, linha_torres, 2, "[TORRES]");

    int torres_ocupadas = sim->recursos.total_torres - sim->recursos.torres_disponiveis;
    
    for (int i = 0; i < sim->recursos.total_torres; i++) {
        // Verificar se algum avião está usando uma torre
        bool torre_ocupada = false;
        int aviao_usando_torre = -1;
        
        // Procurar por aviões que estão em estados que usam torre
        for (int j = 0; j < sim->metricas.total_avioes_criados; j++) {
            if (sim->avioes[j].id > 0 && sim->avioes[j].torre_alocada && 
                (sim->avioes[j].estado == POUSANDO || sim->avioes[j].estado == DESEMBARCANDO || sim->avioes[j].estado == DECOLANDO)) {
                if (!torre_ocupada) { 
                    torre_ocupada = true;
                    aviao_usando_torre = sim->avioes[j].id;
                    break;
                }
            }
        }
        
        if (torre_ocupada && aviao_usando_torre != -1) {
            char id_str[5];
            char tipo_char = sim->avioes[aviao_usando_torre-1].tipo == VOO_DOMESTICO ? 'D' : 'I';
            snprintf(id_str, sizeof(id_str), "%c%02d", tipo_char, aviao_usando_torre);
            int color_pair = sim->avioes[aviao_usando_torre-1].tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            
            wattron(status_panel_win, COLOR_PAIR(color_pair));
            mvwprintw(status_panel_win, linha_torres + 1 + i, 2, "T%d: [%s] %-14s", i, id_str, estado_para_str(sim->avioes[aviao_usando_torre - 1].estado));
            wattroff(status_panel_win, COLOR_PAIR(color_pair));
        } else {
            wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
            mvwprintw(status_panel_win, linha_torres + 1 + i, 2, "T%d: [LIVRE]", i);
            wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS));
        }
    }

    mvwhline(status_panel_win, linha_torres + 1 + sim->recursos.total_torres, 2, ACS_HLINE, getmaxx(status_panel_win) - 4);

    // Secao Legendas
    int linha_legendas = linha_torres + 1 + sim->recursos.total_torres + 1;
    mvwprintw(status_panel_win, linha_legendas, 2, "[LEGENDA]");
    mvwprintw(status_panel_win, linha_legendas + 1, 2, "DXX: Voo Domestico");
    mvwprintw(status_panel_win, linha_legendas + 2, 2, "IXX: Voo Internacional");
    mvwprintw(status_panel_win, linha_legendas + 3, 2, "P:   Pista");
    mvwprintw(status_panel_win, linha_legendas + 4, 2, "G:   Portão");
    mvwprintw(status_panel_win, linha_legendas + 5, 2, "T:   Torre");
    mvwprintw(status_panel_win, linha_legendas + 7, 2, "PLACEHOLDER");
    wattron(status_panel_win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
    mvwprintw(status_panel_win, linha_legendas + 8, 2, "ALERTA = PLACEHOLDER");
    wattroff(status_panel_win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
    wattron(status_panel_win, COLOR_PAIR(PAIR_SUCCESS) | A_BOLD);
    mvwprintw(status_panel_win, linha_legendas + 9, 2, "ALERTA = PLACEHOLDER");
    wattroff(status_panel_win, COLOR_PAIR(PAIR_SUCCESS) | A_BOLD);

    wrefresh(status_panel_win);
    
}

static void draw_fids_panel(SimulacaoAeroporto* sim, int voos_ativos) {
    wclear(fids_win);
    box(fids_win, 0, 0);
    mvwprintw(fids_win, 0, 2, "[FLIGHT INFORMATION DISPLAY SYSTEM]");
    
    wattron(fids_win, A_BOLD);
    mvwhline(fids_win, 1, 2, ACS_HLINE, getmaxx(fids_win) - 4);
    mvwprintw(fids_win, 2, 2, " Voo | Estado             | Recursos | Espera");
    wattroff(fids_win, A_BOLD);
    mvwhline(fids_win, 3, 2, ACS_HLINE, getmaxx(fids_win) - 4);

    int linha_atual = 4;
    int max_linhas = getmaxy(fids_win) - 1;

    for (int i = 0; i < sim->metricas.total_avioes_criados && linha_atual < max_linhas; i++) {
        Aviao* aviao = &sim->avioes[i];
        if (aviao->id > 0 && aviao->estado != FINALIZADO_SUCESSO && aviao->estado < FALHA_DEADLOCK) {
            int color_pair = aviao->tipo == VOO_DOMESTICO ? PAIR_DOM : PAIR_INTL;
            wattron(fids_win, COLOR_PAIR(color_pair));

            char id_str[5];
            snprintf(id_str, sizeof(id_str), "%c%02d", aviao->tipo == VOO_DOMESTICO ? 'D' : 'I', aviao->id);
            
            char recursos_str[8];
            bool tem_pista = false; for(int p=0; p < sim->recursos.total_pistas; ++p) if(sim->recursos.pista_ocupada_por[p] == aviao->id) tem_pista = true;
            bool tem_portao = false; for(int g=0; g < sim->recursos.total_portoes; ++g) if(sim->recursos.portao_ocupado_por[g] == aviao->id) tem_portao = true;
            bool tem_torre = (aviao->estado == POUSANDO || aviao->estado == DESEMBARCANDO || aviao->estado == DECOLANDO);
            //SNPRINTF(FORMATA A STRING PRA SEGUIR UM PADRAO, SEMPRE - - - OU AS VARIAVIES P G T)
            snprintf(recursos_str, sizeof(recursos_str), "%c %c %c", tem_pista ? 'P' : '-', tem_portao ? 'G' : '-', tem_torre ? 'T' : '-');

            time_t agora = time(NULL);
            int espera = (aviao->chegada_na_fila > 0) ? difftime(agora, aviao->chegada_na_fila) : 0;
            
            mvwprintw(fids_win, linha_atual, 2, " %-4s| %-18s | %-8s | %3ds", 
                id_str, estado_para_str(aviao->estado), recursos_str, espera);

            if (espera > 60 && aviao->estado != DECOLANDO && aviao->estado != DESEMBARCANDO && aviao->estado != POUSANDO) {
                wattron(fids_win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
                mvwprintw(fids_win, linha_atual, 47, "[ALERTA]");
                wattroff(fids_win, COLOR_PAIR(PAIR_ALERT) | A_BOLD);
            }

            wattroff(fids_win, COLOR_PAIR(color_pair));
            linha_atual++;
        }
    }
    wrefresh(fids_win);
}

void log_evento_ui(SimulacaoAeroporto* sim, Aviao* aviao, int cor, const char* formato, ...) {
    if (!sim || !log_win || !formato) return;
    char buffer[256];
    va_list args;
    va_start(args, formato);
    vsnprintf(buffer, sizeof(buffer), formato, args);
    va_end(args);
    
    pthread_mutex_lock(&sim->mutex_ui_log);
    if (log_win) {
        wscrl(log_win, 1);

        int y = getmaxy(log_win) - 2;
        int x = 1;
        
        // Formato de tempo: T:[mm:ss]
        int tempo_decorrido = difftime(time(NULL), sim->tempo_inicio);
        int minutos = tempo_decorrido / 60;
        int segundos = tempo_decorrido % 60;
        mvwprintw(log_win, y, x, "[%02d:%02d] ", minutos, segundos);
        x += 9;
        
        if (aviao != NULL) {
            //IDENTIFICA COR PELO TIPO DO VOO
            int flight_color = (aviao->tipo == VOO_DOMESTICO) ? PAIR_DOM : PAIR_INTL;

            //DEFINE PREFIXO
            char prefix[5];
            snprintf(prefix, sizeof(prefix), " %c%02d", (aviao->tipo == VOO_DOMESTICO) ? 'D' : 'I', aviao->id);
            
            //ESCREVE PREFIXO COM COR CORRETA
            wattron(log_win, COLOR_PAIR(flight_color) | A_BOLD);
            mvwprintw(log_win, y, x, "%s", prefix);
            wattroff(log_win, COLOR_PAIR(flight_color) | A_BOLD);
            x += strlen(prefix);
            
            mvwprintw(log_win, y, x, "  ");
            x += 2;
            
            // //DEFINE COR
            // int status_color = PAIR_DEFAULT;
            // if (strstr(buffer, "Obteve") || strstr(buffer, "obteve") || 
            //     strstr(buffer, "concluído") || strstr(buffer, "liberou") ||
            //     strstr(buffer, "CRIADO") || strstr(buffer, "Pousando") ||
            //     strstr(buffer, "Desembarcando") || strstr(buffer, "Decolando") ||
            //     strstr(buffer, "completou")) {
            //     status_color = PAIR_SUCCESS; 
            // } else if (strstr(buffer, "FALHA") || strstr(buffer, "falha") ||
            //           strstr(buffer, "Erro") || strstr(buffer, "erro") ||
            //           strstr(buffer, "ABORTOU")) {
            //     status_color = PAIR_ALERT; 
            // } else if (strstr(buffer, "Solicitando") || strstr(buffer, "Aguard") ||
            //           strstr(buffer, "Espera") || strstr(buffer, "aguardando") ||
            //           strstr(buffer, "Está")) {
            //     status_color = PAIR_WARNING; 
            // }
            
            //ESCREVE A MENSAGEM COM A COR DEFINIDA
            // wattron(log_win, COLOR_PAIR(status_color));
            // mvwprintw(log_win, y, x, "%s", buffer);
            // wattroff(log_win, COLOR_PAIR(status_color));

            //ESCREVE A MENSAGEM COM A COR DEFINIDA
            wattron(log_win, COLOR_PAIR(cor));
            mvwprintw(log_win, y, x, "%s", buffer);
            wattroff(log_win, COLOR_PAIR(cor));

        } else {
            // Mensagens do sistema
            wattron(log_win, COLOR_PAIR(PAIR_SUCCESS) | A_BOLD);
            mvwprintw(log_win, y, x + 1, "[SYSTEM]");
            wattroff(log_win, COLOR_PAIR(PAIR_SUCCESS) | A_BOLD);
            x += 8;
            
            //mvwprintw(log_win, y, x, "  %s", buffer);
            wattron(log_win, COLOR_PAIR(cor));
            mvwprintw(log_win, y, x, "  %s", buffer);
            wattroff(log_win, COLOR_PAIR(cor));
        }
        
        wclrtoeol(log_win);
        box(log_win, 0, 0);
        mvwprintw(log_win, 0, 2, "[LOG]");
        wrefresh(log_win);
    }
    pthread_mutex_unlock(&sim->mutex_ui_log);
}



