#include "analex.h"
#include "asdr.h"
#include "tabsimb.h"
#include "gerador.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

TInfoAtomo lookahead;
// token esperado (usado para mensagens de erro mais informativas)
static int expected_atomo = -1;
static char expected_msg[128] = "";
// rastreia a linha do último token bem-sucedido processado
static int last_linha_token = 1;
// contador de erros (permitir continuar após erros para encontrar mais problemas)
static int error_count = 0;

// pilha de expectativas (útil para parênteses aninhados e similares)
static char expected_stack[32][32];
static int expected_stack_top = 0;

static void push_expected(const char *msg){
    if(expected_stack_top < (int)(sizeof(expected_stack)/sizeof(expected_stack[0]))){
        strncpy(expected_stack[expected_stack_top], msg, sizeof(expected_stack[0]) - 1);
        expected_stack[expected_stack_top][sizeof(expected_stack[0]) - 1] = '\0';
        expected_stack_top++;
    }
}

static void pop_expected(){
    if(expected_stack_top > 0) expected_stack_top--; 
}

// contador simples de parênteses abertos (ajuda a detectar ')' esperado quando ';' aparece)
static int open_paren_count = 0;

static void set_expected_msg(const char *msg){
    if(msg && msg[0]){
        strncpy(expected_msg, msg, sizeof(expected_msg)-1);
        expected_msg[sizeof(expected_msg)-1] = '\0';
    } else {
        expected_msg[0] = '\0';
    }
}

static const char* token_to_string(TipoAtomo t){
    switch(t){
        case sPRG: return "prg";
        case sVAR: return "var";
        case sSUBROT: return "subrot";
        case sRETURN: return "return";
        case sINT: return "int";
        case sFLOAT: return "float";
        case sCHAR: return "char";
        case sVOID: return "void";
        case sBEGIN: return "begin";
        case sEND: return "end";
        case sWRITE: return "write";
        case sREAD: return "read";
        case sIF: return "if";
        case sTHEN: return "then";
        case sELSE: return "else";
        case sFOR: return "for";
        case sWHILE: return "while";
        case sREPEAT: return "repeat";
        case sUNTIL: return "until";
        case sAND: return "and";
        case sOR: return "or";
        case sNOT: return "not";
        case sIDENT: return "identificador";
        case sNUM_INT: return "número_inteiro";
        case sNUM_REAL: return "número_real";
        case sCARACTER: return "carácter";
        case sSTRING: return "string";
        case sATRIB: return "<-";
        case sSOMA: return "+";
        case sSUBTR: return "-";
        case sMULT: return "*";
        case sDIV: return "/";
        case sMAIOR: return ">";
        case sMAIOR_IGUAL: return ">=";
        case sMENOR: return "<";
        case sMENOR_IGUAL: return "<=";
        case sIGUAL: return "=";
        case sDIFERENTE: return "<>";
        case sABRE_PARENT: return "(";
        case sFECHA_PARENT: return ")";
        case sABRE_COLCH: return "[";
        case sFECHA_COLCH: return "]";
        case sPONTO: return ".";
        case sVIRG: return ",";
        case sPONTO_VIRG: return ";";
        case sEOF: return "EOF";
        case sERRO_LEXICO: return "erro_lexico";
        default: return "<desconhecido>";
    }
}

