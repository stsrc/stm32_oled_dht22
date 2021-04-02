#include "stupid_delay.h"
#include <stm32f1xx_hal.h>

uint64_t __IO delay_val;

void delay_init(){
	if(SysTick_Config(SystemCoreClock / 100000UL)){
		while(1);
	}
}

void delay_ms(uint32_t ms)
{
	delay_val = ms * 1000;
	while(delay_val != 0);
}

void delay_us(uint32_t us)
{
	delay_val = us;
	while(delay_val != 0);
}

void SysTick_Handler(void){
	if(delay_val != 0) delay_val -= 10;

	HAL_IncTick();
}
