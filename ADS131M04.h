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
// Read nnn_nnnn plus 1 registers beginning at adddress a_aaaa_a
// Response: dddd_dddd_dddd_dddd IF n==0
// Response: 010a_aaaa_ammm_mmmm IF n>0, followed by register data
#define ADS131M04_CMD_RREG(a)    (0xA000u | (((uint16_t)(a) & 0x3Fu) << 7))
// WREG: 011a_aaaa_annn_nnnn
// Write n+1 registers beginning at adddress a_aaaa_a
#define ADS131M04_CMD_WREG(a)    (0x6000u | (((uint16_t)(a) & 0x3Fu) << 7))
// WREG Response: 010a_aaaa_ammm_mmmm
// m = registers written - 1
#define ADS131M04_RESP_WREG(a)    (0x4000u | (((uint16_t)(a) & 0x3Fu) << 7))

#ifdef __cplusplus
}
#endif
#endif // ADS131M04_H