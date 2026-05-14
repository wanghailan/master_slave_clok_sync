//#############################################################################
//
// FILE:   Drv_W5500.c
//
// TITLE:  CPU controlled data frame transfers with internal or external
//
//#############################################################################

#include <Driver/device/device.h>
#include <Driver/device/driverlib.h>
#include "bsp.h"

// W5500控制接口，由CM通过IO模拟SPI实现
#define GPIO_PIN_W5500_MISO                 85U                // GPIO number for W5500_MISO
#define GPIO_CFG_W5500_MISO                 GPIO_85_GPIO85     // "pinConfig" for W5500_MISO

#define GPIO_PIN_W5500_MOSI                 91U                // GPIO number for W5500_MOSI
#define GPIO_CFG_W5500_MOSI                 GPIO_91_GPIO91     // "pinConfig" for W5500_MOSI

#define GPIO_PIN_W5500_SCK                  150U                // GPIO number for W5500_SCK
#define GPIO_CFG_W5500_SCK                  GPIO_150_GPIO150    // "pinConfig" for W5500_SCK

#define GPIO_PIN_W5500_NSS                  149U                // GPIO number for W5500_NSS
#define GPIO_CFG_W5500_NSS                  GPIO_149_GPIO149    // "pinConfig" for W5500_NSS

#define GPIO_PIN_W5500_RSTN                 93U                // GPIO number for W5500_RSTN
#define GPIO_CFG_W5500_RSTN                 GPIO_93_GPIO93     // "pinConfig" for W5500_RSTN

#define GPIO_PIN_W5500_INT                  92U                // GPIO number for W5500_INT
#define GPIO_CFG_W5500_INT                  GPIO_92_GPIO92     // "pinConfig" for W5500_INT


void Drv_W5500Pin_Init(void)
{
    //W5500扩展IO设置
    GPIO_setPinConfig(GPIO_CFG_W5500_MISO);
    GPIO_setPadConfig(GPIO_PIN_W5500_MISO, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_W5500_MISO, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(GPIO_CFG_W5500_MOSI);
    GPIO_setPadConfig(GPIO_PIN_W5500_MOSI, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_W5500_MOSI, GPIO_DIR_MODE_OUT);
    GPIO_setPinConfig(GPIO_CFG_W5500_SCK);
    GPIO_setPadConfig(GPIO_PIN_W5500_SCK, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_W5500_SCK, GPIO_DIR_MODE_OUT);
    GPIO_setPinConfig(GPIO_CFG_W5500_NSS);
    GPIO_setPadConfig(GPIO_PIN_W5500_NSS, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_W5500_NSS, GPIO_DIR_MODE_OUT);
    GPIO_setPinConfig(GPIO_CFG_W5500_RSTN);
    GPIO_setPadConfig(GPIO_PIN_W5500_RSTN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_W5500_RSTN, GPIO_DIR_MODE_OUT);
    GPIO_setPinConfig(GPIO_CFG_W5500_INT);
    GPIO_setPadConfig(GPIO_PIN_W5500_INT, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_PIN_W5500_INT, GPIO_DIR_MODE_IN);

    GPIO_setMasterCore(GPIO_PIN_W5500_MISO, GPIO_CORE_CM);
    GPIO_setMasterCore(GPIO_PIN_W5500_MOSI, GPIO_CORE_CM);
    GPIO_setMasterCore(GPIO_PIN_W5500_SCK,  GPIO_CORE_CM);
    GPIO_setMasterCore(GPIO_PIN_W5500_NSS,  GPIO_CORE_CM);
    GPIO_setMasterCore(GPIO_PIN_W5500_RSTN, GPIO_CORE_CM);
    GPIO_setMasterCore(GPIO_PIN_W5500_INT,  GPIO_CORE_CM);
}



