#include "libs.h"
// =============== FUNÇÕES DE GERENCIAMENTO DE RECURSOS ===============
int solicitar_pista(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo);
void liberar_pista(SimulacaoAeroporto* sim, int id_aviao, int pista_idx);
int solicitar_portao(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo);
void liberar_portao(SimulacaoAeroporto* sim, int id_aviao, int portao_idx);
int solicitar_torre(SimulacaoAeroporto* sim, int id_aviao, TipoVoo tipo);
void liberar_torre(SimulacaoAeroporto* sim, int id_aviao);
