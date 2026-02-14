# STM32 DHT22 OLED device

Project bases on STM32F1 VL Discovery board, and consists of DHT22 sensor and small OLED screen, hence air temperature and humidity can be measured and presented.

## Building:
 1. If you don't have toolchain for baremetal arm please call `apt update && apt install gcc-arm-none-eabi` on Ubuntu or any similar command on other non-apt OS.
If your package management system does not have arm baremetal compilers, you must provide them on your own. Makefile assumes that their names are like:

      * `arm-none-eabi-gcc`,
      * `arm-none-eabi-ar`,
      * `arm-none-eabi-as`,

      and can be found under `PATH` environment variable,

 2. call `git submodule update --init --recursive`,

 3. enter `make -C src`,

 4. flash `src/src.elf` into your device.

## Technical details:
### Interfaces:
DHT22 has single-wire interface which is handled by stm32 via bitbanging and timer. Single-wire line needs pull-up resistor, like 4.7 kΩ.

OLED screen with SSD1306 driver is more sofisticated - it uses SPI for a communication.

### Wiring:
1. OLED:

   * SPI:
      * NSS - not used,
      * SCK - PA5,
      * MISO - not used,
      * MOSI - PA7.

   * Additional lines:

      * Reset - PA8,
      * DC - PA9,
      * CS - PA10.

2. DHT22:

   * single-wire: PA6.
