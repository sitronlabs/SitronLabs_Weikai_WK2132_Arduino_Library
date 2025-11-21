[![Designed by Sitron Labs](https://img.shields.io/badge/Designed_by-Sitron_Labs-FCE477)](https://www.sitronlabs.com/)
[![Join the Discord community](https://img.shields.io/discord/552242187665145866?logo=discord&logoColor=white&label=Discord&color=%237289da)](https://discord.gg/btnVDeWhfW)
![License](https://img.shields.io/github/license/sitronlabs/SitronLabs_Weikai_WK2132_Arduino_Library)
![Latest Release](https://img.shields.io/github/release/sitronlabs/SitronLabs_Weikai_WK2132_Arduino_Library)
[![Arduino Library Manager](https://www.ardu-badge.com/badge/Sitron%20Labs%20WK2132%20Arduino%20Library.svg?)](https://www.ardu-badge.com/Sitron%20Labs%20WK2132%20Arduino%20Library)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/sitronlabs/library/Sitron_Labs_WK2132_Arduino_Library.svg)](https://registry.platformio.org/libraries/sitronlabs/Sitron_Labs_WK2132_Arduino_Library)

# Sitron Labs Weikai WK2132 Arduino Library

Arduino library for interfacing with the Weikai WK2132 multi-bus to UART converter.

## Description

The WK2132 is a 2-channel UART device with low power consumption, 256-level FIFO, and support for UART/SPI/I2C interfaces. This library provides a simple interface to access the two UART channels via I2C. The UART objects inherit from Arduino's Stream and Print classes, allowing them to be used just like the standard Serial interface.

## Installation

### Arduino IDE

Install via the Arduino Library Manager by searching for "Sitron Labs WK2132".

Alternatively, install manually:
1. Download or clone this repository
2. Place it in your Arduino `libraries` folder
3. Restart the Arduino IDE

### PlatformIO

Install via the PlatformIO Library Manager by searching for "Sitron Labs WK2132".

Alternatively, add the library manually to your `platformio.ini` file:

```ini
lib_deps = 
    https://github.com/sitronlabs/SitronLabs_Weikai_WK2132_Arduino_Library.git
```

## Hardware Connections

Connect the WK2132 to your Arduino using I2C:

- VCC → 3.3V or 5V (check your board's specifications)
- GND → GND
- MP2/SCL → SCL (I2C Clock)
- MP0/SDA → SDA (I2C Data)
- MP1/IA0 → GND or VCC (I2C address selection bit 0)
- MP3/IA1 → GND or VCC (I2C address selection bit 1)

The I2C address is determined by the IA0 and IA1 pins. The base address is 0x10, with IA0 adding 0x20 and IA1 adding 0x40.

## Usage

```cpp
#include <Wire.h>
#include <wk2132.h>

// Create WK2132 device object
wk2132 device;

// Crystal frequency in Hz (must be between 1.8432MHz and 14.7456MHz)
const uint32_t CRYSTAL_FREQUENCY = 14745600;

// I2C address selection pins (IA0, IA1)
// Both LOW = 0x10, IA0 HIGH = 0x30, IA1 HIGH = 0x50, Both HIGH = 0x70
const bool IA0 = false;
const bool IA1 = false;

void setup() {
  Serial.begin(9600);
  
  // Initialize I2C
  Wire.begin();
  
  // Setup the WK2132 (crystal frequency, I2C library, IA0, IA1)
  if (device.setup(CRYSTAL_FREQUENCY, Wire, IA0, IA1) != 0) {
    Serial.println("Failed to setup WK2132");
    return;
  }
  
  // Initialize UART channel 0 at 9600 baud
  if (device.uarts[0].begin(9600) != 0) {
    Serial.println("Failed to initialize UART 0");
    return;
  }
  
  // Initialize UART channel 1 at 115200 baud
  if (device.uarts[1].begin(115200) != 0) {
    Serial.println("Failed to initialize UART 1");
    return;
  }
  
  Serial.println("WK2132 initialized");
}

void loop() {
  // Use UART 0 like a regular Serial port
  if (device.uarts[0].available()) {
    char c = device.uarts[0].read();
    Serial.print("UART 0 received: ");
    Serial.println(c);
    
    // Echo back to UART 0
    device.uarts[0].write(c);
  }
  
  // Use UART 1 like a regular Serial port
  if (device.uarts[1].available()) {
    String message = "Hello from UART 1\n";
    device.uarts[1].print(message);
    delay(1000);
  }
}
```

## API Reference

### setup(uint32_t frequency, TwoWire &i2c_library, bool ia0, bool ia1)

Initializes the WK2132 device.

- `frequency`: Crystal frequency in Hz (must be between 1843200 and 14745600)
- `i2c_library`: I2C library instance to use (typically `Wire`)
- `ia0`: I2C address selection bit 0 (false = LOW, true = HIGH)
- `ia1`: I2C address selection bit 1 (false = LOW, true = HIGH)

Returns 0 on success, or a negative error code otherwise.

### detect(void)

Detects if the WK2132 device is present on the I2C bus.

Returns true if the device is detected, false otherwise.

### uarts[0] and uarts[1]

Two UART channel objects that inherit from Arduino's Stream and Print classes.

#### begin(uint32_t baudrate, wk2132_uart_mode mode = WK2132_UART_MODE_8N1)

Initializes the UART channel.

- `baudrate`: Desired baud rate
- `mode`: UART mode (currently only WK2132_UART_MODE_8N1 is supported)

Returns 0 on success, or a negative error code otherwise.

#### end(void)

Disables the UART channel.

Returns 0 on success, or a negative error code otherwise.

#### Stream and Print Methods

The UART objects support all standard Stream and Print methods:
- `available()`: Returns the number of bytes available to read
- `read()`: Reads a byte from the UART
- `peek()`: Peeks at the next byte without removing it
- `write(uint8_t)`: Writes a byte to the UART
- `print()`: Prints data to the UART
- `println()`: Prints data with a newline
- `flush()`: Flushes the transmit buffer

## Specifications

- Number of UART channels: 2
- FIFO depth: 256 bytes per channel
- Supported crystal frequencies: 1.8432 MHz to 14.7456 MHz
- Communication interface: I2C
- I2C address range: 0x10, 0x30, 0x50, or 0x70 (depending on IA0/IA1)
- Supported UART modes: 8N1 (8 data bits, no parity, 1 stop bit)
