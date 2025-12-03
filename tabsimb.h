#ifndef TABSIMB_H
#define TABSIMB_H

#include "analex.h"
#include <stdio.h>

typedef enum {
    CAT_PROGRAMA,
    CAT_VARIAVEL
} Categoria;


typedef struct {
    char lexema[50];
    Categoria categoria;
    TipoAtomo tipo;
    int endereco;
} RegistroTS;

void ts_iniciar();
RegistroTS* ts_inserir(const char *lexema, Categoria cat, TipoAtomo tipo, int endereco);
RegistroTS* ts_buscar(const char *lexema);
void ts_erro();

#endif

