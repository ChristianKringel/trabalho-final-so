# Nome do executável final
TARGET = air_traffic_simulator

# Diretórios
SRCDIR = src
INCDIR = include
OBJDIR = obj # Diretório para arquivos objeto, será criado se não existir

# Compilador C
CC = gcc

# Flags de compilação
# -Wall: Habilita todos os warnings
# -Wextra: Habilita warnings extras
# -g: Inclui informações de debug (útil para gdb)
# -I$(INCDIR): Adiciona o diretório include ao caminho de busca de cabeçalhos
# -std=c11: Define o padrão C11 (ou c99 se preferir)
CFLAGS = -Wall -Wextra -g -I$(INCDIR) -std=c11

# Bibliotecas
# -pthread: Linka com a biblioteca PThreads
# -lncurses: Linka com a biblioteca ncurses
# -lrt: Linka com a biblioteca de tempo real (clock_gettime, etc., pode ser útil)
LIBS = -pthread -lncurses -lrt

# Encontra todos os arquivos .c no diretório src
SRCS = $(wildcard $(SRCDIR)/*.c)

# Transforma a lista de arquivos .c em arquivos .o, salvando-os no diretório obj
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(OBJDIR) $(TARGET)

# Cria o diretório de objetos se ele não existir
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Regra principal para compilar o executável
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)
	@echo "--- Compilação de $(TARGET) concluída! ---"

# Regra para compilar cada arquivo .c para .o
# $<: nome do primeiro pré-requisito (o arquivo .c)
# $@: nome do alvo (o arquivo .o)
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compilando $<..."

clean:
	@echo "Limpando arquivos gerados..."
	@rm -rf $(OBJDIR) $(TARGET)
	@echo "Limpeza concluída."

run: all
	@echo "--- Executando $(TARGET)... ---"
	@./$(TARGET)