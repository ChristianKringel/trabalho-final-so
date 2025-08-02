#include "libs.h"
#ifndef METRICS_H
#define METRICS_H

void incrementar_metrica_pouso(MetricasSimulacao* metricas);
void incrementar_metrica_desembarque(MetricasSimulacao* metricas);
void incrementar_metrica_decolagem(MetricasSimulacao* metricas);
void incrementar_aviao_sucesso(MetricasSimulacao* metricas);
void incrementar_aviao_starvation(MetricasSimulacao* metricas);
void incrementar_aviao_deadlock(MetricasSimulacao* metricas);
void incrementar_contador_avioes_criados(MetricasSimulacao* metricas);
void incrementar_voos_domesticos(MetricasSimulacao* metricas);
void incrementar_voos_internacionais(MetricasSimulacao* metricas);

#endif