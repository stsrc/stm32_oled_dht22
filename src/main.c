#include <stm32f1xx.h>
#include <core_cm3.h>
#include "stupid_delay.h"
#include "ssd1306.h"
#include <stm32f1xx_hal_spi.h>
#include <string.h>
#include <stdio.h>
#include "dht22.h"

#define PIN GPIO_PIN_6
#define PORT GPIOA


#define LED_port GPIOC
#define LED_Blue (1 << 8)
#define GPIO_setBit(PORT, PIN) (PORT->BSRR |= PIN)
#define GPIO_clearBit(PORT, PIN) (PORT->BSRR |= (PIN << 0x10))

SPI_HandleTypeDef SSD1306_SPI_PORT;

static void init_blue_led() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	LED_port->CRH |= GPIO_CRH_MODE8_0;
	LED_port->CRH &= ~GPIO_CRH_CNF8;
}

void configure_clock() {
        RCC->CR |= (1 << 16); //External high speed clock enabled;
        while(!(RCC->CR & (1 << 17))); //External high-speed clock ready flag check

        RCC->CFGR |= 1 << 17; //LSB of division factor PREDIV1
        RCC->CFGR |= 1 << 16; //PLL entry clock source => clock from prediv1
        RCC->CFGR |= (0b0100 << 18); //PLL multiplication factor => x6

        RCC->CR |= (1 << 24); //PLL enabled;
        while(!(RCC->CR & (1 << 25))); //PLL clock ready flag

        RCC->CR &= ~(1 << 19); //Clock security system disabled
        RCC->CR &= ~(1 << 18); //External high-speed clock not bypassed

        RCC->CR |= 1 << 0; //Internal high-speed clock enabled
        while(!(RCC->CR & (1 << 1))); //Internal high-speed clock ready flag

        RCC->CFGR &= ~0b11; //select PLL as input
        RCC->CFGR |= (0b10);
        while(!(RCC->CFGR & 0b1000)); //wait until PLL is selected

	SystemCoreClock = 24000000;
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	(void)hspi;
	GPIO_InitTypeDef gpio =
	{
		GPIO_PIN_5, //GPIOA5 - sck; GPIOA7 - mosi
		GPIO_MODE_AF_PP,
		GPIO_PULLDOWN,
		GPIO_SPEED_FREQ_HIGH
	};

	GPIO_InitTypeDef gpio_miso =
	{
		GPIO_PIN_6,
		GPIO_MODE_AF_INPUT,
		GPIO_PULLDOWN,
		GPIO_SPEED_FREQ_HIGH
	};

	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	HAL_GPIO_Init(GPIOA, &gpio);

	gpio.Pin = GPIO_PIN_7;
	HAL_GPIO_Init(GPIOA, &gpio);

	HAL_GPIO_Init(GPIOA, &gpio_miso);
}

static void SPI_handler_basic_init(SPI_HandleTypeDef *spi_handler,
				   SPI_TypeDef *inst)
{
	spi_handler->Instance = inst;
	spi_handler->Init.Mode = SPI_MODE_MASTER;
	spi_handler->Init.Direction = SPI_DIRECTION_2LINES;
	spi_handler->Init.DataSize = SPI_DATASIZE_8BIT;
	spi_handler->Init.CLKPolarity = SPI_POLARITY_LOW;
	spi_handler->Init.CLKPhase = SPI_PHASE_1EDGE;
	spi_handler->Init.NSS = SPI_NSS_SOFT;
	spi_handler->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	spi_handler->Init.FirstBit = SPI_FIRSTBIT_MSB;
	spi_handler->Init.TIMode = SPI_TIMODE_DISABLE;
	spi_handler->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
}

static void gpio_init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef gpio =
	{
		GPIO_PIN_8,
		GPIO_MODE_OUTPUT_PP,
		GPIO_PULLDOWN,
		GPIO_SPEED_FREQ_HIGH
	};
	HAL_GPIO_Init(GPIOA, &gpio);
	gpio.Pin = GPIO_PIN_9;
	HAL_GPIO_Init(GPIOA, &gpio);
	gpio.Pin = GPIO_PIN_10;
	HAL_GPIO_Init(GPIOA, &gpio);
}

int main(void) {
	HAL_Init();

	configure_clock();

	gpio_init();
	delay_init();

	init_blue_led();

	SPI_handler_basic_init(&SSD1306_SPI_PORT, SPI1);

	HAL_SPI_Init(&SSD1306_SPI_PORT);

	ssd1306_Init();
	ssd1306_SetDisplayOn(1);
	ssd1306_Fill(Black);

	//TODO: why I can not draw at 0, but it is possible to draw at 129?
	for (int i = 0; i < 64; i++) {
		ssd1306_DrawPixel(129, i, White);
		ssd1306_DrawPixel(3, i, White);
	}
	for (int i = 3; i < 130; i++) {
		ssd1306_DrawPixel(i, 0, White);
		ssd1306_DrawPixel(i, 63, White);
	}
	ssd1306_SetCursor(10, 3);
	ssd1306_UpdateScreen();

	struct dht22 dht22 = dht22_init(GPIOA, GPIO_PIN_6);

	dht22_get_result(&dht22);
	char buffer[32];
	memset(buffer, 0, sizeof(buffer));
	int tone = dht22.temperature;
	int ttwo = dht22.temperature * 100 - tone * 100;
	int three = dht22.humidity;
	snprintf(buffer, sizeof(buffer), "temp: %d.%d", tone, ttwo);
	ssd1306_WriteString(buffer, Font_6x8, White);
	ssd1306_SetCursor(10, 25);
	snprintf(buffer, sizeof(buffer), "humi: %d", three);
	ssd1306_WriteString(buffer, Font_6x8, White);
	ssd1306_UpdateScreen();

	while(1) {
		delay_ms(1000);
		ssd1306_SetDisplayOn(0);
		delay_ms(1000);
		ssd1306_SetDisplayOn(1);
	}

	return 0;
}

void TIM3_IRQHandler(void)
{
}
