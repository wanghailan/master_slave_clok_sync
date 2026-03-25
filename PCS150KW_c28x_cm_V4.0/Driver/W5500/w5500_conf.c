/*
**************************************************************************************************
* @file    		w5500_conf.c
* @author  		WIZnet Software Team 
* @version 		V1.0
* @date    		2015-02-14
* @brief  		配置MCU，移植W5500程序需要修改的文件，配置W5500的MAC和IP地址
**************************************************************************************************
*/
#include <stdio.h> 
#include <string.h>

#include <Driver/device/cm.h>
#include <Driver/device/driverlib_cm.h>
#include "bsp.h"


#include "w5500_conf.h"
#include "bsp.h"
#include "utility.h"
#include "w5500.h"

#define W5500_SPI_READ_               (0x00 << 2) //< SPI interface Read operation in Control Phase
#define W5500_SPI_WRITE_              (0x01 << 2) //< SPI interface Write operation in Control Phase

// W5500控制接口，由CM通过IO模拟SPI实现
#define PIN_W5500_MISO                 85U                // GPIO number for W5500_MISO
#define PIN_W5500_MOSI                 91U                // GPIO number for W5500_MOSI
#define PIN_W5500_SCK                  150U                // GPIO number for W5500_SCK
#define PIN_W5500_NSS                  149U                // GPIO number for W5500_NSS
#define PIN_W5500_RSTN                 93U                // GPIO number for W5500_RSTN
#define PIN_W5500_INT                  92U                // GPIO number for W5500_INT



static __inline void w5500_nss_0(void)        {GPIO_writePin(PIN_W5500_NSS, 0);}
static __inline void w5500_nss_1(void)        {GPIO_writePin(PIN_W5500_NSS, 1);}
static __inline void w5500_sclk_0(void)       {GPIO_writePin(PIN_W5500_SCK, 0);}
static __inline void w5500_sclk_1(void)       {GPIO_writePin(PIN_W5500_SCK, 1);}
static __inline void w5500_mosi_0(void)       {GPIO_writePin(PIN_W5500_MOSI, 0);}
static __inline void w5500_mosi_1(void)       {GPIO_writePin(PIN_W5500_MOSI, 1);}
static __inline void w5500_rst_0(void)        {GPIO_writePin(PIN_W5500_RSTN, 0);}
static __inline void w5500_rst_1(void)        {GPIO_writePin(PIN_W5500_RSTN, 1);}


#define w5500_miso_Read()                     (GPIO_readPin(PIN_W5500_MISO))   // 读SDA口线状态
#define w5500_int_Read()                      (GPIO_readPin(PIN_W5500_INT))   // 读SCL口线状态

CONFIG_MSG  ConfigMsg;																	/*配置结构体*/													/*EEPROM存储信息结构体*/

/*定义MAC地址,如果多块W5500网络适配板在同一现场工作，请使用不同的MAC地址*/
//uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11};

/*定义默认IP信息*/
uint8 local_ip[4]   ={192,168,0,10};	    /*定义W5500默认IP地址*/
uint8 subnet[4]     ={255,255,255,0};		/*定义W5500默认子网掩码*/
uint8 gateway[4]    ={192,168,0,1};			/*定义W5500默认网关*/
uint8 dns_server[4] ={114,114,114,114};	    /*定义W5500默认DNS*/

uint16 local_port=5000;	                    /*定义本地端口*/

///*定义远端IP信息*/
//uint8  remote_ip[4]={192,168,0,5};		/*远端IP地址*/
//uint16 remote_port=5000;					/*远端端口号*/

/**
*@brief		配置W5500的IP地址
*@param		无
*@return	无
*/
void set_w5500_ip(void)
{	
    local_ip[0] = CmLocalParam.pIpAddr2[3];
    local_ip[1] = CmLocalParam.pIpAddr2[2];
    local_ip[2] = CmLocalParam.pIpAddr2[1];
    local_ip[3] = CmLocalParam.pIpAddr2[0];

    subnet[0] = 255;
    subnet[1] = 255;
    subnet[2] = 255;
    subnet[3] = 0;

    gateway[0] = CmLocalParam.pIpAddr2[3];
    gateway[1] = CmLocalParam.pIpAddr2[2];
    gateway[2] = CmLocalParam.pIpAddr2[1];
    gateway[3] = 1;
 /*复制定义的配置信息到配置结构体*/
//	memcpy(ConfigMsg.mac, mac, 6);
	memcpy(ConfigMsg.lip,local_ip,4);
	memcpy(ConfigMsg.sub,subnet,4);
	memcpy(ConfigMsg.gw,gateway,4);
	memcpy(ConfigMsg.dns,dns_server,4);

	/*以下配置信息，根据需要选用*/	
	ConfigMsg.sw_ver[0]=FW_VER_HIGH;
	ConfigMsg.sw_ver[1]=FW_VER_LOW;	

	/*将IP配置信息写入W5500相应寄存器*/	
	setSUBR(ConfigMsg.sub);
	setGAR(ConfigMsg.gw);
	setSIPR(ConfigMsg.lip);
	
	getSIPR (local_ip);			
//	printf(" W5500 IP地址   : %d.%d.%d.%d\r\n", local_ip[0],local_ip[1],local_ip[2],local_ip[3]);
	getSUBR(subnet);
//	printf(" W5500 子网掩码 : %d.%d.%d.%d\r\n", subnet[0],subnet[1],subnet[2],subnet[3]);
	getGAR(gateway);
//	printf(" W5500 网关     : %d.%d.%d.%d\r\n", gateway[0],gateway[1],gateway[2],gateway[3]);
}

