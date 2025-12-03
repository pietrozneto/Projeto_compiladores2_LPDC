#include <stdio.h>
#include "analex.h"
#include "asdr.h"
#include <stdlib.h>
#include "tabsimb.h"
#include "gerador.h"

TInfoAtomo lookahead;
TInfoAtomo temp;

void verifica(TipoAtomo atomo){
    //TipoAtomo tipo = atomo.tipo;
    if(atomo == lookahead.tipo){
        lookahead = obter_atomo();
    }else{
        parse_erro(atomo);
    }
}

void parse_erro(TipoAtomo atomo){
    fprintf(stderr, "Erro sintático encontrado\n");
    fprintf(stderr, "Tipo do token: %d\n", lookahead.tipo);
    fprintf(stderr, "Linha: %d\n", lookahead.linha);
    exit(EXIT_FAILURE);
}
void parse_ini() {
    char *numero;
    numero = NULL;
    int n_var;
    verifica(sPRG);
    temp = lookahead;
    parse_id();
    ts_inserir(temp.lexema.string,CAT_PROGRAMA,sVOID,-1);
    gera_instr_mepa(NULL,"INPP",NULL,NULL);
    verifica(sPONTO_VIRG);
    if (lookahead.tipo == sVAR){
        numero = (char *)calloc(4,sizeof(char));
        n_var = parse_dcl();
        sprintf(numero,"%d",n_var);
        gera_instr_mepa(NULL,"AMEM",numero,NULL);
    }

    parse_bco();
    verifica(sPONTO);
    gera_instr_mepa(NULL,"DMEM",numero,NULL);
    gera_instr_mepa(NULL,"PARA",NULL,NULL);
    free(numero);
}

void parse_id(){
    verifica(sIDENT);
}

