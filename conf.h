#ifndef __CONF_H
#define __CONF_H

// Kutuphaneler
#include "stm32f0xx.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#ifdef __USE_PA9_SERIAL_OUT
#include "stdio.h"
#endif // __USE_PA9_SERIAL_OUT

// Tanimlamalar 
#define __NUMBER_OF_ADC_DATA        128
#define __USART1_BAUDRATE           9600
#define __ADC_TRIGGER_FREQUENCY_HZ  20000


// MCU konfigurasyon fonksiyon prototipleri
static void RCC_Configuration(void);
static void GPIO_Configuration(void);
static void DMA_Configuration(void);
static void NVIC_Configuration(void);
static void ADC_Configuration(void);
static void TIM_Configuration(void);
static void USART_Configuration(void);
void STM32_Configuration(void);


#endif // __CONF_H
