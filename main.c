#include <stdio.h>
#include "analex.h"
#include "asdr.h"

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

    // Bootstrap: carrega o primeiro token
    lookahead = obter_atomo(); 
    
    // Inicia a análise sintática
    parse_ini();

    // Se o último token é EOF, sucesso!
    if (lookahead.tipo == sEOF)
        printf("Código compilado com sucesso!\n");
    
    fclose(fonte);
    return 0;
}
