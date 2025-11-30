#include <stdio.h>
#include "analex.h"
#include "asdr.h"
#include <stdlib.h>

TInfoAtomo lookahead;
void verifica(TipoAtomo atomo){
    //TipoAtomo tipo = atomo.tipo;
    if(atomo == lookahead.tipo){
        printf("deu certo");
        return;
    }
    parse_erro();
}

void parse_erro(){
    perror("Erro na compilacao");
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

}
