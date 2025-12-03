
#ifndef ASDR_H
#define ASDR_H

/*///////////////////////
VARIÁVEIS GLOBAIS
COMPARTILHADAS
///////////////////////*/
extern TInfoAtomo lookahead;

/*///////////////////////
PROTÓTIPOS
///////////////////////*/
void parse_erro();
void parse_ini();
void parse_id();
int parse_dcl();
void parse_bco();
void parse_tpo();
void parse_cmd();
void parse_wr();
void parse_rd();
void parse_if();
void parse_fr();
void parse_wh();
void parse_rpt();
void parse_atr();
void parse_ret();
void parse_e();
void parse_exp();
void parse_exps();
void parse_tmo();
void parse_ftr();
TipoAtomo parse_op_cat3();
TipoAtomo parse_op_cat2();
TipoAtomo parse_op_cat1();


#endif
