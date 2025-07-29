# Name des Ausgabefiles
TARGET = Graviton

# Compiler
CC = cc

# Compiler-Flags mit Include-Pfad
CFLAGS = -Wall -Wextra -Wpedantic 

# Raylib + andere Bibliotheken
LDFLAGS = -Iexternal/raygui/src -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Quellcode-Dateien
SRC = main.c

# Build-Regel
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Clean-Regel
clean:
	rm -f $(TARGET)
