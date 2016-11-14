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
/**
  ******************************************************************************
  * @file    portFreeRTOS/main.c
  * @author  popctrl@163.com
  * @version V1.0.0
  * @date    03-Nov-2015
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
#include "W7500x_gpio.h"
#include "W7500x_uart.h"
#include "print_x.h"
#include "FreeRTOS.h"
#include "task.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/*--------------- Tasks Priority -------------*/
#define TESTCOM_TASK1_PRIO              ( tskIDLE_PRIORITY + 2 )
#define TESTCOM_TASK2_PRIO              ( tskIDLE_PRIORITY + 3 )
#define TESTLED_TASK3_PRIO              ( tskIDLE_PRIORITY + 3 )
#define mainCREATOR_TASK_PRIORITY       ( tskIDLE_PRIORITY + 4 )

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitDef;

/* Private function prototypes -----------------------------------------------*/
void TestCOMtask1(void * pvParameters);
void TestCOMtask2(void * pvParameters);
void TestLEDtask3(void * pvParameters);


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main Function
  */
int main()
{
    /* Set Systme init */
    SystemInit();

    /* UART2 configuration*/
    S_UART_Init(115200);

//    prt_str("\r\n\r\n");
    prt_str("\r\nUART2 print string Test.\r\n");
//    test_print_x();


//    *(volatile uint32_t *)(0x41001014) = 0x0060100; //clock setting 48MHz

    /* CLK OUT Set */
//    PAD_AFConfig(PAD_PA,GPIO_Pin_2, PAD_AF2); // PAD Config - CLKOUT used 3nd Function
    /* GPIO PA_02 CLKOUT Set */
    GPIO_InitDef.GPIO_Pin = GPIO_Pin_2; // Set to PA_02 (CLKOUT)
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_AF; // Set to Mode AF
    GPIO_InitDef.GPIO_Pad = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitDef);
    PAD_AFConfig(PAD_PA,GPIO_Pin_2, PAD_AF2); // PAD Config - LED used 2nd Function

    /* GPIO LED(R) Set */
    GPIO_InitDef.GPIO_Pin = GPIO_Pin_0; // Set to PC_00 (LED(R))
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT; // Set to Mode Output
    GPIO_Init(GPIOC, &GPIO_InitDef);
    PAD_AFConfig(PAD_PC,GPIO_Pin_0, PAD_AF1); // PAD Config - LED used 2nd Function

    GPIO_SetBits(GPIOC, GPIO_Pin_0); // LED(R) Off

    /* Add your application code here */
    xTaskCreate(TestCOMtask1, "task1", configMINIMAL_STACK_SIZE*2, NULL, TESTCOM_TASK1_PRIO, NULL);
    xTaskCreate(TestCOMtask2, "task2", configMINIMAL_STACK_SIZE*2, NULL, TESTCOM_TASK2_PRIO, NULL);
    xTaskCreate(TestLEDtask3, "task3", configMINIMAL_STACK_SIZE*2, NULL, TESTLED_TASK3_PRIO, NULL);

    /* Start scheduler */
    vTaskStartScheduler();

    return 0;

}


/**
  * @brief  Test COM2 task
  * @param  pvParameters not used
  * @retval None
  */
void TestCOMtask1(void * pvParameters)
{
  for( ;; )
  {
    /* Test COM2 each 150*10ms */
    prt_str("\r\nTest COM task01.\r\n");
    vTaskDelay(150);
  }
}

/**
  * @brief  Test COM2 task
  * @param  pvParameters not used
  * @retval None
  */
void TestCOMtask2(void * pvParameters)
{
  for( ;; )
  {
    /* Test COM2 each 200*10ms */
    prt_str("\r\nTest COM task02.\r\n");
    vTaskDelay(200);
  }
}

/**
  * @brief  Test LED task
  * @param  pvParameters not used
  * @retval None
  */
void TestLEDtask3(void * pvParameters)
{
  for( ;; )
  {
    /* Test LED each 100*10ms */
    /* RED LED toggled */
    if(GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_0) != (uint32_t)Bit_RESET){
        GPIO_ResetBits(GPIOC, GPIO_Pin_0);
        prt_str("\r\nLED - ON.\r\n");
    }
    else{
        GPIO_SetBits(GPIOC, GPIO_Pin_0);
        prt_str("\r\nLED - OFF.\r\n");
    }

    vTaskDelay(100);
  }
}

