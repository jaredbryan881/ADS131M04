#include "ADS131M04.h"

// Timing constants (datasheet section 6.6/6.7)
#define T_RESET_CLKS   2048u
#define T_REGACQ_US    5u

// Device state
static uint32_t s_f_clkin_hz = 2048000u;
static uint16_t s_last_response;

uint16_t ads131m04_last_response(void) { return s_last_response; }

/* ***************** */
/* ANSI CRC (CRC-16) */
/* ***************** */
static uint16_t crc16(const uint8_t *p, uint32_t n)
{
	// running remainder of the division
	uint16_t crc = 0xFFFFu;
	while (n--) {
		// Bring the message byte into the top half of the 16-bit working register
		crc ^= (uint16_t)(*p++) << 8;
		// perform the division
		for (unsigned i = 0; i < 8; ++i)
			crc = (crc & 0x8000u) ? (uint16_t)((crc << 1) ^ 0x1021u) : (uint16_t)(crc << 1);
	}
	return crc;
}

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
	if ((e = ADS131M04_read_reg(ADS131M04_REG_ID, &id)) != ADS131M04_OK)
		return e;
	if (((id >> 8) & 0x0Fu) != ADS131M04_NUM_CHANNELS)
		return ADS131M04_ERR_BAD_ID;

	// Clear RESET bit in the STATUS register
	return ADS131M04_write_reg_verify(ADS131M04_REG_MODE, ADS131M04_MODE_INIT);
}

/* ************* */
/* Configuration */
/* ************* */
ADS131M04_err_t ADS131M04_configure_clock(uint8_t ch_en_mask, ADS131M04_osr_t osr, ADS131M04_power_t pwr)
{
	if (ch_en_mask > 0x0Fu || (uint8_t)osr > 7u || (uint8_t)pwr > 2u)
		return ADS131M04_ERR_PARAM;

	// CLOCK register (Table 8-17): CH_EN[11:8] | TBM=0 | OSR[4:2] | PWR[1:0]
	uint16_t clock = (uint16_t)(((uint16_t)ch_en_mask << 8) | ((uint16_t)osr << ADS131M04_CLOCK_OSR_SHIFT) | (uint16_t)pwr);
	return ADS131M04_write_reg_verify(ADS131M04_REG_CLOCK, clock);
}

ADS131M04_err_t ADS131M04_set_gain(uint8_t channel, ADS131M04_gain_t gain)
{
	uint16_t g;
	ADS131M04_err_t e;

	if (channel >= ADS131M04_NUM_CHANNELS || (uint8_t)gain > 7u)
		return ADS131M04_ERR_PARAM;

	// GAIN1 register: PGAGAIN0 in bits [2:0], PGAGAIN1 in bits [6:4] and so on
	if ((e = ADS131M04_read_reg(ADS131M04_REG_GAIN1, &g)) != ADS131M04_OK)
		return e;
	g = (uint16_t)((g & ~ADS131M04_GAIN1_PGAGAIN_MASK(channel)) | ((uint16_t)gain << ADS131M04_GAIN1_PGAGAIN_SHIFT(channel)));
	return ADS131M04_write_reg_verify(ADS131M04_REG_GAIN1, g);
}

ADS131M04_err_t ADS131M04_set_mux(uint8_t channel, ADS131M04_mux_t mux)
{
	uint16_t cfg;
	ADS131M04_err_t e;

	if (channel >= ADS131M04_NUM_CHANNELS || (uint8_t)mux > 3u)
		return ADS131M04_ERR_PARAM;

	// CHn_CFG register MUXn[1:0] bits
	if ((e = ADS131M04_read_reg(ADS131M04_REG_CHn_CFG(channel), &cfg)) != ADS131M04_OK)
		return e;
	cfg = (uint16_t)((cfg & ~ADS131M04_CHn_CFG_MUX_MASK) | (uint16_t)mux);
	return ADS131M04_write_reg_verify(ADS131M04_REG_CHn_CFG(channel), cfg);
}

/* *************** */
/* Data conversion */
/* *************** */
void ADS131M04_parse_frame(const uint8_t rx[ADS131M04_FRAME_BYTES], ADS131M04_data_t *out)
{
	out->status = (uint16_t)((rx[0] << 8) | rx[1]);

	for (uint32_t ch = 0; ch < ADS131M04_NUM_CHANNELS; ++ch) {
		const uint8_t *w = &rx[(ch + 1u) * ADS131M04_WORD_BYTES];
		uint32_t raw = ((uint32_t)w[0] << 16) | ((uint32_t)w[1] << 8) | w[2];
		// sign-extend 24-bit to 32-bit
		out->ch[ch] = (int32_t)(raw << 8) >> 8;
	}

	// CRC is found over words 0-4, MSB-aligned in word 5
	uint16_t dev_crc = (uint16_t)((rx[15] << 8) | rx[16]);
	out->crc_ok = (crc16(rx, 15u) == dev_crc);
}

ADS131M04_err_t ADS131M04_read_data(ADS131M04_data_t *out)
{
	uint8_t rx[ADS131M04_FRAME_BYTES];
	ADS131M04_err_t e;

	if (!out)
		return ADS131M04_ERR_PARAM;

	if ((e = exchange_frame(ADS131M04_CMD_NULL, 0, rx)) != ADS131M04_OK)
		return e;

	ADS131M04_parse_frame(rx, out);
	return ADS131M04_OK;
}

ADS131M04_err_t ADS131M04_data_ready(bool *ready)
{
	uint16_t status;
	ADS131M04_err_t e;

	if (!ready)
		return ADS131M04_ERR_PARAM;

	if ((e = ADS131M04_read_reg(ADS131M04_REG_STATUS, &status)) != ADS131M04_OK)
		return e;
	*ready = (status & ADS131M04_STATUS_DRDY_MASK) != 0u;
	return ADS131M04_OK;
}