# Nome do executável final
TARGET = air_traffic_simulator

# Diretórios
SRCDIR = src
INCDIR = include
OBJDIR = obj

# Compilador e Flags
CC = gcc
CFLAGS = -g -I$(INCDIR) -std=c11

# Bibliotecas a serem linkadas
LIBS = -pthread -lncursesw 

# --- Geração Automática de Arquivos ---

# Encontra todos os arquivos .c em 'src' e suas subpastas
SRCS = $(shell find $(SRCDIR) -name "*.c")

# Gera a lista de arquivos objeto (.o) correspondentes, que serão criados em 'obj'
# Mantém a estrutura de subpastas em obj/
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

# --- Regras Principais ---

# Torna 'all', 'clean', e 'run' alvos "falsos" para que o make sempre os execute.
.PHONY: all clean run

# A regra padrão (executada quando você digita apenas 'make')
all: $(TARGET)

# Regra para linkar todos os objetos e criar o executável final
$(TARGET): $(OBJS)
	@echo "Linkando o executável..."
	$(CC) $(OBJS) -o $@ $(LIBS)
	@echo "--- Compilação de $(TARGET) concluída! ---"
	@echo "Para executar, digite: make run"

# Regra de padrão para compilar um arquivo .c de 'src' para um .o em 'obj'
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@# Garante que o diretório de objetos exista antes de compilar
	@mkdir -p $(@D)
	@echo "Compilando $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# --- Regras de Utilidade ---

# Limpa todos os arquivos gerados
clean:
	@echo "Limpando arquivos gerados..."
	@rm -rf $(OBJDIR) $(TARGET)
	@echo "Limpeza concluída."

# Compila e executa o programa
run: all
	@echo "--- Executando $(TARGET)... ---"
	@./$(TARGET)