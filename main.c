#include <stdio.h>
#include <string.h>
#include "analex.h"
#include "asdr.h"
#include "tabsimb.h"

int main(int argc, char *argv[]) {
    FILE *resultado;
    resultado = fopen("compilado.mepa", "w");

    if (resultado==NULL){
        printf("Error: Could not create/open the file.\n");
        return 1; // Indicate an error
    }
    fprintf(resultado,"TESTE");
    fputs("SUPER TESTE",resultado);

    fclose(resultado);

    if (argc < 2) {
        printf("Uso: %s <arquivo fonte>\n", argv[0]);
        return 1;
    }

    fonte = fopen(argv[1], "r");
    if (!fonte) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    char nome_base[256];
    strcpy(nome_base, argv[1]);
    
    // Remove extensão .lpd se existir
    char *ponto = strrchr(nome_base, '.');
    if (ponto != NULL) {
        *ponto = '\0';
    }

    // Bootstrap: carrega o primeiro token
    lookahead = obter_atomo(); 
    
    // Inicia a análise sintática
    parse_ini();

    // Verifica se houve erros de compilação
    int err_count = parse_get_error_count();
    if (err_count > 0) {
        fprintf(stderr, "Compilação falhou com %d erro(s).\n", err_count);
        fclose(fonte);
        return 1;
    }

    // Sucesso! Gerar arquivo de saída
    printf("Código compilado com sucesso!\n");
    ts_imprimir_arquivo(nome_base);
    
    fclose(fonte);
    return 0;
}
