/**********************RC522实验开发板例程************************
*  MCU: STC89C52RC
*  晶振: 11.0592MHz
*  波特率: 4800
*  当 MCU 为其它型号，以及晶振为其它频率时，波特率会有变动。
*  例如使用 STC89C54RD+ & 22.1184MHz & 6T/双倍速 时，波特率为 19200。
*  控制引脚定义参见 main.h
******************************************************************/

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

//#include "lpcreg.h"
//#include "main.h"
//#include "mfrc522.h"
//#include   <STDIO.H>
//#include <string.h>

/* Private variables ---------------------------------------------------------*/
unsigned char code data1[16] = {0x12,0x34,0x56,0x78,0xED,0xCB,0xA9,0x87,0x12,0x34,0x56,0x78,0x01,0xFE,0x01,0xFE};
//M1卡的某一块写为如下格式，则该块为钱包，可接收扣款和充值命令
//4字节金额（低字节在前）＋4字节金额取反＋4字节金额＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反
unsigned char code data2[4]  = {0,0,0,0x01};
unsigned char code DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void InitializeSystem();


unsigned char g_ucTempbuf[20];
void delay1(unsigned int z)
{
  unsigned int x,y;
  for(x=z;x>0;x--)
  for(y=110;y>0;y--);
}


void main( )
{
  unsigned char status,i;
  unsigned int temp;

  InitializeSystem( );
  PcdReset();
  PcdAntennaOff();
  PcdAntennaOn();

  while ( 1 )
  {
    status = PcdRequest(PICC_REQALL, g_ucTempbuf);//寻卡
    if (status != MI_OK)
    {
//     InitializeSystem( );
     PcdReset();
     PcdAntennaOff();
     PcdAntennaOn();
     continue;
    }
  
    printf("卡的类型: ");
    for(i=0;i<2;i++)
    {
      temp=g_ucTempbuf[i];
      printf("%X",temp);
    }
    printf("\r\n\r\n");
  
    status = PcdAnticoll(g_ucTempbuf);//防冲撞
    if (status != MI_OK)
    {    continue;    }
  
  
    ////////以下为超级终端打印出的内容////////////////////////
    printf("卡序列号: "); //超级终端显示,
    for(i=0;i<4;i++)
    {
      temp=g_ucTempbuf[i];
      printf("%X ",temp);
    }
    printf("\r\n\r\n");
    ///////////////////////////////////////////////////////////
  
    status = PcdSelect(g_ucTempbuf);//选定卡片
    if (status != MI_OK)
    {    continue;    }
  
    status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//验证卡片密码
    if (status != MI_OK)
    {    continue;    }
  
    status = PcdWrite(1, data1);//写块
    if (status != MI_OK)
    {    continue;    }

    while(1)
    {
      status = PcdRequest(PICC_REQALL, g_ucTempbuf);//寻卡
      if (status != MI_OK)
      {
        //InitializeSystem( );
        PcdReset();
        PcdAntennaOff();
        PcdAntennaOn();
        continue;
      }
      status = PcdAnticoll(g_ucTempbuf);//防冲撞
      if (status != MI_OK)
      {    continue;    }
      status = PcdSelect(g_ucTempbuf);//选定卡片
      if (status != MI_OK)
      {    continue;    }
  
      status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//验证卡片密码
      if (status != MI_OK)
      {    continue;    }
  
      status = PcdValue(PICC_DECREMENT,1,data2);//扣款
      if (status != MI_OK)
      {    continue;    }
  
      status = PcdBakValue(1, 2);//块备份
      if (status != MI_OK)
      {    continue;    }
  
      status = PcdRead(2, g_ucTempbuf);//读块
      if (status != MI_OK)
      {    continue;    }
      printf("卡片读块: "); //超级终端显示,
      for(i=0;i<16;i++)
      {
        temp=g_ucTempbuf[i];
        printf("%X",temp);
        if((i==3)||(i==7)||(i==11)) printf(" ");
      }
      printf("\r\n");

      LED_GREEN = 0;
      delay1(800);
      LED_GREEN = 1;
      delay1(800);
//      LED_GREEN = 0;
//      delay1(800);
//      LED_GREEN = 1;
//      delay1(800);
      PcdHalt();
    }
  }
}


/////////////////////////////////////////////////////////////////////
//系统初始化
/////////////////////////////////////////////////////////////////////
void InitializeSystem()
{
  P0M1 = 0x0; P0M2 = 0x0;
  P1M1 = 0x0; P1M2 = 0x0;
  P3M1 = 0x0; P3M2 = 0xFF;
  P0 = 0xFF; P1 = 0xFF; P3 = 0xFF;P2 = 0xFF;

  TMOD=0x21;       //设T0为方式1，GATE=1；
  SCON=0x50;
  TH1=0xFa;          //波特率为4800bps
  TL1=0xFa;
  TH0=0;
  TL0=0;
  TR0=1;
  ET0=1;             //允许T0中断
  TR1=1;         //开启定时器
  TI=1;
  EA=1;         //开启总中断

  ES = 1;
  RI = 1;

}






