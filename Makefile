# Name des Ausgabefiles
TARGET = raylib-test

# Compiler
CC = cc

# Compiler-Flags mit Include-Pfad
CFLAGS = -Wall -Wextra -Wpedantic -Iexternal/raygui/src

# Raylib + andere Bibliotheken
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Quellcode-Dateien
SRC = main.c

# Build-Regel
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Clean-Regel
clean:
	rm -f $(TARGET)
