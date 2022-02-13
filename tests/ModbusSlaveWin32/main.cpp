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
modbus 从机相关
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

//读取输入点
bool mb_read_IX(size_t addr)
{
    return (addr%2)==1;//奇数为真，偶数为假。
}

//写输出线圈
void mb_write_OX(size_t addr, uint16_t data)
{
    printf("写输出线圈:%d=%d\r\n",addr,data);
}

//读输出线圈
bool mb_read_OX(size_t addr)
{
    return (addr%2)==0;//奇数为假，偶数为真。
}

//读保持寄存器
uint16_t mb_read_hold_register(size_t addr)
{
    return addr;//返回地址
}

//写保持寄存器
void mb_write_hold_register(size_t addr, uint16_t data)
{
    printf("写保持寄存器:%d=%d\r\n",addr,data);
}

//读输入寄存器
uint16_t mb_read_input_register(size_t addr)
{
    return (addr+1);//返回地址+1
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
    modbus_slave_context_t ctx= {0};
    ctx.slave_addr=slave_addr;
    ctx.output=mb_output;
    ctx.read_IX=mb_read_IX;
    ctx.write_OX=mb_write_OX;
    ctx.read_OX=mb_read_OX;
    ctx.write_hold_register=mb_write_hold_register;
    ctx.read_hold_register=mb_read_hold_register;
    ctx.read_input_register=mb_read_input_register;

    while(true)
    {
        uint8_t rxbuff[4096]= {0},txbuff[4096]= {0};
        long bytesread=readFromSerialPort(ComHandle,(char *)rxbuff,sizeof(rxbuff));
        if(bytesread>0)
        {
            Modbus_Slave_Parse_Input(&ctx,rxbuff,bytesread,txbuff,sizeof(txbuff));
        }
    }


    //关闭串口
    if(ComHandle!=INVALID_HANDLE_VALUE)
    {
        closeSerialPort(ComHandle);
    }


    return 0;
}
