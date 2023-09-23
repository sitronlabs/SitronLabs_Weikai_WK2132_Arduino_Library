/* Self header */
#include <wk2132.h>

/**
 * @brief
 * @param[in] baudrate
 * @param[in] mode
 * @return 0 in case of success, or a negative error code otherwise.
 * @see https://www.arduino.cc/reference/en/language/functions/communication/serial/begin/
 */
int wk2132::uart::begin(const uint32_t baudrate, const enum wk2132_uart_mode mode) {

    /* Enable uart port */
    uint8_t reg_gena;
    if (m_device.register_read(WK2132_REGISTER_GENA, reg_gena) < 0) {
        return -EIO;
    }
    reg_gena |= (1 << m_uart_id);
    if (m_device.register_write(WK2132_REGISTER_GENA, reg_gena) < 0) {
        return -EIO;
    }

    /* Reset uart port */
    uint8_t reg_grst;
    if (m_device.register_read(WK2132_REGISTER_GRST, reg_grst) < 0) {
        return -EIO;
    }
    reg_grst |= (1 << m_uart_id);
    if (m_device.register_write(WK2132_REGISTER_GRST, reg_grst) < 0) {
        return -EIO;
    }

    /* Enable fifos */
    uint8_t reg_fcr = 0x0D;
    if (page_set(WK2132_PAGE0) < 0 ||  //
        m_device.register_write(WK2132_REGISTER_FCR, reg_fcr) < 0) {
        return -EIO;
    }

    /* Set baudrate-related registers */
    double reg_baud_pres = (m_device.m_frequency / (baudrate * 16));
    uint8_t reg_baud1 = ((uint16_t)(reg_baud_pres - 1)) >> 8;
    uint8_t reg_baud0 = ((uint16_t)(reg_baud_pres - 1)) >> 0;
    uint8_t reg_pres = (reg_baud_pres - floor(reg_baud_pres)) * 10;
    if (page_set(WK2132_PAGE1) < 0 ||                            //
        register_write(WK2132_REGISTER_BAUD1, reg_baud1) < 0 ||  //
        register_write(WK2132_REGISTER_BAUD0, reg_baud0) < 0 ||  //
        register_write(WK2132_REGISTER_PRES, reg_pres) < 0) {
        return -EIO;
    }

    /* Enable port */
    uint8_t reg_scr = 0x03;
    if (page_set(WK2132_PAGE0) < 0 ||  //
        register_write(WK2132_REGISTER_SCR, reg_scr) < 0) {
        return -EIO;
    }

    /* Set mode */
    uint8_t reg_lcr;
    switch (mode) {
        case WK2132_UART_MODE_8N1: {
            reg_lcr = 0x00;
            break;
        }
        default: {
            return -EINVAL;
        }
    }
    if (m_device.register_write(WK2132_REGISTER_LCR, reg_lcr) < 0) {
        return -EIO;
    }

    /* Return success */
    return 0;
}

/**
 * @brief
 * @param
 * @return
 * @see https://www.arduino.cc/reference/en/language/functions/communication/serial/end/
 */
int wk2132::uart::end(void) {

    /* Disable port */
    uint8_t reg_scr = 0x00;
    if (page_set(WK2132_PAGE0) < 0 ||  //
        register_write(WK2132_REGISTER_SCR, reg_scr) < 0) {
        return -EIO;
    }

    /* Return success */
    return 0;
}

/**
 * @brief
 * @see https://www.arduino.cc/reference/en/language/functions/communication/serial/flush/
 */
void wk2132::uart::flush(void) {
    // TODO Maybe ensure fifo is enabled
    // Then wait for tf empty bit to be set
}

/**
 * Returns the number of bytes available in the receive buffer.
 * @see https://www.arduino.cc/reference/en/language/functions/communication/stream/streamavailable/
 * @return The number of bytes available for read.
 */
int wk2132::uart::available(void) {

    /* Read fifo byte count register */
    uint8_t reg_rfcnt;
    if (page_set(WK2132_PAGE0) < 0 ||  //
        register_read(WK2132_REGISTER_RFCNT, reg_rfcnt) < 0) {
        return -EIO;
    }

    /* Handle case where counter if 0 which could mean either empty or full
     * by reading RDAT bit */
    if (reg_rfcnt == 0) {
        uint8_t reg_fsr;
        if (register_read(WK2132_REGISTER_FSR, reg_fsr) < 0) {
            return -EIO;
        }

        /* If the fifo is empty, we return zero */
        if ((reg_fsr & (1 << 3)) == 0) {
            return 0;
        }

        /* But if the fifo is not empty, then we need to read the fifo counter again
         * in case bytes arrived in the mean time */
        else {
            if (register_read(WK2132_REGISTER_RFCNT, reg_rfcnt) < 0) {
                return -EIO;
            }
            if (reg_rfcnt == 0) {
                return 256;
            }
        }
    }

    /* Return number of bytes available */
    return reg_rfcnt;
}

/**
 * @brief
 * @param
 * @return
 * @see https://www.arduino.cc/reference/en/language/functions/communication/serial/peek/
 */
int wk2132::uart::peek(void) {

    /* If we have already performed a peek and no read since,
     * return the byte we extracted the last time we peeked */
    if (m_peek_performed == true) {
        return m_peek_byte;
    }

    /* Otherwise read a byte */
    uint8_t reg_fdat;
    if (page_set(WK2132_PAGE0) < 0 ||  //
        register_read(WK2132_REGISTER_FDAT, reg_fdat) < 0) {
        return -1;
    }

    /* Store byte in case we peek again */
    m_peek_byte = reg_fdat;
    m_peek_performed = true;

    /* Return byte */
    return reg_fdat;
}

