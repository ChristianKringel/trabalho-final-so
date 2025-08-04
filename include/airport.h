#include "libs.h"
#include "terminal.h"

#ifndef AIRPORT_H
#define AIRPORT_H
// =============== FUNÇÕES DE GERENCIAMENTO DE RECURSOS ===============
int solicitar_pista(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo);
int solicitar_pista_com_prioridade(SimulacaoAeroporto* sim, Aviao* aviao);
void liberar_pista(SimulacaoAeroporto* sim, int id_aviao, int pista_idx);

int solicitar_portao(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo);
int solicitar_portao_com_prioridade(SimulacaoAeroporto* sim, Aviao* aviao);
void liberar_portao(SimulacaoAeroporto* sim, int id_aviao, int portao_idx);

int solicitar_torre(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo);
int solicitar_torre_com_prioridade(SimulacaoAeroporto* sim, Aviao* aviao);
void liberar_torre(SimulacaoAeroporto* sim, int id_aviao);

#endif