#include "stm32f0xx_it.h"

// Tanimlamalar
#define __FFT_SIZE __NUMBER_OF_ADC_DATA


// Global degiskenler
// Integer adc dizisi
extern uint16_t adc_data[__NUMBER_OF_ADC_DATA];
// FFT cikis dizisi
float32_t fft_output[__FFT_SIZE];
// Float olarak adc dizisi
float32_t float_adc_data[__NUMBER_OF_ADC_DATA * 2];
// Maksimum genlik
float32_t max_amplitude;
// Maksimum genligin dizideki konumu
uint32_t ma_index;
// Maksimum genligin frekansi
float32_t ma_frequency;
// 
uint32_t esik;


void HardFault_Handler(void)
{
   while(1)
   {
   }
}
float Slope(float in[]);
void DMA1_Channel1_IRQHandler(void)
{
   if(DMA1->ISR & DMA_ISR_TCIF1)
   {
      uint16_t i;
      // ADC dizisinin degisimini engellemek icin DMA yi durdur.
      DMA1_Channel1->CCR &= ~DMA_CCR_EN;
      for(i = 0; i < __NUMBER_OF_ADC_DATA; ++i)
      {
         // Kompleks sayinin reel kisimi.
         float_adc_data[i * 2] = adc_data[i];
         // Kompleks sayini sanal kisimi
         float_adc_data[i * 2 + 1] = 0;
      }
      // ADC okumayi tekrar baslat.
      DMA1_Channel1->CCR |= DMA_CCR_EN;
      
      // FFT islemleri
      arm_cfft_f32(&arm_cfft_sR_f32_len128, float_adc_data, 0, 1);   
      arm_cmplx_mag_f32(float_adc_data, fft_output, __FFT_SIZE);      
      
      // 0Hz frekansin genligini sifirla.
      fft_output[0] = 0;
      
      // Maksimum genligi bul.
      arm_max_f32(fft_output, __FFT_SIZE  / 2, &max_amplitude, &ma_index);
      
      // Freakansin hesaplanmasi.
      ma_frequency = (__ADC_TRIGGER_FREQUENCY_HZ / __NUMBER_OF_ADC_DATA) * ma_index;
      

      // Freakans bu aralikta ise.
      if(ma_frequency > 900 && ma_frequency < 1200)
      {           
        esik++;
      }
      else
      {
         esik = 0;
      }      
      if(esik == 10)
      {
         // Led toggle.
         GPIOA->ODR ^= GPIO_ODR_5;
         // Role toggle.
         GPIOA->ODR ^= GPIO_ODR_9;
         esik = 0;
      }

      /* 
      // FFT cikisinin serial gonder.
      for(i = 0; i < __NUMBER_OF_ADC_DATA; ++i)
      {
         printf("%f\n",fft_output[i]);
      }
      */
      // Interrupti temizle.
      DMA1->IFCR |= DMA_IFCR_CGIF1;
   }
}
