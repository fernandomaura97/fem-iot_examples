/*
    MutichannelGasSensor.cpp
    2015 Copyright (c) Seeed Technology Inc.  All right reserved.

    Author: Jacky Zhang
    2015-3-17
    http://www.seeed.cc/
    modi by Jack, 2015-8

    The MIT License (MIT)

    Copyright (c) 2015 Seeed Technology Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/
 


/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-multigas-sensor
 * 
 *
 *  *
 * \file
 *  Driver for the Multichannel gas sensor of seed v1.0
 *
 * \author
 *         fernando maura rivero
 */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <contiki.h>
#include <math.h>

#include <inttypes.h>
#include "dev/i2c.h"
#include "multigas.h"
#include "dev/zoul-sensors.h"
#include "lib/sensors.h"
static uint8_t enabled;

static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return enabled;
  }
  return 0;
}



/*********************************************************************************************************
** Function name:           begin
** Descriptions:            initialize I2C
*********************************************************************************************************/
void mgsbegin(int address) {

    //if(address == NULL ){address = DEFAULT_I2C_ADDR;}


/*
    i2c_master_enable();	//I2C ARDUINO!************
    i2cAddress = address;	//I2C ARDUINO!************
    __version = getVersion();
*/
    i2c_master_enable();
    i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_NORMAL_BUS_SPEED);
    i2cAddress = address;
    __version = mgs_getVersion();          // version 1/2
    r0_inited = false;
}

unsigned char mgs_getVersion() {

    if (mgsget_addr_dta(CMD_READ_EEPROM, ADDR_IS_SET) == 1126) {     // get version
        __version = 2;
        //printf("version = 2\n");
        return 2;
    }
    //printf("datosssss = %u\n",mgsget_addr_dta(CMD_READ_EEPROM, ADDR_IS_SET));
    __version = 1;
    //printf("version = 1\n");
    return 1;
}

/*void mgsbegin() {
    mgsbegin(DEFAULT_I2C_ADDR);
}*/

/*********************************************************************************************************
** Function name:           sendI2C
** Descriptions:            send one byte to I2C Wire
*********************************************************************************************************/
void mgssendI2C(unsigned char dta) {                    
	i2c_master_enable();
	if(i2c_single_send(DEFAULT_I2C_ADDR, dta) == I2C_MASTER_ERR_NONE)
	{//printf("succesful i2c transmission of data 0x%u\n", dta );
	}
    else{ return MULTIGAS_ERROR; }

}

unsigned int mgsget_addr_dta1(unsigned char addr_reg) {
    
    uint8_t raw[2];
    uint16_t dta = 0;

    i2c_master_enable();

    if(i2c_single_send(DEFAULT_I2C_ADDR, addr_reg) == I2C_MASTER_ERR_NONE){
        
        while(i2c_master_busy());
	    if(i2c_burst_receive(DEFAULT_I2C_ADDR, raw, 2)==I2C_MASTER_ERR_NONE){
 
         dta = concat(raw);
	    //printf("raw[0] = %02X , raw[1] = %02X\n", raw[0],raw[1]);
	    //printf("dttta = %d\n", dta);

	
		 switch (addr_reg) {

			case CH_VALUE_NH3:

			    if (dta > 0) {
				adcValueR0_NH3_Buf = dta;
			    } else {
				dta = adcValueR0_NH3_Buf;
			    }

			    break;

			case CH_VALUE_CO:

			    if (dta > 0) {
				adcValueR0_CO_Buf = dta;
			    } else {
				dta = adcValueR0_CO_Buf;
			    }

			    break;

			case CH_VALUE_NO2:

			    if (dta > 0) {
				adcValueR0_NO2_Buf = dta;
			    } else {
				dta = adcValueR0_NO2_Buf;
			    }

			    break;

			default:;

		} //switch
	 } //if i2c burst receive
	   
	} //if i2c single send
	return dta; 
}


unsigned int mgsget_addr_dta(unsigned char addr_reg, unsigned char __dta) {


   uint8_t raw[2];
   uint8_t addr__dta[2];
   addr__dta[0] = addr_reg;
   addr__dta[1] = __dta;
   uint16_t dta = 0;

   i2c_master_enable();
   if(i2c_burst_send(DEFAULT_I2C_ADDR, addr__dta, 2)==I2C_MASTER_ERR_NONE) {
		
	    while(i2c_master_busy());
	     
	    if( i2c_burst_receive(DEFAULT_I2C_ADDR, raw, 2) == I2C_MASTER_ERR_NONE) { dta=concat(raw);
        return dta; 	}
        else{           
            return MULTIGAS_ERROR;
        }
	}
	
}   




