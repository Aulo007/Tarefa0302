#include <stdio.h>
#include <string.h>
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
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Definição de variáveis globais

static volatile int num;                       // Variável global para definir qual número selecionar na matriz
static volatile int num1 = 1;                  // Variável global para testar doubounce
static volatile int num2 = 1;                  // Variável global para testar doubounce
static volatile int last_time = 0;             // Variável global para amarzenar o tempo do último evento
static volatile bool estado_led_green = false; // Varuável global para amazenar o estado do led verde
static volatile bool estado_led_blue = false;  // Variável global para amarzenar o estado do led azul

static ssd1306_t ssd; // Variável global para o display

static void gpio_irq_handle(uint gpio, uint32_t events);

int main()
{
    // Inicialização dos pinos
    stdio_init_all();

    gpio_init(LED_RED_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_BLUE_PIN);
    gpio_init(BUTTON1_PIN);
    gpio_init(BUTTON2_PIN);

    // Inicializa os pinos da matriz de Leds no pino 7

    npInit(7);

    // Confira direção dos pinos (Entrada ou saída)

    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
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

    // Inicialização e configuração do display SSD1306                                               // Cria a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display com as especificações fornecidas
    ssd1306_config(&ssd);                                         // Configura os parâmetros do display
    ssd1306_send_data(&ssd);                                      // Envia os dados iniciais de configuração para o display

    // Limpeza do display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false); // Preenche o display com o valor especificado (false = apagado)
    ssd1306_send_data(&ssd);   // Envia os dados de preenchimento para o display

    // Loop principal

    int charInt = 0;

    while (true)
    {
        if (stdio_usb_connected())
        { // Certifica-se de que o USB está conectado
            char cmd;
            printf("Digite uma letra ou número: \n");
            if (scanf(" %c", &cmd) == 1)
            { // Lê um caractere da entrada padrão

                if (cmd >= '0' && cmd <= '9')
                {
                    ssd1306_fill(&ssd, false);
                    printf("Você digitou um número: %c\n", cmd);
                    charInt = cmd - '0'; // Converte de char para int
                    setMatrizDeLEDSComIntensidade(caixa_de_desenhos[charInt], 0.007, 0.007, 0.007);
                    ssd1306_draw_char(&ssd, cmd, 50, 50);
                    ssd1306_send_data(&ssd); // Atualiza o display
                }
                else if ((cmd >= 'A' && cmd <= 'Z') || (cmd >= 'a' && cmd <= 'z'))
                {
                    ssd1306_fill(&ssd, false);
                    printf("Você digitou uma letra: %c\n", cmd);
                    ssd1306_draw_char(&ssd, cmd, 50, 50);
                    ssd1306_send_data(&ssd); // Atualiza o display
                }
                else
                {
                    ssd1306_fill(&ssd, false);
                    printf("Caractere inválido\n");
                }
            }
        }
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

        if (gpio == BUTTON1_PIN)
        {
            if (estado_led_green == false)
            {
                ssd1306_fill(&ssd, false);
                estado_led_green = true;
                gpio_put(LED_GREEN_PIN, true);
                printf("Led verde ligado");
                ssd1306_draw_string(&ssd, "Led verde", 20, 30); // Desenha uma string
                ssd1306_draw_string(&ssd, "ligado", 15, 48);    // Desenha uma string
                ssd1306_send_data(&ssd);                        // Atualiza o display
            }
            else
            {
                ssd1306_fill(&ssd, false);
                estado_led_green = false;
                gpio_put(LED_GREEN_PIN, false);
                printf("Led verde desligado");
                ssd1306_draw_string(&ssd, "Led verde", 20, 30); // Desenha uma string
                ssd1306_draw_string(&ssd, "desligado", 15, 48); // Desenha uma string
                ssd1306_send_data(&ssd);
            }
        }
        else if (gpio == BUTTON2_PIN)
        {
            if (estado_led_blue == false)
            {
                ssd1306_fill(&ssd, false);
                estado_led_blue = true;
                gpio_put(LED_BLUE_PIN, true);
                printf("Led azul ligado");
                ssd1306_draw_string(&ssd, "Led azul", 20, 30); // Desenha uma string
                ssd1306_draw_string(&ssd, "ligado", 15, 48);   // Desenha uma string
                ssd1306_send_data(&ssd);
            }
            else
            {
                ssd1306_fill(&ssd, false);
                estado_led_blue = false;
                gpio_put(LED_BLUE_PIN, false);
                printf("Led azul desligado");
                ssd1306_draw_string(&ssd, "Led azul", 20, 30);  // Desenha uma string
                ssd1306_draw_string(&ssd, "desligado", 15, 48); // Desenha uma string
                ssd1306_send_data(&ssd);
            }
        }
    }
}
