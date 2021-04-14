#include "w5500evb.h"

/// <summary>	----------USART specific based on HAL---------- </summary>

#define DEBUG_USART_BAUDRATE						115200

#define DEBUG_USART									USART1

#define DEBUG_USART_CLK_ENABLE()					__HAL_RCC_USART1_CLK_ENABLE()
#define DEBUG_USART_TX_CLK_ENABLE()					__HAL_RCC_GPIOA_CLK_ENABLE()
#define DEBUG_USART_RX_CLK_ENABLE()					__HAL_RCC_GPIOA_CLK_ENABLE()

#define DEBUG_USART_TX_PORT							GPIOA
#define DEBUG_USART_TX_Pin							GPIO_PIN_9

#define DEBUG_USART_RX_PORT							GPIOA
#define DEBUG_USART_RX_Pin							GPIO_PIN_10

#define DEBUG_USART_IRQHandler						USART1_IRQHandler
#define DEBUG_USART_IRQ								USART1_IRQn

static UART_HandleTypeDef DebugUsartHandle;

/// <summary>	----------USART specific based on HAL---------- </summary>

void USARTPIN_GPIO_Init(GPIO_TypeDef* GPIOx, uint32_t Pin, uint32_t Mode, uint32_t Pull, uint32_t Speed)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = Pin;
	GPIO_InitStructure.Mode = Mode;
	GPIO_InitStructure.Pull = Pull;
	GPIO_InitStructure.Speed = Speed;

	HAL_GPIO_Init(GPIOx, &GPIO_InitStructure);
}

void USART_Init(void)
{
	DebugUsartHandle.Instance = DEBUG_USART;

	DebugUsartHandle.Init.BaudRate = DEBUG_USART_BAUDRATE;
	DebugUsartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	DebugUsartHandle.Init.StopBits = UART_STOPBITS_1;
	DebugUsartHandle.Init.Parity = UART_PARITY_NONE;
	DebugUsartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	DebugUsartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	DebugUsartHandle.Init.Mode = UART_MODE_TX_RX;

	HAL_UART_Init(&DebugUsartHandle);

	__HAL_UART_ENABLE_IT(&DebugUsartHandle, UART_IT_RXNE);
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	DEBUG_USART_CLK_ENABLE();
	DEBUG_USART_TX_CLK_ENABLE();
	DEBUG_USART_RX_CLK_ENABLE();

	USARTPIN_GPIO_Init(
		DEBUG_USART_TX_PORT,
		DEBUG_USART_TX_Pin,
		GPIO_MODE_AF_PP,
		GPIO_PULLUP,
		GPIO_SPEED_FREQ_HIGH
	);

	USARTPIN_GPIO_Init(
		DEBUG_USART_RX_PORT,
		DEBUG_USART_RX_Pin,
		GPIO_MODE_AF_PP,
		GPIO_PULLUP,
		GPIO_SPEED_FREQ_HIGH
	);

	HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ);

	HAL_NVIC_SetPriority(DEBUG_USART_IRQ, 7, 0);
}

void DEBUG_USART_IRQHandler(void)
{
	uint8_t temp;

	uint32_t timeout;

	uint32_t maxDelay = 0x1FFFF;

	BaseType_t xHigherPriorityTaskWoken;

	HAL_UART_IRQHandler(&DebugUsartHandle);

	timeout = 0;

	while (HAL_UART_GetState(&DebugUsartHandle) != HAL_UART_STATE_READY)
	{
		timeout++;

		if (timeout > maxDelay)
			break;
	}

	timeout = 0;

	while (HAL_UART_Receive_IT(&DebugUsartHandle, (uint8_t*)&temp, 1) != HAL_OK)
	{
		timeout++;

		if (timeout > maxDelay)
			break;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int fputc(int ch, FILE* f)
{
	uint8_t data;

	data = ch & 0xFF;

	HAL_UART_Transmit(&DebugUsartHandle, (uint8_t*)(&data), 1, 100);

	return (ch);
}
