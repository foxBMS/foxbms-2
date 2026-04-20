# HAL Configuration

## TI TMS570LC4357ZWT HAL Configuration

### App

- The generated startup routine is ignored and `src/app/main/fstartup.c` is
  used instead.
- The SPI configuration is ignored and and `src/app/driver/config/spi_cfg_*.c`
  implementations are used instead.

### Bootloader

- The generated startup routine is ignored and `src/bootloader/main/fstartup.c`
  is used instead.
