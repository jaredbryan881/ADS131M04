#include "ADS131M04.h"

// Timing constants (datasheet section 6.6/6.7)
#define T_RESET_CLKS   2048u
#define T_REGACQ_US    5u

// Device state
static uint32_t s_f_clkin_hz = 2048000u;
static uint16_t s_last_response;

uint16_t ads131m04_last_response(void) { return s_last_response; }

/* ********** */
/* Primitives */
/* ********** */
// Most commands consist of a 6-word frame.
// Word length is set in WLENGTH[1:0] bits and defaults to 24-bits/3-bytes -> each frame is 18-bytes
static ADS131M04_err_t exchange_frame(uint16_t cmd, const uint16_t *wr, uint8_t *rx)
{
	uint8_t tx[ADS131M04_FRAME_BYTES] = {0};

	// 16-bit data MSB-aligned in a 24-bit word
	tx[0] = (uint8_t)(cmd >> 8);
	tx[1] = (uint8_t)cmd;
	// optionally place a payload word in bytes 3-4
	if (wr) {
		tx[3] = (uint8_t)(*wr >> 8);
		tx[4] = (uint8_t)*wr;
	}

	// pull CS low, perform one full-duplex 18-byte transfer, pull CS high
	ADS131M04_port_cs(true);
	int r = ADS131M04_port_spi_transfer(tx, rx, ADS131M04_FRAME_BYTES);
	ADS131M04_port_cs(false);

	// cache response to previous frame
	if (r != 0)
		return ADS131M04_ERR_SPI;
	s_last_response = (uint16_t)((rx[0] << 8) | rx[1]);

	return ADS131M04_OK;
}

/* ***** */
/* Reset */
/* ***** */
ADS131M04_err_t ADS131M04_reset(void)
{
	uint8_t rx[ADS131M04_FRAME_BYTES];
	ADS131M04_err_t e;

	// Pin reset, held longer than 2048 t_CLKIN
	uint32_t hold_us = (uint32_t)(2ull * T_RESET_CLKS * 1000000ull / s_f_clkin_hz) + 1u;
	ADS131M04_port_reset_pin(false);
	ADS131M04_port_delay_us(hold_us);
	ADS131M04_port_reset_pin(true);
	ADS131M04_port_delay_us(10u * T_REGACQ_US);

	// Check that first frame after reset is 0xFF24
	if ((e = exchange_frame(ADS131M04_CMD_NULL, 0, rx)) != ADS131M04_OK)
		return e;
	if (s_last_response != ADS131M04_RESP_RESET_OK)
		return ADS131M04_ERR_NO_RESPONSE;

	return ADS131M04_OK;
}