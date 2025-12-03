#include "tabsimb.h"
#include <string.h>
#include <stdlib.h>


#define MAX_SIMBOLOS 1000

static int total_simbolos;
static RegistroTS tabela[MAX_SIMBOLOS];

void ts_iniciar(){
    total_simbolos = 0;
}

RegistroTS* ts_inserir(const char *lexema, Categoria cat, TipoAtomo tipo, int endereco){
    // Verifica unicidade no mesmo escopo
    if (ts_buscar(lexema) != NULL) {
        ts_erro();
    }

    if (total_simbolos >= MAX_SIMBOLOS) {
        ts_erro();
    }

    RegistroTS reg;
    strcpy(reg.lexema, lexema);
    reg.categoria = cat;
    if(tipo == sVOID){
        reg.tipo = sVOID;
    }else{
        reg.tipo = sINT;
    }
    reg.endereco = endereco;

    tabela[total_simbolos] = reg;
    total_simbolos++;
    return &tabela[total_simbolos];
}


RegistroTS* ts_buscar(const char *lexema) {
    for (int i = total_simbolos - 1; i >=0; i--){
        if (strcmp(tabela[i].lexema, lexema) == 0) {
            return &tabela[i];
        }
    }

    return NULL;
}

void ts_erro(){
    exit(EXIT_FAILURE);
}
