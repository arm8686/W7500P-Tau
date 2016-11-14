/**
  ******************************************************************************
  * @file    print_x.c
  * @author  popctrl@163.com
  * @version V1.0.0
  * @date    2016-11-01
  * @brief   打印字符串，16 进制打印成可显示字符。for W7500P use UART2
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 popctrl@163.com</center></h2>
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "print_x.h"

/*----------------------------------------------------------------------------*/

void prt_str(unsigned char *str);
void prt_hb(unsigned char byte);
void prt_hh(unsigned short halfword);
void prt_hw(unsigned int word);

void test_print_x(void);

/*----------------------------------------------------------------------------*/

void prt_str(unsigned char *str)
{
    while(*str) {
        while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
        UART2->DATA = *str;
        str++;
    }
}

/*----------------------------------------------------------------------------*/

void prt_hb(unsigned char byte)
{
    unsigned char tmp;

    while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
    UART2->DATA = '0';
    while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
    UART2->DATA = 'x';

    tmp = byte / 0x10;
    if(tmp < 10) {
        tmp += 0x30;
    }
    else {
        tmp += 0x37;
    }
    while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
    UART2->DATA = tmp;
    tmp = byte % 0x10;
    if(tmp < 10) {
        tmp += 0x30;
    }
    else {
        tmp += 0x37;
    }
    while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
    UART2->DATA = tmp;
}

/*----------------------------------------------------------------------------*/

void prt_hh(unsigned short halfword)
{
    unsigned char tmp,i;

    while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
    UART2->DATA = '0';
    while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
    UART2->DATA = 'x';

    for(i = 16; i; i -= 8){
        tmp = ((halfword >> (i-8)) & 0xFF) / 0x10;
        if(tmp < 10) {
            tmp += 0x30;
        }
        else {
            tmp += 0x37;
        }
        while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
        UART2->DATA = tmp;
        tmp = ((halfword >> (i-8)) & 0xFF) % 0x10;
        if(tmp < 10) {
            tmp += 0x30;
        }
        else {
            tmp += 0x37;
        }
        while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
        UART2->DATA = tmp;
    }
}

/*----------------------------------------------------------------------------*/

void prt_hw(unsigned int word)
{
    unsigned char tmp,i;

    while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
    UART2->DATA = '0';
    while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
    UART2->DATA = 'x';

    for(i = 32; i; i -= 8){
        tmp = ((word >> (i-8)) & 0xFF) / 0x10;
        if(tmp < 10) {
            tmp += 0x30;
        }
        else {
            tmp += 0x37;
        }
        while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
        UART2->DATA = tmp;
        tmp = ((word >> (i-8)) & 0xFF) % 0x10;
        if(tmp < 10) {
            tmp += 0x30;
        }
        else {
            tmp += 0x37;
        }
        while(UART2->STATE & S_UART_STATE_TX_BUF_FULL);
        UART2->DATA = tmp;
    }
}

/*----------------------------------------------------------------------------*/

void test_print_x(void)
{
    unsigned int i;

    prt_str("\r\n");
    for(i=0; i<256; i++){
      prt_hb(i); prt_str("\r\n");}

    for(i=0xFFFFFFFF; i>0xFFFFFEFF; i--){
      prt_hb(i); prt_str("\r\n");}

    for(i=0; i<256; i++){
      prt_hh(i); prt_str("\r\n");}

    for(i=0xFFFFFFFF; i>0xFFFFFEFF; i--){
      prt_hh(i); prt_str("\r\n");}

    for(i=0; i<256; i++){
      prt_hw(i); prt_str("\r\n");}

    for(i=0xFFFFFFFF; i>0xFFFFFEFF; i--){
      prt_hw(i); prt_str("\r\n");}
}

/*----------------------------------------------------------------------------*/
