#include "w5500evb.h"

int main(void)
{
	HAL_Init();

	SystemClock_Config_HSE();

	Delay_Init(72);

	USART_Init();

	W5500_Init();

	Start_Task_Creat();

	vTaskStartScheduler();
}
