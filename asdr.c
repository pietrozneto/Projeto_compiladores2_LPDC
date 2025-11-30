#include <stdio.h>
#include "analex.h"
#include "asdr.h"
#include <stdlib.h>

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
    perror("Erro na compilacao");
    printf("tipo %d",lookahead.tipo);
    exit(EXIT_FAILURE);
}
void parse_ini() {
    verifica(sPRG);
    parse_id();
    verifica(sPONTO_VIRG);
    if (lookahead.tipo == sVAR){
        parse_dcl();
    }

    parse_bco();
    verifica(sPONTO);
}

void parse_id(){
    verifica(sIDENT);
}

void parse_dcl(){
    int qte = 0;
    verifica(sVAR);
    do{
        parse_tpo();
        parse_id();
        qte++;
        while(lookahead.tipo == sVIRG){
            verifica(sVIRG);
            parse_id();
            qte++;
        }
        verifica(sPONTO_VIRG);
    }while(lookahead.tipo == sINT);

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
