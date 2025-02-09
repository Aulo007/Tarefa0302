#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bibliotecas/matrizRGB.h"
#include "bibliotecas/desenho.h"
#include "bibliotecas/ssd1306.h"
#include "bibliotecas/menu.h"

// Configuração dos pinos

const uint LED_RED_PIN = 13; // Green = 11, Blue = 12 and Red = 13
const uint LED_BLUE_PIN = 12;
const uint LED_GREEN_PIN = 11;

const uint BUTTON1_PIN = 5; // Define o botão 1 como pino 5
const uint BUTTON2_PIN = 6; // Define o botão 2 como pino 6

// Definindo constantes para os parâmetros do I2C e do display
const uint I2C_PORT = i2c1;
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;
const uint endereco = 0x3C;

// Definição de variáveis globais

static volatile int num;           // Variável global para definir qual número selecionar na matriz
static volatile int num1 = 1;      // Variável global para testar doubounce
static volatile int num2 = 1;      // Variável global para testar doubounce
static volatile int last_time = 0; // Variável global para amarzenar o tempo do último evento

static void gpio_irq_handle(uint gpio, uint32_t events);

int main()
{
    // Inicialização dos pinos
    stdio_init_all();

    gpio_init(LED_RED_PIN);
    gpio_init(BUTTON1_PIN);
    gpio_init(BUTTON2_PIN);

    // Inicializa os pinos da matriz de Leds no pino 7

    npInit(7);

    // Confira direção dos pinos (Entrada ou saída)

    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(BUTTON1_PIN, GPIO_IN);
    gpio_set_dir(BUTTON2_PIN, GPIO_IN);

    /* Configuração para habilitar o pull up interno para os botões, evitando o uso de resistores externos e
    garantindo que o pino esteja em nível alto quando o botão não estiver pressionado. */

    gpio_pull_up(BUTTON1_PIN);
    gpio_pull_up(BUTTON2_PIN);

    // Configuração da interrupção com o callback
    gpio_set_irq_enabled_with_callback(BUTTON1_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handle);
    gpio_set_irq_enabled_with_callback(BUTTON2_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handle);

    // Inicialização do I2C a 400 kHz
    i2c_init(I2C_PORT, 400 * 1000);

    // Configura os pinos GPIO para a função I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Configura o pino de dados para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Configura o pino de clock para I2C
    gpio_pull_up(I2C_SDA);                     // Ativa o pull-up no pino de dados
    gpio_pull_up(I2C_SCL);                     // Ativa o pull-up no pino de clock

    // Inicialização e configuração do display SSD1306
    ssd1306_t ssd;                                                // Cria a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display com as especificações fornecidas
    ssd1306_config(&ssd);                                         // Configura os parâmetros do display
    ssd1306_send_data(&ssd);                                      // Envia os dados iniciais de configuração para o display

    // Limpeza do display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false); // Preenche o display com o valor especificado (false = apagado)
    ssd1306_send_data(&ssd);   // Envia os dados de preenchimento para o display

    // Loop principal

    while (true)
    {
        display_menu();

        char cmd = getchar(); // Aguarda entrada do usuário
        printf("Comando recebido: %c\n\r", cmd);

        switch (cmd)
        {
        case '1':
            break;

        case '2':



        default:
            printf("Comando invalido!\n\r");
            break;
        }

        sleep_ms(500); // Pequeno delay entre comandos
    }
}

static void gpio_irq_handle(uint gpio, uint32_t events)
{
    // Cria uma varíavel que pega o tempo atual do sistema em microsegundos.
    uint32_t current_time_us = to_us_since_boot(get_absolute_time());

    // Print para debbugar o efeito do doubounce

    printf("Sinais de transição lidos pelo sistema: %d \n", num1);
    num1++;

    if (current_time_us - last_time > 200000) // 200 ms de doboucing
    {
        last_time = current_time_us; // atualiza o tempo do último evento.
        if (num >= 0 && num <= 9)
        {
            if (gpio == BUTTON1_PIN)
            {
                /* Ternário (Condição ? valor se verdadeiro: valor se falso), atribui valor a variável volátil num
                   Se o número for menor que 9 ele incrementa o num em 1, se não, faz o valor permanecer em 9 */

                num = (num < 9) ? num + 1 : 9;
            }
            else if (gpio == BUTTON2_PIN)
            {
                /* Ternário (Condição ? valor se verdadeiro: valor se falso), atribui valor a variável volátil num
                  Se o número for maior que 0 ele decrementa o num em 1, se não, faz o valor permanecer em 0 */
                num = (num > 0) ? num - 1 : 0;
            }

            // Print para mostrar quantas vezes eu apertei o botão na plaquinha e realmente rodou o código
            printf("O botão foi realmente apertado: %d \n", num2);
            num2++;

            // Escreve desenho atual na plaquinha, os três parâmetro são sobre a intensidade
            setMatrizDeLEDSComIntensidade(caixa_de_desenhos[num], 0.007, 0.007, 0.007);
        }
    }
}
