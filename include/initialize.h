#include "libs.h"

// =============== FUNÇÕES DE INICIALIZAÇÃO ===============
SimulacaoAeroporto* inicializar_simulacao(int pistas, int portoes, int torres, int tempo_simulacao, int max_avioes);
void inicializar_recursos(RecursosAeroporto* recursos, int pistas, int portoes, int torres);
void inicializar_metricas(MetricasSimulacao* metricas);