uint8_t mgswrite_i2c(unsigned char addr, uint8_t *dta, unsigned char dta_len) {
    
/*
    Wire.beginTransmission(addr);		//I2C ARDUINO!************
    for (int i = 0; i < dta_len; i++) {
        Wire.write(dta[i]);			//I2C ARDUINO!************
    }
    Wire.endTransmission();			//I2C ARDUINO!************
*/
i2c_master_enable();
if(i2c_burst_send(addr,dta,dta_len) == I2C_MASTER_ERR_NONE) {
    return MULTIGAS_SUCCESS;
}

}


/*********************************************************************************************************
** Function name:           readData
** Descriptions:            read 4 bytes from I2C slave
*********************************************************************************************************/
int16_t mgsreadData(uint8_t cmd) {
    //uint16_t timeout = 0;
    uint8_t buffer[4];
    uint8_t checksum = 0;
    int16_t rtnData = 0;

    //send command
    mgssendI2C(cmd);
    //wait for a while
				
    while(i2c_master_busy());
    //get response
	
    if(i2c_burst_receive(DEFAULT_I2C_ADDR, buffer, 4)== I2C_MASTER_ERR_NONE) {
    //printf("read data:");
    //printf("%u\n", (unsigned int)buffer);
}
/*
    Wire.requestFrom(i2cAddress, (uint8_t)4);    // request 4 bytes from slave device
    while (Wire.available() == 0) {					
        if (timeout++ > 100) {					//I2C ARDUINO!************
            return -2;    //time out
        }
    clock_delay_usec(2000); 
	}
*/
   
    /*if (Wire.available() != 4) {				//I2C ARDUINO!************
        return -3;    //rtnData length wrong
    }

    buffer[0] = Wire.read();					//I2C ARDUINO!************
    buffer[1] = Wire.read();
    buffer[2] = Wire.read();
    buffer[3] = Wire.read();*/
    checksum = buffer[0] + buffer[1] + buffer[2];
    //printf("chksm: %u\n",(unsigned int)checksum); 

    if (checksum != buffer[3]) {
	///printf("malamlamalmalma\n");
        return -4;    //checksum wrong
    }
    rtnData = ((buffer[1] << 8) + buffer[2]);

    return rtnData;//successful
}

/*********************************************************************************************************
** Function name:           readR0
** Descriptions:            read R0 stored in slave MCU
*********************************************************************************************************/
int16_t mgsreadR0(void) {
    int16_t retenda = 0;

    retenda = mgsreadData(0x11);

    if (retenda > 0) {
        res0[0] = retenda;
    } else {
        return retenda;    //unsuccessful
    }

    retenda = mgsreadData(0x12);
    if (retenda > 0) {
        res0[1] = retenda;
    } else {
        return retenda;    //unsuccessful
    }

    retenda = mgsreadData(0x13);
    if (retenda > 0) {
        res0[2] = retenda;
    } else {
        return retenda;    //unsuccessful
    }

    return 1;//successful
}

/*********************************************************************************************************
** Function name:           readR
** Descriptions:            read resistance value of each channel from slave MCU
*********************************************************************************************************/
int16_t mgsreadR(void) {
    int16_t retendo;

    retendo = mgsreadData(0x01);
    if (retendo >= 0) {
        res[0] = retendo;
    } else {
        return retendo;    //unsuccessful
    }

    retendo = mgsreadData(0x02);
    if (retendo >= 0) {
        res[1] = retendo;
    } else {
        return retendo;    //unsuccessful
    }

    retendo = mgsreadData(0x03);
    if (retendo >= 0) {
        res[2] = retendo;
    } else {
        return retendo;    //unsuccessful
    }

    return 0;//successful
}


uint16_t concat(uint8_t buf[2]) {
    uint16_t result = 0;
    result = (uint16_t)buf[0] << 8;
    result |= (uint16_t)buf[1];
    return result;
}

