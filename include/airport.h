#include "libs.h"
#include "terminal.h"

#ifndef AIRPORT_H
#define AIRPORT_H
// =============== FUNÇÕES DE GERENCIAMENTO DE RECURSOS ===============

void liberar_pista(SimulacaoAeroporto* sim, int id_aviao, int pista_idx);
void liberar_portao(SimulacaoAeroporto* sim, int id_aviao, int portao_idx);
void liberar_torre(SimulacaoAeroporto* sim, int id_aviao);
void liberar_todos_recursos(SimulacaoAeroporto* sim, Aviao* aviao);
#endif