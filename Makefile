# PUC-Rio
# Análise Numérica 2015.2
# Professor: Waldemar Celes
# Gabriel de Quadros Ligneul 1212560
# Matheus Telles
# Trabalho Final: Visualização Volumétrica

target=visualizator
cflags=-std=c99 -Wall -Wextra
lflags=-lm
opt=-O2 -g

$(target): $(wildcard *.c)
	cc $(cflags) $(lflags) $(opt) -o $@ $^
