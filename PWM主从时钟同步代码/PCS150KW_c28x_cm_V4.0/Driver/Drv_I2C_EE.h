/*
 * Drv_I2C_EE.h
 *
 *  Created on: 2024年9月11日
 *      Author: guowei680
 */

#ifndef DRIVER_DRV_I2C_EE_H_
#define DRIVER_DRV_I2C_EE_H_

#define AT24C16
//优化级别设置到2 对时间优化选上 否则也会不成
//F7 F429对M24LC256的写入总是不成功，尚未找到原因
#ifdef M24LC256
    #define EE_MODEL_NAME       "M24LC256"
    #define EE_DEV_ADDR         0xA0            // 设备地址
    #define EE_PAGE_SIZE        64              // 页面大小(字节)
    #define EE_SIZE             (32*1024)       // 总容量(字节)
    #define EE_ADDR_BYTES       2               // 地址字节个数
#endif

#ifdef AT24C16
    #define EE_MODEL_NAME       "AT24C16"
    #define EE_DEV_ADDR         0xA0            /* 设备地址 */
    #define EE_PAGE_SIZE        16              /* 页面大小(字节) */
    #define EE_SIZE             (128*16)        /* 总容量(字节) */
    #define EE_ADDR_BYTES       1               /* 地址字节个数 */
#endif

#ifdef AT24C02
    #define EE_MODEL_NAME       "AT24C02"
    #define EE_DEV_ADDR         0xA0            /* 设备地址 */
    #define EE_PAGE_SIZE        8               /* 页面大小(字节) */
    #define EE_SIZE             (32*8)          /* 总容量(字节) */
    #define EE_ADDR_BYTES       1               /* 地址字节个数 */
#endif

int EE_WriteBuf(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);
int EE_ReadBuf(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);

#endif /* DRIVER_DRV_I2C_FRAM_H_ */
