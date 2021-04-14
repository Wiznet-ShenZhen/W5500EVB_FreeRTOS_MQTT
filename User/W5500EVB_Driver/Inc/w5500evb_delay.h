#ifndef __W5500EVB_DELAY_H
#define __W5500EVB_DELAY_H

void Delay_Init(uint8_t SYSCLK);

void Delay_us(uint32_t nus);

void Delay_ms(uint32_t nms);

void Delay_xms(uint32_t nms);

#endif	
