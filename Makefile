# =========================================
# Konfiguracja projektu
# =========================================

MCU     = atmega328p
F_CPU   = 8000000UL

TARGET  = main

CC      = avr-gcc
OBJCOPY = avr-objcopy
SIZE    = avr-size

# =========================================
# Flagi kompilatora
# =========================================

CFLAGS  = -mmcu=$(MCU)
CFLAGS += -DF_CPU=$(F_CPU)
CFLAGS += -Os
CFLAGS += -Wall -Wextra
CFLAGS += -std=c11

# Ścieżki include
CFLAGS += -I./DataFlash/lib
CFLAGS += -I./lcd/lib
CFLAGS += -I./DS18B20

# =========================================
# Pliki źródłowe
# =========================================

SRC = \
	main.c \
	DataFlash/DataFlash.c \
	lcd/lib/hd44780.c \
	DS18B20/ds18b20.c

# Automatyczna lista plików .o
OBJ = $(SRC:.c=.o)

# =========================================
# Domyślny target
# =========================================

all: $(TARGET).hex

# =========================================
# Linkowanie ELF
# =========================================

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@
	$(SIZE) $@

# =========================================
# Generowanie HEX
# =========================================

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# =========================================
# Kompilacja .c -> .o
# =========================================

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# =========================================
# Flashowanie
# =========================================

flash: $(TARGET).hex
	avrdude -c usbasp -p m328p -U flash:w:$(TARGET).hex:i

# =========================================
# Czyszczenie
# =========================================

clean:
	rm -f $(OBJ)
	rm -f $(TARGET).elf
	rm -f $(TARGET).hex

# =========================================
# Phony
# =========================================

.PHONY: all flash clean
