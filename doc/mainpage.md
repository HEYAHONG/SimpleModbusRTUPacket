# 说明

极其简易的Modbus-RTU模式数据包处理。

一般用于单片机，本代码只包含数据包处理程序，时序部分需要在单片机其它代码中实现。

特点如下:

- 代码极其简易，便于移植。
- 可用于各类带硬件串口的单片机。
- 使用简单

缺陷:

- 串口驱动以及Modbus时序需要自行编写。
- 未进行某些检查。

# 使用

注意事项:

- 本代码中的地址都是从0开始的，但Modbus标准协议中地址规定是从1(数据帧中仍然是从0开始)开始。

- 所有需要缓冲的函数需要的缓冲推荐分配在栈上,因此在操作系统中需要将任务栈分配到足够大（若可以实现的话），对于一个经典的Modbus任务来讲，至少应当在其它需要的基础上额外分配256字节的栈空间专门用于接收与发送缓冲。可采用以下代码定义缓冲：

  ```c++
  /*
  源代码文件应包含Modbus.h
  */
  #include "Modbus.h"
  
  /*
  在需要调用Modbus函数时定义缓冲
  */
  uint8_t buff[MODBUS_RTU_MAX_ADU_LENGTH];
  /*
  定义好缓冲后，即可使用buff作为调用函数的buff参数，sizeof(buff)作为调用函数的buff_length参数。
  */
  ```

  

## 主机

主机主要使用 modbus_master_context_t 结构体及 Modbus_Master开头的函数。主要步骤如下:

- 定义 modbus_master_context_t 结构体,并填写相关成员(回调函数需自行定义，通常不可为NULL)。
- 当需要请求数据时,调用Modbus_Master系列函数。

## 从机

主机主要使用 modbus_slave_context_t 结构体及 Modbus_Slave_Parse_Input 函数。主要步骤如下:

- 定义 modbus_slave_context_t 结构体,并填写相关成员(回调函数需自行定义，通常不可为NULL)。
- 当串口接收到一帧数据时,调用 Modbus_Slave_Parse_Input 函数。