/**
 * @brief
 * @param
 * @return
 * @see https://www.arduino.cc/reference/en/language/functions/communication/serial/read/
 */
int wk2132::uart::read(void) {

    /* If we had previously performed a peek,
     * return this byte and don't empty the actual fifo of the device */
    if (m_peek_performed == true) {
        m_peek_performed = false;
        return m_peek_byte;
    }

    /* Otherwise read a byte */
    uint8_t reg_fdat;
    if (page_set(WK2132_PAGE0) < 0 ||  //
        register_read(WK2132_REGISTER_FDAT, reg_fdat) < 0) {
        return -1;
    }

    /* Return byte */
    return reg_fdat;
}

/**
 * @brief
 * @param data
 * @return
 * @see https://www.arduino.cc/reference/en/language/functions/communication/serial/write/
 */
size_t wk2132::uart::write(uint8_t data) {

    /* Ensure fifo is not full */
    uint8_t reg_fsr;
    if (page_set(WK2132_PAGE0) < 0 ||  //
        register_read(WK2132_REGISTER_FSR, reg_fsr) < 0) {
        return 0;
    }
    if ((reg_fsr & (1 << 1)) != 0) {
        return 0;
    }

    /* Write one byte */
    if (register_write(WK2132_REGISTER_FDAT, data) < 0) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief
 * @param[in] page
 * @return
 */
int wk2132::uart::page_set(const enum wk2132_page page) {

    /* Read register */
    uint8_t reg_spage;
    if (register_read(WK2132_REGISTER_SPAGE, reg_spage) < 0) {
        return -EIO;
    }

    /* Set page bit */
    switch (page) {
        case WK2132_PAGE0:
            reg_spage &= 0xFE;
            break;
        case WK2132_PAGE1:
            reg_spage |= 0x01;
            break;
        default:
            return -EINVAL;
    }
    if (register_write(WK2132_REGISTER_SPAGE, reg_spage) < 0) {
        return -EIO;
    }

    /* Return succes */
    return 0;
}

/**
 * @brief
 * @param[in] reg_addr
 * @param[out] reg_value
 * @return The number of bytes read in case of success, or a negative error code otherwise.
 */
int wk2132::uart::register_read(const enum wk2132_register reg_addr, uint8_t &reg_value) {

    /* Ensure register address is only 4-bits long */
    if ((reg_addr & 0xF0) != 0) {
        return -EINVAL;
    }

    /* Build i2c address */
    uint8_t i2c_address = m_device.m_i2c_address | (m_uart_id << 1);

    /* Send register address */
    m_device.m_i2c_library->beginTransmission(i2c_address);
    m_device.m_i2c_library->write((uint8_t)reg_addr);
    if (m_device.m_i2c_library->endTransmission(true) != 0) return -EIO;

    /* Read byte and return */
    if (m_device.m_i2c_library->requestFrom((uint8_t)i2c_address, (uint8_t)1, (uint8_t) true) != 1) return -EIO;
    int res = m_device.m_i2c_library->read();
    if (res < 0 || res > UINT8_MAX) {
        return -EIO;
    } else {
        reg_value = (uint8_t)res;
        return 1;
    }
}

/**
 * @brief
 * @param[in] reg_addr
 * @param[int] reg_value
 * @return The number of bytes written in case of success, or a negative error code otherwise.
 */
int wk2132::uart::register_write(const enum wk2132_register reg_addr, const uint8_t reg_value) {

    /* Ensure register address is only 4-bits long */
    if ((reg_addr & 0xF0) != 0) {
        return -EINVAL;
    }

    /* Build i2c address */
    uint8_t i2c_address = m_device.m_i2c_address | (m_uart_id << 1);

    /* Send register address and value */
    m_device.m_i2c_library->beginTransmission(i2c_address);
    m_device.m_i2c_library->write((uint8_t)reg_addr);
    m_device.m_i2c_library->write((uint8_t)reg_value);
    if (m_device.m_i2c_library->endTransmission(true) != 0) return -EIO;

    /* Return success */
    return 1;
}

/**
 * @brief
 * @param[out] data
 * @param[in] length
 * @return
 */
int wk2132::uart::fifo_read(uint8_t *const data, const size_t length) {
    int res;

    /* Build i2c address */
    uint8_t i2c_address = m_device.m_i2c_address | (m_uart_id << 1) | (1 << 0);

    /* Read bytes */
    for (size_t i = 0; i < length;) {
        m_device.m_i2c_library->beginTransmission(i2c_address);
        if (m_device.m_i2c_library->endTransmission(true) != 0) return -EIO;
        res = m_device.m_i2c_library->requestFrom(i2c_address, length - i);
        if (res <= 0) {
            return i;
        } else {
            for (size_t j = 0; j < ((unsigned int)res); j++) {
                data[i + j] = m_device.m_i2c_library->read();
            }
            i += res;
        }
    }

    /* Return length */
    return length;
}

/**
 * @brief
 * @param[in] data
 * @param[in] length
 * @return
 */
int wk2132::uart::fifo_write(const uint8_t *const data, const size_t length) {

    /* Build i2c address */
    uint8_t i2c_address = m_device.m_i2c_address | (m_uart_id << 1) | (1 << 0);

    /* Write bytes */
    for (size_t i = 0; i < length;) {
        m_device.m_i2c_library->beginTransmission(i2c_address);
        i += m_device.m_i2c_library->write(&data[i], length - i);
        if (m_device.m_i2c_library->endTransmission(true) != 0) return -EIO;
    }

    /* Return length */
    return length;
}
