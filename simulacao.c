#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

void registrarDados(double tempo_decorrido, double taxa_de_entrada, double taxa_de_saida, double lambda, double en_final, double ew_final, double erro_de_little)
{
    FILE *arquivo = fopen("dados_coleta.txt", "a"); // Abre o arquivo em modo append

    if (arquivo != NULL)
    {
        // Escreve o cabeçalho da tabela uma vez
        if (ftell(arquivo) == 0)
        { // Se o arquivo estiver vazio
            fprintf(arquivo, "Tempo_Decorrido Taxa_de_Entrada Taxa_de_Saída Lambda E[W] E[N] Erro_de_Little\n");
        }

        // Escreve os dados separados por um único espaço
        fprintf(arquivo, "%f %f %f %f %f %f %f\n",
                tempo_decorrido, taxa_de_entrada, taxa_de_saida,
                lambda, ew_final, en_final, erro_de_little);

        fclose(arquivo);
    }
    else
    {
        fprintf(stderr, "Erro ao abrir o arquivo!\n");
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
    // u == 0 --> ln(u) <-- problema
    // limitando u entre (0,1]
    u = 1.0 - u;
    return u;
}

double gera_tempo(double l)
{
    return (-1.0 / l) * log(uniforme());
}

double min(double n1, double n2)
{
    if (n1 < n2)
        return n1;
    return n2;
}

void inicia_little(little *n)
{
    n->num_eventos = 0;
    n->soma_areas = 0.0;
    n->tempo_anterior = 0.0;
}

int main()
{
    srand(time(NULL));
    double parametro_chegada;
    printf("Informe o tempo médio entre as chegadas (s): ");
    scanf("%lF", &parametro_chegada);
    parametro_chegada = 1.0 / parametro_chegada;

    double parametro_saida;
    printf("Informe o tempo médio de atendimento (s): ");
    scanf("%lF", &parametro_saida);
    parametro_saida = 1.0 / parametro_saida;

    double tempo_simulacao;
    printf("Informe o tempo de simulacao (s): ");
    scanf("%lF", &tempo_simulacao);

    double tempo_decorrido = 0.0;

    double tempo_chegada = gera_tempo(parametro_chegada);
    double tempo_saida = DBL_MAX;
    double tempo_coleta = 100;

    unsigned long int fila = 0;
    unsigned long int fila_max = 0;

    double soma_ocupacao = 0.0;

    /**
     * variaveis little
     */

    little en;
    little ew_chegadas;
    little ew_saidas;
    inicia_little(&en);
    inicia_little(&ew_chegadas);
    inicia_little(&ew_saidas);

    while (tempo_decorrido <= tempo_simulacao)
    {
        tempo_decorrido =
            min(tempo_chegada, min(tempo_saida, tempo_coleta));

        // chegada
        if (tempo_decorrido == tempo_chegada)
        {
            // sistema esta ocioso?
            if (!fila)
            {
                tempo_saida =
                    tempo_decorrido +
                    gera_tempo(parametro_saida);

                soma_ocupacao += tempo_saida -
                                 tempo_decorrido;
            }
            fila++;
            fila_max = fila > fila_max ? fila : fila_max;

            tempo_chegada =
                tempo_decorrido +
                gera_tempo(parametro_chegada);

            /**
             * little
             */
            en.soma_areas += (tempo_decorrido -
                              en.tempo_anterior) *
                             en.num_eventos;
            en.num_eventos++;
            en.tempo_anterior = tempo_decorrido;

            ew_chegadas.soma_areas +=
                (tempo_decorrido -
                 ew_chegadas.tempo_anterior) *
                ew_chegadas.num_eventos;
            ew_chegadas.num_eventos++;
            ew_chegadas.tempo_anterior =
                tempo_decorrido;
        }
        else if (tempo_decorrido == tempo_saida)
        {
            fila--;
            tempo_saida = DBL_MAX;
            // tem mais requisicoes na fila?
            if (fila)
            {
                tempo_saida =
                    tempo_decorrido +
                    gera_tempo(parametro_saida);

                soma_ocupacao += tempo_saida -
                                 tempo_decorrido;
            }

            /**
             * little
             */
            en.soma_areas += (tempo_decorrido -
                              en.tempo_anterior) *
                             en.num_eventos;
            en.num_eventos--;
            en.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas +=
                (tempo_decorrido -
                 ew_saidas.tempo_anterior) *
                ew_saidas.num_eventos;
            ew_saidas.num_eventos++;
            ew_saidas.tempo_anterior = tempo_decorrido;
        }
        else if (tempo_decorrido == tempo_coleta)
        {
            // Coleta
            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;

            ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_chegadas.num_eventos++;
            ew_chegadas.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
            ew_saidas.num_eventos++;
            ew_saidas.tempo_anterior = tempo_decorrido;

            ew_saidas.num_eventos++;
            ew_saidas.tempo_anterior = tempo_decorrido;

            // Cálculos
            double taxa_de_entrada = ew_chegadas.num_eventos / tempo_decorrido;
            double taxa_de_saida = ew_saidas.num_eventos / tempo_decorrido;
            double lambda = ew_chegadas.num_eventos / tempo_decorrido;
            double en_final = en.soma_areas / tempo_decorrido;
            double ew_final = (ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos;
            double erro_de_little = en_final - lambda * ew_final;
            double taxa_de_ocupacao = soma_ocupacao / tempo_decorrido;

            // Chamada para registrar os dados
            registrarDados(tempo_decorrido, taxa_de_entrada, taxa_de_saida, lambda, en_final, ew_final, erro_de_little);

            // Atualiza tempo de coleta
            tempo_coleta += 100.0;
        }
    }

    en.soma_areas += (tempo_decorrido -
                      en.tempo_anterior) *
                     en.num_eventos;

    ew_chegadas.soma_areas +=
        (tempo_decorrido -
         ew_chegadas.tempo_anterior) *
        ew_chegadas.num_eventos;

    ew_saidas.soma_areas +=
        (tempo_decorrido -
         ew_saidas.tempo_anterior) *
        ew_saidas.num_eventos;

    printf("Maior tamanho de fila alcancado: %ld\n", fila_max);
    printf("Ocupacao: %lF\n", soma_ocupacao / tempo_decorrido);
    double en_final = en.soma_areas / tempo_decorrido;
    double ew_final = (ew_chegadas.soma_areas -
                       ew_saidas.soma_areas) /
                      ew_chegadas.num_eventos;
    double lambda = ew_chegadas.num_eventos /
                    tempo_decorrido;

    printf("E[N]: %lF\n", en_final);
    printf("E[W]: %lF\n", ew_final);
    printf("Erro de Little: %lF\n",
           en_final - lambda * ew_final);

    return 0;
}

/**
 *
 *
 *
 *
 */