static const char* token_enum_name(TipoAtomo t){
    switch(t){
        case sPRG: return "sPRG";
        case sVAR: return "sVAR";
        case sSUBROT: return "sSUBROT";
        case sRETURN: return "sRETURN";
        case sINT: return "sINT";
        case sFLOAT: return "sFLOAT";
        case sCHAR: return "sCHAR";
        case sVOID: return "sVOID";
        case sBEGIN: return "sBEGIN";
        case sEND: return "sEND";
        case sWRITE: return "sWRITE";
        case sREAD: return "sREAD";
        case sIF: return "sIF";
        case sTHEN: return "sTHEN";
        case sELSE: return "sELSE";
        case sFOR: return "sFOR";
        case sWHILE: return "sWHILE";
        case sREPEAT: return "sREPEAT";
        case sUNTIL: return "sUNTIL";
        case sAND: return "sAND";
        case sOR: return "sOR";
        case sNOT: return "sNOT";
        case sIDENT: return "sIDENT";
        case sNUM_INT: return "sNUM_INT";
        case sNUM_REAL: return "sNUM_REAL";
        case sCARACTER: return "sCARACTER";
        case sSTRING: return "sSTRING";
        case sATRIB: return "sATRIB";
        case sSOMA: return "sSOMA";
        case sSUBTR: return "sSUBTR";
        case sMULT: return "sMULT";
        case sDIV: return "sDIV";
        case sMAIOR: return "sMAIOR";
        case sMAIOR_IGUAL: return "sMAIOR_IGUAL";
        case sMENOR: return "sMENOR";
        case sMENOR_IGUAL: return "sMENOR_IGUAL";
        case sIGUAL: return "sIGUAL";
        case sDIFERENTE: return "sDIFERENTE";
        case sABRE_PARENT: return "sABRE_PARENT";
        case sFECHA_PARENT: return "sFECHA_PARENT";
        case sABRE_COLCH: return "sABRE_COLCH";
        case sFECHA_COLCH: return "sFECHA_COLCH";
        case sPONTO: return "sPONTO";
        case sVIRG: return "sVIRG";
        case sPONTO_VIRG: return "sPONTO_VIRG";
        case sEOF: return "sEOF";
        case sERRO_LEXICO: return "sERRO_LEXICO";
        default: return "sDESCONHECIDO";
    }
}

void verifica(TipoAtomo atomo){
    expected_atomo = atomo;
    if(atomo == lookahead.tipo){
        /* consumiu token esperado: limpa o esperado e pega o próximo */
        last_linha_token = lookahead.linha;
        expected_atomo = -1;
        expected_msg[0] = '\0';
        /* atualizar contador de parênteses quando consumimos eles */
        if (atomo == sABRE_PARENT) {
            open_paren_count++;
        }
        if (atomo == sFECHA_PARENT && open_paren_count > 0) {
            open_paren_count--;
        }
        lookahead = obter_atomo();
    }else{
        parse_erro();
        /* Recuperação de erro: pular tokens até encontrar um ponto seguro */
        if (atomo == sPONTO_VIRG) {
            while (lookahead.tipo != sPONTO_VIRG && lookahead.tipo != sEOF && lookahead.tipo != sEND) {
                lookahead = obter_atomo();
            }
            if (lookahead.tipo == sPONTO_VIRG) {
                last_linha_token = lookahead.linha;
                lookahead = obter_atomo();
            }
        }
    }
}

void parse_ini() {
    char *numero = NULL;
    int n_var = 0;
    TInfoAtomo temp;
    
    verifica(sPRG);
    temp = lookahead;
    parse_id();
    ts_inserir(temp.lexema.string, CAT_PROGRAMA, TIPO_VOID);
    gera_instr_mepa(NULL, "INPP", NULL, NULL);
    
    set_expected_msg(";");
    verifica(sPONTO_VIRG);
    
    if (lookahead.tipo == sVAR){
        numero = (char *)calloc(4, sizeof(char));
        n_var = parse_dcl();
        sprintf(numero, "%d", n_var);
        gera_instr_mepa(NULL, "AMEM", numero, NULL);
    }

    parse_bco();
    verifica(sPONTO);
    
    if (numero != NULL) {
        gera_instr_mepa(NULL, "DMEM", numero, NULL);
        free(numero);
    }
    gera_instr_mepa(NULL, "PARA", NULL, NULL);
}

void parse_id(){
    expected_atomo = sIDENT;
    expected_msg[0] = '\0';
    if (lookahead.tipo != sIDENT) {
        parse_erro();
    }
    verifica(sIDENT);
}

int parse_dcl(){
    int qte = 0;
    TInfoAtomo temp;
    TipoSimbolo tipo;
    
    verifica(sVAR);

    while (lookahead.tipo == sINT || lookahead.tipo == sFLOAT) {
        tipo = (lookahead.tipo == sINT) ? TIPO_INT : TIPO_FLOAT;
        verifica(lookahead.tipo);
        
        temp = lookahead;
        parse_id();
        ts_inserir(temp.lexema.string, CAT_VARIAVEL, tipo);
        qte++;

        while (lookahead.tipo == sVIRG) {
            verifica(sVIRG);
            temp = lookahead;
            parse_id();
            ts_inserir(temp.lexema.string, CAT_VARIAVEL, tipo);
            qte++;
        }
        set_expected_msg(";");
        verifica(sPONTO_VIRG);
    }
    return qte;
}