/*********************************************************************************************************
** Function name:           readR
** Descriptions:            calculate gas concentration of each channel from slave MCU
** Parameters:
                            gas - gas type
** Returns:
                            float value - concentration of the gas
*********************************************************************************************************/
float calcGas(int gas) {

    float ratio0, ratio1, ratio2;

	ratio0 = (float)res[0] / res0[0];
    ratio1 = (float)res[1] / res0[1];
    ratio2 = (float)res[2] / res0[2];

    if (1 == __version) {
        if (!r0_inited) {
            if (mgsreadR0() >= 0) {
                r0_inited = true;
            } else {
                return -1.0f;
            }
        }

        if (mgsreadR() < 0) {
            return -2.0f;
        }
 
    } else if (2 == __version) {
        // how to calc ratio/123
        mgsledOn();
        int A0_0 = mgsget_addr_dta(6, ADDR_USER_ADC_HN3);
        int A0_1 = mgsget_addr_dta(6, ADDR_USER_ADC_CO);
        int A0_2 = mgsget_addr_dta(6, ADDR_USER_ADC_NO2);

        int An_0 = mgsget_addr_dta1(CH_VALUE_NH3);
        int An_1 = mgsget_addr_dta1(CH_VALUE_CO);
        int An_2 = mgsget_addr_dta1(CH_VALUE_NO2);

        ratio0 = (float)An_0 / (float)A0_0 * (1023.0 - A0_0) / (1023.0 - An_0);
        ratio1 = (float)An_1 / (float)A0_1 * (1023.0 - A0_1) / (1023.0 - An_1);
        ratio2 = (float)An_2 / (float)A0_2 * (1023.0 - A0_2) / (1023.0 - An_2);

    }

	
    float c = 0;

    switch (gas) {
        case CO: {
                c = pow(ratio1, -1.179) * 4.385; //mod by jack
                break;
            }
        case NO2: {
                c = pow(ratio2, 1.007) / 6.855; //mod by jack
                break;
            }
        case NH3: {
                c = pow(ratio0, -1.67) / 1.47; //modi by jack
                break;
            }
        case C3H8: { //add by jack
                c = pow(ratio0, -2.518) * 570.164;
                break;
            }
        case C4H10: { //add by jack
                c = pow(ratio0, -2.138) * 398.107;
                break;
            }
        case CH4: { //add by jack
                c = pow(ratio1, -4.363) * 630.957;
                break;
            }
        case H2: { //add by jack
                c = pow(ratio1, -1.8) * 0.73;
                break;
            }
        case C2H5OH: { //add by jack
                c = pow(ratio1, -1.552) * 1.622;
                break;
            }
        default:
            break;
    }

    if (2 == __version) {
        mgsledOff();
    }
    return isnan(c) ? -3 : c;
}

/*********************************************************************************************************
** Function name:           changeI2cAddr
** Descriptions:            change I2C address of the slave MCU, and this address will be stored in EEPROM of slave MCU
*********************************************************************************************************/
void mgschangeI2cAddr(uint8_t newAddr) {
  
    unsigned char buf2[2];
    buf2[0] = 0x23;
    buf2[1] = newAddr;
    i2c_burst_send(DEFAULT_I2C_ADDR, buf2, 2);

    if(i2c_burst_send(DEFAULT_I2C_ADDR, addr__dta, 2)==I2C_MASTER_ERR_NONE) {
        i2cAddr = newAddr;
    }
    else{
        printf( "error");
    }
    //i2c_single_send(DEFAULT_I2C_ADDR, 0x23);
    //i2c_single_send(DEFAULT_I2C_ADDR, newAddr);
//quizas no sea valido esto, en el arduino envía los dos bytes seguidos, usar burst_send()??
   
}

/*********************************************************************************************************
** Function name:           doCalibrate
** Descriptions:            tell slave to do a calibration, it will take about 8s
                            after the calibration, must reread the R0 values
*********************************************************************************************************/
void mgsdoCalibrate(void) {

    if (1 == __version) {
    START:

        mgssendI2C(0x22);
        if (mgsreadR0() > 0) {
            for (int i = 0; i < 3; i++) {
                printf("%u\n", (unsigned int)res0[i]);
                printf("\t");
            }
        } 
	else {
            RTIMER_BUSYWAIT(1*CLOCK_SECOND);
            printf("continue...");
            for (int i = 0; i < 3; i++) {
                printf("%u\n", (unsigned int) res0[i]);
                printf("\t");
            }
            printf("\n");
            goto START;
	   
        }
    } else if (2 == __version) {
        unsigned int i, a0, a1, a2;
        while (1) {
            a0 = mgsget_addr_dta1(CH_VALUE_NH3);
            a1 = mgsget_addr_dta1(CH_VALUE_CO);
            a2 = mgsget_addr_dta1(CH_VALUE_NO2);

            printf("%u\t", a0);
           
            printf("%u\t",a1);
            printf("%u\t",a2);
            
            mgsledOn();

            int cnt = 0;
            for (i = 0; i < 20; i++) {
                if ((a0 - mgsget_addr_dta1(CH_VALUE_NH3)) > 2 || (mgsget_addr_dta1(CH_VALUE_NH3) - a0) > 2) {
                    cnt++;
                }
                if ((a1 - mgsget_addr_dta1(CH_VALUE_CO)) > 2 || (mgsget_addr_dta1(CH_VALUE_CO) - a1) > 2) {
                    cnt++;
                }
                if ((a2 - mgsget_addr_dta1(CH_VALUE_NO2)) > 2 || (mgsget_addr_dta1(CH_VALUE_NO2) - a2) > 2) {
                    cnt++;
                }

                if (cnt > 5) {
                    break;
                }
                RTIMER_BUSYWAIT(1*CLOCK_SECOND);
            }

            mgsledOff();
            if (cnt <= 5) {
                break;
            }
           RTIMER_BUSYWAIT(CLOCK_SECOND/5);
        }

        printf("write user adc value: ");
        printf("%u\t",a0);
        printf("%u\t",a1); 
        printf("%u\t",a2);

        unsigned char tmp[7];

        tmp[0] = 7;

        tmp[1] = a0 >> 8;
        tmp[2] = a0 & 0xff;

        tmp[3] = a1 >> 8;
        tmp[4] = a1 & 0xff;

        tmp[5] = a2 >> 8;
        tmp[6] = a2 & 0xff;

        mgswrite_i2c(i2cAddress, tmp, 7);
    }
}

