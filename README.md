# STM32 DHT22 OLED device

Project bases on STM32F1 VL Discovery board, and consist of DHT22 sensor and small OLED screen, hence air temperature and humidity can be measured and presented.

## Building:
 1.  call `apt update && apt install gcc-arm-none-eabi` on Ubuntu or any similar command on other non-apt OS.
If you package management system does not have arm baremetal compilers, you must provide them on your own. Makefile assumes that their names are like:

    * `arm-none-eabi-gcc`,
    * `arm-none-eabi-ar`,
    * `arm-none-eabi-as`,

    and can be found under `PATH` environment variable,

 2. call `git submodule update --init --recursive`,

 3. go to src and enter `make`,

 4. flash src.elf into your device.
