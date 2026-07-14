#ifndef ADS131M04_PORT_H // Include guard
#define ADS131M04_PORT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus // C++ compatibility wrapper
extern "C" {
#endif

// Full-duplex blocking SPI exchange of `len` bytes. Return 0 on success.
int  ads131m04_port_spi_transfer(const uint8_t *tx, uint8_t *rx, uint16_t len);

// assert=true drives CS LOW.
void ads131m04_port_cs(bool assert);

// Wait at least `us` microseconds.
void ads131m04_port_delay_us(uint32_t us);

// Drive the SYNC/RESET pin; level=true is HIGH (deasserted).
void ads131m04_port_reset_pin(bool level);

#ifdef __cplusplus
}
#endif
#endif // ADS131M04_PORT_H
