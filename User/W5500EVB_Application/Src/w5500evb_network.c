#include "w5500evb.h"

#define W5500EVB_NETWORK_TASK_PRIO		0
#define W5500EVB_NETWORK_STK_SIZE		128

TaskHandle_t W5500EVB_Network_Task_Handler;

void W5500EVB_Network_Init(void)
{
	uint8_t MAC[6] = { 0x00,0x08,0xDC,0xCB,0x5C,0x86 };
	uint8_t IP[4] = { 192,168,1,150 };
	uint8_t SUBNET[4] = { 255,255,255,0 };
	uint8_t GATEWAY[4] = { 192,168,1 ,1 };
	/*uint8_t DNS[4] = { 8,8,8,8 };*/

	uint8_t TX_Buffer_Size[MAX_SOCK_NUM] = { 2,2,2,2,2,2,2,2 };
	uint8_t RX_Buffer_Size[MAX_SOCK_NUM] = { 2,2,2,2,2,2,2,2 };

	uint8_t Temp[4];

	setSHAR(MAC);
	setSIPR(IP);
	setSUBR(SUBNET);
	setGAR(GATEWAY);

	sysinit(TX_Buffer_Size, RX_Buffer_Size);

	setRTR(2000);																			// 设置超时时间
	setRCR(3);

	printf("Init Network\r\n");

	getSIPR(Temp);
	printf("IP : \t\t%d.%d.%d.%d\r\n", Temp[0], Temp[1], Temp[2], Temp[3]);

	getSUBR(Temp);
	printf("SUBNET : \t%d.%d.%d.%d\r\n", Temp[0], Temp[1], Temp[2], Temp[3]);

	getGAR(Temp);
	printf("GATEWAY : \t%d.%d.%d.%d\r\n", Temp[0], Temp[1], Temp[2], Temp[3]);
}

void W5500EVB_Network_Task(void* pvParameters)
{
	W5500_Reset();

	W5500EVB_Network_Init();

	printf("Enter MQTT Task\r\n");

	taskENTER_CRITICAL();

	W5500EVB_MQTT_Task_Creat();

	vTaskDelete(NULL);

	taskEXIT_CRITICAL();
}

void W5500EVB_Network_Task_Creat(void)
{
	xTaskCreate(
		(TaskFunction_t)W5500EVB_Network_Task,
		(const char*)"Network",
		(uint16_t)W5500EVB_NETWORK_STK_SIZE,
		(void*)NULL,
		(UBaseType_t)W5500EVB_NETWORK_TASK_PRIO,
		(TaskHandle_t*)&W5500EVB_Network_Task_Handler
	);
}
