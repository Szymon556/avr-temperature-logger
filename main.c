

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include "hd44780.h"
#include "SPI.h"
#include "DataFlash.h"
#include "AT45CMD.h"
#include "ds18b20.h"






void lcd_init();
void lcd_putchar(char);
void lcd_puttext_P(const char *txt);
void lcd_goto(uint8_t, uint8_t);
void lcd_defchar_P(uint8_t charno, const uint8_t *chardef);
void lcd_cls();
uint8_t SPI_SendByte(uint8_t byte);
int16_t convTemp(uint8_t *temp);

typedef struct{
        uint8_t ROM_ID[8];
        uint8_t TEMPERATURE[2];
}TEMPERATURE_DATA;



DataFLASH_Status DF_Status;
DataFLASH_ID DF_ID;
TEMPERATURE_DATA TMP_DATA;

void get_Temperature(TEMPERATURE_DATA *dataSave );
void show_Temp(TEMPERATURE_DATA *dataShow);
void DataFLASH_saveTemperature(TEMPERATURE_DATA *TMP_DATA);
void DataFLASH_getTemperature(TEMPERATURE_DATA *TMP_DATA);

int main(void){
    bool conf_ready = false; 
    //lcd_defchar_P(0,degree_char);
    lcd_init();
 
    lcd_puttext_P(PSTR("Pomiary Temperatury.\n"));
    conf_ready = initDS18B20();
    
    if(conf_ready){
        get_Temperature(&TMP_DATA);
            
    }
    

//Konfiguracja przycisku do pobierania danych
    DDRD &= ~(1 << PD7);
    PORTD |= (1 << PD7);
//KOnfiguracja lini SS
    DDRD |= (1 << PD5);
    SPI_init(300);
//Pobierz dane na temat producenta pamięci
    DataFLASH_SendCmd(AT45DBX_CMDC_RD_MNFCT_DEV_ID_SM,ADDRANDSIZE(DF_ID));
// Odczytaj aktualny rejestr statusu
    DataFLASH_SendCmd(AT45DBX_CMDC_RD_STATUS_REG,ADDRANDSIZE(DF_Status));
       

        while(1){
           _delay_ms(20);
           DataFLASH_saveTemperature(&TMP_DATA);
           if(!(PIND & (1 << PD7)))
           {
            _delay_ms(1);
            lcd_cls(); 
            lcd_puttext_P(PSTR("Pomiary Temperatury.\n"));  
            show_Temp(&TMP_DATA);     
            lcd_putchar(0xDF);
           }

    }
    return 0;
}

// Funkcja zapisująca temperaturę do pamięci.
void DataFLASH_saveTemperature(TEMPERATURE_DATA *TMP_DATA){
    get_Temperature(TMP_DATA);
    DataFLASH_SendCmdAddr(AT45DBX_CMDC_WR_BUF1,0,CS_Low);
    for(uint16_t i=0; i<256;i++)
    {   if(i > 2){
        SPI_SendByte(0);
         } else{
            SPI_SendByte(TMP_DATA->TEMPERATURE[i]);
        }

    }
    DataFLASH_CSEnable(false);



}

// Funkcja pobierająca temperaturę z pamięci
void DataFLASH_getTemperature(TEMPERATURE_DATA *TMP_DATA )
{
    uint8_t dummy[254];
    DataFLASH_SendCmdAddr(AT45DBX_CMDC_RD_BUF1_AF_SM,0,CS_Low);
    for(uint16_t i=0; i<256;i++){
        if(i > 2){
          dummy[i]=SPI_SendByte(0);
         } else{
            TMP_DATA->TEMPERATURE[i] = SPI_SendByte(0);
        }

    }
    DataFLASH_CSEnable(false);
    
}    

void lcd_init(){
    hd44780_init();
    hd44780_outcmd(HD44780_CLR);
    hd44780_wait_ready(1000);
    hd44780_outcmd(HD44780_ENTMODE(1,0));
    hd44780_wait_ready(1000);
    hd44780_outcmd(HD44780_DISPCTL(1,0,0));
    hd44780_wait_ready(1000);
}

