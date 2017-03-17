/*******************************************************************************************************************************************************
 * Copyright 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************************************************************************************************/
/**
  ******************************************************************************
  * @file    main.c 
  * @author  popctrl@163.com
  * @version V1.0.0
  * @date    24-Nov-2016
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, WIZnet SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2016 </center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
//#include <stdio.h>
#include "W7500x.h"
//#include "W7500x_gpio.h"
#include "mmc_sd.h"
#include "print_x.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/
#define BLOCK_SIZE            512

#define NUMBER_OF_BLOCKS      2  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE    (BLOCK_SIZE * NUMBER_OF_BLOCKS)

//#define PA02_CLOCK_OUT

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//UART_InitTypeDef UART_InitStructure;
uint8_t Buffer_Block_Tx[BLOCK_SIZE];
uint8_t Buffer_Block_Rx[BLOCK_SIZE];
uint8_t Buffer_MultiBlock_Tx[MULTI_BUFFER_SIZE];
uint8_t Buffer_MultiBlock_Rx[MULTI_BUFFER_SIZE];
volatile TestStatus EraseStatus = FAILED;
volatile TestStatus TransferStatus1 = FAILED;
volatile TestStatus TransferStatus2 = FAILED;

/* Private function prototypes -----------------------------------------------*/
void SD_SingleBlockTest(void);
void SD_MultiBlockTest(void);
void delay_ms(__IO uint32_t nCount);
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);
TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength);

#ifdef PA02_CLOCK_OUT
void PA02_clkout(void);
#endif


/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Main Function
  */
int main()
{
    /* Set System init */
    SystemInit();

#ifdef PA02_CLOCK_OUT
    PA02_clkout();
#endif

    /* Configure UART2 */
    UART2->BAUDDIV = 417;   /* (48000000 / 115200) */
    UART2->CTRL |= (S_UART_CTRL_RX_EN | S_UART_CTRL_TX_EN);
#ifdef PA02_CLOCK_OUT
    prt_str("\r\nW7500P --- Monitoring Clock output at pin: PA_02 --- Test!\r\n\r\n");
#endif
    prt_str("\r\nW7500P --- FatFs R012b --- Test!\r\n\r\n");

//     test_print_x();

    /*SD_GPIO_Initailization*/
    bsp_sd_gpio_init();

    /*SD_Initialization*/
    SD_Init();

    /*Single Block Test*/
    SD_SingleBlockTest();

    /*Multi(32) Block Test*/
    SD_MultiBlockTest();
            
	while(1)
	{
	}
}

/**
  * @brief  Tests the SD card Single Blocks operations.
  * @param  None
  * @retval None
  */
void SD_SingleBlockTest(void)
{    
  uint8_t tmp;
  uint16_t i,j;

  /* Read block of 512 bytes on address 0 */
  SD_ReadSingleBlock (0x00, Buffer_Block_Rx);

  prt_str("\r\nRead Singleblock of 512 bytes on address 0:\r\n\r\n");
  for (i=0;i<32;i++){ for (j=0;j<16;j++) {prt_hb(Buffer_Block_Rx[i*16+j]); prt_str(" ");} prt_str("\r\n");}

  for(tmp=0; tmp<20; tmp++){
    delay_ms(50);
//    GPIO_ResetBits(GPIOC, GPIO_Pin_0); // LED(R) On
    GPIOC->LB_MASKED[1] = 0x00; // LED(R) On
    delay_ms(50);
//    GPIO_SetBits(GPIOC, GPIO_Pin_0); // LED(R) Off
    GPIOC->LB_MASKED[1] = 0x01; // LED(R) Off
  }

}

/**
  * @brief  Tests the SD card Single Blocks operations.
  * @param  None
  * @retval None
  */
void SD_MultiBlockTest(void)
{
  uint8_t tmp;
  uint16_t i,j;

  /* Read block of 512 bytes on address 0 */
//  while(1)
  SD_ReadMultiBlock (0x00, Buffer_MultiBlock_Rx, NUMBER_OF_BLOCKS);

  prt_str("\r\nRead Multiblock of 512 bytes on address 0:\r\n\r\n");
  for (i=0;i<32;i++){ for (j=0;j<16;j++) {prt_hb(Buffer_MultiBlock_Rx[i*16+j]); prt_str(" ");} prt_str("\r\n");}

  for(tmp=0; tmp<10; tmp++)
  {
    delay_ms(150);
//    GPIO_ResetBits(GPIOC, GPIO_Pin_0); // LED(R) On
    GPIOC->LB_MASKED[1] = 0x00;
    delay_ms(150);
//    GPIO_SetBits(GPIOC, GPIO_Pin_0); // LED(R) Off
    GPIOC->LB_MASKED[1] = 0x01;
  }

}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer1 identical to pBuffer2
  *         FAILED: pBuffer1 differs from pBuffer2
  */
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return PASSED;
}

/**
  * @brief  Checks if a buffer has all its values are equal to zero.
  * @param  pBuffer: buffer to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer values are zero
  *         FAILED: At least one value from pBuffer buffer is different from zero.
  */
TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    /* In some SD Cards the erased state is 0xFF, in others it's 0x00 */
    if ((*pBuffer != 0xFF) && (*pBuffer != 0x00))
    {
      return FAILED;
    }

    pBuffer++;
  }

  return PASSED;
}

/**
  * @brief  Delay Function
  */
void delay_ms(__IO uint32_t nCount)
{
    volatile uint32_t delay = nCount * 2500; // approximate loops per ms at 24 MHz, Debug config
    for(; delay != 0; delay--)
        __NOP();
}

/**
  * @brief  Clock Output Function
  */
#ifdef PA02_CLOCK_OUT
void PA02_clkout(void)
{
    /* GPIO PA_02 CLKOUT Set */

//    GPIO_InitTypeDef GPIO_InitDef;
//    GPIO_InitDef.GPIO_Pin = GPIO_Pin_2; // Set to PA_02 (CLKOUT)
//    GPIO_InitDef.GPIO_Mode = GPIO_Mode_AF; // Set to Mode AF
//    GPIO_InitDef.GPIO_Pad = GPIO_PuPd_DOWN;
//    GPIO_Init(GPIOA, &GPIO_InitDef);
//    PAD_AFConfig(PAD_PA,GPIO_Pin_2, PAD_AF2); // PAD Config - used 2nd Function

    CRG->MONCLK_SSR = CRG_MONCLK_SSR_OCLK;  //select External oscillator: 12MHz
//    CRG->MONCLK_SSR = CRG_MONCLK_SSR_MCLK;  //select PLL output clock: 48MHz
//    CRG->MONCLK_SSR = CRG_MONCLK_SSR_RCLK;  //select External oscillator: 8MHz
    GPIOA->OUTENSET = 0x01;
    PA_PCR->Port[2] = Px_PCR_PUPD_DOWN;
    PA_AFSR->Port[2] = Px_AFSR_AF2;
}
#endif

