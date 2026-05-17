/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * Modified for ATmega328P by ChatGPT
 * ----------------------------------------------------------------------------
 *
 * HD44780 LCD display driver
 *
 * The LCD controller is used in 4-bit mode with a full bi-directional
 * interface (i.e. R/W is connected) so the busy flag can be read.
 */
#include "defines.h"

#include <stdbool.h>
#include <stdint.h>

#include <avr/io.h>
#include <util/delay.h>

#include "hd44780.h"

/*
 * ============================================================
 *  Rejestry portów dla klasycznych AVR (ATmega328P itd.)
 *
 *  OUT -> PORTx
 *  DIR -> DDRx
 *  IN  -> PINx
 * ============================================================
 */

#define CAT2(a, b) a##b
#define CAT(a, b)  CAT2(a, b)

#define REG_OUT(p) CAT(PORT, p)
#define REG_DIR(p) CAT(DDR,  p)
#define REG_IN(p)  CAT(PIN,  p)

#define JOIN_OUT(p) REG_OUT(p)
#define JOIN_DIR(p) REG_DIR(p)
#define JOIN_IN(p)  REG_IN(p)

/* wybór rejestru zależnie od typu */
#define JOIN_(what, p) JOIN_##what(p)
#define JOIN(what, p)  JOIN_(what, p)

/* single-bit macros, used for control bits */
#define SET_(what, p, m)   (JOIN(what, p) |=  (1 << (m)))
#define SET(what, x)       SET_(what, x)

#define CLR_(what, p, m)   (JOIN(what, p) &= ~(1 << (m)))
#define CLR(what, x)       CLR_(what, x)

#define GET_(p, m)         (JOIN(IN, p) & (1 << (m)))
#define GET(x)             GET_(x)

/* nibble macros, used for data path */
#define ASSIGN_(what, p, m, v) \
    do { \
        JOIN(what, p) = (JOIN(what, p) & \
                        ~((1 << (m)) | (1 << ((m) + 1)) | \
                          (1 << ((m) + 2)) | (1 << ((m) + 3)))) | \
                        (((v) & 0x0F) << (m)); \
    } while (0)

#define READ_(what, p, m) \
    ((JOIN(what, p) & ((1 << (m)) | (1 << ((m) + 1)) | \
                       (1 << ((m) + 2)) | (1 << ((m) + 3)))) >> (m))

#define ASSIGN(what, x, v) ASSIGN_(what, x, v)
#define READ(what, x)      READ_(what, x)

#define HD44780_BUSYFLAG 0x80

/*
 * Send one pulse to the E signal (enable).  Mind the timing
 * constraints.  If readback is set to true, read the HD44780 data
 * pins right before the falling edge of E, and return that value.
 */
static inline uint8_t
hd44780_pulse_e(bool readback) __attribute__((always_inline));

static inline uint8_t
hd44780_pulse_e(bool readback)
{
    uint8_t x;

    SET(OUT, HD44780_E);

#if F_CPU > 4000000UL
    _delay_us(0.5);
#else
    if (readback)
        __asm__ volatile("nop");
#  if F_CPU > 1000000UL
    __asm__ volatile("nop");
#    if F_CPU > 2000000UL
    __asm__ volatile("nop");
    __asm__ volatile("nop");
#    endif
#  endif
#endif

    if (readback)
        x = READ(IN, HD44780_D4);
    else
        x = 0;

    CLR(OUT, HD44780_E);

    return x;
}

/*
 * Send one nibble out to the LCD controller.
 */
static void
hd44780_outnibble(uint8_t n, uint8_t rs)
{
    CLR(OUT, HD44780_RW);

    if (rs)
        SET(OUT, HD44780_RS);
    else
        CLR(OUT, HD44780_RS);

    ASSIGN(OUT, HD44780_D4, n);
    (void)hd44780_pulse_e(false);
}

/*
 * Send one byte to the LCD controller.  As we are in 4-bit mode, we
 * have to send two nibbles.
 */
void
hd44780_outbyte(uint8_t b, uint8_t rs)
{
    hd44780_outnibble(b >> 4, rs);
    hd44780_outnibble(b & 0x0F, rs);
}

/*
 * Read one nibble from the LCD controller.
 */
static uint8_t
hd44780_innibble(uint8_t rs)
{
    uint8_t x;

    SET(OUT, HD44780_RW);
    ASSIGN(DIR, HD44780_D4, 0x00);

    if (rs)
        SET(OUT, HD44780_RS);
    else
        CLR(OUT, HD44780_RS);

    x = hd44780_pulse_e(true);

    ASSIGN(DIR, HD44780_D4, 0x0F);
    CLR(OUT, HD44780_RW);

    return x;
}

/*
 * Read one byte (i.e. two nibbles) from the LCD controller.
 */
uint8_t
hd44780_inbyte(uint8_t rs)
{
    uint8_t x;

    x = hd44780_innibble(rs) << 4;
    x |= hd44780_innibble(rs);

    return x;
}

/*
 * Wait until the busy flag is cleared.
 */
void
hd44780_wait_ready(bool longwait)
{
#if USE_BUSY_BIT
    while (hd44780_incmd() & HD44780_BUSYFLAG)
        ;
#else
    if (longwait)
        _delay_ms(1.52);
    else
        _delay_us(37);
#endif
}

/*
 * Initialize the LCD controller.
 *
 * The initialization sequence has a mandatory timing so the
 * controller can safely recognize the type of interface desired.
 * This is the only area where timed waits are really needed as
 * the busy flag cannot be probed initially.
 */
void
hd44780_init(void)
{
    SET(DIR, HD44780_RS);
    SET(DIR, HD44780_RW);
    SET(DIR, HD44780_E);
    ASSIGN(DIR, HD44780_D4, 0x0F);

    _delay_ms(15);
    hd44780_outnibble(HD44780_FNSET(1, 0, 0) >> 4, 0);
    _delay_ms(4.1);
    hd44780_outnibble(HD44780_FNSET(1, 0, 0) >> 4, 0);
    _delay_us(100);
    hd44780_outnibble(HD44780_FNSET(1, 0, 0) >> 4, 0);
    _delay_us(37);

    hd44780_outnibble(HD44780_FNSET(0, 1, 0) >> 4, 0);
    hd44780_wait_ready(false);
    hd44780_outcmd(HD44780_FNSET(0, 1, 0));
    hd44780_wait_ready(false);
    hd44780_outcmd(HD44780_DISPCTL(0, 0, 0));
    hd44780_wait_ready(false);
}

/*
 * Prepare the LCD controller pins for powerdown.
 */
void
hd44780_powerdown(void)
{
    ASSIGN(OUT, HD44780_D4, 0);
    CLR(OUT, HD44780_RS);
    CLR(OUT, HD44780_RW);
    CLR(OUT, HD44780_E);
}
