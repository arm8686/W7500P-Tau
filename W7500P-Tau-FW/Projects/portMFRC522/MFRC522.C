//#include <intrins.h>
//#include "lpcreg.h"
//#include "main.h"
#include "MFRC522.h"
#include "W7500x.h"
#include "W7500x_ssp.h"

////////////////////////////////////////////////////////////////////////////////
#define MAXRLEN 18
#define USED_MFRC522_PA

#define MFRC522_RST_HIGH()     GPIOB->LB_MASKED[0x01] = 0x01
#define MFRC522_RST_LOW()      GPIOB->LB_MASKED[0x01] = 0x00

#define MFRC522_NSS_HIGH()     GPIOC->LB_MASKED[0x10] = 0x10
#define MFRC522_NSS_LOW()      GPIOC->LB_MASKED[0x10] = 0x00

////////////////////////////////////////////////////////////////////////////////
char PcdReset(void);
void PcdAntennaOn(void);
void PcdAntennaOff(void);
char PcdRequest(unsigned char req_code,unsigned char *pTagType);
char PcdAnticoll(unsigned char *pSnr);
char PcdSelect(unsigned char *pSnr);
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr);
char PcdRead(unsigned char addr,unsigned char *pData);
char PcdWrite(unsigned char addr,unsigned char *pData);
char PcdValue(unsigned char dd_mode,unsigned char addr,unsigned char *pValue);
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr);
char PcdHalt(void);
char PcdComMF522(unsigned char Command,
                 unsigned char *pInData,
                 unsigned char InLenByte,
                 unsigned char *pOutData,
                 unsigned int  *pOutLenBit);
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData);
void WriteRawRC(unsigned char Address,unsigned char value);
unsigned char ReadRawRC(unsigned char Address);
void SetBitMask(unsigned char reg,unsigned char mask);
void ClearBitMask(unsigned char reg,unsigned char mask);

////////////////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
////////////////////////////////////////////////////////////////////////////////
char PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
  char status;
  unsigned int  unLen;
  unsigned char ucComMF522Buf[MAXRLEN];
  
  ClearBitMask(Status2Reg,0x08);
  WriteRawRC(BitFramingReg,0x07);
  SetBitMask(TxControlReg,0x03);

  ucComMF522Buf[0] = req_code;

  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

  if ((status == MI_OK) && (unLen == 0x10))
  {
    *pTagType     = ucComMF522Buf[0];
    *(pTagType+1) = ucComMF522Buf[1];
  }
  else
  {   
    status = MI_ERR;
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回 MI_OK
////////////////////////////////////////////////////////////////////////////////
char PcdAnticoll(unsigned char *pSnr)
{
  char status;
  unsigned char i,snr_check=0;
  unsigned int  unLen;
  unsigned char ucComMF522Buf[MAXRLEN];

  ClearBitMask(Status2Reg,0x08);
  WriteRawRC(BitFramingReg,0x00);
  ClearBitMask(CollReg,0x80);

  ucComMF522Buf[0] = PICC_ANTICOLL1;
  ucComMF522Buf[1] = 0x20;

  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

  if (status == MI_OK)
  {
    for (i=0; i<4; i++)
    {
      *(pSnr+i)  = ucComMF522Buf[i];
      snr_check ^= ucComMF522Buf[i];

    }
    if (snr_check != ucComMF522Buf[i])
    {   status = MI_ERR;    }
  }

  SetBitMask(CollReg,0x80);
  return status;
}

////////////////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回 MI_OK
////////////////////////////////////////////////////////////////////////////////
char PcdSelect(unsigned char *pSnr)
{
  char status;
  unsigned char i;
  unsigned int  unLen;
  unsigned char ucComMF522Buf[MAXRLEN];

  ucComMF522Buf[0] = PICC_ANTICOLL1;
  ucComMF522Buf[1] = 0x70;
  ucComMF522Buf[6] = 0;
  for (i=0; i<4; i++)
  {
    ucComMF522Buf[i+2] = *(pSnr+i);
    ucComMF522Buf[6]  ^= *(pSnr+i);
  }
  CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);

  ClearBitMask(Status2Reg,0x08);

  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);

  if ((status == MI_OK) && (unLen == 0x18))
  {   status = MI_OK;  }
  else
  {   status = MI_ERR;    }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
