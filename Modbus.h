#ifndef __MODBUS_H__
#define __MODBUS_H__


#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	检查数据的crc,payload：整帧数据(包含CRC),payload_length:长度(包含CRC)
*/
bool Modbus_Payload_Check_CRC(uint8_t *payload,size_t payload_length);

/*
	添加末尾的crc校验,payload：整帧数据(不包含CRC),payload_length:长度(包含CRC)
*/
bool Modbus_Payload_Append_CRC(uint8_t *payload,size_t payload_length);

/*
	slave的上下文
*/
typedef struct
{
    uint8_t slave_addr;//从机地址

    void (*output)(uint8_t *data,size_t data_length);//数据输出,正常情况不可为NULL

    bool (*read_IX)(size_t addr);//读取输入线圈

    void (*write_OX)(size_t addr,uint16_t data);//写线圈
    bool (*read_OX)(size_t addr);//读线圈

    uint16_t (*read_hold_register)(size_t addr);//读保持寄存器
    void (*write_hold_register)(size_t addr,uint16_t data);//写保持寄存器

    uint16_t (*read_input_register)(size_t addr);//读输入寄存器


} modbus_slave_context_t;

/*
Modbus从机解析输入。ctx：上下文指针,input_data:输入数据指针,input_data_length:输入数据长度,buff:缓冲(存放临时数据),buff_length:缓冲长度(需大于等于输入数据长度)
*/

bool Modbus_Slave_Parse_Input(modbus_slave_context_t *ctx,uint8_t *input_data,size_t input_data_length,uint8_t *buff,size_t buff_length);

/*
    master的上下文
*/
typedef struct
{
    uint8_t slave_addr;

    void (*output)(uint8_t *buff,size_t buff_length);

    size_t (*request_reply)(uint8_t *buff,size_t buff_length);//请求数据，返回成功读取的数据长度

} modbus_master_context_t;

/*
Modbus主机请求读取系列函数。ctx:上下文指针,start_addr:起始地址(寻址地址),data:待读取/写入的数据指针,number：待读取/写入的数据长度，buff:缓冲，buff_length:缓冲长度
*/

bool Modbus_Master_Read_IX(modbus_master_context_t *ctx,uint16_t start_addr,bool *data,size_t number,uint8_t *buff,size_t buff_length);

bool Modbus_Master_Read_OX(modbus_master_context_t *ctx,uint16_t start_addr,bool *data,size_t number,uint8_t *buff,size_t buff_length);

bool Modbus_Master_Read_Hold_Register(modbus_master_context_t *ctx,uint16_t start_addr,uint16_t *data,size_t number,uint8_t *buff,size_t buff_length);

bool Modbus_Master_Read_Input_Register(modbus_master_context_t *ctx,uint16_t start_addr,uint16_t *data,size_t number,uint8_t *buff,size_t buff_length);

bool Modbus_Master_Write_OX(modbus_master_context_t *ctx,uint16_t start_addr,bool *data,size_t number,uint8_t *buff,size_t buff_length);

bool Modbus_Master_Write_Hold_Register(modbus_master_context_t *ctx,uint16_t start_addr,uint16_t *data,size_t number,uint8_t *buff,size_t buff_length);

#ifdef __cplusplus
}
#endif


#endif

