#include "analex.h"
#include "asdr.h"
#include "tabsimb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TInfoAtomo lookahead;
void verifica(TipoAtomo atomo){
    //TipoAtomo tipo = atomo.tipo;
    if(atomo == lookahead.tipo){
        lookahead = obter_atomo();
    }else{
         parse_erro();
    }
}

void parse_erro(){
    fprintf(stderr, "Erro sintático encontrado\n");
    fprintf(stderr, "Tipo do token: %d\n", lookahead.tipo);
    fprintf(stderr, "Linha: %d\n", lookahead.linha);
    exit(EXIT_FAILURE);
}
void parse_ini() {
    verifica(sPRG);

    TInfoAtomo info_prog = lookahead;
    verifica(sIDENT);

    ts_iniciar();
    ts_inserir(info_prog.lexema.string, CAT_PROGRAMA, TIPO_VOID);

    verifica(sPONTO_VIRG);

    if (lookahead.tipo == sVAR){
        parse_dcl(); //processa declarações de variáveis
    }

    parse_bco();
    verifica(sPONTO);
}

void parse_id(){
    if (lookahead.tipo != sIDENT) {
        parse_erro();
    }
    if (strlen(lookahead.lexema.string) > 0 && ts_buscar(lookahead.lexema.string) == NULL) {
        char erro[300];
        snprintf(erro, sizeof(erro), "Identificador '%s' não declarado", lookahead.lexema.string);
        ts_erro(erro);
    }
    verifica(sIDENT);
}

int parse_dcl(){
    int qte = 0;
    verifica(sVAR);

    while (lookahead.tipo == sINT || lookahead.tipo == sFLOAT) {
        TipoSimbolo tipo = (lookahead.tipo == sINT) ? TIPO_INT : TIPO_FLOAT;
        verifica(lookahead.tipo); // consome int ou float

        TInfoAtomo info_var = lookahead;
        verifica(sIDENT);
        ts_inserir(info_var.lexema.string, CAT_VARIAVEL, tipo);
        qte++;

        while (lookahead.tipo == sVIRG) {
            verifica(sVIRG);
            info_var = lookahead;
            verifica(sIDENT);
            ts_inserir(info_var.lexema.string, CAT_VARIAVEL, tipo);
            qte++;
        }
        verifica(sPONTO_VIRG);
    }
    return qte;
}

void parse_bco(){
    verifica(sBEGIN);
    while(lookahead.tipo != sEND){
        parse_cmd();
        verifica(sPONTO_VIRG);
    }
    verifica(sEND);
}

void parse_tpo(){
    verifica(sINT);
}

void parse_cmd(){
    switch(lookahead.tipo){
        case sWRITE:
            parse_wr();
            break;
        case sREAD:
            parse_rd();
            break;
        case sIF:
            parse_if();
            break;
        case sFOR:
            parse_fr();
            break;
        case sWHILE:
            parse_wh();
            break;
        case sREPEAT:
            parse_rpt();
            break;
        case sRETURN:
            parse_ret();
            break;
        default:
            parse_atr();


    }
}
void parse_wr(){
    verifica(sWRITE);
    verifica(sABRE_PARENT);
    parse_e();
    while(lookahead.tipo ==sVIRG){
        verifica(sVIRG);
        parse_e();
    }
    verifica(sFECHA_PARENT);
}
void parse_rd(){
    verifica(sREAD);
    verifica(sABRE_PARENT);
    parse_id();
    verifica(sFECHA_PARENT);
}
void parse_if(){
    verifica(sIF);
    verifica(sABRE_PARENT);
    parse_exp();
    verifica(sFECHA_PARENT);
    verifica(sTHEN);
    parse_cmd();
    if(lookahead.tipo ==sELSE){
        verifica(sELSE);
        parse_cmd();
    }
}
void parse_fr(){
    verifica(sFOR);
    verifica(sABRE_PARENT);
    if(lookahead.tipo != sPONTO_VIRG){
        parse_atr();
    }
    verifica(sPONTO_VIRG);
    parse_exp();
    verifica(sPONTO_VIRG);
    if(lookahead.tipo != sFECHA_PARENT){
        parse_atr();
    }
    verifica(sFECHA_PARENT);
    parse_cmd();

}
void parse_wh(){
    verifica(sWHILE);
    verifica(sABRE_PARENT);
    parse_exp();
    verifica(sFECHA_PARENT);
    parse_cmd();
}
void parse_rpt(){
    verifica(sREPEAT);
    while(lookahead.tipo != sUNTIL){
        parse_cmd();
        verifica(sPONTO_VIRG);
    }
    verifica(sUNTIL);
    verifica(sABRE_PARENT);
    parse_exp();
    verifica(sFECHA_PARENT);
}
void parse_atr(){
    parse_id();
    verifica(sATRIB);
    parse_exp();
}
void parse_ret(){
    verifica(sRETURN);
    parse_exp();
}

void parse_e(){
    if(lookahead.tipo == sNUM_INT){
        verifica(sNUM_INT);
    }else if(lookahead.tipo == sNUM_REAL){
        verifica(sNUM_REAL);
    }else{
        parse_id();
    }
}
void parse_exp(){
    parse_exps();
    while(lookahead.tipo == sMAIOR || lookahead.tipo == sMAIOR_IGUAL || lookahead.tipo == sMENOR || lookahead.tipo == sMENOR_IGUAL || lookahead.tipo == sIGUAL || lookahead.tipo == sDIFERENTE){
        parse_op_cat3();
        parse_exps();
    }
}
void parse_exps(){
    parse_tmo();
    while(lookahead.tipo == sSOMA || lookahead.tipo == sSUBTR || lookahead.tipo == sOR){
        parse_op_cat2();
        parse_tmo();
    }
}
void parse_tmo(){
    parse_ftr();
    while(lookahead.tipo ==sMULT || lookahead.tipo == sDIV || lookahead.tipo == sAND){
        parse_op_cat1();
        parse_ftr();
    }
}
void parse_ftr(){
    if(lookahead.tipo == sNOT){
        verifica(sNOT);
        parse_ftr();
    }else if(lookahead.tipo == sABRE_PARENT){
        verifica(sABRE_PARENT);
        parse_exp();
        verifica(sFECHA_PARENT);
    }else{
        parse_e();
    }
}
void parse_op_cat3(){
    switch (lookahead.tipo){
        case sMAIOR:
            verifica(sMAIOR);
            break;
        case sMAIOR_IGUAL:
            verifica(sMAIOR_IGUAL);
            break;
        case sMENOR:
            verifica(sMENOR);
            break;
        case sMENOR_IGUAL:
            verifica(sMENOR_IGUAL);
            break;
        case sIGUAL:
            verifica(sIGUAL);
            break;
        case sDIFERENTE:
            verifica(sDIFERENTE);
            break;
        default:
            parse_erro();
    }
}
void parse_op_cat2(){
    switch (lookahead.tipo){
        case sSOMA:
            verifica(sSOMA);
            break;
        case sSUBTR:
            verifica(sSUBTR);
            break;
        case sOR:
            verifica(sOR);
            break;
        default:
            parse_erro();
    }
}
void parse_op_cat1(){
    switch (lookahead.tipo){
        case sMULT:
            verifica(sMULT);
            break;
        case sDIV:
            verifica(sDIV);
            break;
        case sAND:
            verifica(sAND);
            break;
        default:
            parse_erro();
    }
}
