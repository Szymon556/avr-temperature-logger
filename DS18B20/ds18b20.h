#ifndef DS18B20_H_
#define DS18B20_H_

#define DS18B20_SKIP_ROM          0xCC
#define DS18B20_READ_SCRATCHPAD   0xBE
#define DS18B20_START_CONV        0x44
#define DS18B20_READ_ROM          0x33
#define  SET_HIGH  do {                   \
                     DDRC &= ~(1 << PC0); \
                     PORTC &= ~(1 << PC0);\
                 }while(0)                
#define SET_LOW  do{                  \
                   DDRC |= (1 << PC0); \
                   PORTC &= ~(1 << PC0);\
                   }while(0) 

void togglePin_DS18B20(); 
void sendCMD_DS18B20(const uint8_t cmd);
void readData_DS18B20(uint8_t len, uint8_t *readData);
bool initDS18B20();


#endif