int parse_dcl(){
    int qte = 0;

    verifica(sVAR);
    do{
        parse_tpo();
        temp = lookahead;
        parse_id();
        ts_inserir(temp.lexema.string, CAT_VARIAVEL, temp.tipo, qte);
        qte++;
        while(lookahead.tipo == sVIRG){
            verifica(sVIRG);
            temp = lookahead;
            parse_id();
            ts_inserir(temp.lexema.string, CAT_VARIAVEL, temp.tipo, qte);
            qte++;
        }
        verifica(sPONTO_VIRG);
    }while(lookahead.tipo == sINT);
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
    gera_instr_mepa(NULL,"IMPR",NULL,NULL);
    while(lookahead.tipo ==sVIRG){
        verifica(sVIRG);
        parse_e();
        gera_instr_mepa(NULL,"IMPR",NULL,NULL);
    }
    verifica(sFECHA_PARENT);
}
void parse_rd(){
    verifica(sREAD);
    gera_instr_mepa(NULL,"LEIT",NULL,NULL);

    verifica(sABRE_PARENT);
    RegistroTS *temp;
    temp = ts_buscar(lookahead.lexema.string);
    char *endereco;
    endereco = (char *)calloc(30,sizeof(char));
    sprintf(endereco,"%d",temp->endereco);
    parse_id();
    verifica(sFECHA_PARENT);
    gera_instr_mepa(NULL,"ARMZ","0",endereco);
}
void parse_if(){
    char *label1;
    char *label2;
    label1 = novo_rotulo();
    label2 = novo_rotulo();
    verifica(sIF);
    verifica(sABRE_PARENT);
    parse_exp();
    gera_instr_mepa(NULL,"DSVF",label1,NULL);
    verifica(sFECHA_PARENT);
    verifica(sTHEN);
    parse_cmd();
    gera_instr_mepa(NULL,"DSVS",label2,NULL);
    if(lookahead.tipo ==sELSE){
        verifica(sELSE);
        gera_instr_mepa(label1,NULL,NULL,NULL);
        parse_cmd();
    }
    gera_instr_mepa(label2,NULL,NULL,NULL);
}
void parse_fr(){
    char *label1;
    char *label2;
    char *label3;
    char *label4;
    label1 = novo_rotulo();
    label2 = novo_rotulo();
    label3 = novo_rotulo();
    label4 = novo_rotulo();
    verifica(sFOR);
    verifica(sABRE_PARENT);
    if(lookahead.tipo != sPONTO_VIRG){
        parse_atr();
        gera_instr_mepa(label1,NULL,NULL,NULL);
    }
    verifica(sPONTO_VIRG);
    parse_exp();
    gera_instr_mepa(NULL,"DSVF",label4,NULL);
    gera_instr_mepa(NULL,"DSVS",label3,NULL);
    gera_instr_mepa(label2,NULL,NULL,NULL);
    verifica(sPONTO_VIRG);
    if(lookahead.tipo != sFECHA_PARENT){
        parse_atr();
        gera_instr_mepa(NULL,"DSVS",label1,NULL);
        gera_instr_mepa(label3,NULL,NULL,NULL);
    }
    verifica(sFECHA_PARENT);
    parse_cmd();
    gera_instr_mepa(NULL,"DSVS",label2,NULL);
    gera_instr_mepa(label4,NULL,NULL,NULL);

}
void parse_wh(){
    char *label1;
    char *label2;
    label1 = novo_rotulo();
    label2 = novo_rotulo();
    verifica(sWHILE);
    gera_instr_mepa(label1,NULL,NULL,NULL);
    verifica(sABRE_PARENT);
    parse_exp();
    gera_instr_mepa(NULL,"DSVF",label2,NULL);
    verifica(sFECHA_PARENT);
    parse_cmd();
    gera_instr_mepa(NULL,"DSVS",label1,NULL);
    gera_instr_mepa(label2,NULL,NULL,NULL);
}
void parse_rpt(){
    char *label1;
    label1 = novo_rotulo();
    gera_instr_mepa(label1,NULL,NULL,NULL);
    verifica(sREPEAT);
    while(lookahead.tipo != sUNTIL){
        parse_cmd();
        verifica(sPONTO_VIRG);
    }
    verifica(sUNTIL);
    verifica(sABRE_PARENT);
    parse_exp();
    gera_instr_mepa(NULL,"NEGA",NULL,NULL);
    gera_instr_mepa(NULL,"DSVF",label1,NULL);
    verifica(sFECHA_PARENT);
}
void parse_atr(){
    RegistroTS *temp;
    temp = ts_buscar(lookahead.lexema.string);
    char *endereco;
    endereco = (char *)calloc(30,sizeof(char));
    sprintf(endereco,"%d",temp->endereco);
    parse_id();
    verifica(sATRIB);
    parse_exp();
    gera_instr_mepa(NULL,"ARMZ","0",endereco);
    free(endereco);
}
void parse_ret(){
    verifica(sRETURN);
    parse_exp();
}

