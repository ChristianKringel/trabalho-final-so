#include "libs.h"
#include "airport.h"
#include "utils.h"
#include "initialize.h"
#include "metrics.h"
#include "terminal.h"

#ifndef AIRPLANE_H
#define AIRPLANE_H

int pouso_internacional(Aviao* aviao, SimulacaoAeroporto* sim);
int pouso_domestico(Aviao* aviao, SimulacaoAeroporto* sim);
int desembarque_internacional(Aviao* aviao, SimulacaoAeroporto* sim);
int desembarque_domestico(Aviao* aviao, SimulacaoAeroporto* sim);
int decolagem_internacional(Aviao* aviao, SimulacaoAeroporto* sim);
int decolagem_domestico(Aviao* aviao, SimulacaoAeroporto* sim);

// =============== VERSÕES ATÔMICAS PARA EVITAR DEADLOCK ===============
int pouso_internacional_atomico(Aviao* aviao, SimulacaoAeroporto* sim);
int pouso_domestico_atomico(Aviao* aviao, SimulacaoAeroporto* sim);
int desembarque_internacional_atomico(Aviao* aviao, SimulacaoAeroporto* sim);
int desembarque_domestico_atomico(Aviao* aviao, SimulacaoAeroporto* sim);
int decolagem_internacional_atomica(Aviao* aviao, SimulacaoAeroporto* sim);
int decolagem_domestica_atomica(Aviao* aviao, SimulacaoAeroporto* sim);

Aviao* criar_aviao(int id, TipoVoo tipo);
void* criador_avioes(void* arg);
TipoVoo gerar_tipo_voo_aleatorio();

#endif