////////////////////////////////////////////////////////////////////////////////
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
  char status;
  unsigned int  unLen;
  unsigned char i,ucComMF522Buf[MAXRLEN];

  ucComMF522Buf[0] = auth_mode;
  ucComMF522Buf[1] = addr;
  for (i=0; i<6; i++)
  {    ucComMF522Buf[i+2] = *(pKey+i);   }
  for (i=0; i<6; i++)
  {    ucComMF522Buf[i+8] = *(pSnr+i);   }
 //   memcpy(&ucComMF522Buf[2], pKey, 6);
 //   memcpy(&ucComMF522Buf[8], pSnr, 4);

  status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
  if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
  {   status = MI_ERR;   }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
//功    能：读取 M1 卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
////////////////////////////////////////////////////////////////////////////////
char PcdRead(unsigned char addr,unsigned char *pData)
{
  char status;
  unsigned int  unLen;
  unsigned char i,ucComMF522Buf[MAXRLEN];

  ucComMF522Buf[0] = PICC_READ;
  ucComMF522Buf[1] = addr;
  CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
 //   {   memcpy(pData, ucComMF522Buf, 16);   }
  if ((status == MI_OK) && (unLen == 0x90))
  {
    for (i=0; i<16; i++)
    { *(pData+i) = ucComMF522Buf[i];   }
  }
  else
  { status = MI_ERR;   }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
//功    能：写数据到 M1 卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
////////////////////////////////////////////////////////////////////////////////
char PcdWrite(unsigned char addr,unsigned char *pData)
{
  char status;
  unsigned int  unLen;
  unsigned char i,ucComMF522Buf[MAXRLEN];

  ucComMF522Buf[0] = PICC_WRITE;
  ucComMF522Buf[1] = addr;
  CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

  if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
  {   status = MI_ERR;   }

  if (status == MI_OK)
  {
    //memcpy(ucComMF522Buf, pData, 16);
    for (i=0; i<16; i++)
    {    ucComMF522Buf[i] = *(pData+i);   }
    CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
//功    能：扣款和充值
//参数说明: dd_mode[IN]：命令字
//               0xC0 = 扣款
//               0xC1 = 充值
//          addr[IN]：钱包地址
//          pValue[IN]：4字节增(减)值，低位在前
//返    回: 成功返回MI_OK
////////////////////////////////////////////////////////////////////////////////
char PcdValue(unsigned char dd_mode,unsigned char addr,unsigned char *pValue)
{
  char status;
  unsigned int  unLen;
  unsigned char i,ucComMF522Buf[MAXRLEN];

  ucComMF522Buf[0] = dd_mode;
  ucComMF522Buf[1] = addr;
  CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

  if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
  {   status = MI_ERR;   }

  if (status == MI_OK)
  {
    // memcpy(ucComMF522Buf, pValue, 4);
    for (i=0; i<16; i++)
    {    ucComMF522Buf[i] = *(pValue+i);   }
    CalulateCRC(ucComMF522Buf,4,&ucComMF522Buf[4]);
    unLen = 0;
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
    if (status != MI_ERR)
    {    status = MI_OK;    }
  }

  if (status == MI_OK)
  {
    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
  }
  return status;
}

////////////////////////////////////////////////////////////////////////////////
//功    能：备份钱包
//参数说明: sourceaddr[IN]：源地址
//          goaladdr[IN]：目标地址
//返    回: 成功返回MI_OK
////////////////////////////////////////////////////////////////////////////////
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr)
{
  char status;
  unsigned int  unLen;
  unsigned char ucComMF522Buf[MAXRLEN];

  ucComMF522Buf[0] = PICC_RESTORE;
  ucComMF522Buf[1] = sourceaddr;
  CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

  if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
  {   status = MI_ERR;   }

  if (status == MI_OK)
  {
    ucComMF522Buf[0] = 0;
    ucComMF522Buf[1] = 0;
    ucComMF522Buf[2] = 0;
    ucComMF522Buf[3] = 0;
    CalulateCRC(ucComMF522Buf,4,&ucComMF522Buf[4]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
    if (status != MI_ERR)
    {    status = MI_OK;    }
  }

  if (status != MI_OK)
  { return MI_ERR;   }

  ucComMF522Buf[0] = PICC_TRANSFER;
  ucComMF522Buf[1] = goaladdr;

  CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

  if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
  {   status = MI_ERR;   }

  return status;
}


////////////////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回 MI_OK
////////////////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
  char status;
  unsigned int  unLen;
  unsigned char ucComMF522Buf[MAXRLEN];

  ucComMF522Buf[0] = PICC_HALT;
  ucComMF522Buf[1] = 0;
  CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

  status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

  return MI_OK;
}

////////////////////////////////////////////////////////////////////////////////
//用 MFRC522 计算 CRC16 函数
////////////////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
  unsigned char i,n;
  ClearBitMask(DivIrqReg,0x04);
  WriteRawRC(CommandReg,PCD_IDLE);
  SetBitMask(FIFOLevelReg,0x80);
  for (i=0; i<len; i++)
  {   WriteRawRC(FIFODataReg, *(pIndata+i));   }
  WriteRawRC(CommandReg, PCD_CALCCRC);
  i = 0xFF;
  do
  {
    n = ReadRawRC(DivIrqReg);
    i--;
  }
  while ((i!=0) && !(n&0x04));
  pOutData[0] = ReadRawRC(CRCResultRegL);
  pOutData[1] = ReadRawRC(CRCResultRegM);
}

////////////////////////////////////////////////////////////////////////////////
//功    能：复位 MFRC522
//返    回: 成功返回 MI_OK
////////////////////////////////////////////////////////////////////////////////
char PcdReset(void)
{
  volatile unsigned char i;

  MFRC522_RST_HIGH();
  for(i=0;i<127;i++)  __NOP();

  MFRC522_RST_LOW();
  for(i=0;i<255;i++)  __NOP();
  for(i=0;i<255;i++)  __NOP();

  MFRC522_RST_HIGH();
  for(i=0;i<255;i++)  __NOP();
  for(i=0;i<255;i++)  __NOP();
  for(i=0;i<255;i++)  __NOP();

//  WriteRawRC(CommandReg,PCD_RESETPHASE);
//  for(i=0;i<255;i++)  __NOP();
//  for(i=0;i<255;i++)  __NOP();
//  for(i=0;i<255;i++)  __NOP();

  WriteRawRC(ModeReg,0x3D);            // 和Mifare卡通讯，CRC初始值0x6363
  WriteRawRC(TReloadRegL,30);
  WriteRawRC(TReloadRegH,0);
  WriteRawRC(TModeReg,0x8D);
  WriteRawRC(TPrescalerReg,0x3E);
  WriteRawRC(TxAutoReg,0x40);
#ifdef USED_MFRC522_PA
  /* 调整接收增益，输出电抗等参数，使 MFRC522-PA 模块有更远的读写卡距离 */
  /* MFRC522-PA, https://item.taobao.com/item.htm?id=552002236051       */
  /* 以下参数可能并不适用于其他类似的模块，但可作为参考                 */
  /* 至于为什么这样调整，请参看 MFRC522 芯片手册以及 NXP 的相关文档     */
  WriteRawRC(RFCfgReg, 0x50);       // RxGain = 38dB
  //WriteRawRC(RFCfgReg, 0x70);       // RxGain = 48dB
  WriteRawRC(GsNReg, 0xF4);         // default = 0x88
  WriteRawRC(CWGsCfgReg, 0x3F);     // default = 0x20
  WriteRawRC(ModGsCfgReg, 0x11);    // default = 0x20
#endif
  return MI_OK;
}

////////////////////////////////////////////////////////////////////////////////
//功    能：读 MFRC522 寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
////////////////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address)
{
  unsigned char i, ucAddr;
  unsigned short retry;

  MFRC522_NSS_LOW();
  __NOP();  __NOP();
  __NOP();  __NOP();
//  for(i=0;i<127;i++)  __NOP();

  retry = 0;
  /* Loop while DR register in not emplty */
  while((SSP1->SR & SSP_FLAG_TNF) == RESET);
//  while((SSP1->SR & SSP_FLAG_TFE) == RESET);
//  {
//    retry++;
//    if(retry>40000){
//      return 0;
//    }
//  }
  SSP1->DR = ((Address<<1)&0x7E)|0x80;

  retry = 0;
  /* Wait to receive a byte */
  // while (SSP_GetFlagStatus (SSP1, SSP_FLAG_RNE) == RESET)
//  while((SSP1->SR & SSP_FLAG_TNF) == RESET);
  while((SSP1->SR & SSP_FLAG_RNE) == RESET);
//  {
//    retry++;
//    if(retry>40000){
//      return 0;
//    }
//  }
  i = SSP1->DR;

  retry=0;
  /* Loop while DR register in not emplty */
  while((SSP1->SR & SSP_FLAG_TNF) == RESET);
//  while((SSP1->SR & SSP_FLAG_TNF) == RESET);
//  {
//    retry++;
//    if(retry>40000){
//      return 0;
//    }
//  }
  SSP1->DR = 0x00;

  retry=0;
  /* Wait to receive a byte */
  // while (SSP_GetFlagStatus (SSP1, SSP_FLAG_RNE) == RESET)
//  while((SSP1->SR & SSP_FLAG_TNF) == RESET);
  while((SSP1->SR & SSP_FLAG_RNE) == RESET);
//  {
//    retry++;
//    if(retry>40000){
//      return 0;
//    }
//  }

//  for(i=0;i<127;i++)  __NOP();
  __NOP();  __NOP();
  __NOP();  __NOP();
  i = SSP1->DR;
  MFRC522_NSS_HIGH();
  return i;
}

////////////////////////////////////////////////////////////////////////////////
//功    能：写 MFRC522 寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
////////////////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char Address, unsigned char value)
{
  unsigned char i, ucAddr;
  unsigned short retry=0;

  MFRC522_NSS_LOW();
  __NOP();  __NOP();
  __NOP();  __NOP();
//  for(i=0;i<127;i++)  __NOP();
  /* Loop while DR register in not emplty */
  while((SSP1->SR & SSP_FLAG_TNF) == RESET);
//  while((SSP1->SR & SSP_FLAG_TFE) == RESET);
//  {
//    retry++;
//    if(retry>40000){
//      break;
//    }
//  }
  SSP1->DR = ((Address<<1)&0x7E);
  retry=0;
//  while((SSP1->SR & SSP_FLAG_BSY) == SSP_FLAG_BSY);
  while((SSP1->SR & SSP_FLAG_RNE) == RESET);
//  {
//    retry++;
//    if(retry>40000){
//      break;
//    }
//  }
  i = SSP1->DR;

  while((SSP1->SR & SSP_FLAG_TNF) == RESET);
  SSP1->DR = value;
//  retry=0;
//  while((SSP1->SR & SSP_FLAG_TNF) == RESET)
//  {
//    retry++;
//    if(retry>40000){
//      break;
//    }
//  }
  retry=0;
//  while((SSP1->SR & SSP_FLAG_BSY) == SSP_FLAG_BSY);
  while((SSP1->SR & SSP_FLAG_RNE) == RESET);
//  while((SSP1->SR & SSP_FLAG_TFE) == RESET);
//  {
//    retry++;
//    if(retry>40000){
//      break;
//    }
//  }
  i = SSP1->DR;

//  for(i=0;i<127;i++)  __NOP();
  __NOP();  __NOP();
  __NOP();  __NOP();
  MFRC522_NSS_HIGH();

//  MF522_SCK = 0;
//  MF522_NSS = 0;
//  ucAddr = ((Address<<1)&0x7E);

//  for(i=8;i>0;i--)
//  {
//    MF522_SI = ((ucAddr&0x80)==0x80);
//    MF522_SCK = 1;
//    ucAddr <<= 1;
//    MF522_SCK = 0;
//  }

//  for(i=8;i>0;i--)
//  {
//    MF522_SI = ((value&0x80)==0x80);
//    MF522_SCK = 1;
//    value <<= 1;
//    MF522_SCK = 0;
//  }
//  MF522_NSS = 1;
//  MF522_SCK = 1;
}

////////////////////////////////////////////////////////////////////////////////
//功    能：置 MFRC522 寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
////////////////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char reg,unsigned char mask)
{
  char tmp = 0x0;
  tmp = ReadRawRC(reg);
  WriteRawRC(reg,tmp | mask);  // set bit mask
}

////////////////////////////////////////////////////////////////////////////////
//功    能：清 MFRC522 寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
////////////////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg,unsigned char mask)
{
  char tmp = 0x0;
  tmp = ReadRawRC(reg);
  WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

////////////////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]: MFRC522 命令字
//          pInData[IN]: 通过 MFRC522 发送到卡片的数据
//          InLenByte[IN]: 发送数据的字节长度
//          pOutData[OUT]: 接收到的卡片返回数据
//          *pOutLenBit[OUT]: 返回数据的位长度
////////////////////////////////////////////////////////////////////////////////
char PcdComMF522(unsigned char Command,
                 unsigned char *pInData,
                 unsigned char InLenByte,
                 unsigned char *pOutData,
                 unsigned int  *pOutLenBit)
{
  char status = MI_ERR;
  unsigned char irqEn   = 0x00;
  unsigned char waitFor = 0x00;
  unsigned char lastBits;
  unsigned char n;
  unsigned long int i;

  switch (Command)
  {
    case PCD_AUTHENT:
      irqEn   = 0x12;
      waitFor = 0x10;
      break;
    case PCD_TRANSCEIVE:
      irqEn   = 0x77;
      waitFor = 0x30;
      break;
    default:
      break;
  }

  WriteRawRC(ComIEnReg,irqEn|0x80);
  ClearBitMask(ComIrqReg,0x80);
  WriteRawRC(CommandReg,PCD_IDLE);
  SetBitMask(FIFOLevelReg,0x80);

  for (i=0; i<InLenByte; i++)
  {   WriteRawRC(FIFODataReg, pInData[i]);    }
  WriteRawRC(CommandReg, Command);

  if (Command == PCD_TRANSCEIVE)
  {    SetBitMask(BitFramingReg,0x80);  }

//  i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
  i = 600000;//根据时钟频率调整，操作M1卡最大等待时间25ms
  do
  {
    n = ReadRawRC(ComIrqReg);
    i--;
  } while ((i!=0) && !(n&0x01) && !(n&waitFor));

  ClearBitMask(BitFramingReg,0x80);

  if (i!=0)
  {
    if(!(ReadRawRC(ErrorReg)&0x1B))
    {
      status = MI_OK;
      if (n & irqEn & 0x01)
      {   status = MI_NOTAGERR;   }
      if (Command == PCD_TRANSCEIVE)
      {
        n = ReadRawRC(FIFOLevelReg);
        lastBits = ReadRawRC(ControlReg) & 0x07;
        if (lastBits)
        {   *pOutLenBit = (n-1)*8 + lastBits;   }
        else
        {   *pOutLenBit = n*8;   }
        if (n == 0)
        {   n = 1;    }
        if (n > MAXRLEN)
        {   n = MAXRLEN;   }
        for (i=0; i<n; i++)
        {   pOutData[i] = ReadRawRC(FIFODataReg);    }
      }
    }
    else
    {   status = MI_ERR;   }

 }

  SetBitMask(ControlReg,0x80);           // stop timer now
  WriteRawRC(CommandReg,PCD_IDLE);
  return status;
}

////////////////////////////////////////////////////////////////////////////////
//开启天线
//每次启动或关闭天线发射之间应至少有1ms的间隔
////////////////////////////////////////////////////////////////////////////////
void PcdAntennaOn()
{
  unsigned char i;
  i = ReadRawRC(TxControlReg);
  if (!(i & 0x03))
  {
    SetBitMask(TxControlReg, 0x03);
  }
}

////////////////////////////////////////////////////////////////////////////////
//关闭天线
////////////////////////////////////////////////////////////////////////////////
void PcdAntennaOff()
{
  ClearBitMask(TxControlReg, 0x03);
}