/**
*@brief		配置W5500的MAC地址
*@param		无
*@return	无
*/
void set_w5500_mac(void)
{
    ConfigMsg.mac[0] = ((CmIpc_cpu2cm.uniqueID_L >> 0) & 0xff);
    ConfigMsg.mac[1] = ((CmIpc_cpu2cm.uniqueID_L >> 8) & 0xff);
    ConfigMsg.mac[2] = ((CmIpc_cpu2cm.uniqueID_M >> 0) & 0xff);
    ConfigMsg.mac[3] = ((CmIpc_cpu2cm.uniqueID_M >> 8) & 0xff);
    ConfigMsg.mac[4] = 0x08;
    ConfigMsg.mac[5] = 0x02;
//	memcpy(ConfigMsg.mac, mac, 6);
	setSHAR(ConfigMsg.mac);	/**/
//	memcpy(DHCP_GET.mac, mac, 6);
}

/**
*@brief		配置W5500的GPIO接口
*@param		无
*@return	无
*/
void gpio_for_w5500_config(void)
{
    w5500_nss_1();
    w5500_sclk_0();
    w5500_mosi_0();
}

/**
 * @brief  模拟SPI收发一个字节（全双工）
 * @param  tx_data: 要发送的字节
 * @retval 接收的字节
 */
// SPI单字节收发
static uint8_t W5500_SPI_Transfer(uint8_t tx_data)
{
    uint8_t rx_data = 0;
    uint8_t i;

    for(i = 0; i < 8; i++)  // 逐位收发（高位先行）
    {
        // 1. 输出MOSI位（先送高位）
        if(tx_data & (0x80 >> i))
            w5500_mosi_1();
        else
            w5500_mosi_0();

        // 2. 拉高SCLK，产生时钟上升沿（外设采样MOSI）
        w5500_sclk_1();
//        delay_us(1);  // 延时保证时序稳定（可根据外设调整）

        // 3. 采样MISO位（同步接收）
        if(1==w5500_miso_Read())
        {
            rx_data |= (0x80 >> i);
        }

        // 4. 拉低SCLK，准备下一位
        w5500_sclk_0();
//        delay_us(1);
    }
    return rx_data;
}


/**
*@brief		设置W5500的片选端口SCSn为低
*@param		无
*@return	无
*/
void iinchip_cs_low(void)
{
    w5500_nss_0();  // 拉低CS
}

/**
*@brief		设置W5500的片选端口SCSn为高
*@param		无
*@return	无
*/
void iinchip_cs_high(void)
{	
    w5500_nss_1();  // 拉高CS
}

/**
*@brief		W5500复位设置函数
*@param		无
*@return	无
*/
void reset_w5500(void)
{
    w5500_rst_0();
    delay_ms(10);  // 延时10ms（需实现毫秒延时函数）
    w5500_rst_1();
    delay_ms(100);
}

void w5500_init(void)
{
    gpio_for_w5500_config();
    reset_w5500();
    set_w5500_mac();
    set_w5500_ip();
    socket_buf_init(txsize, rxsize);
}

/**
*@brief		STM32 SPI1读写8位数据
*@param		dat：写入的8位数据
*@return	无
*/
uint8  IINCHIP_SpiSendData(uint8 dat)
{
   return(W5500_SPI_Transfer(dat));
}

/**
*@brief		写入一个8位数据到W5500
*@param		addrbsb: 写入数据的地址
*@param   data：写入的8位数据
*@return	无
*/
void IINCHIP_WRITE( uint32 addrbsb,  uint8 data)
{
    iinchip_cs_low();
   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);	
   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
   IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + W5500_SPI_WRITE_);
   IINCHIP_SpiSendData(data);                   
   iinchip_cs_high();
}

/**
*@brief		从W5500读出一个8位数据
*@param		addrbsb: 写入数据的地址
*@param   data：从写入的地址处读取到的8位数据
*@return	无
*/
uint8 IINCHIP_READ(uint32 addrbsb)
{
   uint8 data = 0;
   iinchip_cs_low();
   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
   IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + W5500_SPI_READ_);
   data = IINCHIP_SpiSendData(0x00);            
   iinchip_cs_high();
   return data;    
}

/**
*@brief		向W5500写入len字节数据
*@param		addrbsb: 写入数据的地址
*@param   buf：写入字符串
*@param   len：字符串长度
*@return	len：返回字符串长度
*/
uint16 wiz_write_buf(uint32 addrbsb,uint8* buf,uint16 len)
{
   uint16 idx = 0;
   iinchip_cs_low();
   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
   IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + W5500_SPI_WRITE_);
   for(idx = 0; idx < len; idx++)
   {
     IINCHIP_SpiSendData(buf[idx]);
   }
   iinchip_cs_high();
   return len;  
}

/**
*@brief		从W5500读出len字节数据
*@param		addrbsb: 读取数据的地址
*@param 	buf：存放读取数据
*@param		len：字符串长度
*@return	len：返回字符串长度
*/
uint16 wiz_read_buf(uint32 addrbsb, uint8* buf,uint16 len)
{
  uint16 idx = 0;
  iinchip_cs_low();
  IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
  IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
  IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + W5500_SPI_READ_);
  for(idx = 0; idx < len; idx++)                   
  {
    buf[idx] = IINCHIP_SpiSendData(0x00);
  }
  iinchip_cs_high();
  return len;
}




