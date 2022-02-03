/** \file Modbus.h
 *  \brief     Modbus RTU模式下数据包处理头文件
 *  \author    何亚红
 *  \version   20220203
 *  \date      2022
 *  \copyright MIT License.
 */

#ifndef __MODBUS_H__
#define __MODBUS_H__


#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \brief 检查一帧数据的crc
 *
 * \param payload 整帧数据(包含CRC)的指针
 * \param payload_length 长度(包含CRC)
 * \return CRC是否通过
 *
 */
bool Modbus_Payload_Check_CRC(uint8_t *payload,size_t payload_length);



/** \brief 在数据帧末尾的添加crc校验
 *
 * \param payload 整帧数据的指针，需要在末尾留两个字节以填写CRC
 * \param payload_length 长度(包含CRC)
 * \return 是否调用成功
 *
 */
bool Modbus_Payload_Append_CRC(uint8_t *payload,size_t payload_length);


typedef struct
{


    uint8_t slave_addr;/**< 从机地址,主要指本从机地址 */

    /** \brief 串口输出函数,当解析完成后，将调用此函数输出数据，不可为NULL。
     *
     * \param data 串口输出数据的指针
     * \param data_length 串口输出数据长度
     * \return
     *
     */
    void (*output)(uint8_t *data,size_t data_length);

    /** \brief 读取输入点（线圈）
     *
     * \param addr 地址
     * \return bool 当前状态
     *
     */
    bool (*read_IX)(size_t addr);

    /** \brief 写输出线圈
     *
     * \param addr 地址
     * \param data 数据,数据定义查看Modbus协议定义
     * \return
     *
     */
    void (*write_OX)(size_t addr,uint16_t data);
    /** \brief 读输出线圈
     *
     * \param addr 地址
     * \return 当前状态
     *
     */
    bool (*read_OX)(size_t addr);

    /** \brief 读保持寄存器
     *
     * \param addr 地址
     * \return 数据
     *
     */
    uint16_t (*read_hold_register)(size_t addr);
    /** \brief 写保持寄存器
     *
     * \param addr 地址
     * \param data 数据
     * \return
     *
     */
    void (*write_hold_register)(size_t addr,uint16_t data);

    /** \brief 读输入寄存器
     *
     * \param addr 地址
     * \return 数据
     *
     */
    uint16_t (*read_input_register)(size_t addr);


} modbus_slave_context_t/**< 从机的上下文结构定义 */;



/** \brief Modbus从机解析输入。
 * 当从机接收到一帧数据后，调用此函数。
 * 此函数会自动调用相关回调函数完成Modbus输出。
 * \param ctx 上下文指针,需要自行定义
 * \param input_data 输入数据指针
 * \param input_data_length 输入数据长度
 * \param buff 缓冲(存放临时数据)
 * \param buff_length 缓冲长度(需大于等于输入数据长度，足够存放输出数据)
 * \return 是否成功执行
 *
 */
bool Modbus_Slave_Parse_Input(modbus_slave_context_t *ctx,uint8_t *input_data,size_t input_data_length,uint8_t *buff,size_t buff_length);


typedef struct
{
    uint8_t slave_addr;/**< 从机地址，主要指与此主机通信的从机地址 */

    /** \brief 串口输出函数,当解析完成后，将调用此函数输出数据，不可为NULL。
     *
     * \param data 串口输出数据的指针
     * \param data_length 串口输出数据长度
     * \return
     *
     */
    void (*output)(uint8_t *data,size_t data_length);


    /** \brief 请求数据(读串口输入),当Modbus请求发出后，会调用此函数等待从机回应，不可为NULL。
     *
     * \param data 请求数据的指针
     * \param data_length 请求数据的长度(最大)
     * \return size_t 成功读取的数据长度
     *
     */
    size_t (*request_reply)(uint8_t *data,size_t data_length);

} modbus_master_context_t/**< 主机的上下文结构定义 */;



/** \brief Modbus主机读取输入点
 *
 * \param ctx 上下文指针,需要自行定义
 * \param start_addr 起始地址(寻址地址)
 * \param data 待读取的数据指针
 * \param number 待读取数据长度
 * \param buff 缓冲,用于发送和接收数据，尽量大
 * \param buff_length 缓冲长度
 * \return 是否成功执行
 *
 */
bool Modbus_Master_Read_IX(modbus_master_context_t *ctx,uint16_t start_addr,bool *data,size_t number,uint8_t *buff,size_t buff_length);

/** \brief Modbus主机读取输出线圈
 *
 * \param ctx 上下文指针,需要自行定义
 * \param start_addr 起始地址(寻址地址)
 * \param data 待读取的数据指针
 * \param number 待读取数据长度
 * \param buff 缓冲,用于发送和接收数据，尽量大
 * \param buff_length 缓冲长度
 * \return 是否成功执行
 *
 */
bool Modbus_Master_Read_OX(modbus_master_context_t *ctx,uint16_t start_addr,bool *data,size_t number,uint8_t *buff,size_t buff_length);

/** \brief Modbus主机读取保持寄存器
 *
 * \param ctx 上下文指针,需要自行定义
 * \param start_addr 起始地址(寻址地址)
 * \param data 待读取的数据指针
 * \param number 待读取数据长度
 * \param buff 缓冲,用于发送和接收数据，尽量大
 * \param buff_length 缓冲长度
 * \return 是否成功执行
 *
 */
bool Modbus_Master_Read_Hold_Register(modbus_master_context_t *ctx,uint16_t start_addr,uint16_t *data,size_t number,uint8_t *buff,size_t buff_length);

/** \brief Modbus主机读取输入寄存器
 *
 * \param ctx 上下文指针,需要自行定义
 * \param start_addr 起始地址(寻址地址)
 * \param data 待读取的数据指针
 * \param number 待读取数据长度
 * \param buff 缓冲,用于发送和接收数据，尽量大
 * \param buff_length 缓冲长度
 * \return 是否成功执行
 *
 */
bool Modbus_Master_Read_Input_Register(modbus_master_context_t *ctx,uint16_t start_addr,uint16_t *data,size_t number,uint8_t *buff,size_t buff_length);

/** \brief Modbus主机写输出线圈
 *
 * \param ctx 上下文指针,需要自行定义
 * \param start_addr 起始地址(寻址地址)
 * \param data 待写入的数据指针
 * \param number 待写入数据长度
 * \param buff 缓冲,用于发送和接收数据，尽量大
 * \param buff_length 缓冲长度
 * \return 是否成功执行
 *
 */
bool Modbus_Master_Write_OX(modbus_master_context_t *ctx,uint16_t start_addr,bool *data,size_t number,uint8_t *buff,size_t buff_length);

/** \brief Modbus主机写保持寄存器
 *
 * \param ctx 上下文指针,需要自行定义
 * \param start_addr 起始地址(寻址地址)
 * \param data 待写入的数据指针
 * \param number 待写入数据长度
 * \param buff 缓冲,用于发送和接收数据，尽量大
 * \param buff_length 缓冲长度
 * \return 是否成功执行
 *
 */
bool Modbus_Master_Write_Hold_Register(modbus_master_context_t *ctx,uint16_t start_addr,uint16_t *data,size_t number,uint8_t *buff,size_t buff_length);

#ifdef __cplusplus
}
#endif


#endif

