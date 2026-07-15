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

/* *************** */
/* Register access */
/* *************** */
ADS131M04_err_t ADS1341M04_read_reg(uint8_t addr, uint16_t *val)
{
	uint8_t rx[ADS131M04_FRAME_BYTES];
	ADS131M04_err_t e;

	if (addr > 0x3Fu || !val)
		return ADS131M04_ERR_PARAM;

	// Frame N: request. Frame N+1: register value.
	if ((e = exchange_frame(ADS131M04_CMD_RREG(addr), 0, rx)) != ADS131M04_OK)
		return e;
	if ((e = exchange_frame(ADS131M04_CMD_NULL, 0, rx)) != ADS131M04_OK)
		return e;

	*val = s_last_response;
	return ADS131M04_OK;
}

ADS131M04_err_t ADS1341M04_write_reg(uint8_t addr, uint16_t val)
{
	uint8_t rx[ADS131M04_FRAME_BYTES];
	ADS131M04_err_t e;

	if (addr > 0x3Fu)
		return ADS131M04_ERR_PARAM;

	// Command + payload packaged in the same frame
	// Response (01a_aaaa_a000_0000) arrives in the next frame
	if ((e = exchange_frame(ADS131M04_CMD_WREG(addr), &val, rx)) != ADS131M04_OK)
		return e;
	if ((e = exchange_frame(ADS131M04_CMD_NULL, 0, rx)) != ADS131M04_OK)
		return e;

	return (s_last_response == ADS131M04_RESP_WREG(addr)) ? ADS131M04_OK : ADS131M04_ERR_VERIFY;
}

ADS131M04_err_t ADS131M04_write_reg_verify(uint8_t addr, uint16_t val)
{
	uint16_t rb;
	ads131m04_err_t e;

	if ((e = ADS131M04_write_reg(addr, val)) != ADS131M04_OK) return e;
	if ((e = ADS131M04_read_reg(addr, &rb))  != ADS131M04_OK) return e;
	return (rb == val) ? ADS131M04_OK : ADS131M04_ERR_VERIFY;
}

/* ************** */
/* Initialization */
/* ************** */
ADS131M04_err_t ADS131M04_init(uint32_t f_clkin_hz)
{
	uint16_t id;
	ADS131M04_err_t e;

	if (f_clkin_hz == 0u)
		return ADS131M04_ERR_PARAM;
	s_f_clkin_hz = f_clkin_hz;

	if ((e = ADS131M04_reset()) != ADS131M04_OK)
		return e;

	// Check that channel count is 4 as expected just to test the SPI link
	if ((e = ads131m04_read_reg(ADS131M04_REG_ID, &id)) != ADS131M04_OK)
		return e;
	if (((id >> 8) & 0x0Fu) != ADS131M04_NUM_CHANNELS)
		return ADS131M04_ERR_BAD_ID;

	// Clear RESET bit in the STATUS register
	return ADS131M04_write_reg_verify(ADS131M04_REG_MODE, ADS131M04_MODE_INIT);
}