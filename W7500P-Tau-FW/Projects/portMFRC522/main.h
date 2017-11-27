
/* 控制引脚定义，使用 I/O 来模拟 SPI 接口 */
/* 用户可根据所使用的板卡资源进行修改     */
/* 请注意板卡上的引脚是否复用到其它模块   */

sbit  MF522_NSS = P1^7;
sbit  MF522_SCK = P1^6;
sbit  MF522_SI  = P1^5;
sbit  MF522_SO  = P1^4;

/* 使用 【MFRC522-PA】 模块时，+5V 工作的 MCU 不能接复位控制引脚 */
/* 因为该模块的复位电路上有二极管 1N4148，当 MCU 的复位管件输出为高时，影响到模块的电源 */
sbit     MF522_RST  = P2^0;   

/* 指示灯控制引脚，可不使用 */
sbit     LED_GREEN = P0^0;

/* 另一个控制引脚例子，供参考 */
//sbit     MF522_NSS  = P2^4;
//sbit     MF522_SCK  = P2^5;
//sbit     MF522_SI   = P2^3;
//sbit     MF522_SO   = P2^2;
//sbit     MF522_RST  = P2^1;
