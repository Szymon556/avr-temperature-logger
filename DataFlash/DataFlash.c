#include <stdlib.h>
#include <avr/io.h>
#include <stdbool.h>
#include <math.h>
#include "DataFlash.h"
#include "AT45CMD.h"
#include "SPI.h"

const uint16_t DataFLASH_PageSize = 1056;
const _Bool DataFLASH_Granularity = false;

void DataFLASH_CSEnable(bool cs){
    if(!cs){
        PORTD |= (1<<PD5);
    }else{
        PORTD &= ~(1<<PD5);
    }
}

void DataFLASH_SendCmd(uint8_t cmd, void *result, uint8_t result_size){
    DataFLASH_CSEnable(true);
    SPI_SendByte(cmd);
    for(uint8_t i=0; i<result_size;i++) *(uint8_t*)result++=SPI_SendByte(0);
    DataFLASH_CSEnable(false);
}


void DataFLASH_SendCmdAddr(uint8_t cmd, uint8_t addbytes, __uint24 addr, DataFLASHCS cs){
    if(DataFLASH_Granularity==false){
        if(cmd != 0xC7)
        {
            addr=((addr/DataFLASH_PageSize) << (uint8_t)round(log(DataFLASH_PageSize)/log(2) + 0.5)) | (addr % DataFLASH_PageSize);
        }
        
    }
    DataFLASH_CSEnable(true);
    SPI_SendByte(cmd);
    SPI_SendByte((addr>>16) & 0xFF);
    SPI_SendByte((addr>>8) & 0xFF);
    SPI_SendByte(addr & 0xFF);
    for(int i = 0; i < addbytes; i++) SPI_SendByte(0);
    DataFLASH_CSEnable(!cs);
}

void DataFLASH_ReadSeq(__uint24 addr, void *bufor, __uint24 size){
    DataFLASH_SendCmdAddr(AT45DBX_CMDA_RD_ARRAY_AF_SM,addr,CS_Low);
    for(__uint24 i = 0; i < size; i++)
    {
        *(uint8_t *)bufor=SPI_SendByte(0);
        ++bufor;
    }
    DataFLASH_CSEnable(false);
}



void DataFLASH_WaitForBusy()
{
    DataFLASH_Status DF_Status;
    do{
        DataFLASH_SendCmd(AT45DBX_CMDC_RD_STATUS_REG,ADDRANDSIZE(DF_Status));
    }while(DF_Status.BUSY==0);
}






























