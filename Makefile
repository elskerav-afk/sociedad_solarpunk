CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
SRCS = main.c listas.c archivos.c sociedad.c consumo.c emergencias.c economia.c biblioteca.c arbol_avl.c arbol_chaflaneado.c
TARGET = programa

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) test_bib *.o *.gch