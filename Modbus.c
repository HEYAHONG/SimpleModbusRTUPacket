/** \file Modbus.c
 *  \brief     Modbus RTU模式下数据包处理C源代码
 *  \author    何亚红
 *  \version   20220203
 *  \date      2022
 *  \copyright MIT License.
 */

#include "Modbus.h"

static uint16_t Modbus_ReadUint16_From_2Bytes(uint8_t *pos)
{
    //modbus的16位数据高字节在前,低字节在后
    uint16_t ret=pos[0];
    ret<<=8;
    ret+=pos[1];

    return ret;
};

static void Modbus_WriteUint16_To_2Bytes(uint8_t *pos,uint16_t dat)
{
    //modbus的16位数据高字节在前,低字节在后
    pos[0]=(dat>>8);
    pos[1]=(dat&0xff);
}

static uint16_t CRC16(uint8_t *arr_buff,size_t len)
{
    uint16_t crc=0xFFFF;
    size_t  i, j;
    for ( j=0; j<len; j++)
    {
        crc=crc ^*arr_buff++;
        for ( i=0; i<8; i++)
        {
            if( ( crc&0x0001) >0)
            {
                crc=crc>>1;
                crc=crc^ 0xa001;
            }
            else
                crc=crc>>1;
        }
    }
    return ( crc);
}

/*
	检查数据的crc,payload：整帧数据(包含CRC),payload_length:长度(包含CRC)
*/
bool Modbus_Payload_Check_CRC(uint8_t *payload,size_t payload_length)
{
    if(payload_length<=2)
    {
        return false;
    }

    uint16_t crc1=CRC16(payload,payload_length-2);
    uint16_t crc2=payload[payload_length-1];
    crc2<<=8;
    crc2+=payload[payload_length-2];

    return crc1==crc2;

}

/*
	添加末尾的crc校验,payload：整帧数据(不包含CRC),payload_length:长度(包含CRC)
*/
bool Modbus_Payload_Append_CRC(uint8_t *payload,size_t payload_length)
{
    if(payload_length<=2)
    {
        return false;
    }

    uint16_t crc=CRC16(payload,payload_length-2);

    payload[payload_length-2]=(crc&0xff);
    payload[payload_length-1]=(crc>>8);

    return true;
}

/*
Modbus从机解析输入。ctx：上下文指针,input_data:输入数据指针,input_data_length:输入数据长度,buff:缓冲(存放临时数据),buff_length:缓冲长度
*/

