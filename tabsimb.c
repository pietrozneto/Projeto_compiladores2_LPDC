#include "tabsimb.h"
#include <string.h>
#include <stdlib.h>

#define MAX_SIMBOLOS 1000

static RegistroTS tabela[MAX_SIMBOLOS];
static int total_simbolos = 0;
static int nivel_atual = 0;
static int deslocamento_atual = 0;

void ts_iniciar(){
    total_simbolos = 0;
    nivel_atual = 0;
    deslocamento_atual = 0;
}

void ts_inserir(const char *lexema, Categoria cat, TipoSimbolo tipo) {
    // Verifica unicidade no mesmo escopo
    if (ts_buscar_no_nivel(lexema, nivel_atual) != NULL) {
        char erro[200];
        sprintf(erro, "Identificador '%s' já declarado neste escopo", lexema);
        ts_erro(erro);
    }

    if (total_simbolos >= MAX_SIMBOLOS) {
        ts_erro("Tabela de símbolos cheia");
    }

    RegistroTS reg;
    strcpy(reg.lexema, lexema);
    reg.categoria = cat;
    reg.tipo = tipo;
    reg.nivel = nivel_atual;

    if (cat == CAT_VARIAVEL) {
        reg.endereco = deslocamento_atual++;
    } else {
        reg.endereco = -1; //função sem endereço
    }
    
    tabela[total_simbolos] = reg;
    total_simbolos++;
}

RegistroTS* ts_buscar(const char *lexema) {
    for (int i = total_simbolos - 1; i >=0; i--){
        if (strcmp(tabela[i].lexema, lexema) == 0) {
            return &tabela[i];
        }
    }
    return NULL;
}

RegistroTS* ts_buscar_no_nivel(const char *lexema, int nivel) {
    for (int i = total_simbolos - 1; i >= 0; i--) {
        if (tabela[i].nivel == nivel && strcmp(tabela[i].lexema, lexema) == 0) {
            return &tabela[i];
        }
    }
    return NULL;
}

int ts_contar_variaveis_nivel_atual() {
    int count = 0;
    for (int i = 0; i < total_simbolos; i++) {
        if (tabela[i].nivel == nivel_atual && tabela[i].categoria == CAT_VARIAVEL) {
            count++;
        }
    }
    return count;
}

void ts_imprimir_arquivo(const char *nome_base) {
    char arquivo_ts[256];
    sprintf(arquivo_ts, "%s.ts", nome_base);

    FILE *arq = fopen(arquivo_ts, "w");
    if (!arq) {
        printf("Erro ao criar arquivo .ts\n");
        return;
    }

    for (int i = 0; i < total_simbolos; i++) {
        RegistroTS r = tabela[i];
        fprintf(arq, "TS[ lex: %-8s | cat: %-8s | tip: %-6s | end: %d ]\n",
                r.lexema,
                cat_para_string(r.categoria),
                tipo_para_string(r.tipo),
                r.endereco);
    }
    fclose(arq);
}

void ts_erro(const char *msg) {
    extern TInfoAtomo lookahead;
    printf("Erro semântico (linha %d): %s\n", lookahead.linha, msg);
    exit(1);
}

const char* tipo_para_string(TipoSimbolo t) {
    switch(t) {
        case TIPO_VOID:  return "void";
        case TIPO_INT:   return "int";
        case TIPO_FLOAT: return "float";
        default:         return "desconhecido";
    }
}

const char* cat_para_string(Categoria c) {
    switch(c) {
        case CAT_PROGRAMA:   return "programa";
        case CAT_VARIAVEL:   return "variável";
        case CAT_PARAMETRO:  return "parâmetro";
        default:             return "desconhecida";
    }
}
