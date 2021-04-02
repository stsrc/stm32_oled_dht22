#ifndef _DHT22_H_
#define _DHT22_H_

#include <stm32f1xx_hal_gpio.h>

struct dht22 {
	GPIO_TypeDef *gpio;
	uint32_t pin;
	float temperature;
	float humidity;
};
struct dht22 dht22_init(GPIO_TypeDef *gpio, uint32_t pin);
void dht22_get_result(struct dht22 *dht22);

#endif
