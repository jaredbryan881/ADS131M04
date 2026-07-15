#ifndef ADS131M04_H // Include guard
#define ADS131M04_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus // C++ compatibility wrapper
extern "C" {
#endif

// Frame geometry
#define ADS131M04_NUM_CHANNELS   4u
#define ADS131M04_WORD_BYTES     3u
#define ADS131M04_FRAME_WORDS    (ADS131M04_NUM_CHANNELS + 2u)
#define ADS131M04_FRAME_BYTES    (ADS131M04_FRAME_WORDS * ADS131M04_WORD_BYTES)

/* ******************************** */
/* Command Definitions (Table 8-11) */
/* ******************************** */
// Commands
#define ADS131M04_CMD_NULL       0x0000u // Response: STATUS register
#define ADS131M04_CMD_RESET      0x0011u
#define ADS131M04_CMD_STANDBY    0x0022u
#define ADS131M04_CMD_WAKEUP     0x0033u
#define ADS131M04_CMD_LOCK       0x0555u
#define ADS131M04_CMD_UNLOCK     0x0655u
// Responses
#define ADS131M04_RESP_RESET_OK   0xFF24u
#define ADS131M04_RESPRESET_NOK   0x0011u
#define ADS131M04_RESP_STANDBY    0x0022u
#define ADS131M04_RESP_WAKEUP     0x0033u
#define ADS131M04_RESP_LOCK       0x0555u
#define ADS131M04_RESP_UNLOCK     0x0655u
// RREG: 101a_aaaa_annn_nnnn
// Read nnn_nnnn plus 1 registers beginning at address a_aaaa_a
// Response: dddd_dddd_dddd_dddd IF n==0, i.e. response is just the data at that register
// Response: 111a_aaaa_ammm_mmmm IF n>0, followed by the n+1 register data words
#define ADS131M04_CMD_RREG(a)    (0xA000u | (((uint16_t)(a) & 0x3Fu) << 7))
// WREG: 011a_aaaa_annn_nnnn
// Write n+1 registers beginning at adddress a_aaaa_a
#define ADS131M04_CMD_WREG(a)    (0x6000u | (((uint16_t)(a) & 0x3Fu) << 7))
// WREG Response: 010a_aaaa_ammm_mmmm
// m = registers written - 1
#define ADS131M04_RESP_WREG(a)    (0x4000u | (((uint16_t)(a) & 0x3Fu) << 7))

/* ************************* */
/* Register Map (Table 8-12) */
/* ************************* */
// Device settings and indicators (read-only registers)
#define ADS131M04_REG_ID          0x00u  // [11:8] CHANCNT = 0100b, reset value 0x24xx
#define ADS131M04_REG_STATUS      0x01u  // reset value 0x0500
// Global settings across channels
#define ADS131M04_REG_MODE        0x02u  // reset value 0x0510
#define ADS131M04_REG_CLOCK       0x03u  // reset value 0x0F0E
#define ADS131M04_REG_GAIN1       0x04u  // PGA gains, one nibble per channel
#define ADS131M04_REG_CFG         0x06u  // reset value 0x0600
#define ADS131M04_REG_THRSHLD_MSB 0x07u  // reset value 0x0000
#define ADS131M04_REG_THRSHLD_LSB 0x08u  // reset value 0x0000
// Channel-specific settings
// CH0 settings start at 0x09 and channels have stride of 5
#define ADS131M04_REG_CHn_CFG(ch)      (0x09u + 5u * (ch)) // reset value 0x0000
#define ADS131M04_REG_CHn_OCAL_MSB(ch) (0x0Au + 5u * (ch)) // reset value 0x0000
#define ADS131M04_REG_CHn_OCAL_LSB(ch) (0x0Bu + 5u * (ch)) // reset value 0x0000
#define ADS131M04_REG_CHn_GCAL_MSB(ch) (0x0Cu + 5u * (ch)) // reset value 0x8000
#define ADS131M04_REG_CHn_GCAL_LSB(ch) (0x0Du + 5u * (ch)) // reset value 0x0000
// Register map CRC and reserved registers
#define ADS131M04_REG_REGMAP_CRC 0x3Eu // reset value 0x0000

/* ********************************* */
/* STATUS register bits (Table 8-15) */
/* ********************************* */
#define ADS131M04_STATUS_LOCK         (1u << 15) // 0b=Unlocked, 1b=Locked, reset value 0b
#define ADS131M04_STATUS_F_RESYNC     (1u << 14) // 0b=No sync, 1b=Resync occurred, reset value 0b
#define ADS131M04_STATUS_REG_MAP      (1u << 13) // 0b-No change in register map CRC, 1b=change, reset value 0b
#define ADS131M04_STATUS_CRC_ERR      (1u << 12) // 0b=No CRC error, 1b=error, reset value 0b
#define ADS131M04_STATUS_CRC_TYPE     (1u << 11) // 0b=16 bit CCITT, 1b=16 bit ANSI, reset value 0b
#define ADS131M04_STATUS_RESET        (1u << 10) // 0b=Not reset, 1b=reset, reset value 1b
#define ADS131M04_STATUS_WLENGTH_MASK 0x0300u    // bits [9:8]: 00b=16 bit, 01b=24 bits, 10b=32 bits zero pad, 11b=32 bits sign extended, reset value 01b
#define ADS131M04_STATUS_DRDY_MASK    0x000Fu    // DRDY[3210], 0b=No new data, 1b=new data, reset value 0000

/* ******************************* */
/* MODE register bits (Table 8-16) */
/* ******************************* */
#define ADS131M04_MODE_REG_CRC_EN    (1u << 13) // 0b=Register CRC disabled, 1b=enabled, reset value 0b
#define ADS131M04_MODE_RX_CRC_EN     (1u << 12) // 0b=disabled, 1b=enabled, reset value 0b
#define ADS131M04_MODE_CRC_TYPE      (1u << 11) // 0b=16-bit CCITT, 1b=16-bit ANSI, reset value 0b
#define ADS131M04_MODE_RESET         (1u << 10) // 0b=No reset, 1b=reset, reset value 1b
#define ADS131M04_MODE_WLENGTH_MASK  0x0300u    // 00b=16 bit, 01b=24 bits, 10b=32 bits zero pad, 11b=32 bits sign extended, reset value 01b
#define ADS131M04_MODE_WLENGTH_24BIT 0x0100u
#define ADS131M04_MODE_TIMEOUT       (1u << 4)  // 0b=disabled, 1b=enabled, reset value 1b
#define ADS131M04_MODE_DRDY_SEL_MASK 0x000Cu    // bits[3:2]: 00b=most lagging enabled channel, 01b=logic OR of all enabled channels, 10b=11b=most leading enabled channel, reset value 00b
#define ADS131M04_MODE_DRDY_HiZ      (1u << 1)  // DRDY when data not ready: 0b=logic high, 1b=Hi-Z, reset value 0b
#define ADS131M04_MODE_DRDY_FMT      (1u << 0)  // DRDY when data is ready: 0b=logic low, 1b=low pulse, reset value 0b
// Value written by ADS131M04_Init(): all reset defaults except the reset
#define ADS131M04_MODE_INIT (ADS131M04_MODE_WLENGTH_24BIT | ADS131M04_MODE_TIMEOUT) // 0x0110

/* ******************************** */
/* CLOCK register bits (Table 8-17) */
/* ******************************** */
#define ADS131M04_CLOCK_CHn_EN(ch)  (1u << (8u + (ch))) // 0b=disabled, 1b=enabled, reset value 1b for all four channels
#define ADS131M04_CLOCK_CH_EN_MASK  0x0F00u
#define ADS131M04_CLOCK_TBM         (1u << 5) // 0b=OSR set by OSR[2:0], 1b=OSR of 64
#define ADS131M04_OSR_MASK          (0x001Cu) // bits[4:2]: see ADS131M04_osr_t, reset value 011b
#define ADS131M04_CLOCK_OSR_SHIFT   2u
#define ADS131M04_PWR_MASK          0x0003u // bits[1:0]: see ADS131M04_power_t, reset value 10b

// CLOCK.PWR[1:0]. Max f_CLKIN: VLP 2.048 MHz, LP 4.096 MHz, HR 8.192 MHz.
// With 2.048 MHz clock, VLP is the only legal mode.
typedef enum{
	ADS131M04_PWR_VLP = 0u, // very low power; f_MOD = f_CLKIN / 2
	ADS131M04_PWR_LP  = 1u, // low power
	ADS131M04_PWR_HR  = 2u  // high resultion (default)
} ADS131M04_power_t;

// CLOCK.OSR[2:0]. f_DATA = f_CLKIN / 2 / OSR.
// At f_CLKIN = 2.048 MHz: OSR 4096 -> 250 SPS
typedef enum {
	ADS131M04_OSR_128   = 0u,
	ADS131M04_OSR_256   = 1u,
	ADS131M04_OSR_512   = 2u,
	ADS131M04_OSR_1024  = 3u, // default on reset
	ADS131M04_OSR_2048  = 4u,
	ADS131M04_OSR_4096  = 5u, // 250 SPS @ 2.048 MHz
	ADS131M04_OSR_8192  = 6u,
	ADS131M04_OSR_16256 = 7u  // not 16384 as you might expect
} ADS131M04_osr_t;

/* ******************************** */
/* GAIN1 register bits (Table 8-18) */
/* ******************************** */
// One nibble per channel: ch0[2:0], ch1[6:4], ch2[10:8], ch4[14:12]
// Gain=2^code, reset value 0x0000 (Gain=1)
#define ADS131M04_GAIN1_PGAGAIN_SHIFT(ch) (4u * (ch))
#define ADS131M04_GAIN1_PGAGAIN_MASK(ch) (0x7u << ADS131M04_GAIN1_PGAGAIN_SHIFT(ch))

typedef enum {
	ADS131M04_GAIN_1 = 0u, 
	ADS131M04_GAIN_2,
	ADS131M04_GAIN_4,
	ADS131M04_GAIN_8,
	ADS131M04_GAIN_16,
	ADS131M04_GAIN_32,
	ADS131M04_GAIN_64,
	ADS131M04_GAIN_128
} ADS131M04_gain_t;

/* ****************************** */
/* CFG register bits (Table 8-19) */
/* ****************************** */
#define ADS131M04_CFG_GC_DLY_MASK 0x1E00u   // bits[12:9]: chop delay=2^(code+1), reset value 0011b
#define ADS131M04_CFG_GC_EN       (1u << 8) // 0b=disabled, 1b=enabled, reset value 0b
#define ADS131M04_CFG_CD_ALLCH    (1u << 7) // 0b=any channel, 1b=all channels, reset value 0b
#define ADS131M04_CFG_CD_NUM_MASK 0x0070u   // bits[6:4]: detections needed: 2^code, reset value 000b
#define ADS131M04_CFG_CD_LEN_MASK 0x000Eu   // bits[3:1]: detection window, 2^(code+7), reset value 000b
#define ADS131M04_CFG_CD_EN       (1u << 0) // 0b=disabled, 1b=enabled

/* *********************************************** */
/* THRSHLD_MSB/LSB register bits (Table 8-21/8-22) */
/* *********************************************** */
// CD_TH[23:0] = MSB reg [15:0] + LSB reg[15:8]
#define ADS131M04_THRSHLD_LSB_CD_TH_MASK 0xFF00u
// DC block high-pass filter coefficient a = 1/(2^(code+1))
// 0000b = DC block disabled, reset value 0000b
#define ADS131M04_THRSHLD_LSB_DCBLOCK_MASK 0x000Fu

/* ***************************************** */
/* CHn_CFG register bits (Table 8-23+(ch*7)) */
/* ***************************************** */
#define ADS131M04_CHn_CFG_PHASE_MASK 0xFFC0u   // bits[15:6], phase delay in modulator clocks
#define ADS131M04_CHn_CFG_DCBLK_DIS  (1u << 2) // 1b=controlled by DCBLOCK[3:0], 1b=disabled
#define ADS131M04_CHn_CFG_MUX_MASK   0x0003u   // bits[1:0]

// CHn_CFG.MUX[1:0]: what the ADC actually digitizes for CHn
typedef enum {
	ADS131M04_MUX_INPUT     = 0u, // AINxP/AINxN pins, default on reset
	ADS131M04_MUX_SHORTED   = 1u, // ADC inputs shorted internally
	ADS131M04_MUX_TEST_POS  = 2u, // positive DC test signal
	ADS131M04_MUX_TEST_NEG  = 3u  // negative DC test signal
} ADS131M04_mux_t;

/* ********************************************** */
/* CHn_OCAL_MSB register bits (Table 8-24..27+(ch*7)) */
/* ********************************************** */
#define ADS131M04_CAL_LSB_MASK 0xFF00u

/* ************************************* */
/* REGMAP_CRC register bits (Table 8-43) */
/* ************************************* */

/* ********************* */
/* Driver: return codes  */
/* ********************* */
typedef enum {
	ADS131M04_OK = 0,
	ADS131M04_ERR_SPI,         // hal.spi_transfer failure
	ADS131M04_ERR_NO_RESPONSE, // reset "ready" word (0xFF24) never seen
	ADS131M04_ERR_BAD_ID,      // ID.CHANCNT != 4
	ADS131M04_ERR_VERIFY,      // response mismatch
	ADS131M04_ERR_PARAM
} ADS131M04_err_t;

/* ******************** */
/* Driver: data struct  */
/* ******************** */
typedef struct {
	int32_t  ch[ADS131M04_NUM_CHANNELS]; // sign-extended 24-bit samples
	uint16_t status;                     // STATUS contents that headed this frame
	bool     crc_ok;                     // device output CRC matched our recompute
} ADS131M04_data_t;

/* ************ */
/* Driver: API  */
/* ************ */
// Reset via SYNC/RESET pin, confirm response is 0xFF24 and CHANCNT == 4,
// then write MODE = ADS131M04_MODE_INIT (return to defaults and clear the reset flag)
ADS131M04_err_t ADS131M04_init(uint32_t f_clkin_hz);

// Reset via SYNC/RESET pin, confirm response is 0xFF24
ADS131M04_err_t ADS131M04_reset(void);

// Single-register access
// Each is a two-frame exchange: command, then NULL to collect the response.
ADS131M04_err_t ADS131M04_read_reg(uint8_t addr, uint16_t *val);
// write_reg checks the 010a_aaaa_a000_0000 response
ADS131M04_err_t ADS131M04_write_reg(uint8_t addr, uint16_t val);
// write_reg_verify also reads the register back and compares
ADS131M04_err_t ADS131M04_write_reg_verify(uint8_t addr, uint16_t val);

// Set the CLOCK register: channel enables, OSR, power mode
ADS131M04_err_t ADS131M04_configure_clock(uint8_t ch_en_mask, ADS131M04_osr_t osr, ADS131M04_power_t pwr);

// Program one channel's PGA gain
ADS131M04_err_t ADS131M04_set_gain(uint8_t channel, ADS131M04_gain_t gain);

// Set a channel's input mux
ADS131M04_err_t ADS131M04_set_mux(uint8_t channel, ADS131M04_mux_t mux);

// Unpack an 18-byte frame
void ADS131M04_parse_frame(const uint8_t rx[ADS131M04_FRAME_BYTES], ADS131M04_data_t *out);

// Exchange one NULL frame and parse it (blocking read)
ads131m04_err_t ads131m04_read_data(ads131m04_data_t *out);

// Poll DRDY bit of STATUS register
ADS131M04_err_t ADS131M04_data_ready(bool *ready);

// Get the first word of the most recent frame
uint16_t ADS131M04_last_response(void);

#ifdef __cplusplus
}
#endif
#endif // ADS131M04_H