void parse_bco(){
    verifica(sBEGIN);
    while(lookahead.tipo != sEND && lookahead.tipo != sEOF){
        parse_cmd();
        set_expected_msg(";");
        verifica(sPONTO_VIRG);
    }
    verifica(sEND);
}

void parse_erro(){
    error_count++;
    const char *found_token_name = token_to_string(lookahead.tipo);
    const char *found_enum = token_enum_name(lookahead.tipo);
    char found_repr[512];
    
    int erro_linha = last_linha_token;
    if (lookahead.linha > last_linha_token) {
        erro_linha = last_linha_token;
    } else {
        erro_linha = lookahead.linha;
    }
    
    if (lookahead.tipo == sEOF) {
        snprintf(found_repr, sizeof(found_repr), "<fim de arquivo>");
    } else if (lookahead.linha > last_linha_token) {
        snprintf(found_repr, sizeof(found_repr), "<fim de linha>");
    } else if (lookahead.lexema.string[0] != '\0') {
        snprintf(found_repr, sizeof(found_repr), "%s", lookahead.lexema.string);
    } else {
        snprintf(found_repr, sizeof(found_repr), "%s (%s)", found_token_name, found_enum);
    }

    if (open_paren_count > 0) {
        fprintf(stderr, "Erro (%d): Esperado token ')' encontrado '%s'\n", erro_linha, found_repr);
    } else if(expected_stack_top > 0){
        const char *stk = expected_stack[expected_stack_top - 1];
        fprintf(stderr, "Erro (%d): Esperado token '%s' encontrado '%s'\n", erro_linha, stk, found_repr);
    } else if(expected_msg[0] != '\0'){
        fprintf(stderr, "Erro (%d): Esperado token(s) '%s' encontrado '%s'\n", erro_linha, expected_msg, found_repr);
    } else {
        const char *exp = (expected_atomo >= 0) ? token_to_string((TipoAtomo)expected_atomo) : "<desconhecido>";
        fprintf(stderr, "Erro (%d): Esperado token '%s' encontrado '%s'\n", erro_linha, exp, found_repr);
    }
}

int parse_get_error_count(){
    return error_count;
}

void parse_tpo(){
    verifica(sINT);
}

void parse_cmd(){
    if (lookahead.tipo == sINT || lookahead.tipo == sFLOAT) {
        TipoSimbolo tipo = (lookahead.tipo == sINT) ? TIPO_INT : TIPO_FLOAT;
        verifica(lookahead.tipo);
        TInfoAtomo info_var = lookahead;
        set_expected_msg("identificador");
        verifica(sIDENT);
        ts_inserir(info_var.lexema.string, CAT_VARIAVEL, tipo);
        while (lookahead.tipo == sVIRG) {
            verifica(sVIRG);
            info_var = lookahead;
            set_expected_msg("identificador");
            verifica(sIDENT);
            ts_inserir(info_var.lexema.string, CAT_VARIAVEL, tipo);
        }
        return;
    }

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
    push_expected(")");
    parse_e();
    gera_instr_mepa(NULL, "IMPR", NULL, NULL);
    
    while(lookahead.tipo == sVIRG){
        verifica(sVIRG);
        parse_e();
        gera_instr_mepa(NULL, "IMPR", NULL, NULL);
    }
    verifica(sFECHA_PARENT);
    pop_expected();
}

void parse_rd(){
    verifica(sREAD);
    verifica(sABRE_PARENT);
    push_expected(")");
    
    gera_instr_mepa(NULL, "LEIT", NULL, NULL);
    RegistroTS *temp = ts_buscar(lookahead.lexema.string);
    char *endereco = (char *)calloc(30, sizeof(char));
    if (temp) {
        sprintf(endereco, "%d", temp->endereco);
    }
    
    parse_id();
    gera_instr_mepa(NULL, "ARMZ", "0", endereco);
    
    verifica(sFECHA_PARENT);
    pop_expected();
    free(endereco);
}

void parse_if(){
    char *label1 = novo_rotulo();
    char *label2 = novo_rotulo();
    
    verifica(sIF);
    verifica(sABRE_PARENT);
    push_expected(")");
    parse_exp();
    gera_instr_mepa(NULL, "DSVF", label1, NULL);
    verifica(sFECHA_PARENT);
    pop_expected();
    
    verifica(sTHEN);
    parse_cmd();
    gera_instr_mepa(NULL, "DSVS", label2, NULL);
    
    if(lookahead.tipo == sELSE){
        verifica(sELSE);
        gera_instr_mepa(label1, NULL, NULL, NULL);
        parse_cmd();
    }
    gera_instr_mepa(label2, NULL, NULL, NULL);
}

