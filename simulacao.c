#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

void registrarDados(const char *nome_arquivo, double tempo_decorrido, double ocupacao, double lambda, double en_final, double ew_final, double erro_de_little)
{
    FILE *arquivo = fopen(nome_arquivo, "a"); // Abre o arquivo em modo append

    if (arquivo != NULL)
    {
        // Escreve o cabeçalho da tabela uma vez
        if (ftell(arquivo) == 0)
        { // Se o arquivo estiver vazio
            fprintf(arquivo, "Tempo_Decorrido Ocupacao Lambda E[W] E[N] Erro_de_Little\n");
        }

        // Escreve os dados separados por um único espaço
        fprintf(arquivo, "%f %f %f %f %f %f\n",
                tempo_decorrido, ocupacao, lambda, en_final, ew_final, erro_de_little);

        fclose(arquivo);
    }
    else
    {
        fprintf(stderr, "Erro ao abrir o arquivo %s!\n", nome_arquivo);
    }
}

typedef struct
{
    unsigned long int num_eventos;
    double tempo_anterior;
    double soma_areas;
} little;

double uniforme()
{
    double u = rand() / ((double)RAND_MAX + 1);
    u = 1.0 - u; // limitando u entre (0,1]
    return u;
}

double gera_tempo(double l)
{
    return (-1.0 / l) * log(uniforme());
}

double min(double n1, double n2)
{
    return (n1 < n2) ? n1 : n2;
}

void inicia_little(little *n)
{
    n->num_eventos = 0;
    n->soma_areas = 0.0;
    n->tempo_anterior = 0.0;
}

int main()
{
    srand(777);

    // Definindo a taxa média de chegada
    double parametro_chegada = 10; // Chegadas por segundo

    // Taxas de saída para diferentes percentuais
    double percentuais[] = {0.85, 0.90, 0.95, 0.99};
    double parametro_saida[4];

    // Calculando a taxa de saída correspondente
    for (int i = 0; i < 4; i++)
    {
        parametro_saida[i] = parametro_chegada / percentuais[i]; // Calcula a taxa de saída
    }

    double tempo_simulacao = 100000;

    // Loop para cada taxa de saída calculada
    for (int i = 0; i < 4; i++)
    {
        double tempo_decorrido = 0.0;
        double tempo_chegada = gera_tempo(parametro_chegada);
        double tempo_saida = DBL_MAX;
        double tempo_coleta = 100;

        unsigned long int fila = 0;
        unsigned long int fila_max = 0;

        double soma_ocupacao = 0.0;

        // Variáveis Little
        little en;
        little ew_chegadas;
        little ew_saidas;

        inicia_little(&en);
        inicia_little(&ew_chegadas);
        inicia_little(&ew_saidas);

        printf("\nSimulação com taxa de saída para %.0f%%: %.2f\n", percentuais[i] * 100, parametro_saida[i]);

        while (tempo_decorrido <= tempo_simulacao)
        {
            tempo_decorrido = min(tempo_chegada, min(tempo_saida, tempo_coleta));

            // Chegada
            if (tempo_decorrido == tempo_chegada)
            {
                if (!fila) // Sistema ocioso?
                {
                    tempo_saida = tempo_decorrido + gera_tempo(parametro_saida[i]);
                    soma_ocupacao += tempo_saida - tempo_decorrido;
                }
                fila++;
                fila_max = fila > fila_max ? fila : fila_max;

                tempo_chegada = tempo_decorrido + gera_tempo(parametro_chegada);

                // Little
                en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
                en.num_eventos++;
                en.tempo_anterior = tempo_decorrido;

                ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
                ew_chegadas.num_eventos++;
                ew_chegadas.tempo_anterior = tempo_decorrido;
            }
            else if (tempo_decorrido == tempo_saida)
            {
                fila--;
                tempo_saida = DBL_MAX;
                if (fila)
                {
                    tempo_saida = tempo_decorrido + gera_tempo(parametro_saida[i]);
                    soma_ocupacao += tempo_saida - tempo_decorrido;
                }

                // Little
                en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
                en.num_eventos--;
                en.tempo_anterior = tempo_decorrido;

                ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
                ew_saidas.num_eventos++;
                ew_saidas.tempo_anterior = tempo_decorrido;
            }
            else if (tempo_decorrido == tempo_coleta)
            {
                // Coleta
                en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;

                ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
                ew_chegadas.tempo_anterior = tempo_decorrido;

                ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
                ew_saidas.tempo_anterior = tempo_decorrido;

                en.tempo_anterior = tempo_decorrido;

                // Cálculos
                double ocupacao = soma_ocupacao / tempo_decorrido;
                double en_final = en.soma_areas / tempo_decorrido;
                double ew_final = (ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos;
                double lambda = ew_chegadas.num_eventos / tempo_decorrido;
                double erro_de_little = en_final - lambda * ew_final;

                // Nome do arquivo para salvar os dados
                char nome_arquivo[30];
                sprintf(nome_arquivo, "dados_simulacao_%d.txt", i + 1);

                // Chamada para registrar os dados
                registrarDados(nome_arquivo, tempo_decorrido, ocupacao, lambda, en_final, ew_final, erro_de_little);

                // Atualiza tempo de coleta
                tempo_coleta += 100.0;
            }
        }

        // Finalizando as somas para o cálculo médio
        ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
        ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;

        printf("Maior tamanho de fila alcançado: %lu\n", fila_max);
        printf("Ocupação: %.2f\n", soma_ocupacao / tempo_decorrido);
        double en_final = en.soma_areas / tempo_decorrido;
        double ew_final = (ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos;
        double lambda = ew_chegadas.num_eventos / tempo_decorrido;

        printf("E[N]: %.2f\n", en_final);
        printf("E[W]: %.2f\n", ew_final);
        printf("Erro de Little: %.20f\n", en_final - lambda * ew_final);
    }

    return 0;
}
