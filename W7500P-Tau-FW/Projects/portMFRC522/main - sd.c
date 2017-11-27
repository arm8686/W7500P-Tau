/**
  ******************************************************************************
  * @file    portMFRC522/main.c 
  * @author  popctrl@163.com
  * @version V1.0.0
  * @date    2017/11/24
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
  * <h2><center>&copy; COPYRIGHT 2017 </center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/

#include "W7500x.h"
//#include "W7500x_gpio.h"
#include "W7500x_ssp.h"
//#include "W7500x_uart.h"
#include "mmc_sd.h"
#include "MFRC522.h"
#include "print_x.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/
#define BLOCK_SIZE            512 /* Can not be used 512 Block */

#define NUMBER_OF_BLOCKS      2  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE    (BLOCK_SIZE * NUMBER_OF_BLOCKS)

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
void SPI1_Init(void);

/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Main Function
  */
int main()
{
//    GPIO_InitTypeDef GPIO_InitDef;
    /* Set System init */
    SystemInit();
    SPI1_Init();

    /* Configure UART2, Baudrate = 115200 */
//    S_UART_Init(115200);
    UART2->BAUDDIV = 417;   /* (48000000 / 115200) */
    UART2->CTRL |= (S_UART_CTRL_RX_EN | S_UART_CTRL_TX_EN);

    prt_str("W7500P ---MFRC522 (read & write M1 card)--- Test!\r\n\r\n");

    /* GPIO PA_02 CLKOUT Set */
//    GPIO_InitDef.GPIO_Pin = GPIO_Pin_2; // Set to PA_02 (CLKOUT)
//    GPIO_InitDef.GPIO_Mode = GPIO_Mode_AF; // Set to Mode AF
//    GPIO_InitDef.GPIO_Pad = GPIO_PuPd_DOWN;
//    GPIO_Init(GPIOA, &GPIO_InitDef);
//    PAD_AFConfig(PAD_PA,GPIO_Pin_2, PAD_AF2); // PAD Config - used 3rd Function

//    test_print_x();
    

    /*SD_GPIO_Initailization*/
//    bsp_sd_gpio_init();

    /*SD_Initialization*/
//    SD_Init();

    /*Single Block Test*/
//    SD_SingleBlockTest();

    /*Multi(32) Block Test*/
//    SD_MultiBlockTest();
            
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

  for(tmp=0; tmp<20; tmp++)
  {
    delay_ms(50);
//    GPIO_ResetBits(GPIOC, GPIO_Pin_0); // LED(R) On
    GPIOC->LB_MASKED[1] = 0x00;
    delay_ms(50);
//    GPIO_SetBits(GPIOC, GPIO_Pin_0); // LED(R) Off
    GPIOC->LB_MASKED[1] = 0x01;
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
  * @brief  SPI1_Init Function
  */
void SPI1_Init(void)
{
    SSP1->CR0 |= (uint32_t)( SSP_FrameFormat_MO | SSP_CPHA_1Edge | SSP_CPOL_Low | SSP_DataSize_8b );
    SSP1->CR1 |= (uint32_t)(SSP_SOD_RESET | SSP_Mode_Master | SSP_NSS_Hard | SSP_SSE_SET | SSP_LBM_RESET );
    SSP1->CPSR = SSP_BaudRatePrescaler_2;
    SSP1->CPSR = 24;   // 48,000,000 / 24 = 2,000,000 (2MHz)

    /* GPIO PC_04 SPI1 nCS Set */
    GPIOC->OUTENSET = (0x01<<4);
    PC_PCR->Port[4] = 0x06;         // High driving strength & pull-up
    PC_AFSR->Port[4] = 0x01;        // GPIOC_4
    GPIOC->LB_MASKED[0x10] = 0x10;  // set GPIOC_4 high

    /* GPIO PB_00 nReset MFRC522 Set */
    GPIOB->OUTENSET = 0x01;
    PB_PCR->Port[0] = 0x06;         // High driving strength & pull-up
    PB_AFSR->Port[0] = 0x01;        // GPIOB_0
    GPIOB->LB_MASKED[0x01] = 0x01;  // set GPIOB_0 high

//    GPIO_InitDef.GPIO_Pin = GPIO_Pin_0; // Set to PC_00 (LED(R))
//    GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT; // Set to Mode Output
//    GPIO_Init(GPIOC, &GPIO_InitDef);
//    PAD_AFConfig(PAD_PC,GPIO_Pin_0, PAD_AF1); // PAD Config - LED used 2nd Function
//    GPIO_SetBits(GPIOC, GPIO_Pin_0); // LED(R) Off

//    GPIOC->OUTENSET = 0x01;
//    PC_PCR->Port[0] = 0x06;
//    PC_AFSR->Port[0] = 0x01;
//    GPIOC->LB_MASKED[1] = 1;
    GPIOC->OUTENSET = (0x01<<4);
    PC_PCR->Port[4] = 0x06;         // High driving strength & pull-up
    PC_AFSR->Port[4] = 0x01;        // GPIOC_4
    GPIOC->LB_MASKED[0x10] = 0x10;  // set GPIOC_4 high
}