/*********************************************************************************************************
** Function name:           mgspowerOn
** Descriptions:            power on sensor heater
*********************************************************************************************************/
void mgspowerOn(void) {
    if (__version == 1) {
        mgssendI2C(0x21);
    } else if (__version == 2) {
        dta_test[0] = 11;
        dta_test[1] = 1;
        mgswrite_i2c(i2cAddress, dta_test, 2);
    }
}

/*********************************************************************************************************
** Function name:           powerOff
** Descriptions:            power off sensor heater
*********************************************************************************************************/
void mgspowerOff(void) {
    if (__version == 1) {
        mgssendI2C(0x20);
    } else if (__version == 2) {
        dta_test[0] = 11;
        dta_test[1] = 0;
        mgswrite_i2c(i2cAddress, dta_test, 2);
    }
}

void mgs_display_eeprom() {
    if (__version == 1) {
        printf("ERROR: display_eeprom() is NOT support by V1 firmware.");
        return;
    }

    printf("ADDR_IS_SET = "); printf("%u\n",mgsget_addr_dta(CMD_READ_EEPROM, ADDR_IS_SET));
    printf("ADDR_FACTORY_ADC_NH3 = "); printf("%u\n",mgsget_addr_dta(CMD_READ_EEPROM, ADDR_FACTORY_ADC_NH3));
    printf("ADDR_FACTORY_ADC_CO = "); printf("%u\n",mgsget_addr_dta(CMD_READ_EEPROM, ADDR_FACTORY_ADC_CO));
    printf("ADDR_FACTORY_ADC_NO2 = "); printf("%u\n",mgsget_addr_dta(CMD_READ_EEPROM, ADDR_FACTORY_ADC_NO2));
    printf("ADDR_USER_ADC_HN3 = "); printf("%u\n",mgsget_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_HN3));
    printf("ADDR_USER_ADC_CO = "); printf("%u\n",mgsget_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_CO));
    printf("ADDR_USER_ADC_NO2 = "); printf("%u\n",mgsget_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_NO2));
    printf("ADDR_I2C_ADDRESS = "); printf("%u\n",mgsget_addr_dta(CMD_READ_EEPROM, ADDR_I2C_ADDRESS));
}

float mgsgetR0(unsigned char ch) {       // 0:CH3, 1:CO, 2:NO2
    if (__version == 1) {
        printf("ERROR: getR0() is NOT support by V1 firmware.\n");
        return -1;
    }

    int a = 0;
    switch (ch) {
        case 0:         // CH3
            a = mgsget_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_HN3);
            printf("a_ch3 = %d\n", a);
            
            break;

        case 1:         // CO
            a = mgsget_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_CO);
            printf("a_co = %d\n",a);
            
            break;

        case 2:         // NO2
            a = mgsget_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_NO2);
            printf("a_no2 = %d\n",a);
           
            break;
//???
        default:;
    }

    float r = 56.0 * (float)a / (1023.0 - (float)a);
    return r;
}

