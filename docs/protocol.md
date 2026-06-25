
MOVE command (10 bytes total):
Byte 0     : command type   (0x01 = MOVE)
Byte 1     : direction      (0 = backward, 1 = forward)
Byte 2-5   : step count     (4 bytes, unsigned, big machines need more than 255 steps)
Byte 6-9   : step delay     (4 bytes, microseconds between pulses, controls speed)

also we are using  Big-Endian [Network byte Order]

[ Linux Host ] --( Sends 10 Bytes via USB )--> [ Nucleo ST-LINK ]
                                                         |
                                             ( Converts to UART pulses )
                                                         |
                                                         v
 [ main.c Loop ] <--- (Copies 10 bytes) <--- [ rx_buffer in UART Driver ]
       |
 (Parses Struct)
       |
       +---> [ GPIOA Pin 4 ] -----> Sets Motor DIR Pin (High/Low)
       +---> [ TIM3 Counter ] ----> Starts Pulsing Motor STEP Pin
       |
       +---> [ UART_SendByte(0x55) ] --> Sends "SUCCESS" back to Linux!