void parse_e(){
    if(lookahead.tipo == sNUM_INT){
        char *numero;
        numero = (char *)calloc(30,sizeof(char));
        sprintf(numero,"%d",lookahead.lexema.valorInt);
        gera_instr_mepa(NULL,"CRCT",numero,NULL);
        free(numero);
        verifica(sNUM_INT);
    }else{
        RegistroTS *temp;
        temp = ts_buscar(lookahead.lexema.string);
        if (!temp) { printf("Erro: variável não declarada\n"); exit(1); }

        char *endereco;
        endereco = (char *)calloc(30,sizeof(char));
        sprintf(endereco,"%d",temp->endereco);
        parse_id();
        gera_instr_mepa(NULL,"CVLR","0",endereco);
        free(endereco);
    }
}
void parse_exp(){
    parse_exps();
    while(lookahead.tipo == sMAIOR || lookahead.tipo == sMAIOR_IGUAL || lookahead.tipo == sMENOR || lookahead.tipo == sMENOR_IGUAL || lookahead.tipo == sIGUAL || lookahead.tipo == sDIFERENTE){
        TipoAtomo tipo = parse_op_cat3();
        parse_exps();
        switch (tipo){
            case sMAIOR:
                gera_instr_mepa(NULL,"CMMA",NULL,NULL);
                break;
            case sMAIOR_IGUAL:
                gera_instr_mepa(NULL,"CMAG",NULL,NULL);
                break;
            case sMENOR:
                gera_instr_mepa(NULL,"CMME",NULL,NULL);
                break;
            case sMENOR_IGUAL:
                gera_instr_mepa(NULL,"CMEG",NULL,NULL);
                break;
            case sIGUAL:
                gera_instr_mepa(NULL,"CMIG",NULL,NULL);
                break;
            case sDIFERENTE:
                gera_instr_mepa(NULL,"CMDG",NULL,NULL);
                break;
            default:
                parse_erro(lookahead.tipo);
        }
    }
}
void parse_exps(){
    parse_tmo();
    while(lookahead.tipo == sSOMA || lookahead.tipo == sSUBTR || lookahead.tipo == sOR){
        TipoAtomo tipo = parse_op_cat2();
        parse_tmo();
        switch (tipo){
            case sSOMA:
                gera_instr_mepa(NULL,"SOMA",NULL,NULL);
                break;
            case sSUBTR:
                gera_instr_mepa(NULL,"SUBT",NULL,NULL);
                break;
            case sOR:
                gera_instr_mepa(NULL,"DISJ",NULL,NULL);
                break;
            default:
                parse_erro(lookahead.tipo);
        }
    }
}
void parse_tmo(){
    parse_ftr();
    while(lookahead.tipo ==sMULT || lookahead.tipo == sDIV || lookahead.tipo == sAND){
        TipoAtomo tipo = parse_op_cat1();
        parse_ftr();
        switch (tipo){
            case sMULT:
                gera_instr_mepa(NULL,"MULT",NULL,NULL);
                break;
            case sDIV:
                gera_instr_mepa(NULL,"DIVI",NULL,NULL);
                break;
            case sAND:
                gera_instr_mepa(NULL,"CONJ",NULL,NULL);
                break;
            default:
        }

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
TipoAtomo parse_op_cat3(){
    switch (lookahead.tipo){
        case sMAIOR:
            verifica(sMAIOR);
            return(sMAIOR);
            break;
        case sMAIOR_IGUAL:
            verifica(sMAIOR_IGUAL);
            return(sMAIOR_IGUAL);
            break;
        case sMENOR:
            verifica(sMENOR);
            return(sMENOR);
            break;
        case sMENOR_IGUAL:
            verifica(sMENOR_IGUAL);
            return(sMENOR_IGUAL);
            break;
        case sIGUAL:
            verifica(sIGUAL);
            return(sIGUAL);
            break;
        case sDIFERENTE:
            verifica(sDIFERENTE);
            return(sDIFERENTE);
            break;
        default:
            parse_erro(lookahead.tipo);
            return sERRO_LEXICO;
    }
}
TipoAtomo parse_op_cat2(){
    switch (lookahead.tipo){
        case sSOMA:
            verifica(sSOMA);
            return(sSOMA);
            break;
        case sSUBTR:
            verifica(sSUBTR);
            return(sSUBTR);
            break;
        case sOR:
            verifica(sOR);
            return(sOR);
            break;
        default:
            parse_erro(lookahead.tipo);
            return sERRO_LEXICO;
    }
}
TipoAtomo parse_op_cat1(){
    switch (lookahead.tipo){
        case sMULT:
            verifica(sMULT);
            return(sMULT);
            break;
        case sDIV:
            verifica(sDIV);
            return(sDIV);
            break;
        case sAND:
            verifica(sAND);
            return(sAND);
            break;
        default:
            parse_erro(lookahead.tipo);
            return sERRO_LEXICO;
    }
}
