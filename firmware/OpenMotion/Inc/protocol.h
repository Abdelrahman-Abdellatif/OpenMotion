#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define CMD_MOVE 0x01

/* --- New Response Status Codes --- */
#define STATUS_OK                 0x55  /* Command accepted and executing */
#define ERR_INVALID_COMMAND       0x99  /* Received unknown command ID */
#define ERR_INVALID_PARAMETER     0xAA  /* Parameters (like delay=0) are dangerous */
#define STATUS_MOVE_COMPLETE      0x77  /* Motor physically finished all steps */

typedef struct {
	uint8_t command;
	uint8_t direction;
	uint32_t steps;
	uint32_t delay_us;
} MoveCommand_t;

uint8_t Protocol_Parse(const uint8_t *raw, MoveCommand_t *out);

#endif