bool Modbus_Slave_Parse_Input(modbus_slave_context_t *ctx,uint8_t *input_data,size_t input_data_length,uint8_t *buff,size_t buff_length)
{
    if(ctx==NULL  || input_data ==NULL || input_data_length <=2 || buff==NULL || buff_length <=2 || buff_length<input_data_length)
    {
        return false;
    }

    if(!Modbus_Payload_Check_CRC(input_data,input_data_length))
    {
        return false;
    }

    size_t output_length=0;

    switch(input_data[1])
    {
    case 0x01:
    {
        //读取线圈

        if(input_data[0]!=ctx->slave_addr)
        {
            //非本从机
            break;
        }

        if(input_data!=buff)
        {
            //将下发的指令复制到buff
            memcpy(buff,input_data,input_data_length);
        }


        if(ctx->read_OX!=NULL)
        {
            uint16_t start_addr=Modbus_ReadUint16_From_2Bytes(&buff[2]);
            uint16_t length=Modbus_ReadUint16_From_2Bytes(&buff[4]);
            uint8_t byte_count=length/8+((length%8!=0)?1:0);
            buff[2]=byte_count;

            output_length=5+byte_count;

            if(output_length>buff_length)
            {
                break;
            }

            for(size_t i=0; i<length; i++)
            {
                if(i%8==0)
                {
                    buff[3+i/8]=0xff;
                }

                if(ctx->read_OX(start_addr+i))
                {
                    buff[3+i/8]|=(0x01<<(i%8));
                }
                else
                {
                    buff[3+i/8]&=(~(0x01<<(i%8)));
                }
            }


        }

    }
    break;

    case 0x02:
    {
        //读输入线圈

        if(input_data[0]!=ctx->slave_addr)
        {
            //非本从机
            break;
        }

        if(input_data!=buff)
        {
            //将下发的指令复制到buff
            memcpy(buff,input_data,input_data_length);
        }


        if(ctx->read_IX!=NULL)
        {
            uint16_t start_addr=Modbus_ReadUint16_From_2Bytes(&buff[2]);
            uint16_t length=Modbus_ReadUint16_From_2Bytes(&buff[4]);
            uint8_t byte_count=length/8+((length%8!=0)?1:0);
            buff[2]=byte_count;

            output_length=5+byte_count;

            if(output_length>buff_length)
            {
                break;
            }

            for(size_t i=0; i<length; i++)
            {
                if(i%8==0)
                {
                    buff[3+i/8]=0xff;
                }

                if(ctx->read_IX(start_addr+i))
                {
                    buff[3+i/8]|=(0x01<<(i%8));
                }
                else
                {
                    buff[3+i/8]&=(~(0x01<<(i%8)));
                }
            }


        }

    }
    break;
    case 0x03:
    {
        //读保持寄存器
        if(input_data[0]!=ctx->slave_addr)
        {
            //非本从机
            break;
        }

        if(input_data!=buff)
        {
            //将下发的指令复制到buff
            memcpy(buff,input_data,input_data_length);
        }


        if(ctx->read_hold_register!=NULL)
        {
            uint16_t start_addr=Modbus_ReadUint16_From_2Bytes(&buff[2]);
            uint16_t length=Modbus_ReadUint16_From_2Bytes(&buff[4]);
            if(length >127)
            {
                break;
            }
            uint8_t byte_count=length*2;
            buff[2]=byte_count;

            output_length=5+byte_count;

            if(output_length>buff_length)
            {
                break;
            }

            for(size_t i=0; i<length; i++)
            {
                uint16_t dat=ctx->read_hold_register(start_addr+i);
                Modbus_WriteUint16_To_2Bytes(&buff[3+2*i],dat);
            }


        }
    }
    break;

    case 0x04:
    {
        //读输入寄存器
        if(input_data[0]!=ctx->slave_addr)
        {
            //非本从机
            break;
        }

        if(input_data!=buff)
        {
            //将下发的指令复制到buff
            memcpy(buff,input_data,input_data_length);
        }


        if(ctx->read_input_register!=NULL)
        {
            uint16_t start_addr=Modbus_ReadUint16_From_2Bytes(&buff[2]);
            uint16_t length=Modbus_ReadUint16_From_2Bytes(&buff[4]);
            if(length >127)
            {
                break;
            }
            uint8_t byte_count=length*2;
            buff[2]=byte_count;

            output_length=5+byte_count;

            if(output_length>buff_length)
            {
                break;
            }

            for(size_t i=0; i<length; i++)
            {
                uint16_t dat=ctx->read_input_register(start_addr+i);
                Modbus_WriteUint16_To_2Bytes(&buff[3+2*i],dat);
            }


        }

    }
    break;

    case 0x05:
    {
        //强制设置单个输出线圈
        if(input_data[0]!=ctx->slave_addr)
        {
            //非本从机
            break;
        }

        if(input_data!=buff)
        {
            //将下发的指令复制到buff
            memcpy(buff,input_data,input_data_length);
        }

        output_length=input_data_length;

        if(ctx->write_OX!=NULL)
        {
            uint16_t addr=Modbus_ReadUint16_From_2Bytes(&buff[2]);
            uint16_t data=Modbus_ReadUint16_From_2Bytes(&buff[4]);
            ctx->write_OX(addr,data);
        }

    }
    break;

    case 0x06:
    {
        //设置单个保持寄存器
        if(input_data[0]!=ctx->slave_addr)
        {
            //非本从机
            break;
        }

        if(input_data!=buff)
        {
            //将下发的指令复制到buff
            memcpy(buff,input_data,input_data_length);
        }

        output_length=input_data_length;

        if(ctx->write_hold_register!=NULL)
        {
            uint16_t addr=Modbus_ReadUint16_From_2Bytes(&buff[2]);
            uint16_t data=Modbus_ReadUint16_From_2Bytes(&buff[4]);
            ctx->write_hold_register(addr,data);
        }

    }
    break;

    case 0x0F:
    {
        //设置多个输出线圈
        if(input_data[0]!=ctx->slave_addr && input_data[0]==0)
        {
            //非本从机或广播地址
            break;
        }

        if(input_data!=buff)
        {
            //将下发的指令复制到buff
            memcpy(buff,input_data,input_data_length);
        }

        buff[0]=ctx->slave_addr;
        output_length=8;

        if(ctx->write_OX!=NULL)
        {
            uint16_t start_addr=Modbus_ReadUint16_From_2Bytes(&buff[2]);
            uint16_t length=Modbus_ReadUint16_From_2Bytes(&buff[4]);

            for(size_t i=0; i<length; i++)
            {
                if((buff[7+i/8]&(0x01<<(i%8)))!=0)
                {
                    ctx->write_OX(start_addr+i,0xFF00);
                }
                else
                {
                    ctx->write_OX(start_addr+i,0x0000);
                }
            }


        }
    }
    break;
    case 0x10:
    {
        //设置多个保持寄存器
        if(input_data[0]!=ctx->slave_addr && input_data[0]==0)
        {
            //非本从机或广播地址
            break;
        }

        if(input_data!=buff)
        {
            //将下发的指令复制到buff
            memcpy(buff,input_data,input_data_length);
        }

        buff[0]=ctx->slave_addr;
        output_length=8;

        if(ctx->write_hold_register!=NULL)
        {
            uint16_t start_addr=Modbus_ReadUint16_From_2Bytes(&buff[2]);
            uint16_t length=Modbus_ReadUint16_From_2Bytes(&buff[4]);

            for(size_t i=0; i<length; i++)
            {
                ctx->write_hold_register(start_addr+i,Modbus_ReadUint16_From_2Bytes(&buff[7+2*i]));
            }

        }


    }
    break;

    default:
        break;
    }

    if(output_length>2)
    {
        if(buff_length>=output_length)
        {
            Modbus_Payload_Append_CRC(buff,output_length);
            if(ctx->output!=NULL)
            {
                ctx->output(buff,output_length);
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool Modbus_Master_Read_OX(modbus_master_context_t *ctx,uint16_t start_addr,bool *data,size_t number,uint8_t *buff,size_t buff_length)
{
    if(ctx==NULL || data ==NULL || buff == NULL || number == 0 || buff_length == 0 || ctx->output ==NULL ||ctx->request_reply ==NULL)
    {
        //参数不正确
        return false;
    }

    if(number > MODBUS_MAX_READ_BITS)
    {
        return false;
    }

    size_t output_length=8;
    size_t input_length=5+number/8+((number%8==0)?0:1);

    if(output_length>buff_length || input_length> buff_length)
    {
        return false;
    }

    {
        //填写参数
        buff[0]=ctx->slave_addr;
        buff[1]=0x01;//查询输出状态
        Modbus_WriteUint16_To_2Bytes(&buff[2],start_addr);
        Modbus_WriteUint16_To_2Bytes(&buff[4],number);
        Modbus_Payload_Append_CRC(buff,output_length);
    }

    ctx->output(buff,output_length);

    if(input_length== ctx->request_reply(buff,input_length))
    {
        if(Modbus_Payload_Check_CRC(buff,input_length))
        {
            for(size_t i=0; i<number; i++)
            {
                data[i]=((buff[3+i/8]&(0x01<<(i%8)))!=0);
            }

            return true;
        }
    }



    return false;

}

bool Modbus_Master_Read_IX(modbus_master_context_t *ctx,uint16_t start_addr,bool *data,size_t number,uint8_t *buff,size_t buff_length)
{
    if(ctx==NULL || data ==NULL || buff == NULL || number == 0 || buff_length == 0 || ctx->output ==NULL ||ctx->request_reply ==NULL)
    {
        //参数不正确
        return false;
    }

    if(number > MODBUS_MAX_READ_BITS)
    {
        return false;
    }

    size_t output_length=8;
    size_t input_length=5+number/8+((number%8==0)?0:1);

    if(output_length>buff_length || input_length> buff_length)
    {
        return false;
    }

    {
        //填写参数
        buff[0]=ctx->slave_addr;
        buff[1]=0x02;//查询输入状态
        Modbus_WriteUint16_To_2Bytes(&buff[2],start_addr);
        Modbus_WriteUint16_To_2Bytes(&buff[4],number);
        Modbus_Payload_Append_CRC(buff,output_length);
    }

    ctx->output(buff,output_length);

    if(input_length== ctx->request_reply(buff,input_length))
    {
        if(Modbus_Payload_Check_CRC(buff,input_length))
        {
            for(size_t i=0; i<number; i++)
            {
                data[i]=((buff[3+i/8]&(0x01<<(i%8)))!=0);
            }

            return true;
        }
    }



    return false;

}

bool Modbus_Master_Read_Hold_Register(modbus_master_context_t *ctx,uint16_t start_addr,uint16_t *data,size_t number,uint8_t *buff,size_t buff_length)
{
    if(ctx==NULL || data ==NULL || buff == NULL || number == 0 || buff_length == 0 || ctx->output ==NULL ||ctx->request_reply ==NULL)
    {
        //参数不正确
        return false;
    }

    if(number > MODBUS_MAX_READ_REGISTERS)
    {
        return false;
    }

    size_t output_length=8;
    size_t input_length=5+number*2;

    if(output_length>buff_length || input_length> buff_length)
    {
        return false;
    }

    {
        //填写参数
        buff[0]=ctx->slave_addr;
        buff[1]=0x03;//查询保持寄存器
        Modbus_WriteUint16_To_2Bytes(&buff[2],start_addr);
        Modbus_WriteUint16_To_2Bytes(&buff[4],number);
        Modbus_Payload_Append_CRC(buff,output_length);
    }

    ctx->output(buff,output_length);

    if(input_length== ctx->request_reply(buff,input_length))
    {
        if(Modbus_Payload_Check_CRC(buff,input_length))
        {
            for(size_t i=0; i<number; i++)
            {
                data[i]=Modbus_ReadUint16_From_2Bytes(&buff[3+2*i]);
            }

            return true;
        }
    }



    return false;

}

bool Modbus_Master_Read_Input_Register(modbus_master_context_t *ctx,uint16_t start_addr,uint16_t *data,size_t number,uint8_t *buff,size_t buff_length)
{
    if(ctx==NULL || data ==NULL || buff == NULL || number == 0 || buff_length == 0 || ctx->output ==NULL ||ctx->request_reply ==NULL)
    {
        //参数不正确
        return false;
    }

    if(number > MODBUS_MAX_READ_REGISTERS)
    {
        return false;
    }

    size_t output_length=8;
    size_t input_length=5+number*2;

    if(output_length>buff_length || input_length> buff_length)
    {
        return false;
    }

    {
        //填写参数
        buff[0]=ctx->slave_addr;
        buff[1]=0x04;//查询输入寄存器
        Modbus_WriteUint16_To_2Bytes(&buff[2],start_addr);
        Modbus_WriteUint16_To_2Bytes(&buff[4],number);
        Modbus_Payload_Append_CRC(buff,output_length);
    }

    ctx->output(buff,output_length);

    if(input_length== ctx->request_reply(buff,input_length))
    {
        if(Modbus_Payload_Check_CRC(buff,input_length))
        {
            for(size_t i=0; i<number; i++)
            {
                data[i]=Modbus_ReadUint16_From_2Bytes(&buff[3+2*i]);
            }

            return true;
        }
    }



    return false;

}

/*
单个线圈
*/
static bool Modbus_Master_Write_OX_05(modbus_master_context_t *ctx,uint16_t start_addr,bool *data,size_t number,uint8_t *buff,size_t buff_length)
{
    if(ctx==NULL || data ==NULL || buff == NULL || number == 0 || buff_length == 0 || ctx->output ==NULL ||ctx->request_reply ==NULL)
    {
        //参数不正确
        return false;
    }

    size_t output_length=8;
    size_t input_length=8;

    if(output_length>buff_length || input_length> buff_length)
    {
        return false;
    }

    {
        //填写参数
        buff[0]=ctx->slave_addr;
        buff[1]=0x05;//强制单个线圈
        Modbus_WriteUint16_To_2Bytes(&buff[2],start_addr);
        Modbus_WriteUint16_To_2Bytes(&buff[4],data[0]?(0xFF00):(0x0000));
        buff[6]=number/8+((number%8!=0)?1:0);


        Modbus_Payload_Append_CRC(buff,output_length);
    }

    ctx->output(buff,output_length);

    if(input_length== ctx->request_reply(buff,input_length))
    {
        if(Modbus_Payload_Check_CRC(buff,input_length))
        {
            return true;
        }
    }



    return false;
}

bool Modbus_Master_Write_OX(modbus_master_context_t *ctx,uint16_t start_addr,bool *data,size_t number,uint8_t *buff,size_t buff_length)
{
    if(ctx==NULL || data ==NULL || buff == NULL || number == 0 || buff_length == 0 || ctx->output ==NULL ||ctx->request_reply ==NULL)
    {
        //参数不正确
        return false;
    }

    if(number > MODBUS_MAX_WRITE_BITS)
    {
        return false;
    }

    if(number==1)
    {
        return Modbus_Master_Write_OX_05(ctx,start_addr,data,number,buff,buff_length);
    }

    size_t output_length=9+number/8+((number%8!=0)?1:0);
    size_t input_length=8;

    if(output_length>buff_length || input_length> buff_length)
    {
        return false;
    }

    {
        //填写参数
        buff[0]=ctx->slave_addr;
        buff[1]=0x0F;//强制多个线圈
        Modbus_WriteUint16_To_2Bytes(&buff[2],start_addr);
        Modbus_WriteUint16_To_2Bytes(&buff[4],number);
        buff[6]=number/8+((number%8!=0)?1:0);

        for(size_t i=0; i<number; i++)
        {
            if(i%8==0)
            {
                buff[7+i/8]=0xFF;
            }
            if(data[i])
            {
                buff[7+i/8]|=(0x01<<(i%8));
            }
            else
            {
                buff[7+i/8]&=(~(0x01<<(i%8)));
            }
        }

        Modbus_Payload_Append_CRC(buff,output_length);
    }

    ctx->output(buff,output_length);

    if(input_length== ctx->request_reply(buff,input_length))
    {
        if(Modbus_Payload_Check_CRC(buff,input_length))
        {
            return true;
        }
    }



    return false;
}


static bool Modbus_Master_Write_Hold_Register_06(modbus_master_context_t *ctx,uint16_t start_addr,uint16_t *data,size_t number,uint8_t *buff,size_t buff_length)
{
    if(ctx==NULL || data ==NULL || buff == NULL || number == 0 || buff_length == 0 || ctx->output ==NULL ||ctx->request_reply ==NULL)
    {
        //参数不正确
        return false;
    }

    size_t output_length=8;
    size_t input_length=8;

    if(output_length>buff_length || input_length> buff_length)
    {
        return false;
    }

    {
        //填写参数
        buff[0]=ctx->slave_addr;
        buff[1]=0x06;//强制单个保持寄存器
        Modbus_WriteUint16_To_2Bytes(&buff[2],start_addr);
        Modbus_WriteUint16_To_2Bytes(&buff[4],data[0]);
        Modbus_Payload_Append_CRC(buff,output_length);
    }

    ctx->output(buff,output_length);

    if(input_length== ctx->request_reply(buff,input_length))
    {
        if(Modbus_Payload_Check_CRC(buff,input_length))
        {
            return true;
        }
    }



    return false;
}

bool Modbus_Master_Write_Hold_Register(modbus_master_context_t *ctx,uint16_t start_addr,uint16_t *data,size_t number,uint8_t *buff,size_t buff_length)
{
    if(ctx==NULL || data ==NULL || buff == NULL || number == 0 || buff_length == 0 || ctx->output ==NULL ||ctx->request_reply ==NULL)
    {
        //参数不正确
        return false;
    }

    if(number > MODBUS_MAX_WRITE_REGISTERS)
    {
        return false;
    }

    if(number==1)
    {
        return Modbus_Master_Write_Hold_Register_06(ctx,start_addr,data,number,buff,buff_length);
    }

    size_t output_length=9+number*2;
    size_t input_length=8;

    if(output_length>buff_length || input_length> buff_length)
    {
        return false;
    }

    {
        //填写参数
        buff[0]=ctx->slave_addr;
        buff[1]=0x10;//强制多个线圈
        Modbus_WriteUint16_To_2Bytes(&buff[2],start_addr);
        Modbus_WriteUint16_To_2Bytes(&buff[4],number);
        buff[6]=number*2;

        for(size_t i=0; i<number; i++)
        {
            Modbus_WriteUint16_To_2Bytes(&buff[7+i*2],data[i]);
        }

        Modbus_Payload_Append_CRC(buff,output_length);
    }

    ctx->output(buff,output_length);

    if(input_length== ctx->request_reply(buff,input_length))
    {
        if(Modbus_Payload_Check_CRC(buff,input_length))
        {
            return true;
        }
    }



    return false;
}
