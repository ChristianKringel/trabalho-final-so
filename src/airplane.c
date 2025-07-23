// VAI TER A STRUCT DO AVIAO 

// ENUM PARA TIPOS DE AVIAO
// ENUM PARA ESTADOS DO AVIAO
// =============== FUNCOES DE CRIACAO  ===============
#include "libs.h"

Aviao* criar_aviao(int id, TipoVoo tipo);
void* criador_avioes(void* arg);
TipoVoo gerar_tipo_voo_aleatorio();

Aviao* criar_aviao(int id, TipoVoo tipo) {
    Aviao* aviao = (Aviao*)malloc(sizeof(Aviao));
    if (aviao == NULL) {
        return NULL; 
    }
    
    aviao->id = id;
    aviao->tipo = tipo;
    aviao->estado = AGUARDANDO_POUSO;
    aviao->tempo_criacao = time(NULL);
    aviao->tempo_inicio_espera = 0;
    aviao->tempo_fim_operacao = 0;
    aviao->thread_id = pthread_self();
    aviao->pista_ocupada = 0;
    aviao->portao_ocupado = 0;
    aviao->usando_torre = 0;

    return aviao;
}


TipoVoo gerar_tipo_voo_aleatorio() {
    return rand() % 2; // 0 para VOO_DOMESTICO, 1 para VOO_INTERNACIONAL
}

// =============== FUNÇÕES DE OPERACAO DE AVIOES ===============
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

int realizar_decolagem(Aviao* aviao, SimulacaoAeroporto* sim) {
    if (solicitar_torre(&sim->recursos, aviao->id, aviao->tipo) != 0) {
        return -1;
    }
    
    sleep(3);
    
    liberar_torre(&sim->recursos, aviao->id);
    atualizar_estado_aviao(aviao, 4);
    
    incrementar_metrica_decolagem(&sim->metricas);
    return 0; 
}