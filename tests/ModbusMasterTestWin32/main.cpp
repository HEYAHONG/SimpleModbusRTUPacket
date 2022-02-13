#include "argtable3.h"
#include "Modbus.h"

extern "C"
{
#include "serialport.h"
}

/*
CLI程序命令参数表
*/
static struct arg_str *argcom=arg_strn("S","SerialPort","COMX",1,1,"Windows下的串口号");
static struct arg_lit *help=arg_lit0(NULL,"help", "打印帮助");

static void * argtable[]=
{
    argcom,
    help,
    arg_end(20),
};

static void argtable_parse_arg(int argc,char *argv[])
{
    int nerrors = arg_parse(argc,argv,argtable);
    if(nerrors>0 || help->count>0)
    {
        if(nerrors>0 && help->count == 0)
        {
            printf("命令行参数有误!\r\n");
        }
        printf("%s:\r\n","帮助");
        printf("%s ",argv[0]);
        arg_print_syntax(stdout,argtable,"\r\n");
        arg_print_glossary(stdout,argtable,"%-40s %s\n");
        //退出程序
        exit(0);
    }


}

/*
modbus 主机相关
*/
const uint8_t slave_addr=1;

extern HANDLE ComHandle;

//串口输出回调
void  mb_output(uint8_t *data, size_t data_length)
{
    //串口输出
    if(ComHandle!=INVALID_HANDLE_VALUE)
    {
        writeToSerialPort(ComHandle,(char *)data,data_length);
    }
}

size_t mb_request_reply(uint8_t *data, size_t data_length)
{
    //串口输入
    if(ComHandle!=INVALID_HANDLE_VALUE)
    {
        return readFromSerialPort(ComHandle,(char *)data,data_length);
    }

    return 0;
}

/*
主程序
*/
HANDLE ComHandle=INVALID_HANDLE_VALUE;
int main(int argc,char *argv[])
{
    //关闭输出缓冲
    setbuf(stdout,NULL);

    //检查命令参数
    argtable_parse_arg(argc,argv);


    //打开串口
    if((ComHandle=openSerialPort(*argcom->sval,B115200,one,off))==INVALID_HANDLE_VALUE)
    {
        printf("打开串口失败!\r\n");
        exit(0);
    }
    else
    {
        printf("打开串口%s成功!\r\n",*argcom->sval);
    }

    //初始化modbus上下文
    modbus_master_context_t ctx= {0};
    ctx.slave_addr=slave_addr;
    ctx.output=mb_output;
    ctx.request_reply=mb_request_reply;

    //测试Modbus_Master_Read_IX
    {
        uint8_t buff[4096]= {0};
        printf("测试函数:%s\r\n","Modbus_Master_Read_IX");
        bool data[20]= {0};
        Modbus_Master_Read_IX(&ctx,0,data,sizeof(data)/sizeof(data[0]),buff,sizeof(buff));

        for(size_t i=0; i<sizeof(data)/sizeof(data[0]); i++)
        {
            printf("%d=%-5s\t",i,data[i]?"true":"false");
        }
        printf("\r\n");
    }

    //测试Modbus_Master_Read_OX
    {
        uint8_t buff[4096]= {0};
        printf("测试函数:%s\r\n","Modbus_Master_Read_OX");
        bool data[20]= {0};
        Modbus_Master_Read_OX(&ctx,0,data,sizeof(data)/sizeof(data[0]),buff,sizeof(buff));

        for(size_t i=0; i<sizeof(data)/sizeof(data[0]); i++)
        {
            printf("%d=%-5s\t",i,data[i]?"true":"false");
        }
        printf("\r\n");
    }

    //测试Modbus_Master_Read_Hold_Register
    {
        uint8_t buff[4096]= {0};
        printf("测试函数:%s\r\n","Modbus_Master_Read_Hold_Register");
        uint16_t data[20]= {0};
        Modbus_Master_Read_Hold_Register(&ctx,0,data,sizeof(data)/sizeof(data[0]),buff,sizeof(buff));

        for(size_t i=0; i<sizeof(data)/sizeof(data[0]); i++)
        {
            printf("%d=%d\t",i,data[i]);
        }
        printf("\r\n");
    }
    //测试Modbus_Master_Read_Input_Register
    {
        uint8_t buff[4096]= {0};
        printf("测试函数:%s\r\n","Modbus_Master_Read_Input_Register");
        uint16_t data[20]= {0};
        Modbus_Master_Read_Input_Register(&ctx,0,data,sizeof(data)/sizeof(data[0]),buff,sizeof(buff));

        for(size_t i=0; i<sizeof(data)/sizeof(data[0]); i++)
        {
            printf("%d=%d\t",i,data[i]);
        }
        printf("\r\n");
    }

    //测试Modbus_Master_Write_Hold_Register
    {
        uint8_t buff[4096]= {0};
        printf("测试函数:%s\r\n","Modbus_Master_Write_Hold_Register");
        uint16_t data[5]= {1,4,3,2,5};
        Modbus_Master_Write_Hold_Register(&ctx,0,data,sizeof(data)/sizeof(data[0]),buff,sizeof(buff));

        for(size_t i=0; i<sizeof(data)/sizeof(data[0]); i++)
        {
            printf("%d=%d\t",i,data[i]);
        }
        printf("\r\n");
    }


    //测试Modbus_Master_Write_OX
    {
        uint8_t buff[4096]= {0};
        printf("测试函数:%s\r\n","Modbus_Master_Write_OX");
        bool data[5]= {true,false,false,true,true};
        Modbus_Master_Write_OX(&ctx,0,data,sizeof(data)/sizeof(data[0]),buff,sizeof(buff));

        for(size_t i=0; i<sizeof(data)/sizeof(data[0]); i++)
        {
            printf("%d=%-5s\t",i,data[i]?"true":"false");
        }
        printf("\r\n");
    }



    //关闭串口
    if(ComHandle!=INVALID_HANDLE_VALUE)
    {
        closeSerialPort(ComHandle);
    }


    return 0;
}
