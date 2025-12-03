#ifndef TABSIMB_H
#define TABSIMB_H

#include "analex.h"
#include <stdio.h>

typedef enum {
    CAT_PROGRAMA,
    CAT_VARIAVEL,
    CAT_PARAMETRO
} Categoria;

typedef enum {
    TIPO_VOID,
    TIPO_INT,
    TIPO_FLOAT
} TipoSimbolo;

typedef struct {
    char lexema[50];
    Categoria categoria;
    TipoSimbolo tipo;
    int endereco;
    int nivel;
} RegistroTS;

// Funções públicas
void ts_iniciar();
void ts_inserir(const char *lexema, Categoria cat, TipoSimbolo tipo);
RegistroTS* ts_buscar(const char *lexema);
RegistroTS* ts_buscar_no_nivel(const char *lexema, int nivel);
void ts_imprimir_arquivo(const char *nome_base);
int ts_contar_variaveis_nivel_atual();
void ts_erro(const char *msg);

// Função auxiliar para converter tipoSimbolo para string  string
const char* tipo_para_string(TipoSimbolo t);
const char* cat_para_string(Categoria c);

#endif
