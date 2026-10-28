CC = gcc
CFLAGS = -O2 -std=c11 -Wall -Wextra -Wpedantic -Werror
LDLIBS = -pthread
PROGRAMAS = estadisticas histograma productores_consumidores

.PHONY: all clean pruebas mediciones
all: $(PROGRAMAS)
$(PROGRAMAS): %: %.c comun.h
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

pruebas: all
	python3 pruebas.py

mediciones: all
	python3 medir.py

clean:
	rm -f $(PROGRAMAS)
