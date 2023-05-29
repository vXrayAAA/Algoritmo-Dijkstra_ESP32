#include <stdio.h>
#include <limits.h>
#include "esp_log.h"
#include "geralv2r1.h"


#define V 9 // Define o número de vértices no grafo


/* Saida */
#define ___gpoCK	GPIO_NUM_12                       			                // Seleciona o pino de 'clock' para o registrador.
#define ___gpoDT	GPIO_NUM_27                       			                // Seleciona o pino de 'data out' para o registrador.
#define ___gpoLD	GPIO_NUM_14                       			                // Seleciona o pino de 'load' para o registrador.

static const char *TAG = "N1K";

unsigned char ___charVlr=0;														// Variavel global para recerregar o valor da saida.




/**
	@brief Limpa os pinos do registrador(74HC595).
*/
void __regClear(void)															// Limpa o registrador.
{
	/* Limpa o registrador */
	unsigned char __tmp010;									                    // Var local temporaria.					
	for(__tmp010=0;__tmp010<8;__tmp010++)                                       // Laco para zerar o registrador.
	{
		gpio_set_level(___gpoCK,1);									            // Gera um pulso de clock no registrador.
		gpio_set_level(___gpoCK,0);                                             // 
	}
}

/* Bloco Inicio: GPIOs */

/**
*	@brief Inicializa os pinos do hardware e limpa os pinos do registrador(74HC595) da saida. Obrigatorio antes da funcao de escrita.
*/
void gpoIniciar(void)															// Inicializa o hardware da saida.
{
    gpio_reset_pin(___gpoCK);													// Limpa configuracoes anteriores.
	gpio_reset_pin(___gpoDT);													// Limpa configuracoes anteriores.
	gpio_reset_pin(___gpoLD);													// Limpa configuracoes anteriores.
    gpio_set_direction(___gpoCK, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(___gpoDT, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(___gpoLD, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
	gpio_set_level(___gpoCK,0);													// Limpa o pino.
	gpio_set_level(___gpoDT,0);													// Limpa o pino.
	gpio_set_level(___gpoLD,0);													// Limpa o pino.
	__regClear();																// Limpa conteudo do registrador.
}


void gpoDado(char vlrSaida)														// Envia um dado para o GPO (saida).
{
	unsigned char __tmp001;														// Var local temporaria.
	___charVlr=vlrSaida;														// Salva uma copia do valor da saida.  
	for(__tmp001=0;__tmp001<8;__tmp001++)										// Laco para serializar.
	{
		if(bitX(vlrSaida,(7-__tmp001)))gpio_set_level(___gpoDT,1);				// Verifica o valor do bit, se 1...
		else gpio_set_level(___gpoDT,0);										// ... e se for 0.				
		gpio_set_level(___gpoCK,1);									            // Gera um pulso de clock no registrador.
		gpio_set_level(___gpoCK,0);                                             // ...
	}							
	gpio_set_level(___gpoCK,0);													// Limpa o pino.
	gpio_set_level(___gpoDT,0);													// Limpa o pino.
	gpio_set_level(___gpoLD,1); 							                    // Gera um pulso para carregar o dado.
	gpio_set_level(___gpoLD,0);                                                 // ...
}


int minDistance(int dist[], bool sptSet[])          // Função para encontrar o vértice com a menor distância, a partir do conjunto de vértices ainda não processados
{
    int min = INT_MAX, min_index;

    for (int v = 0; v < V; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}


void printSolution(int dist[])      // Função para imprimir a matriz de distâncias calculada
{
    ESP_LOGI("Dijkstra", "Vertex \t Distância da Fonte");
    for (int i = 0; i < V; i++)
        ESP_LOGI("Dijkstra", "%d \t\t %d", i, dist[i]);
}


void dijkstra(int graph[V][V], int src)                 // Função que implementa o algoritmo de Dijkstra para um grafo representado por uma matriz de adjacência
{
    int dist[V]; // A matriz de distâncias. dist[i] conterá a menor distância de src até i

    bool sptSet[V]; // sptSet[i] será verdadeiro se o vértice i estiver incluído no caminho mais curto

    
    for (int i = 0; i < V; i++)                     // Inicializa todas as distâncias como infinito e sptSet[] como falso
        dist[i] = INT_MAX, sptSet[i] = false;

    // A distância do vértice de origem para si mesmo é sempre 0
    dist[src] = 0;

    // Encontre o caminho mais curto para todos os vértices
    for (int count = 0; count < V - 1; count++) {
        // Escolhe o vértice de menor distância do conjunto de vértices ainda não processados
        int u = minDistance(dist, sptSet);

        // Marca o vértice escolhido como processado
        sptSet[u] = true;

        // Atualiza o valor da distância dos vértices adjacentes ao vértice escolhido
        for (int v = 0; v < V; v++)
            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v])
                dist[v] = dist[u] + graph[u][v];
    }

    // Imprime a matriz de distâncias calculada
    printSolution(dist);

    gpoDado(0x0F);
    ESP_LOGI(TAG,"numero de saidas ligadas: 0x0F");


}

// Função principal para executar o algoritmo de Dijkstra
void app_main(void) {
    int grafico[V][V] = {
        {0, 4, 0, 0, 0, 0, 0, 8, 0},
        {4, 0, 8, 0, 0, 0, 0, 11, 0},
        {0, 8, 0, 7, 0, 4, 0, 0, 2},
        {0, 0, 7, 0, 9, 14, 0, 0, 0},
        {0, 0, 0, 9, 0, 10, 0, 0, 0},
        {0, 0, 4, 14, 10, 0, 2, 0, 0},
        {0, 0, 0, 0, 0, 2, 0, 1, 6},
        {8, 11, 0, 0, 0, 0, 1, 0, 7},
        {0, 0, 2, 0, 0, 0, 6, 7, 0}
    };

    dijkstra(grafico, 0);
}
