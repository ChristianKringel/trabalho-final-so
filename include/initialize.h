#include "libs.h"
#ifndef INITIALIZE_H
#define INITIALIZE_H
// =============== FUNÇÕES DE INICIALIZAÇÃO ===============
SimulacaoAeroporto* inicializar_simulacao(int pistas, int portoes, int torres, int tempo_simulacao, int max_avioes);
void inicializar_recursos(RecursosAeroporto* recursos, int pistas, int portoes, int torres);
void inicializar_metricas(MetricasSimulacao* metricas);
void inicializar_banqueiro(Banqueiro* banco, int pistas, int portoes, int torres);
void inicializar_pistas(RecursosAeroporto* recursos, int pistas);
void inicializar_portoes(RecursosAeroporto* recursos, int portoes);
void inicializar_torre(RecursosAeroporto* recursos, int capacidade);
#endif
