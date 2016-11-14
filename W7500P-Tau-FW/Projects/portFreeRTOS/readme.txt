建立了 3 个任务。

任务 1：UART2 发送字符串“Test COM task01.”

任务 2：UART2 发送字符串“Test COM task02.”

任务 3：LED(D2)闪烁。
        当 LED 亮时，UART2 发送字符串“LED - ON.”
        当 LED 灭时，UART2 发送字符串“LED - OFF.”


使用串口终端软件，可看到返回的信息如下所示（仅截取一段）：

Test COM task01.

LED - OFF.

Test COM task02.

LED - ON.

Test COM task01.

LED - OFF.

Test COM task01.

Test COM task02.

LED - ON.

LED - OFF.