void parse_fr(){
    char *label1 = novo_rotulo();
    char *label2 = novo_rotulo();
    char *label3 = novo_rotulo();
    char *label4 = novo_rotulo();
    
    verifica(sFOR);
    verifica(sABRE_PARENT);
    push_expected(")");
    
    if(lookahead.tipo != sPONTO_VIRG){
        parse_atr();
        gera_instr_mepa(label1, NULL, NULL, NULL);
    }
    
    set_expected_msg(";");
    verifica(sPONTO_VIRG);
    parse_exp();
    gera_instr_mepa(NULL, "DSVF", label4, NULL);
    gera_instr_mepa(NULL, "DSVS", label3, NULL);
    gera_instr_mepa(label2, NULL, NULL, NULL);
    
    set_expected_msg(";");
    verifica(sPONTO_VIRG);
    
    if(lookahead.tipo != sFECHA_PARENT){
        parse_atr();
        gera_instr_mepa(NULL, "DSVS", label1, NULL);
        gera_instr_mepa(label3, NULL, NULL, NULL);
    }
    verifica(sFECHA_PARENT);
    pop_expected();
    
    parse_cmd();
    gera_instr_mepa(NULL, "DSVS", label2, NULL);
    gera_instr_mepa(label4, NULL, NULL, NULL);
}

void parse_wh(){
    char *label1 = novo_rotulo();
    char *label2 = novo_rotulo();
    
    verifica(sWHILE);
    gera_instr_mepa(label1, NULL, NULL, NULL);
    verifica(sABRE_PARENT);
    push_expected(")");
    parse_exp();
    gera_instr_mepa(NULL, "DSVF", label2, NULL);
    verifica(sFECHA_PARENT);
    pop_expected();
    
    parse_cmd();
    gera_instr_mepa(NULL, "DSVS", label1, NULL);
    gera_instr_mepa(label2, NULL, NULL, NULL);
}

void parse_rpt(){
    char *label1 = novo_rotulo();
    gera_instr_mepa(label1, NULL, NULL, NULL);
    
    verifica(sREPEAT);
    while(lookahead.tipo != sUNTIL){
        parse_cmd();
        set_expected_msg(";");
        verifica(sPONTO_VIRG);
    }
    verifica(sUNTIL);
    verifica(sABRE_PARENT);
    push_expected(")");
    parse_exp();
    gera_instr_mepa(NULL, "NEGA", NULL, NULL);
    gera_instr_mepa(NULL, "DSVF", label1, NULL);
    verifica(sFECHA_PARENT);
    pop_expected();
}

void parse_atr(){
    RegistroTS *temp = ts_buscar(lookahead.lexema.string);
    char *endereco = (char *)calloc(30, sizeof(char));
    if (temp) {
        sprintf(endereco, "%d", temp->endereco);
    } else {
        /* Declaração implícita em atribuição */
        ts_inserir(lookahead.lexema.string, CAT_VARIAVEL, TIPO_INT);
        temp = ts_buscar(lookahead.lexema.string);
        if (temp) {
            sprintf(endereco, "%d", temp->endereco);
        }
    }
    
    parse_id();
    verifica(sATRIB);
    parse_exp();
    gera_instr_mepa(NULL, "ARMZ", "0", endereco);
    free(endereco);
}

void parse_ret(){
    verifica(sRETURN);
    parse_exp();
}

void parse_e(){
    if(lookahead.tipo == sNUM_INT){
        char *numero = (char *)calloc(30, sizeof(char));
        sprintf(numero, "%d", lookahead.lexema.valorInt);
        gera_instr_mepa(NULL, "CRCT", numero, NULL);
        verifica(sNUM_INT);
        free(numero);
    } else if(lookahead.tipo == sNUM_REAL){
        char *numero = (char *)calloc(30, sizeof(char));
        sprintf(numero, "%f", lookahead.lexema.valorFloat);
        gera_instr_mepa(NULL, "CRCT", numero, NULL);
        verifica(sNUM_REAL);
        free(numero);
    } else {
        RegistroTS *temp = ts_buscar(lookahead.lexema.string);
        if (!temp) {
            char erro[300];
            snprintf(erro, sizeof(erro), "Identificador '%s' não declarado", lookahead.lexema.string);
            ts_erro(erro);
        }
        char *endereco = (char *)calloc(30, sizeof(char));
        if (temp) {
            sprintf(endereco, "%d", temp->endereco);
        }
        parse_id();
        gera_instr_mepa(NULL, "CVLR", "0", endereco);
        free(endereco);
    }
}

