/**
  ******************************************************************************
  * @file    portW5500_Loopback/main.c 
  * @author  popctrl@163.com
  * @version V1.0.0
  * @date    2017/11/28
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
//#include <stdio.h>
#include "W7500x.h"
#include "W7500x_ssp.h"
#include "print_x.h"

#include "wizchip_conf.h"
#include "loopback.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CHIP_RST_HIGH()     GPIOC->LB_MASKED[0x01] = 0x01
#define CHIP_RST_LOW()      GPIOC->LB_MASKED[0x01] = 0x00

// Socket & Port number definition for Examples
#define SOCK_TCPS       0
#define SOCK_UDPS       1
#define PORT_TCPS       5000
#define PORT_UDPS       3000

// Shared Buffer Definition for Loopback test
//#define DATA_BUF_SIZE   2048  // defined in loopback.h
uint8_t gDATABUF[DATA_BUF_SIZE];

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},
                            .ip = {192, 168, 137, 20},
                            .sn = {255, 255, 255, 0},
                            .gw = {192, 168, 137, 1},
                            .dns = {0, 0, 0, 0},
                            .dhcp = NETINFO_STATIC };


/* Private function prototypes -----------------------------------------------*/
void wizchip_select(void);
void wizchip_deselect(void);
uint8_t wizchip_rw(uint8_t byte);
void wizchip_write(uint8_t wb);
uint8_t wizchip_read(void);

void SPI1_Init(void);
void W5500_Init(void);
void delay_ms(__IO uint32_t nCount);

/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Main Function
  */
int main()
{
    uint8_t tmp;
//    GPIO_InitTypeDef GPIO_InitDef;
    /* Set System init */
    SystemInit();
    SPI1_Init();

    /* Configure UART2, Baudrate = 115200, 8, 1, None */
    UART2->BAUDDIV = 417;   /* (48000000 / 115200) */
    UART2->CTRL |= (S_UART_CTRL_RX_EN | S_UART_CTRL_TX_EN);

    prt_str("\r\n( W7500P <-> W5500 ) --- Loopback Test! ---\r\n\r\n");

    W5500_Init();

    tmp = getVERSIONR();

    prt_str("W5500 Chip Version: "); prt_hb(tmp); prt_str(".\r\n");

    /* wizchip netconf */
    ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
            
	  while(1)
	  {
        loopback_udps(SOCK_UDPS, gDATABUF, PORT_UDPS);
        loopback_tcps(SOCK_TCPS, gDATABUF, PORT_TCPS);
	  }
}

/**
  * @brief  Chip Select  Wiznet W5500 operations.
  * @param  None
  * @retval None
  */
void wizchip_select(void)
{
    GPIOC->LB_MASKED[0x10] = 0x00;
    __NOP(); __NOP();
}

/**
  * @brief  Chip Deselect  Wiznet W5500 operations.
  * @param  None
  * @retval None
  */
void wizchip_deselect(void)
{
    __NOP(); __NOP();
    GPIOC->LB_MASKED[0x10] = 0x10;
}

/**
  * @brief  wizchip_rw Function
  */
uint8_t wizchip_rw(uint8_t byte)
{
    /* Loop while DR register in not emplty */
    while((SSP1->SR & SSP_FLAG_TNF) == RESET);
    SSP1->DR = byte;
    /* Wait to receive a byte */
    while((SSP1->SR & SSP_FLAG_RNE) == RESET);
    return SSP1->DR;
}

/**
  * @brief  wizchip_write Function
  */
void wizchip_write(uint8_t wb)
{
    wizchip_rw(wb);
}

/**
  * @brief  wizchip_read Function
  */
uint8_t wizchip_read(void)
{
    return wizchip_rw(0xFF);
}

/**
  * @brief  SPI1_Init Function
  */
void SPI1_Init(void)
{
    SSP1->CR0 |= (uint32_t)( SSP_FrameFormat_MO | SSP_CPHA_1Edge | SSP_CPOL_Low | SSP_DataSize_8b );
    SSP1->CR1 |= (uint32_t)(SSP_SOD_RESET | SSP_Mode_Master | SSP_NSS_Hard | SSP_SSE_SET | SSP_LBM_RESET );
//    SSP1->CPSR = SSP_BaudRatePrescaler_2;
    SSP1->CPSR = 24;   // 48,000,000 / 24 = 2,000,000 (2MHz)

    /* GPIO PC_04 SPI1 nCS Set */
    GPIOC->OUTENSET = (0x01<<4);
    PC_PCR->Port[4] = 0x06;         // High driving strength & pull-up
    PC_AFSR->Port[4] = 0x01;        // GPIOC_4
    GPIOC->LB_MASKED[0x10] = 0x10;  // set GPIOC_4 high

    /* GPIO PB_00 Set */
    GPIOB->OUTENSET = 0x01;
    PB_PCR->Port[0] = 0x06;         // High driving strength & pull-up
    PB_AFSR->Port[0] = 0x01;        // GPIOB_0
    GPIOB->LB_MASKED[0x01] = 0x01;  // set GPIOB_0 high

    /* GPIO PC_00 nReset W5500 Set */
    GPIOC->OUTENSET = 0x01;
    PC_PCR->Port[0] = 0x06;
    PC_AFSR->Port[0] = 0x01;
    GPIOC->LB_MASKED[1] = 1;
}

/**
  * @brief  W5500 Init Function
  */
void W5500_Init(void)
{
    CHIP_RST_HIGH();
    delay_ms(3);
    CHIP_RST_LOW();
    delay_ms(3);
    CHIP_RST_HIGH();
    delay_ms(3);

    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
    reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);
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

