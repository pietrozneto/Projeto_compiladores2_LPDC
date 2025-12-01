#include "analex.h"
#include "asdr.h"
#include "tabsimb.h"

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

static void set_expected_msg(const char *msg){
    if(msg && msg[0]){
        strncpy(expected_msg, msg, sizeof(expected_msg)-1);
        expected_msg[sizeof(expected_msg)-1] = '\0';
    } else {
        expected_msg[0] = '\0';
    }
}

// pilha de expectativas
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
        // consumiu token esperado: limpa o esperado e pega o próximo
        last_linha_token = lookahead.linha;  // rastreia a linha do último token consumido
        expected_atomo = -1;
        expected_msg[0] = '\0';
        // atualizar contador de parênteses quando consumimos eles
        if (atomo == sABRE_PARENT) {
            open_paren_count++;
        }
        if (atomo == sFECHA_PARENT && open_paren_count > 0) {
            open_paren_count--;
        }
        lookahead = obter_atomo();
    }else{
        parse_erro();
        /* Recuperação de erro: pular tokens até encontrar um ponto seguro
           (por exemplo, um ponto-e-vírgula que marca fim de comando) */
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
    verifica(sPRG);

    TInfoAtomo info_prog = lookahead;
    verifica(sIDENT);

    ts_iniciar();
    ts_inserir(info_prog.lexema.string, CAT_PROGRAMA, TIPO_VOID);

    set_expected_msg(";");
    set_expected_msg(";");
    verifica(sPONTO_VIRG);

    if (lookahead.tipo == sVAR){
        parse_dcl(); //processa declarações de variáveis
    }

    parse_bco();
    verifica(sPONTO);
}

void parse_id(){
    // esperamos um identificador aqui
    expected_atomo = sIDENT;
    expected_msg[0] = '\0';
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
    
    /* Determinar a linha para reportar o erro:
       Se lookahead está em uma linha diferente do último token processado,
       usar last_linha_token (onde o erro estava implícito). */
    int erro_linha = last_linha_token;
    if (lookahead.linha > last_linha_token) {
        erro_linha = last_linha_token;
    } else {
        erro_linha = lookahead.linha;
    }
    
    /* Se o lookahead avançou para uma nova linha e esperávamos um token,
       isso significa fim de linha implícito sem aquele token.
       Se chegamos a EOF, mostrar "fim de arquivo". */
    int is_eol_implicit = 0;
    if (lookahead.tipo == sEOF) {
        snprintf(found_repr, sizeof(found_repr), "<fim de arquivo>");
    } else if (lookahead.linha > last_linha_token) {
        is_eol_implicit = 1;
        snprintf(found_repr, sizeof(found_repr), "<fim de linha>");
    } else if (lookahead.lexema.string[0] != '\0') {
        snprintf(found_repr, sizeof(found_repr), "%s", lookahead.lexema.string);
    } else {
        snprintf(found_repr, sizeof(found_repr), "%s (%s)", found_token_name, found_enum);
    }

    if (open_paren_count > 0) {
        {
            size_t n = strlen(lookahead.lexema.string);
            int printable = 0;
            if (n > 0) {
                printable = 1;
                for (size_t i = 0; i < n; ++i) {
                    unsigned char c = (unsigned char)lookahead.lexema.string[i];
                    if (!isprint(c)) { printable = 0; break; }
                }
            }
            if (printable) {
                snprintf(found_repr, sizeof(found_repr), "%s", lookahead.lexema.string);
            } else {
                snprintf(found_repr, sizeof(found_repr), "%s (%s)", token_to_string(lookahead.tipo), token_enum_name(lookahead.tipo));
            }
        }
        // Priorizar mensagem de parêntese aberto não fechado
        fprintf(stderr, "Erro (%d): Esperado token ')' encontrado '%s'\n", erro_linha, found_repr);
    } else if(expected_stack_top > 0){
        const char *stk = expected_stack[expected_stack_top - 1];
        fprintf(stderr, "Erro (%d): Esperado token '%s' encontrado '%s'\n", erro_linha, stk, found_repr);
    } else if(expected_msg[0] != '\0'){
        /* Se a expectativa é ';', mas na linha atual há parenteses abertos não fechados,
           preferir reportar ')' faltando. Lemos a linha do arquivo fonte para contar. */
        if (strcmp(expected_msg, ";") == 0 && fonte != NULL && !is_eol_implicit) {
            long pos = ftell(fonte);
            rewind(fonte);
            char buf[1024];
            int current = 0;
            int opens = 0, closes = 0;
            while (fgets(buf, sizeof(buf), fonte)) {
                current++;
                if (current == lookahead.linha) {
                    for (char *p = buf; *p; ++p) {
                        if (*p == '(') opens++;
                        if (*p == ')') closes++;
                    }
                    break;
                }
            }
            fseek(fonte, pos, SEEK_SET);
            if (opens > closes) {
                    fprintf(stderr, "Erro (%d): Esperado token ')' encontrado '%s'\n", erro_linha, found_repr);
                    return;
            }
        }
            fprintf(stderr, "Erro (%d): Esperado token(s) '%s' encontrado '%s'\n", erro_linha, expected_msg, found_repr);
    } else {
        const char *exp = (expected_atomo >= 0) ? token_to_string((TipoAtomo)expected_atomo) : "<desconhecido>";
            fprintf(stderr, "Erro (%d): Esperado token '%s' encontrado '%s'\n", erro_linha, exp, found_repr);
    }
    // Não saímos aqui — voltamos para o chamador para continuar o parsing
}

