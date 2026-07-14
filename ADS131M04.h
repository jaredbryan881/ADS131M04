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
// Response: dddd_dddd_dddd_dddd IF n==0
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

/* ******************************** */
/* CLOCK register bits (Table 8-17) */
/* ******************************** */
#define ADS131M04_CLOCK_CHn_EN(ch)  (1u << (8u + (ch))) // 0b=disabled, 1b=enabled, reset value 1b for all four channels
#define ADS131M04_CLOCK_CH_EN_MASK  0x0F00u
#define ADS131M04_CLOCK_TBM         (1u << 5) // 0b=OSR set by OSR[2:0], 1b=OSR of 64
#define ADS131M04_OSR_MASK          (0x001Cu) // bits[4:2]: 000b=128, 001b=256, 010b=512, 011b=1024, 100b=2048, 101b=4096, 110b=8192, 111b=16256, reset value 011b
#define ADS131M04_CLOCK_OSR_SHIFT   2u
#define ADS131M04_PWR_MASK          0x0003u // bits[1:0]: 00b=very low power, 01b=low power, 10b=high resolution, 11b=high resolution, reset value 10b

/* ******************************** */
/* GAIN1 register bits (Table 8-18) */
/* ******************************** */
// One nibble per channel: ch0[2:0], ch1[6:4], ch2[10:8], ch4[14:12]
// Gain=2^code, reset value 0x0000 (Gain=1)
#define ADS131M04_GAIN1_PGAGAIN_SHIFT(ch) (4u * (ch))
#define ADS131M04_GAIN1_PGAGAIN_MASK(ch) (0x7u << ADS131M04_GAIN1_PGAGAIN_SHIFT(ch))

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
#define ADS131M04_CHn_MUX_MASK       0x0003u   // bits[1:0]

/* ********************************************** */
/* CHn_OCAL_MSB register bits (Table 8-24..27+(ch*7)) */
/* ********************************************** */
#define ADS131M04_CAL_LSB_MASK 0xFF00u

/* ************************************* */
/* REGMAP_CRC register bits (Table 8-43) */
/* ************************************* */

#ifdef __cplusplus
}
#endif
#endif // ADS131M04_H