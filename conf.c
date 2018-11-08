#include "conf.h"

/*
 -------------------------------
         STM32F030F4P6
 -------------------------------
	1  BOOT0
	2  PF0 - X
	3  PF1 - X
	4  NRST
	5  VDDA
	6  PA0 - ADC1_IN0
	7  PA1 - X
	8  PA2 - X
	9  PA3 - X
	10 PA4 - X
	11 PA5 - GP Output - LED
	12 PA6 - X
	13 PA7 - X
	14 PB1 - X
	15 VSS
	16 VDD
	17 PA9  - USART1_TX / RELAY
	18 PA10 - X
	19 PA13 - SYS_SWCLK
	20 PA14 - SYS_SWDIO
*/

// Global degiskenler
uint16_t adc_data[__NUMBER_OF_ADC_DATA];

static void RCC_Configuration(void)
{
   //-----------------------------------------
   // AHB Clock
   //-----------------------------------------
   // GPIOA clock enable
   // DMA1 clock enable
   RCC->AHBENR |= RCC_AHBENR_GPIOAEN \
               | RCC_AHBENR_DMA1EN;
   
   //-----------------------------------------
   // APB1 Clock
   //-----------------------------------------
   // TIM3 clock enable
   RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
   
   //-----------------------------------------
   // APB2 Clock
   //-----------------------------------------
   // ADC1 clock enable
   // TIM1 clock enable
   // USART1 clock enable
   RCC->APB2ENR |= RCC_APB2ENR_ADC1EN \
                | RCC_APB2ENR_TIM1EN;
#ifdef __USE_PA9_SERIAL_OUT
   RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
#endif // __USE_PA9_SERIAL_OUT
}


static void GPIO_Configuration(void)
{
   //-----------------------------------------
   // GPIOA
   //-----------------------------------------
   // Mode
   // PA0 - Analog mode
   // PA5 - GP output
   // PA9 - GP output
   // PA9 - Alternate function
   GPIOA->MODER |= GPIO_MODER_MODER0 \
                | GPIO_MODER_MODER5_0;
#ifdef __USE_PA9_SERIAL_OUT
   GPIOA->MODER |= GPIO_MODER_MODER9_1;
#else
   GPIOA->MODER |= GPIO_MODER_MODER9_0;
#endif // __USE_PA9_SERIAL_OUT
   
   // Speed
   // PA5 - Very high speed
   // PA9 - Very high speed
   GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5 \
                  | GPIO_OSPEEDER_OSPEEDR9;
               
   // AF
   // PA9 - AF1
#ifdef __USE_PA9_SERIAL_OUT
   GPIOA->AFR[1] |= (1 << 4);
#endif // __USE_PA9_SERIAL_OUT

}

#ifdef __USE_PA9_SERIAL_OUT
static void USART_Configuration(void)
{
   //-----------------------------------------
   // USART1
   //-----------------------------------------
   // Baudrate __USART1_BAUDRATE
   USART1->BRR = (SystemCoreClock / __USART1_BAUDRATE);
   // Transmitter enable
   // USART1 enable
   USART1->CR1 |= USART_CR1_TE \
               | USART_CR1_UE;
}

struct __FILE {
    int dummy;
};
 
FILE __stdout;
 
int fputc(int ch, FILE *f) {
   // TXE flagini bekle.
   while(!(USART1->ISR & USART_ISR_TXE))
   {
   }
   USART1->TDR = ch;
   return ch;
}
#endif // __USE_PA9_SERIAL_OUT

void ADC_Configuration(void)
{
   //-----------------------------------------
   // ADC1
   //-----------------------------------------
   // ADC calibration
   ADC1->CR |= ADC_CR_ADCAL;
   // Kalibrasyonun tamamlanmasini bekle.
   while(ADC1->CR & ADC_CR_ADCAL);
   // DMA circular mode
   // DMA enable
   // External trigger enable, TRGO - TIM1
   ADC1->CFGR1 |= ADC_CFGR1_DMACFG \
               | ADC_CFGR1_DMAEN \
               | ADC_CFGR1_EXTEN_0;
   // 13.5 ADC clock cycles
   ADC1->SMPR |= ADC_SMPR1_SMPR_1;
   // ADC1_IN0
   ADC1->CHSELR |= ADC_CHSELR_CHSEL0;
   // ADC1 enable
   ADC1->CR |= ADC_CR_ADEN;
   // ADCnin hazir olmasini bekle.
   while(ADC1->ISR & ADC_ISR_ADRDY);
   // Start conversation
   ADC1->CR |= ADC_CR_ADSTART;
}

void TIM_Configuration(void)
{
   //-----------------------------------------
   //  TIM1
   //-----------------------------------------
   // TIM1 Frekansi __ADC_TRIGGER_FREQUENCY_HZ
   TIM1->PSC = (SystemCoreClock / (__ADC_TRIGGER_FREQUENCY_HZ * 100)) - 1;
   // Period 100
   TIM1->ARR = 100;
   // Master mode - Update - ADC1 trigger
   TIM1->CR2 |= TIM_CR2_MMS_1;
   // TIM1 counter enable
   TIM1->CR1 |= TIM_CR1_CEN; 
}

void DMA_Configuration(void)
{
   //-----------------------------------------
   // DMA1_Channel1
   //-----------------------------------------  
   // Very high priority
   // Memory size : 16bit
   // Peripheral size : 16bit
   // Memory increment mode
   // Circular mode enable
   // TC interrupt enabled
   DMA1_Channel1->CCR |= DMA_CCR_PL \
                      | DMA_CCR_MSIZE_0 \
                      | DMA_CCR_PSIZE_0 \
                      | DMA_CCR_MINC \
                      | DMA_CCR_CIRC \
                      | DMA_CCR_TCIE;   
   // Number of data to transfer
   DMA1_Channel1->CNDTR = __NUMBER_OF_ADC_DATA;   
   // Peripheral address
   DMA1_Channel1->CPAR = (uint32_t)(&(ADC1->DR));   
   // Memory address
   DMA1_Channel1->CMAR = (uint32_t)adc_data;   
   // DMA1_Channel1 enable
   DMA1_Channel1->CCR |= DMA_CCR_EN;
}

void NVIC_Configuration(void)
{
   //-----------------------------------------
   // NVIC
   //----------------------------------------- 
   // DMA1_Channel1 IRQ enable
   NVIC_EnableIRQ(DMA1_Channel1_IRQn);
   // Very high priority
   NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
}

void STM32_Configuration(void)
{
   // MCU konfigurasyonu
   RCC_Configuration();
   GPIO_Configuration();
#ifdef __USE_PA9_SERIAL_OUT   
   USART_Configuration();
#endif // __USE_PA9_SERIAL_OUT
   NVIC_Configuration();
   ADC_Configuration();
   TIM_Configuration();
   DMA_Configuration();
}
