#include "libs.h"
// =============== FUNÇÕES ESPECÍFICAS POR TIPO DE VOO ===============
int pouso_internacional(Aviao* aviao, SimulacaoAeroporto* sim);
int pouso_domestico(Aviao* aviao, SimulacaoAeroporto* sim);
int desembarque_internacional(Aviao* aviao, SimulacaoAeroporto* sim);
int desembarque_domestico(Aviao* aviao, SimulacaoAeroporto* sim);
int decolagem_internacional(Aviao* aviao, SimulacaoAeroporto* sim);
int decolagem_domestico(Aviao* aviao, SimulacaoAeroporto* sim);

// =============== FUNÇÕES DE CRIAÇÃO DE AVIÕES ===============
Aviao* criar_aviao(int id, TipoVoo tipo);
void* criador_avioes(void* arg);
TipoVoo gerar_tipo_voo_aleatorio();
