#include "w5500evb.h"

#define W5500EVB_MQTT_TASK_PRIO		0
#define W5500EVB_MQTT_STK_SIZE		512

TaskHandle_t W5500EVB_MQTT_Task_Handler;

//#define MQTT_TASK 1

void messageArrived(MessageData* data)
{
	printf("Message arrived on topic %.*s: %.*s\n",
		data->topicName->lenstring.len,
		data->topicName->lenstring.data,
		data->message->payloadlen,
		(char*)data->message->payload);
}

static void prvMQTTEchoTask(void* pvParameters)
{
	/* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */
	MQTTClient client;
	Network network;
	unsigned char sendbuf[80], readbuf[80];
	int rc = 0, count = 0;

	uint8_t address[4] = { 118,89,70,79 };

	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

	NetworkInit(&network);
	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));


	if ((rc = NetworkConnect(&network, (char*)address, 1883)) != 0)

	while (!(getPHYStatus() & 0x01));

	printf("PHY is ready\r\n");

	while (!NetworkConnect(&network, (char*)address, 1883));

	printf("TCP Connection is ready\r\n");

	/*if ((rc = NetworkConnect(&network, (char*)address, 1883)) != 0)
		printf("Return code from network connect is %d\n", rc);
	else
		printf("Return code from network connect is %d\n", rc);*/

#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&client)) != pdPASS)
		printf("Return code from start tasks is %d\n", rc);
#endif

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "41A7E676A8DL98SZNS2AP7AB";
	connectData.username.cstring = "CKBfrTDpxtZrbPrVn6BCZfkwUb";
	connectData.password.cstring = "8YdRycz2ynhMRwNFVH9Yz9s8YinVU5XP";
	connectData.cleansession = 1;
	connectData.keepAliveInterval = 60;



	while (MQTTConnect(&client, &connectData) != SUCCESS)
	{
		//printf("321");
	}

	printf("MQTT Connected\n");

	/*if ((rc = MQTTConnect(&client, &connectData)) != 0)
		printf("Return code from MQTT connect is %d\n", rc);
	else
		printf("MQTT Connected\n");*/

	if ((rc = MQTTSubscribe(&client, "/device/41A7E676A8DL98SZNS2AP7AB/r", QOS2, messageArrived)) != 0)
		printf("Return code from MQTT subscribe is %d\n", rc);

	while (++count)
	{
		MQTTMessage message;
		char payload[30];

		message.qos = QOS1;
		message.retained = 0;
		message.payload = payload;
		sprintf(payload, "message number %d", count);
		message.payloadlen = strlen(payload);

		if ((rc = MQTTPublish(&client, "/device/41A7E676A8DL98SZNS2AP7AB/s", &message)) != 0)
			printf("Return code from MQTT publish is %d\n", rc);
#if !defined(MQTT_TASK)
		if ((rc = MQTTYield(&client, 1000)) != 0)
			printf("Return code from yield is %d\n", rc);
#endif
	}

	/* do not return */
}

void W5500EVB_MQTT_Task_Creat(void)
{
	xTaskCreate(
		(TaskFunction_t)prvMQTTEchoTask,
		(const char*)"MQTT",
		(uint16_t)W5500EVB_MQTT_STK_SIZE,
		(void*)NULL,
		(UBaseType_t)W5500EVB_MQTT_TASK_PRIO,
		(TaskHandle_t*)&W5500EVB_MQTT_Task_Handler
	);
}
