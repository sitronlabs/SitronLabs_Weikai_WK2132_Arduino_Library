/* Self header */
#include <wk2132.h>

/* Config */
#ifndef CONFIG_WK2132_DEBUG_ENABLED
#define CONFIG_WK2132_DEBUG_ENABLED 1
#endif

/* Macros */
#if CONFIG_WK2132_DEBUG_ENABLED
#define CONFIG_WK2132_DEBUG_FUNCTION(x) Serial.println(x)
#else
#define CONFIG_WK2132_DEBUG_FUNCTION(x)
#endif

/**
 *
 * @param i2c_address
 */
wk2132::wk2132(void) : uarts{
                           uart(*this, 0),
                           uart(*this, 1),
                       } {
}

/**
 * Configures the driver with access over I2C.
 * @note Call this from the Arduino setup function.
 * @note Make sure the I2C library has been initialized, (with a call to Wire.begin() for example).
 * @param[in] frequency The frequecy of the crystal, in Hz.
 * @param[in] i2c_library A pointer to the i2c library to use.
 * @param[in] ia0
 * @param[in] ia1
 * @return 0 in case of success, or a negative error code otherwise.
 */
int wk2132::setup(const uint32_t frequency, TwoWire &i2c_library, const bool ia0, const bool ia1) {

    /* Ensure frequency is valid */
    if (frequency < 1843200UL || frequency > 14745600UL) {
        Serial.println("Invalid frequency");
        return -EINVAL;
    }

    /* Store frequency */
    m_frequency = frequency;

    /* Enable i2c */
    m_i2c_library = &i2c_library;
    m_i2c_address = (ia1 << 6) | (ia0 << 5) | 0x10;

    /* Return success */
    return 0;
}

/**
 * @brief
 * @param[in] reg_addr
 * @param[out] reg_value
 * @return The number of bytes read in case of success, or a negative error code otherwise.
 */
int wk2132::register_read(const enum wk2132_register reg_addr, uint8_t &reg_value) {

    /* Send register address */
    m_i2c_library->beginTransmission(m_i2c_address);
    m_i2c_library->write((uint8_t)reg_addr);
    if (m_i2c_library->endTransmission(true) != 0) return -EIO;

    /* Read byte */
    if (m_i2c_library->requestFrom((uint8_t)m_i2c_address, (uint8_t)1, (uint8_t) true) != 1) return -EIO;
    int res = m_i2c_library->read();
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
 * @param[in] reg_value
 * @return The number of bytes written in case of success, or a negative error code otherwise.
 */
int wk2132::register_write(const enum wk2132_register reg_addr, const uint8_t reg_value) {

    /* Send register address and value */
    m_i2c_library->beginTransmission(m_i2c_address);
    m_i2c_library->write((uint8_t)reg_addr);
    m_i2c_library->write((uint8_t)reg_value);
    if (m_i2c_library->endTransmission(true) != 0) return -EIO;

    /* Return success */
    return 1;
}
