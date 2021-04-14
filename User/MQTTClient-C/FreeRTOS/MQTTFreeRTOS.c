/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    Ian Craggs - convert to FreeRTOS
 *******************************************************************************/

#include "MQTTFreeRTOS.h"

int ThreadStart(Thread* thread, void (*fn)(void*), void* arg)
{
	int rc = 0;
	uint16_t usTaskStackSize = (configMINIMAL_STACK_SIZE * 5);
	UBaseType_t uxTaskPriority = uxTaskPriorityGet(NULL); /* set the priority as the same as the calling task*/

	rc = xTaskCreate(fn,	/* The function that implements the task. */
		"MQTTTask",			/* Just a text name for the task to aid debugging. */
		usTaskStackSize,	/* The stack size is defined in FreeRTOSIPConfig.h. */
		arg,				/* The task parameter, not used in this case. */
		uxTaskPriority,		/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
		&thread->task);		/* The task handle is not used. */

	return rc;
}

void MutexInit(Mutex* mutex)
{
	mutex->sem = xSemaphoreCreateMutex();
}

int MutexLock(Mutex* mutex)
{
	return xSemaphoreTake(mutex->sem, portMAX_DELAY);
}

int MutexUnlock(Mutex* mutex)
{
	return xSemaphoreGive(mutex->sem);
}

void TimerCountdownMS(Timer* timer, unsigned int timeout_ms)
{
	timer->xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	vTaskSetTimeOutState(&timer->xTimeOut); /* Record the time at which this function was entered. */
}

void TimerCountdown(Timer* timer, unsigned int timeout)
{
	TimerCountdownMS(timer, timeout * 1000);
}

int TimerLeftMS(Timer* timer)
{
	xTaskCheckForTimeOut(&timer->xTimeOut, &timer->xTicksToWait); /* updates xTicksToWait to the number left */
	return (timer->xTicksToWait < 0) ? 0 : (timer->xTicksToWait * portTICK_PERIOD_MS);
}

char TimerIsExpired(Timer* timer)
{
	return xTaskCheckForTimeOut(&timer->xTimeOut, &timer->xTicksToWait) == pdTRUE;
}

void TimerInit(Timer* timer)
{
	timer->xTicksToWait = 0;
	memset(&timer->xTimeOut, '\0', sizeof(timer->xTimeOut));
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Network read.(Modified for W5500EVB) </summary>
///
/// <remarks>	Tony Wang, 16:10 12/4/21. </remarks>
///
/// <param name="n">		 	[in,out] If non-null, a Network to process. </param>
/// <param name="buffer">	 	[in,out] If non-null, the buffer. </param>
/// <param name="len">		 	The length. </param>
/// <param name="timeout_ms">	The timeout in milliseconds. </param>
///
/// <returns>	Sum of bytes received. </returns>
///-------------------------------------------------------------------------------------------------

int FreeRTOS_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	TimeOut_t xTimeOut;
	int recvLen = 0;

	vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */

	do
	{
		int rc = 0;

		if (getSn_SR(n->my_socket) == SOCK_ESTABLISHED)
		{
			if (getSn_IR(n->my_socket) & Sn_IR_CON)
				setSn_IR(n->my_socket, Sn_IR_CON);

			if (getSn_RX_RSR(n->my_socket))
				rc = recv(n->my_socket, buffer + recvLen, len - recvLen);

			if (rc > 0)
				recvLen += rc;
			else
				break;
		}

	} while (recvLen < len && xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);

	return recvLen;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Network write.(Modified for W5500EVB) </summary>
///
/// <remarks>	Tony Wang, 16:10 12/4/21. </remarks>
///
/// <param name="n">		 	[in,out] If non-null, a Network to process. </param>
/// <param name="buffer">	 	[in,out] If non-null, the buffer. </param>
/// <param name="len">		 	The length. </param>
/// <param name="timeout_ms">	The timeout in milliseconds. </param>
///
/// <returns>	Sum of bytes sent. </returns>
///-------------------------------------------------------------------------------------------------

int FreeRTOS_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	TimeOut_t xTimeOut;
	int sentLen = 0;

	vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */

	do
	{
		int rc = 0;

		if (getSn_SR(n->my_socket) == SOCK_ESTABLISHED)
		{
			if (getSn_IR(n->my_socket) & Sn_IR_CON)
				setSn_IR(n->my_socket, Sn_IR_CON);

			rc = send(n->my_socket, buffer + sentLen, len - sentLen);

			if (rc > 0)
				sentLen += rc;
			else
				break;
		}

	} while (sentLen < len && xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);

	return sentLen;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Network disconnect.(Modified for W5500EVB) </summary>
///
/// <remarks>	Tony Wang, 16:10 12/4/21. </remarks>
///
/// <param name="n">	[in,out] If non-null, a Network to process. </param>
///-------------------------------------------------------------------------------------------------

void FreeRTOS_disconnect(Network* n)
{
	disconnect(n->my_socket);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Network initialize.(Modified for W5500EVB) </summary>
///
/// <remarks>	Tony Wang, 16:9 12/4/21. </remarks>
///
/// <param name="n">	[in,out] If non-null, a Network to process. </param>
///-------------------------------------------------------------------------------------------------

void NetworkInit(Network* n)
{
	n->my_socket = 0;
	n->mqttread = FreeRTOS_read;
	n->mqttwrite = FreeRTOS_write;
	n->disconnect = FreeRTOS_disconnect;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Network connect.(Modified for W5500EVB) </summary>
///
/// <remarks>	Tony Wang, 16:12 12/4/21. </remarks>
///
/// <param name="n">   	[in,out] If non-null, a Network to process. </param>
/// <param name="addr">	[in,out] If non-null, the address. </param>
/// <param name="port">	The port. </param>
///
/// <returns>	1 for success else 0. </returns>
///-------------------------------------------------------------------------------------------------

int NetworkConnect(Network* n, char* addr, int port)
{
	if (getSn_SR(n->my_socket) != SOCK_CLOSED)
		close(n->my_socket);

	while (getSn_SR(n->my_socket) != SOCK_CLOSED);

	socket(n->my_socket, Sn_MR_TCP, 50231, Sn_MR_ND);

	while (getSn_SR(n->my_socket) != SOCK_INIT);

	return connect(n->my_socket, (uint8_t*)addr, port);
}
