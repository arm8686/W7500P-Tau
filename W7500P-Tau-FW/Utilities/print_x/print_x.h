/**
  ******************************************************************************
  * @file    print_x.h
  * @author  popctrl@163.com
  * @version V1.0.1
  * @date    2017-11-24
  * @brief   for W7500P use UART2
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 popctrl@163.com</center></h2>
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PRINT_X_H
#define __PRINT_X_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include <stdio.h>

#ifdef USE_PRINT_X_DBG
    #define DBG_OUT       5   //调试串口的输出level控制
    #define DBG_L1        1   //Level 1
    #define DBG_L2        2
    #define DBG_L3        3
    #define DBG_L4        4
#endif

#include "W7500x.h"

/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

extern void prt_str(char *str);
extern void prt_hb(unsigned char byte);
extern void prt_hh(unsigned short halfword);
extern void prt_hw(unsigned int word);

extern void test_print_x(void);

#ifdef __cplusplus
}
#endif

#endif /* __PRINT_X_H */

/************************** (C) COPYRIGHT popctrl@163.com *****END OF FILE*****/
