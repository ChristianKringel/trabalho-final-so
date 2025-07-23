// VAI TER A STRUCT DO AVIAO 

// ENUM PARA TIPOS DE AVIAO
// ENUM PARA ESTADOS DO AVIAO
// =============== FUNÇÕES DE OPERAÇÕES DE AVIÕES ===============
#include "libs.h"
void* thread_aviao(void* arg);
int realizar_pouso(Aviao* aviao, SimulacaoAeroporto* sim);
int realizar_desembarque(Aviao* aviao, SimulacaoAeroporto* sim);
int realizar_decolagem(Aviao* aviao, SimulacaoAeroporto* sim);

int realizar_pouso(Aviao* aviao, SimulacaoAeroporto* sim) {
    if (solicitar_pista(&sim->recursos, aviao->id, aviao->tipo) != 0) {
        return -1;
    }
    
    sleep(2);
    
    liberar_pista(&sim->recursos, aviao->id);
    atualizar_estado_aviao(aviao, 2);
    
    incrementar_metrica_pouso(&sim->metricas);
    return 0; 
}


int realizar_desembarque(Aviao* aviao, SimulacaoAeroporto* sim) {
    if (solicitar_portao(&sim->recursos, aviao->id, aviao->tipo) != 0) {
        return -1;
    }
    
    sleep(1);
    
    liberar_portao(&sim->recursos, aviao->id);
    atualizar_estado_aviao(aviao, 3);
    
    incrementar_metrica_desembarque(&sim->metricas);
    return 0; 
}

