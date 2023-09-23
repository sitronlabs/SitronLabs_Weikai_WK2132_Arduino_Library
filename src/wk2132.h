#ifndef WK2132_H
#define WK2132_H

/* Arduino libraries */
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

/* C/C++ libraries */
#include <errno.h>
#include <stdint.h>

/**
 * List of internal registers.
 */
enum wk2132_register {

    /* Global registers */
    WK2132_REGISTER_GENA = 0b000000,   //!< Global Control Register
    WK2132_REGISTER_GRST = 0b000001,   //!< Global sub-serial port reset register
    WK2132_REGISTER_GMUT = 0b000010,   //!< Global master serial port control register
    WK2132_REGISTER_GAMES = 0b010000,  //!< Global Interrupt Register
    WK2132_REGISTER_GIFR = 0b010001,   //!< Global Interrupt Flag Register

    /* Per-uart register */
    WK2132_REGISTER_SPAGE = 0b0011,  //!< Sub-serial page control register
    WK2132_REGISTER_SCR = 0b0100,    //!< Sub-serial port control register (Page 0)
    WK2132_REGISTER_LCR = 0b0101,    //!< Sub-serial port configuration register (Page 0)
    WK2132_REGISTER_FCR = 0b0110,    //!< Sub-serial port FIFO control register (Page 0)
    WK2132_REGISTER_TFCNT = 0b1001,  //!< Sub-serial port send FIFO count register (Page 0)
    WK2132_REGISTER_RFCNT = 0b1010,  //!< Sub-serial port receive FIFO count register (Page 0)
    WK2132_REGISTER_FSR = 0b1011,    //!< Serial port FIFO status register (Page 0)
    WK2132_REGISTER_LSR = 0b1100,    //!< Sub-serial port receiving status register (Page 0)
    WK2132_REGISTER_FDAT = 0b1101,   //!< Sub-serial port FIFO data register (Page 0)
    WK2132_REGISTER_BAUD1 = 0b0100,  // Sub-serial port baud rate configuration register high byte (Page 1)
    WK2132_REGISTER_BAUD0 = 0b0101,  // Sub-serial port baud rate configuration register low byte (Page 1)
    WK2132_REGISTER_PRES = 0b0110,   // Sub-serial port baud rate configuration register fractional part (Page 1)
};

/**
 *
 */
enum wk2132_page {
    WK2132_PAGE0,
    WK2132_PAGE1,
};

/**
 * List of possible uart modes.
 */
enum wk2132_uart_mode {
    WK2132_UART_MODE_8N1,
};

/**
 *
 */
class wk2132 {

   public:
    wk2132(void);

    /* Setup */
    int setup(const uint32_t frequency, TwoWire &i2c_library, const bool ia0, const bool ia1);
    bool detect(void);

    /* Register access */
    int register_read(const enum wk2132_register reg_addr, uint8_t &reg_value);
    int register_write(const enum wk2132_register reg_addr, const uint8_t reg_value);

    /**
     * Represents each of the two transceivers integrated within the WK2132 device.
     * Implements functions from Stream and Print classes to use this a regular serial port.
     * @see https://stackoverflow.com/questions/3058267/nested-class-member-function-cant-access-function-of-enclosing-class-why
     */
    class uart : public Stream {
       public:
        uart(wk2132 &device, unsigned int uart_id) : m_device(device), m_uart_id(uart_id) {
        }

        /* Mimics traditional Serial classes */
        int begin(const uint32_t baudrate, const enum wk2132_uart_mode mode = WK2132_UART_MODE_8N1);
        int end(void);

        /* Inherited from Stream and Print classes */
        void flush(void);
        int available(void);
        int peek(void);
        int read(void);
        size_t write(uint8_t);

        /* Register access */
        int page_set(const enum wk2132_page page);
        int register_read(const enum wk2132_register reg_addr, uint8_t &reg_value);
        int register_write(const enum wk2132_register reg_addr, const uint8_t reg_values);

        /* Fifo access */
        int fifo_read(uint8_t *const data, const size_t length);
        int fifo_write(const uint8_t *const data, const size_t length);

       protected:
        wk2132 &m_device;
        uint8_t m_uart_id;
        bool m_peek_performed = false;
        uint8_t m_peek_byte;
    } uarts[2];

   protected:
    TwoWire *m_i2c_library = NULL;
    uint8_t m_i2c_address = 0;
    uint32_t m_frequency = 0;
};

#endif