int parse_get_error_count(){
    return error_count;
}

void parse_tpo(){
    verifica(sINT);
}

void parse_cmd(){
    /* permitir declarações locais dentro do bloco: int a; */
    if (lookahead.tipo == sINT || lookahead.tipo == sFLOAT) {
        /* declarações locais (mesma lógica de parse_dcl sem o 'var') */
        TipoSimbolo tipo = (lookahead.tipo == sINT) ? TIPO_INT : TIPO_FLOAT;
        verifica(lookahead.tipo); // consome int ou float

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
    while(lookahead.tipo ==sVIRG){
        verifica(sVIRG);
        parse_e();
    }
    verifica(sFECHA_PARENT);
    pop_expected();
}
void parse_rd(){
    verifica(sREAD);
    verifica(sABRE_PARENT);
    push_expected(")");
    parse_id();
    verifica(sFECHA_PARENT);
    pop_expected();
}
void parse_if(){
    verifica(sIF);
    verifica(sABRE_PARENT);
    push_expected(")");
    parse_exp();
    verifica(sFECHA_PARENT);
    pop_expected();
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
    push_expected(")");
    if(lookahead.tipo != sPONTO_VIRG){
        parse_atr();
    }
    set_expected_msg(";");
    verifica(sPONTO_VIRG);
    parse_exp();
    verifica(sPONTO_VIRG);
    if(lookahead.tipo != sFECHA_PARENT){
        parse_atr();
    }
    verifica(sFECHA_PARENT);
    pop_expected();
    parse_cmd();

}
void parse_wh(){
    verifica(sWHILE);
    verifica(sABRE_PARENT);
    push_expected(")");
    parse_exp();
    verifica(sFECHA_PARENT);
    pop_expected();
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
    push_expected(")");
    parse_exp();
    verifica(sFECHA_PARENT);
    pop_expected();
}
void parse_atr(){
    // No lado esquerdo de uma atribuição, o identificador pode ser implicitamente declarado
    expected_atomo = sIDENT;
    expected_msg[0] = '\0';
    if (lookahead.tipo != sIDENT) {
        parse_erro();
    } else {
        // Se o identificador não está declarado, inseri-lo como variável local (tipo INT por padrão)
        if (ts_buscar(lookahead.lexema.string) == NULL) {
            ts_inserir(lookahead.lexema.string, CAT_VARIAVEL, TIPO_INT);
        }
        verifica(sIDENT);
    }
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
        push_expected(")");
        parse_exp();
        verifica(sFECHA_PARENT);
        pop_expected();
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
            set_expected_msg(">, >=, <, <=, =, <>");
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
            set_expected_msg("+, -, or");
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
            set_expected_msg("*, /, and");
            parse_erro();
    }
}
