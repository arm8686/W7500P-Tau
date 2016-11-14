/*******************************************************************************************************************************************************
 * Copyright ¨Ï 2016 <WIZnet Co.,Ltd.> 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ¡°Software¡±), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED ¡°AS IS¡±, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************************************************************************************************/
/**************************************************************************/
/**
 * @file    system_W7500x.c 
 * @author  IOP Team
 * @version V1.0.0
 * @date    01-May-2015
 * @brief   CMSIS Cortex-M0 Core Peripheral Access Layer Source File for
 *          Device W7500x
 ******************************************************************************
 *
 * @attention
 * @par Revision history
 *    <2015/05/01> 1st Release
 *
 * <h2><center>&copy; COPYRIGHT 2015 WIZnet Co.,Ltd.</center></h2>
 ******************************************************************************
 */

#include "system_W7500x.h"


/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/
//#define SYSCLK_EXTERN_OSC

//#define SYSCLK_PLL_FOUT_96MHZ



/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemFrequency = 0;    /*!< System Clock Frequency (Core Clock)  */
uint32_t SystemCoreClock = 0;    /*!< Processor Clock Frequency            */


/*----------------------------------------------------------------------------
  Clock functions
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
    uint8_t M,N,OD;

#ifdef SYSCLK_EXTERN_OSC
    CRG->PLL_IFSR = CRG_PLL_IFSR_OCLK;
#else
    CRG->PLL_IFSR = CRG_PLL_IFSR_RCLK;
#endif    
    OD = (1 << (CRG->PLL_FCR & 0x01)) * (1 << ((CRG->PLL_FCR & 0x02) >> 1));
    N = (CRG->PLL_FCR >>  8 ) & 0x3F;
    M = (CRG->PLL_FCR >> 16 ) & 0x3F;

#ifdef SYSCLK_EXTERN_OSC
    SystemCoreClock = EXTERN_XTAL * M / N * 1 / OD;
#else
    SystemCoreClock = INTERN_XTAL * M / N * 1 / OD;
#endif
}

uint32_t GetSystemClock()
{
#ifdef SYSCLK_PLL_FOUT_96MHZ
    return SystemFrequency/2;
#else
    return SystemFrequency;
#endif
}


/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 */
void SystemInit (void)
{
    uint8_t M,N,OD;
    
    (*((volatile uint32_t *)(W7500x_TRIM_BGT))) = (*((volatile uint32_t *)(W7500x_INFO_BGT)));
    (*((volatile uint32_t *)(W7500x_TRIM_OSC))) = (*((volatile uint32_t *)(W7500x_INFO_OSC)));

    // Set PLL input frequency
#ifdef SYSCLK_EXTERN_OSC

#ifdef SYSCLK_PLL_FOUT_96MHZ
    CRG->FCLK_PVSR = CRG_FCLK_PVSR_DIV2;
    CRG->UARTCLK_PVSR = CRG_UARTCLK_PVSR_DIV2;
    CRG->ADCCLK_PVSR = CRG_ADCCLK_PVSR_DIV2;

    CRG->TIMER0CLK_PVSR = CRG_TIMERCLK_PVSR_DIV2;
    CRG->TIMER1CLK_PVSR = CRG_TIMERCLK_PVSR_DIV2;

    CRG->PWM0CLK_PVSR = CRG_PWMCLK_PVSR_DIV2;
    CRG->PWM1CLK_PVSR = CRG_PWMCLK_PVSR_DIV2;
    CRG->PWM2CLK_PVSR = CRG_PWMCLK_PVSR_DIV2;
    CRG->PWM3CLK_PVSR = CRG_PWMCLK_PVSR_DIV2;
    CRG->PWM4CLK_PVSR = CRG_PWMCLK_PVSR_DIV2;
    CRG->PWM5CLK_PVSR = CRG_PWMCLK_PVSR_DIV2;
    CRG->PWM6CLK_PVSR = CRG_PWMCLK_PVSR_DIV2;
    CRG->PWM7CLK_PVSR = CRG_PWMCLK_PVSR_DIV2;

    CRG->WDOGCLK_HS_PVSR = CRG_WDOGCLK_HS_PVSR_DIV4;
    CRG->MONCLK_SSR = CRG_MONCLK_SSR_FCLK;

    CRG->PLL_FCR = 0x00100200;      /* M=16, N=2, OD=0 */
    CRG->PLL_IFSR = CRG_PLL_IFSR_OCLK;
#else
    CRG->PLL_FCR = 0x00080200;
    CRG->PLL_IFSR = CRG_PLL_IFSR_OCLK;
#endif

#else
    CRG->PLL_IFSR = CRG_PLL_IFSR_RCLK;
#endif    
    OD = (1 << (CRG->PLL_FCR & 0x01)) * (1 << ((CRG->PLL_FCR & 0x02) >> 1));
    N = (CRG->PLL_FCR >>  8 ) & 0x3F;
    M = (CRG->PLL_FCR >> 16 ) & 0x3F;

#ifdef SYSCLK_EXTERN_OSC
    SystemFrequency = EXTERN_XTAL * M / N * 1 / OD;
#else
    SystemFrequency = INTERN_XTAL * M / N * 1 / OD;
#endif
    SystemCoreClock = SystemFrequency;
}
