#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdbool.h>
#include "ds18b20.h"


bool initDS18B20()
{   
    SET_HIGH;
    _delay_us(500);
    SET_LOW;    
    _delay_us(470);
    SET_HIGH;
    _delay_us(300);
    if(PINC & (1 << PC0))
    {
        return true;
    }
    return false;
}

void sendCMD_DS18B20(const uint8_t cmd){
    int8_t counter = 0; 
    while(counter <= 7){
        if(cmd & (1 << (counter))){
            SET_LOW;
            _delay_us(10);
            SET_HIGH;
            _delay_us(70);
        }else{
            SET_LOW;
            _delay_us(70); 
            SET_HIGH;
            _delay_us(10);
        }
      counter++;
    }
}

void readData_DS18B20(uint8_t len, uint8_t *readData){
    for(int8_t i = 0; i < len; i++)
     {   readData[i] = 0; 
         for(uint8_t j = 0; j < 8; j++ )
         {
             SET_LOW; 
             _delay_us(5);
             SET_HIGH;
             _delay_us(7);
             if(PINC &(1<< PC0)) readData[i] |= (1 << j);
            _delay_us(70);
         
        }
    }
}

void togglePin_DS18B20(){
         SET_LOW; 
         _delay_us(10);
         SET_HIGH;
         _delay_us(70);
}

