移植 W5500 的 loopback 例程到 W7500P 板子。
使用的是：ioLibrary_Driver
先初始化 W7500P 的 SPI1。使用硬件方式作为 CS 信号，即 PC_04 作为 GPIO 输出。
    PC_04 -> CS
    PB_01 -> SCK
    PB_02 <- MISO
    PB_03 -> MOSI
    PC_00 -> nReset

然后注册到 ioLibrary_Driver 里：
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
    reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);

void wizchip_select(void);
void wizchip_deselect(void);
uint8_t wizchip_read(void);
void wizchip_write(uint8_t wb);

在移植过程中，参考了两个实际工程例子： WIZ550SR 与 W5500_EVB。
在前一个工程例子中，找到了接口驱动的具体实现。
在后一个工程例子中，找到了 loopback 的具体实现。

WIZ550SR 可参见： https://github.com/Wiznet/WIZ550SR.git
W5500_EVB 可参见：https://github.com/Wiznet/W5500_EVB.git