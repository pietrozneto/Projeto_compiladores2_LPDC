#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int n_labels = 1;


void gera_instr_mepa(char *rotulo, char *mnemonico, char *parametro1, char *parametro2){
    FILE *resultado;
    char *inst;
    resultado = fopen("compilado.mepa", "a+");
    inst = (char *)calloc(30,sizeof(char));
    if(mnemonico==NULL){
        sprintf(inst,"%s: NADA\n",rotulo);
        fputs(inst,resultado);
    }else if(parametro1==NULL){
        sprintf(inst,"    %s\n",mnemonico);
        fputs(inst,resultado);
    }else if(parametro2==NULL){
        sprintf(inst,"    %s %s\n",mnemonico,parametro1);
        fputs(inst,resultado);
    }else {
        sprintf(inst,"    %s %s,%s\n",mnemonico,parametro1,parametro2);
        fputs(inst,resultado);
    }
        fclose(resultado);
}
char* novo_rotulo(){
    char *label;
    char *numero;
    label = (char *)calloc(4,sizeof(char));
    numero = (char *)calloc(4,sizeof(char));
    strcpy(label,"L");
    sprintf(numero,"%d",n_labels);
    strcat(label,numero);
    n_labels++;
    return label;
}
