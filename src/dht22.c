#include "dht22.h"
#include "stupid_delay.h"
#include <stdbool.h>
#include <stm32f1xx_hal_dma.h>
#include <stm32f1xx_hal_tim.h>
#include <stm32f1xx_hal.h>
#include <string.h>

__IO int32_t bitpos = -1;

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

TIM_HandleTypeDef htim3;
struct dht22 dht22_init(GPIO_TypeDef *gpio, uint32_t pin)
{
	struct dht22 dht22;
	dht22.gpio = gpio;
	dht22.pin = pin;

	GPIO_InitTypeDef gpios = {0};
	gpios.Pin = pin;
	gpios.Mode = GPIO_MODE_OUTPUT_OD;
	gpios.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(gpio, &gpios);

	__HAL_RCC_TIM3_CLK_ENABLE();

	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_IC_InitTypeDef      sConfigIC;


	htim3.Instance               = TIM3;
	htim3.Init.Prescaler         = 23;
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

	    /* TIM3 interrupt Init */
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	return dht22;
}

static __IO uint8_t result[5];
static __IO uint16_t last_val = 0;

void dht22_get_result(struct dht22 *dht22)
{
	bitpos = -1;
	memset((void *) result, 0, sizeof(result));
	last_val = 0;

	dht22_switch_pin_direction(dht22, true);
	HAL_GPIO_WritePin(dht22->gpio, dht22->pin, GPIO_PIN_RESET);
	delay_ms(10);
	HAL_GPIO_WritePin(dht22->gpio, dht22->pin, GPIO_PIN_SET);
	dht22_switch_pin_direction(dht22, false);

	__HAL_TIM_SET_COUNTER(&htim3, 0);

	if (HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1) != HAL_OK) {
		while(1);
	}

	while (bitpos < 5 * 8) {
		delay_ms(20);
	}

	if (HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1) != HAL_OK) {
		while(1);
	}

	dht22->humidity = ((float) ((result[0] << 8) | result[1])) * 0.1f;
	dht22->temperature = ((float) (((result[2] & 0x7F) << 8) | result[3])) * 0.1f;
	if (result[2] & 0x80) {
		dht22->temperature *= -1.0f;
	}
}


static inline uint16_t get_pulse_length(void) {

    uint16_t current_val = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1);
    uint16_t dt      = current_val - last_val;
    last_val = current_val;
    return dt;
}

static inline void write_one(void) {
    uint8_t byten = bitpos / 8;
    uint8_t bitn  = 7 - bitpos % 8;
    result[byten] |= 1 << bitn;
}

static inline void write_zero(void) {
    uint8_t byten = bitpos / 8;
    uint8_t bitn  = 7 - bitpos % 8;
    result[byten] &= ~(1 << bitn);
}

#define BETWEEN(a, b) (dt >= a && dt <= b)

void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);

	uint16_t dt = get_pulse_length();
	if (bitpos == -1) { // end of the start bit
		if (BETWEEN(120, 200)) { // [20 to 40]us + 80us + 80us - CPU time
			bitpos++;
		} else if (BETWEEN(0, 40)) { // [20-40]us - CPU time
			// fast CPU, caught beginning of the start bit
			// do nothing
		} else {
		}
	} else {					// data bits
		if (BETWEEN(70, 100)) { // zero: 50us + [26-28]us
			write_zero();
			bitpos++;
		} else if (BETWEEN(110, 150)) { // one: 50us + 70us
			write_one();
			bitpos++;
		} else { // invalid
		}
	}
}
