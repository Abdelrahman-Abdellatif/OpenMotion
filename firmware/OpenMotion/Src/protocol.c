#include "protocol.h"

/*----------------------------------------------------------------
 * Protocol_Parse
 *
 * Takes a raw 10-byte buffer from the UART driver, extracts the
 * bytes using Big-Endian formatting, and populates the output struct.
 *
 * Returns 1 if the command is recognized (valid), 0 otherwise.
 *----------------------------------------------------------------*/
uint8_t Protocol_Parse(const uint8_t *raw, MoveCommand_t *out)
{
    /* 1. Direct 1-byte copies for simple parameters */
    out->command   = raw[0];
    out->direction = raw[1];

    /* 2. Check if the command type is known before doing extra math */
    if (out->command != CMD_MOVE)
    {
        return 0; /* Unknown command type! Do not trust the data */
    }

    /* 3. Reconstruct the 4-byte Step Count (Big-Endian format)
     * raw[2] is the Most Significant Byte (MSB), shifted to the far left.
     * Typecasting to (uint32_t) prevents accidental bit overflow during shifts. */
    out->steps = ((uint32_t)raw[2] << 24) |
                 ((uint32_t)raw[3] << 16) |
                 ((uint32_t)raw[4] << 8)  |
                 ((uint32_t)raw[5]);

    /* 4. Reconstruct the 4-byte Step Delay (Big-Endian format) */
    out->delay_us = ((uint32_t)raw[6] << 24) |
                    ((uint32_t)raw[7] << 16) |
                    ((uint32_t)raw[8] << 8)  |
                    ((uint32_t)raw[9]);

    return 1; /* Success! Parse complete and recognized */
}
