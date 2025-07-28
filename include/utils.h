#include "libs.h"
// =============== FUNÇÕES AUXILIARES ===============
//const char* estado_para_string(EstadoAviao estado);
//const char* tipo_voo_para_string(TipoVoo tipo);
//double calcular_tempo_decorrido(time_t inicio, time_t fim);
void dormir_operacao(int min_ms, int max_ms);
void dormir_operacao_com_pausa(SimulacaoAeroporto* sim, int min_ms, int max_ms);
int gerar_numero_aleatorio(int min, int max);
void init_terminal_ncurses();
void close_terminal_ncurses();
void update_terminal_display(SimulacaoAeroporto* sim);
void log_evento_ui(SimulacaoAeroporto* sim, const char* formato, ...);


// =============== FUNÇÕES DE MONITORAMENTO ===============
void* monitor_starvation(void* arg);
int verificar_starvation(Aviao* aviao, time_t tempo_atual);
int detectar_deadlock(SimulacaoAeroporto* sim);
void atualizar_estado_aviao(Aviao* aviao, EstadoAviao novo_estado);

// =============== FUNÇÕES DE LIMPEZA ===============
void finalizar_simulacao(SimulacaoAeroporto* sim);
void destruir_recursos(RecursosAeroporto* recursos);
void liberar_memoria(SimulacaoAeroporto* sim);

// =============== FUNÇÕES DE RELATÓRIO ===============
void imprimir_status_operacao(int id_aviao, TipoVoo tipo, const char* operacao, const char* status);
void imprimir_status_recursos(RecursosAeroporto* recursos);
void gerar_relatorio_final(SimulacaoAeroporto* sim);
void imprimir_resumo_aviao(Aviao* aviao);

// // =============== FUNÇÕES DE SIMULAÇÃO PRINCIPAL ===============
// void iniciar_simulacao(SimulacaoAeroporto* sim);
// void parar_simulacao(SimulacaoAeroporto* sim);
// int simulacao_ativa(SimulacaoAeroporto* sim);
