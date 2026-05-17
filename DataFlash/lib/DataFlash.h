
#ifndef DataFlash_H_
#define DataFlash_H_

#include <stdbool.h>
#define ADDRANDSIZE(x) &x,sizeof(x)

extern const uint16_t DataFLASH_PageSize;
extern const _Bool DataFLASH_Granularity;

typedef union
{
    struct
    {
        uint8_t PAGE_SIZE : 1;
        uint8_t PROTECT : 1;
        uint8_t ID : 1;
        uint8_t COMP : 1;
        uint8_t BUSY : 1;
    };
    uint8_t byte;
}DataFLASH_Status;

typedef union
{
    struct
    {
        uint8_t ManufacturerID;
        uint8_t DensityCode :5;
        uint8_t FamilyCode  :3;
        uint8_t VersionCode :5;
        uint8_t SLCCode     :3;
        uint8_t ExLength;
    };
    uint8_t byte[0];
}DataFLASH_ID;

typedef enum {CS_Low=0, CS_High=1} DataFLASHCS;

void DataFLASH_CSEnable(_Bool cs);
void DataFLASH_SendCmd(uint8_t cmd, void *result, uint8_t result_size);
void DataFLASH_SendCmdAddr(uint8_t cmd, uint8_t addbytes, __uint24 addr, DataFLASHCS cs);
void DataFLASH_ReadSeq(__uint24 addr, void *bufor, __uint24 size);
#endif
