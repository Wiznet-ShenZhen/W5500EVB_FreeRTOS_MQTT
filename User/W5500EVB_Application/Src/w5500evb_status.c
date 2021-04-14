#include "w5500evb.h"

#define W5500EVB_STATUS_TASK_PRIO		0
#define W5500EVB_STATUS_STK_SIZE		128

TaskHandle_t W5500EVB_Status_Task_Handler;

void W5500EVB_Status_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void W5500EVB_Status_Task(void* pvParameters)
{
	W5500EVB_Status_Gpio_Init();

	for (;;)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

		Delay_ms(1000);

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);

		Delay_ms(1000);
	}
}

void W5500EVB_Status_Task_Creat(void)
{
	xTaskCreate(
		(TaskFunction_t)W5500EVB_Status_Task,
		(const char*)"Status",
		(uint16_t)W5500EVB_STATUS_STK_SIZE,
		(void*)NULL,
		(UBaseType_t)W5500EVB_STATUS_TASK_PRIO,
		(TaskHandle_t*)&W5500EVB_Status_Task_Handler
	);
}
