
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-multigas-sensor Seed Multichannel Gas sensor v1.0
 * @{
 *
 * \file
 *  Driver for the Multichannel gas sensor of seed v1.0
 *
 * \author
 *         fernando maura rivero
 */
/*---------------------------------------------------------------------------*/
#ifndef MULTIGAS_H_
#define MULTIGAS_H_

#include <stdio.h>
#include "dev/i2c.h"
#include "lib/sensors.h"
#include "dev/zoul-sensors.h"

/*#if defined (ARDUINO_SAMD_ZERO)
    #define _SERIAL SerialUSB
#elif defined(ARDUINO_ARCH_SEEED_STM32F4)
    #define _SERIAL SerialUSB
#elif defined (ARDUINO_ARCH_SAMD)
    #define _SERIAL Serial
#elif defined (ARDUINO_ARCH_AVR)
    #define _SERIAL Serial
#else
    #error "Architecture not matched"
#endif
*/
#define MULTIGAS_SENSOR                  "multigas sensooor"

#define MULTIGAS_SUCCESS         0x00
#define MULTIGAS_ERROR           -1

#define DEFAULT_I2C_ADDR    0x04

#define ADDR_IS_SET             0           // if this is the first time to run, if 1126, set 
#define ADDR_FACTORY_ADC_NH3    2
#define ADDR_FACTORY_ADC_CO     4
#define ADDR_FACTORY_ADC_NO2    6

#define ADDR_USER_ADC_HN3       8
#define ADDR_USER_ADC_CO        10
#define ADDR_USER_ADC_NO2       12
#define ADDR_IF_CALI            14          // IF USER HAD CALI

#define ADDR_I2C_ADDRESS        20

#define CH_VALUE_NH3            1
#define CH_VALUE_CO             2
#define CH_VALUE_NO2            3

#define CMD_ADC_RES0            1           // NH3
#define CMD_ADC_RES1            2           // CO
#define CMD_ADC_RES2            3           // NO2
#define CMD_ADC_RESALL          4           // ALL CHANNEL
#define CMD_CHANGE_I2C          5           // CHANGE I2C
#define CMD_READ_EEPROM         6           // READ EEPROM VALUE, RETURN UNSIGNED INT
#define CMD_SET_R0_ADC          7           // SET R0 ADC VALUE
#define CMD_GET_R0_ADC          8           // GET R0 ADC VALUE
#define CMD_GET_R0_ADC_FACTORY  9           // GET FACTORY R0 ADC VALUE
#define CMD_CONTROL_LED         10
#define CMD_CONTROL_PWR         11

/*

#define DEFAULT_I2C_ADDR    0x04

#define ADDR_IS_SET             0x00           // if this is the first time to run, if 1126, set 
#define ADDR_FACTORY_ADC_NH3    0x02
#define ADDR_FACTORY_ADC_CO     0x04
#define ADDR_FACTORY_ADC_NO2    0x06

#define ADDR_USER_ADC_HN3       0x08
#define ADDR_USER_ADC_CO        0x10
#define ADDR_USER_ADC_NO2       0x12
#define ADDR_IF_CALI            0x14          // IF USER HAD CALI

#define ADDR_I2C_ADDRESS        0x20

#define CH_VALUE_NH3            0x01
#define CH_VALUE_CO             0x02
#define CH_VALUE_NO2            0x03

#define CMD_ADC_RES0            0x01           // NH3
#define CMD_ADC_RES1            0x02           // CO
#define CMD_ADC_RES2            0x03           // NO2
#define CMD_ADC_RESALL          0x04           // ALL CHANNEL
#define CMD_CHANGE_I2C          0x05           // CHANGE I2C
#define CMD_READ_EEPROM         0x06           // READ EEPROM VALUE, RETURN UNSIGNED INT
#define CMD_SET_R0_ADC          0x07           // SET R0 ADC VALUE
#define CMD_GET_R0_ADC          0x08           // GET R0 ADC VALUE
#define CMD_GET_R0_ADC_FACTORY  0x09           // GET FACTORY R0 ADC VALUE
#define CMD_CONTROL_LED         0x10
#define CMD_CONTROL_PWR         0x11
*/

enum {CO, NO2, NH3, C3H8, C4H10, CH4, H2, C2H5OH};



    int __version;
    unsigned char dta_test[20];
    
    unsigned int readChAdcValue(int ch);
    static unsigned int adcValueR0_NH3_Buf;
    static unsigned int adcValueR0_CO_Buf;
    static unsigned int adcValueR0_NO2_Buf;

 
    uint8_t i2cAddress;     //I2C address of this MCU
    uint16_t res0[3];       //sensors res0
    uint16_t res[3];        //sensors res
    bool r0_inited;

    //MutichannelGasSensor();
    //unsigned int mgsget_addr_dta(unsigned char addr_reg); //conflicting types for 'mgsget_addr_dta
    unsigned int mgsget_addr_dta(unsigned char addr_reg, unsigned char __dta);
    uint8_t mgswrite_i2c(unsigned char addr, unsigned char* dta, unsigned char dta_len);

    void mgssendI2C(unsigned char dta);
    int16_t mgsreadData(uint8_t cmd);
    int16_t mgsreadR0(void);
    int16_t mgsreadR(void);
    float calcGas(int gas);
	
    void multigas_init(void);	
    uint8_t multigas_read(uint16_t *data);
    uint16_t concat(uint8_t buf[2]);
   

    void mgsbegin(int address);
    void mgsbegin();
    void mgschangeI2cAddr(uint8_t newAddr);
    void mgspowerOn(void);
    void mgspowerOff(void);
    void mgsdoCalibrate(void);

    //get gas concentration, unit: ppm
    float measure_CO();
    float measure_NO2();
    float measure_NH3();
    float measure_C3H8();
    float measure_C4H10();
    float measure_CH4();
    float measure_H2();
    float measure_C2H5OH();
    float mgsgetR0(unsigned char ch);      // 0:CH3, 1:CO, 2:NO2
    float mgsgetRs(unsigned char ch);      // 0:CH3, 1:CO, 2:NO2

    void mgsledOn();

    void mgsledOff();

    void mgs_display_eeprom();
    void mgs_factory_setting();
    void mgs_change_i2c_address(unsigned char addr);
    unsigned char mgs_getVersion();
    void floatsensor(float f);
    
    unsigned short d1(float f);
// digits after point
    unsigned short d2(float f);


//extern gas;

extern const struct sensors_sensor multigas_sensor;

#endif
/**
 * @}
 * @}
 */


