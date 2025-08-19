#include "logger.h"

static void format_file_timestamp(char* timestamp, size_t size) {
    time_t agora = time(NULL);
    struct tm* tm_info = localtime(&agora);
    if (tm_info) {
        strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", tm_info);
    } else {
        strcpy(timestamp, "UNKNOWN");
    }
}

void create_log_file() {
    FILE* arquivo_log = fopen("simulacao_log.txt", "w");

    if (!arquivo_log) return;

    fprintf(arquivo_log, "Iniciando simulação...\n");
    fclose(arquivo_log);
}

void append_to_log_file(SimulacaoAeroporto* sim, Aviao* aviao, const char* buffer) {
    FILE* arquivo_log = fopen("simulacao_log.txt", "a");
    if (!arquivo_log) return;

    char timestamp[64];
    format_file_timestamp(timestamp, sizeof(timestamp));

    if (aviao && aviao->id > 0) {
        char tipo_char = (aviao->tipo == VOO_DOMESTICO) ? 'D' : 'I';
        fprintf(arquivo_log, "[%s] %c%02d  %s\n", timestamp, tipo_char, aviao->id, buffer);
    } else {
        fprintf(arquivo_log, "[%s] [SYSTEM]  %s\n", timestamp, buffer);
    }

    fclose(arquivo_log);
}