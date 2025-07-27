# Name des Ausgabefiles
TARGET = raylib-test

# Compiler
CC = cc

# Compiler-Flags (z. B. Warnungen einschalten)
CFLAGS = -Wall -Wextra -Wpedantic

# Raylib + andere Bibliotheken
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Quellcode-Dateien
SRC = main.c

# Regel: Was passiert, wenn du "make" eingibst
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Optional: "make clean" löscht das kompiliertes Programm
clean:
	rm -f $(TARGET)
