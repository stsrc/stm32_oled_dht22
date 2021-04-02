#include "dht22.h"
#include "stupid_delay.h"
#include <stdbool.h>

static void dht22_switch_pin_direction(struct dht22 *dht22, bool out)
{
	GPIO_InitTypeDef gpios;
	gpios.Pin = GPIO_PIN_6;
	gpios.Pull = GPIO_PULLUP;
	gpios.Speed = GPIO_SPEED_FREQ_HIGH;
	if (out) {
		gpios.Mode = GPIO_MODE_OUTPUT_OD;
	} else {
		gpios.Mode = GPIO_MODE_INPUT;
	}

	HAL_GPIO_Init(dht22->gpio, &gpios);
}

struct dht22 dht22_init(GPIO_TypeDef *gpio, uint32_t pin)
{
	struct dht22 dht22;
	dht22.gpio = gpio;
	dht22.pin = pin;

	GPIO_InitTypeDef gpios;
	gpios.Pin = GPIO_PIN_6;
	gpios.Mode = GPIO_MODE_OUTPUT_OD;
	gpios.Pull = GPIO_PULLUP;
	gpios.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(gpio, &gpios);


	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_IC_InitTypeDef      sConfigIC;

	htim3.Instance               = TIM3;
	htim3.Init.Prescaler         = 7;
	htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
	htim3.Init.Period            = 65535;
	htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_IC_Init(&htim3) != HAL_OK) {
		while(1);
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) !=
	    HAL_OK) {
		while(1);
	}

	sConfigIC.ICPolarity  = TIM_INPUTCHANNELPOLARITY_FALLING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter    = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) {
		while(1);
	}


	return dht22;
}

void dht22_get_result(struct dht22 *dht22)
{
	dht22_switch_pin_direction(dht22, true);
	HAL_GPIO_WritePin(dht22->gpio, dht22->pin, GPIO_PIN_RESET);
	delay_ms(10);
	HAL_GPIO_WritePin(dht22->gpio, dht22->pin, GPIO_PIN_SET);
	dht22_switch_pin_direction(dht22, false);
	while(HAL_GPIO_ReadPin(dht22->gpio, dht22->pin) == GPIO_PIN_SET); //1
	// dht22 answer
	while(HAL_GPIO_ReadPin(dht22->gpio, dht22->pin) == GPIO_PIN_RESET); //0
	while(HAL_GPIO_ReadPin(dht22->gpio, dht22->pin) == GPIO_PIN_SET); //1
	//now starts sends data
	uint8_t result[5];
	for (int i = 0; i < 5; i++) {
		uint8_t byte = 0;
		for (int j = 0; j < 8; j++) {
			while(HAL_GPIO_ReadPin(dht22->gpio, dht22->pin) == GPIO_PIN_RESET); //0
			delay_us(30);
			if (HAL_GPIO_ReadPin(dht22->gpio, dht22->pin) == GPIO_PIN_SET) {
				//1
				byte |= 1 << (7 - j);
				while(HAL_GPIO_ReadPin(dht22->gpio, dht22->pin) == GPIO_PIN_SET);
			} else {
				//0
			}
		}
		result[i] = byte;
	}

	dht22->humidity = ((float) ((result[0] << 8) | result[1])) * 0.1f;
	dht22->temperature = ((float) (((result[2] & 0x7F) << 8) | result[3])) * 0.1f;
	if (result[2] & 0x80) {
		dht22->temperature *= -1.0f;
	}
}

void TIM3_IRQHandler(void)
{
}