void lcd_putchar(char c)
{
    static bool second_nl_seen;
    static uint8_t line = 0;
    if((second_nl_seen) && (c!='\n') && (line == 0)){
        hd44780_wait_ready(40);
        hd44780_outcmd(HD44780_CLR);
        hd44780_wait_ready(1600);
        second_nl_seen = false;
    }

    if(c == '\n'){
        if(line == 0){
            line++;
            hd44780_outcmd(HD44780_DDADDR(64));
            hd44780_wait_ready(1000);
        }else{
            second_nl_seen=true;
            line=0;
        }
    }else{
        hd44780_outdata(c);
        hd44780_wait_ready(40);
    }
}


void lcd_puttext_P(const char *txt){
   char ch;
   while((ch=pgm_read_byte(txt)))
   {
        lcd_putchar(ch);
        txt++;
   }
}



void lcd_goto(uint8_t x, uint8_t y){
    hd44780_outcmd(HD44780_DDADDR(0x40*y+x));
    hd44780_wait_ready(1000);
}

void lcd_cls(){
    hd44780_outcmd(HD44780_CLR);
    hd44780_wait_ready(false);
}


void lcd_defchar_P(uint8_t charno, const uint8_t *chardef){
    hd44780_outcmd(HD44780_CGADDR(charno*8));
    hd44780_wait_ready(false);
    for(uint8_t c=0;c < 8;c++){
        hd44780_outdata(pgm_read_byte(chardef));
        hd44780_wait_ready(false);
        chardef++;
    }
}


int16_t convTemp(uint8_t * temp){
    int16_t sum = 0;
    sum = temp[0] | (temp[1] << 8); 
    return sum;
 
}

void get_Temperature(TEMPERATURE_DATA *dataSave){
    sendCMD_DS18B20(DS18B20_READ_ROM);
    readData_DS18B20(8,dataSave->ROM_ID);
    initDS18B20();
    sendCMD_DS18B20(DS18B20_SKIP_ROM);
    sendCMD_DS18B20(DS18B20_START_CONV);
    togglePin_DS18B20();
    _delay_us(700);
    if(PINC & (1 << PC0)){
        initDS18B20();
        sendCMD_DS18B20(DS18B20_SKIP_ROM);
        sendCMD_DS18B20(DS18B20_READ_SCRATCHPAD);
        readData_DS18B20(2,dataSave->TEMPERATURE);
    }

}

void show_Temp(TEMPERATURE_DATA *dataShow){
    DataFLASH_getTemperature(dataShow);
    int16_t temp = 0;
    int16_t temp_int = 0;
    int8_t temp_frac = 0;
    char temp_buf[2] = {0x80,0x80};

    temp = convTemp(dataShow->TEMPERATURE);
    temp_int = temp >> 4; 
    itoa(temp_int,temp_buf,10);
    for(int i = 0; i < 2; i++)lcd_putchar(temp_buf[i]);
    lcd_putchar('.');
    temp_frac = ((temp & 0x0F) * 625) / 100;
    itoa(temp_frac,temp_buf,10);
    for(int i = 0; i < 2; i++)lcd_putchar(temp_buf[i]);

}


void SPI_init(unsigned int baud){
    // Najpierw ustaw 0, potem konfiguruj
    UBRR0 = 0;
    
    // Ustawienie wyjścia XCK0 (PD4) - kluczowe dla MSPI
    DDRD |= (1 << PD4);
    
    // UMSEL01 i UMSEL00 ustawiają tryb Master SPI
    UCSR0C = (1 << UMSEL01) | (1 << UMSEL00) | (0 << UCPHA0) | (0 << UCPOL0) | ( 0 << UDORD0 );
    
    // Włącz odbiornik i nadajnik
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    
    // Ustaw prędkość (np. dla 1MHz przy 16MHz zegarze, baud powinno wynosić 7)
    UBRR0 = baud; 
}

uint8_t SPI_SendByte(uint8_t byte){
    
    // 1. Czekaj aż bufor nadawczy (UDR0) będzie gotowy na nowe dane
    while (!(UCSR0A & (1 << UDRE0)));

    // 2. Wyślij bajt
    UDR0 = byte;

    // 3. Czekaj aż flaga odbioru zostanie ustawiona (transmisja zakończona)
    while (!(UCSR0A & (1 << RXC0)));

    // 4. Zwróć odebrany bajt (odczyt czyści flagę RXC0)
    return UDR0;

}