void parse_exp(){
    parse_exps();
    while(lookahead.tipo == sMAIOR || lookahead.tipo == sMAIOR_IGUAL || lookahead.tipo == sMENOR || 
          lookahead.tipo == sMENOR_IGUAL || lookahead.tipo == sIGUAL || lookahead.tipo == sDIFERENTE){
        TipoAtomo tipo = parse_op_cat3();
        parse_exps();
        switch (tipo){
            case sMAIOR:
                gera_instr_mepa(NULL, "CMMA", NULL, NULL);
                break;
            case sMAIOR_IGUAL:
                gera_instr_mepa(NULL, "CMAG", NULL, NULL);
                break;
            case sMENOR:
                gera_instr_mepa(NULL, "CMME", NULL, NULL);
                break;
            case sMENOR_IGUAL:
                gera_instr_mepa(NULL, "CMEG", NULL, NULL);
                break;
            case sIGUAL:
                gera_instr_mepa(NULL, "CMIG", NULL, NULL);
                break;
            case sDIFERENTE:
                gera_instr_mepa(NULL, "CMDG", NULL, NULL);
                break;
            default:
                break;
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
                gera_instr_mepa(NULL, "SOMA", NULL, NULL);
                break;
            case sSUBTR:
                gera_instr_mepa(NULL, "SUBT", NULL, NULL);
                break;
            case sOR:
                gera_instr_mepa(NULL, "DISJ", NULL, NULL);
                break;
            default:
                break;
        }
    }
}

void parse_tmo(){
    parse_ftr();
    while(lookahead.tipo == sMULT || lookahead.tipo == sDIV || lookahead.tipo == sAND){
        TipoAtomo tipo = parse_op_cat1();
        parse_ftr();
        switch (tipo){
            case sMULT:
                gera_instr_mepa(NULL, "MULT", NULL, NULL);
                break;
            case sDIV:
                gera_instr_mepa(NULL, "DIVI", NULL, NULL);
                break;
            case sAND:
                gera_instr_mepa(NULL, "CONJ", NULL, NULL);
                break;
            default:
                break;
        }
    }
}

void parse_ftr(){
    if(lookahead.tipo == sNOT){
        verifica(sNOT);
        parse_ftr();
    } else if(lookahead.tipo == sABRE_PARENT){
        verifica(sABRE_PARENT);
        push_expected(")");
        parse_exp();
        verifica(sFECHA_PARENT);
        pop_expected();
    } else {
        parse_e();
    }
}

TipoAtomo parse_op_cat3(){
    switch (lookahead.tipo){
        case sMAIOR:
            verifica(sMAIOR);
            return sMAIOR;
        case sMAIOR_IGUAL:
            verifica(sMAIOR_IGUAL);
            return sMAIOR_IGUAL;
        case sMENOR:
            verifica(sMENOR);
            return sMENOR;
        case sMENOR_IGUAL:
            verifica(sMENOR_IGUAL);
            return sMENOR_IGUAL;
        case sIGUAL:
            verifica(sIGUAL);
            return sIGUAL;
        case sDIFERENTE:
            verifica(sDIFERENTE);
            return sDIFERENTE;
        default:
            parse_erro();
            return sERRO_LEXICO;
    }
}

TipoAtomo parse_op_cat2(){
    switch (lookahead.tipo){
        case sSOMA:
            verifica(sSOMA);
            return sSOMA;
        case sSUBTR:
            verifica(sSUBTR);
            return sSUBTR;
        case sOR:
            verifica(sOR);
            return sOR;
        default:
            parse_erro();
            return sERRO_LEXICO;
    }
}

TipoAtomo parse_op_cat1(){
    switch (lookahead.tipo){
        case sMULT:
            verifica(sMULT);
            return sMULT;
        case sDIV:
            verifica(sDIV);
            return sDIV;
        case sAND:
            verifica(sAND);
            return sAND;
        default:
            parse_erro();
            return sERRO_LEXICO;
    }
}
