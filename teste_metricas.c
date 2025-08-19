#include "include/libs.h"
#include "include/metrics.h"
#include "include/initialize.h"

int main() {
    MetricasSimulacao metricas;
    inicializar_metricas(&metricas);
    
    // Simular algumas métricas
    metricas.total_avioes_criados = 10;
    metricas.avioes_finalizados_sucesso = 7;
    metricas.avioes_falha_starvation = 2;
    metricas.avioes_falha_deadlock = 1;
    metricas.voos_domesticos_total = 6;
    metricas.voos_internacionais_total = 4;
    metricas.operacoes_pouso = 10;
    metricas.operacoes_desembarque = 7;
    metricas.operacoes_decolagem = 7;
    
    // Salvar métricas
    salvar_metricas_finais(&metricas, "teste_metricas_finais.txt");
    
    printf("Teste concluído! Verifique o arquivo teste_metricas_finais.txt\n");
    
    pthread_mutex_destroy(&metricas.mutex_metricas);
    return 0;
}
