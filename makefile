SRC_DIR := src
OBJ_DIR := obj
INCLUDE_DIR := include

TARGET := contar-estrelas
CC := mpicc                     
CFLAGS := -Wall -Wextra -I$(INCLUDE_DIR)

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(OCUPACOES_DIR) $(RESULTADOS_DIR)

$(shell mkdir -p $(OBJ_DIR))

.PHONY: clean run