float mgsgetRs(unsigned char ch) {       // 0:CH3, 1:CO, 2:NO2

    if (__version == 1) {
        printf("ERROR: getRs() is NOT support by V1 firmware.\n");
        return -1;
    }

    int a = 0;
    switch (ch) {
        case 0:         // NH3
            a = mgsget_addr_dta1(1);
            break;

        case 1:         // CO
            a = mgsget_addr_dta1(2);
            break;

        case 2:         // NO2
            a = mgsget_addr_dta1(3);
            break;

        default:;
    }

    float r = 56.0 * (float)a / (1023.0 - (float)a);
    return r;
}

// 1. change i2c address to 0x04
// 2. change adc value of R0 to default
void mgs_factory_setting() {

    unsigned char tmp[7];

    unsigned char error;
    unsigned char address = 0;

    for (address = 1; address < 127; address++) {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        
	

        i2c_master_command(I2C_MASTER_CMD_BURST_SEND_START);
	    i2c_master_command(I2C_MASTER_CMD_BURST_SEND_FINISH);
	    if(i2c_master_error() == 0) { error = 0;} 
        
        if (error == 0) {
            // change i2c to 0x04

            printf("I2C address is: 0x%d\n", address); //address as an int or HEX?
            printf("Change I2C address to 0x04\n");

            dta_test[0] = CMD_CHANGE_I2C;
            dta_test[1] = 0x04;
            i2c_burst_send(address, dta_test, 2); //I2C ARDUINO

            i2cAddress = 0x04;
            RTIMER_BUSYWAIT(CLOCK_SECOND/10);
            mgs_getVersion();
            break;
        }
    }

    unsigned int a0 = mgsget_addr_dta(CMD_READ_EEPROM, ADDR_FACTORY_ADC_NH3);
    unsigned int a1 = mgsget_addr_dta(CMD_READ_EEPROM, ADDR_FACTORY_ADC_CO);
    unsigned int a2 = mgsget_addr_dta(CMD_READ_EEPROM, ADDR_FACTORY_ADC_NO2);

    tmp[0] = 7;
    tmp[1] = a0 >> 8;
    tmp[2] = a0 & 0xff;
    tmp[3] = a1 >> 8;
    tmp[4] = a1 & 0xff;

    tmp[5] = a2 >> 8;
    tmp[6] = a2 & 0xff;
    clock_delay(100);
    i2c_burst_send(i2cAddress, tmp, 7); //I2C ARDUINO
    clock_delay(100);
}

void mgs_change_i2c_address(unsigned char addr) {
    dta_test[0] = CMD_CHANGE_I2C;
    dta_test[1] = addr;
    mgswrite_i2c(i2cAddress, dta_test, 2);


    printf("FUNCTION: CHANGE I2C ADDRESS: 0X");
    printf("%" PRIu8 "\n",i2cAddress); //printf in int or hex?
    printf(" > 0x%u\n",addr);//print in hex? may lead to problems because printing char as unsigned int
    //printf(addr, HEX);

    i2cAddress = addr;
}

//extern gas;

static int configure(int type, int value)
{	

	printf("type: %d\n", type);
	printf("type: %d\n", value);
 return 0;


}



static int value(int type) {
	
printf("%d\n", type);
return 0;
}

/////////////////FIX MULTIPLE DEFINITION///////////////////////

    float measure_CO() {
        return calcGas(CO);
    }
    float measure_NO2() {
        return calcGas(NO2);
    }
    float measure_NH3() {
        return calcGas(NH3);
    }
    float measure_C3H8() {
        return calcGas(C3H8);
    }
    float measure_C4H10() {
        return calcGas(C4H10);
    }
    float measure_CH4() {
        return calcGas(CH4);
    }
    float measure_H2() {
        return calcGas(H2);
    }
    float measure_C2H5OH() {
        return calcGas(C2H5OH);
    }

 void mgsledOn() {
        dta_test[0] = CMD_CONTROL_LED;
        dta_test[1] = 1;
        i2c_burst_send(i2cAddress, dta_test, 2);
    }

    void mgsledOff() {
        dta_test[0] = CMD_CONTROL_LED;
        dta_test[1] = 0;
        i2c_burst_send(i2cAddress, dta_test, 2);
    }

void floatsensor(float f)
{
    unsigned int a1, a2;
    a1 = (unsigned int) f;
    a2 = (unsigned int) (f * 100) % 100;
 
    
    if(a2<10){
	printf("%u.0%u",a1,a2);
	
	}	
    else{
	printf("%u.%u",a1,a2);
	
	}
}


// digits before point
unsigned short d1(float f){
    return((unsigned short)f);
}

// digits after point
unsigned short d2(float f){
    return(100*(f-d1(f)));
}

SENSORS_SENSOR(multigas_sensor, MULTIGAS_SENSOR, value, configure, status);
/** @} */

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
