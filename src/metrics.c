#include "metrics.h"

void incrementar_metrica_pouso(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->operacoes_pouso++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_metrica_desembarque(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->operacoes_desembarque++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_metrica_decolagem(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->operacoes_decolagem++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_aviao_sucesso(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->avioes_finalizados_sucesso++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_aviao_falha_starvation(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->avioes_falha_starvation++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_aviao_deadlock(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->avioes_falha_deadlock++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_contador_avioes_criados(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->total_avioes_criados++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_voos_domesticos(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->voos_domesticos_total++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void incrementar_voos_internacionais(MetricasSimulacao* metricas) {
    if (metricas == NULL) {
        return; 
    }
    
    pthread_mutex_lock(&metricas->mutex_metricas);
    metricas->voos_internacionais_total++;
    pthread_mutex_unlock(&metricas->mutex_metricas);
}

void salvar_metricas_finais(MetricasSimulacao* metricas, const char* nome_arquivo) {
    if (metricas == NULL || nome_arquivo == NULL) {
        return;
    }

    FILE* arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        perror("Erro ao criar arquivo de métricas finais");
        return;
    }

    // Obter timestamp atual
    time_t tempo_atual = time(NULL);
    struct tm* info_tempo = localtime(&tempo_atual);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", info_tempo);

    pthread_mutex_lock(&metricas->mutex_metricas);

    // Cabeçalho do arquivo
    fprintf(arquivo, "=======================================================\n");
    fprintf(arquivo, "           RELATÓRIO FINAL DE MÉTRICAS\n");
    fprintf(arquivo, "           SIMULADOR DE TRÁFEGO AÉREO\n");
    fprintf(arquivo, "=======================================================\n");
    fprintf(arquivo, "Data/Hora: %s\n", timestamp);
    fprintf(arquivo, "=======================================================\n\n");

    // Métricas de aviões
    fprintf(arquivo, "ESTATÍSTICAS DE AVIÕES:\n");
    fprintf(arquivo, "------------------------\n");
    fprintf(arquivo, "Total de aviões criados:        %d\n", metricas->total_avioes_criados);
    fprintf(arquivo, "Aviões finalizados com sucesso: %d\n", metricas->avioes_finalizados_sucesso);
    fprintf(arquivo, "Aviões com falha por starvation: %d\n", metricas->avioes_falha_starvation);
    fprintf(arquivo, "Aviões com falha por deadlock:  %d\n", metricas->avioes_falha_deadlock);
    
    // Calcular taxas de sucesso e falha
    if (metricas->total_avioes_criados > 0) {
        double taxa_sucesso = (double)metricas->avioes_finalizados_sucesso / metricas->total_avioes_criados * 100.0;
        double taxa_starvation = (double)metricas->avioes_falha_starvation / metricas->total_avioes_criados * 100.0;
        double taxa_deadlock = (double)metricas->avioes_falha_deadlock / metricas->total_avioes_criados * 100.0;
        
        fprintf(arquivo, "\nTAXAS DE PERFORMANCE:\n");
        fprintf(arquivo, "---------------------\n");
        fprintf(arquivo, "Taxa de sucesso:     %.2f%%\n", taxa_sucesso);
        fprintf(arquivo, "Taxa de starvation:  %.2f%%\n", taxa_starvation);
        fprintf(arquivo, "Taxa de deadlock:    %.2f%%\n", taxa_deadlock);
    }

    // Métricas de tipos de voo
    fprintf(arquivo, "\nTIPOS DE VOO:\n");
    fprintf(arquivo, "-------------\n");
    fprintf(arquivo, "Voos domésticos:      %d\n", metricas->voos_domesticos_total);
    fprintf(arquivo, "Voos internacionais:  %d\n", metricas->voos_internacionais_total);
    
    if (metricas->total_avioes_criados > 0) {
        double percentual_domestico = (double)metricas->voos_domesticos_total / metricas->total_avioes_criados * 100.0;
        double percentual_internacional = (double)metricas->voos_internacionais_total / metricas->total_avioes_criados * 100.0;
        
        fprintf(arquivo, "Percentual doméstico:     %.2f%%\n", percentual_domestico);
        fprintf(arquivo, "Percentual internacional: %.2f%%\n", percentual_internacional);
    }

    // Métricas de operações
    fprintf(arquivo, "\nOPERAÇÕES REALIZADAS:\n");
    fprintf(arquivo, "--------------------\n");
    fprintf(arquivo, "Operações de pouso:      %d\n", metricas->operacoes_pouso);
    fprintf(arquivo, "Operações de desembarque: %d\n", metricas->operacoes_desembarque);
    fprintf(arquivo, "Operações de decolagem:   %d\n", metricas->operacoes_decolagem);
    
    // Totais e estatísticas finais
    int total_operacoes = metricas->operacoes_pouso + metricas->operacoes_desembarque + metricas->operacoes_decolagem;
    fprintf(arquivo, "Total de operações:       %d\n", total_operacoes);

    // Análise de eficiência
    fprintf(arquivo, "\nANÁLISE DE EFICIÊNCIA:\n");
    fprintf(arquivo, "----------------------\n");
    if (metricas->avioes_finalizados_sucesso > 0) {
        double media_operacoes_por_aviao = (double)total_operacoes / metricas->avioes_finalizados_sucesso;
        fprintf(arquivo, "Média de operações por avião bem-sucedido: %.2f\n", media_operacoes_por_aviao);
    }
    
    if (metricas->total_avioes_criados > 0) {
        int avioes_com_falha = metricas->avioes_falha_starvation + metricas->avioes_falha_deadlock;
        double taxa_falha_total = (double)avioes_com_falha / metricas->total_avioes_criados * 100.0;
        fprintf(arquivo, "Taxa total de falhas: %.2f%%\n", taxa_falha_total);
    }

    pthread_mutex_unlock(&metricas->mutex_metricas);

    fprintf(arquivo, "\n=======================================================\n");
    fprintf(arquivo, "              FIM DO RELATÓRIO\n");
    fprintf(arquivo, "=======================================================\n");

    fclose(arquivo);
    printf("Métricas finais salvas em: %s\n", nome_arquivo);
}