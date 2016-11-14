![Fig.directory_structure](https://github.com/arm8686/W7500P-Tau/blob/master/pic/W7500P-Tau1.jpg)

该开发板可使用 CoLinkEx、Ulink、CMSIS-DAP 进行调试仿真，基于 Keil 编译器与 SWD 接口（没有 JTAG 接口）。

电路板尺寸：78mm × 55mm
安装孔位置：70mm × 47mm，孔径：3.2mm

所有的 GPIO 管脚全部引出，方便使用杜邦线连接外围模块，具体请参看原理图。

CN1：TF 卡座（SPI 接口）。
CN2：HR91005A RJ45 网络接口。
CN3：5.5×2.1mm 电源插座，输入为 DC 5V，内正外负。

CN4：SWD 调试接口 & ISP 调试接口。

CN5：2.54mm 间距 10Pin 双排母，引出 SPI 总线；可连接常见的 nRF24L01 无线模块，本店的 W5100 / W5200 / W5500 网络模块。排母引出的供电为+3.3V。

J13，J14，J15，J16：Arduino UNO R3 接口，默认为不焊接。

SW1：复位按键。
SW2：用户按键。
