#include "w5500evb.h"

extern void xPortSysTickHandler(void);

static uint8_t fac_us = 0;

static uint16_t fac_ms = 0;

void SysTick_Handler(void)
{
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		xPortSysTickHandler();
	}

	HAL_IncTick();
}

void Delay_Init(uint8_t SYSCLK)
{
	uint32_t reload;

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	fac_us = SYSCLK;

	reload = SYSCLK;
	reload *= 1000000 / configTICK_RATE_HZ;

	fac_ms = 1000 / configTICK_RATE_HZ;

	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	SysTick->LOAD = reload;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void Delay_us(uint32_t nus)
{
	uint32_t ticks;

	uint32_t told, tnow, tcnt = 0;

	uint32_t reload = SysTick->LOAD;

	ticks = nus * fac_us;

	told = SysTick->VAL;

	while (1)
	{
		tnow = SysTick->VAL;

		if (tnow != told)
		{
			if (tnow < told)
			{
				tcnt += told - tnow;
			}
			else
			{
				tcnt += reload - tnow + told;
			}

			told = tnow;

			if (tcnt >= ticks)
				break;
		}
	}
}

void Delay_ms(uint32_t nms)
{
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		if (nms >= fac_ms)
		{
			vTaskDelay(nms / fac_ms);
		}

		nms %= fac_ms;
	}

	Delay_us((uint32_t)(nms * 1000));
}

void Delay_xms(uint32_t nms)
{
	uint32_t i;

	for (i = 0; i < nms; i++)
	{
		Delay_us(1000);
	}
}
