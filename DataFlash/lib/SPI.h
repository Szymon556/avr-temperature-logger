#ifndef SPI_H_
#define SPI_H_
#include <stdint.h>

void SPI_init(unsigned int baud);
uint8_t SPI_SendByte(uint8_t byte);

#endif
