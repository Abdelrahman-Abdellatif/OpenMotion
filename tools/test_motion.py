import serial
import struct
import time

# Update this to match your actual port name from Step 2
SERIAL_PORT = '/dev/ttyACM0' 
BAUD_RATE = 115200

# Protocol Constants
CMD_MOVE = 0x01
STATUS_OK = 0x55
STATUS_MOVE_COMPLETE = 0x77
ERR_INVALID_COMMAND = 0x99
ERR_INVALID_PARAMETER = 0xAA

def send_move_command(ser, direction, steps, delay_us):
    """
    Packs data into 10 bytes using Big-Endian format (>)
    Format mapping:
    > B B I I
    >   = Big-Endian
    B   = 1 byte (unsigned char) -> Command
    B   = 1 byte (unsigned char) -> Direction
    I   = 4 bytes (unsigned int) -> Steps
    I   = 4 bytes (unsigned int) -> Delay
    Total: 1 + 1 + 4 + 4 = 10 bytes
    """
    packet = struct.pack('>BBII', CMD_MOVE, direction, steps, delay_us)
    
    print(f"\n🚀 Sending: {steps} steps | Direction: {direction} | Speed Delay: {delay_us}us")
    ser.write(packet)
    
    # 1. Wait for instant ACK response
    ack = ser.read(1)
    if not ack:
        print("❌ Error: No response from STM32 (Timeout)")
        return False
        
    ack_code = ack[0]
    if ack_code == STATUS_OK:
        print("✅ STM32 says: Command Accepted! Motor is running...")
    elif ack_code == ERR_INVALID_COMMAND:
        print("❌ STM32 says: Error! Invalid Command ID")
        return False
    elif ack_code == ERR_INVALID_PARAMETER:
        print("❌ STM32 says: Error! Invalid Parameter (e.g., Delay is 0)")
        return False
    else:
        print(f"❓ Received unknown status byte: {hex(ack_code)}")
        return False

    # 2. Block and wait for physical completion notification
    print("⏳ Waiting for motor to finish moving...")
    complete_code = ser.read(1)
    if complete_code and complete_code[0] == STATUS_MOVE_COMPLETE:
        print("🎉 STM32 says: MOVE COMPLETE! Motor is completely still.")
        return True
    else:
        print("❌ Error: Missing move complete signal.")
        return False

def main():
    try:
        # Open serial port with a 5-second timeout
        print(f"🔌 Opening serial port {SERIAL_PORT}...")
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=5) as ser:
            # Clear buffers
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            
            # Read the initial boot message printed by main.c
            time.sleep(0.5) # Give the port a split second to settle
            if ser.in_waiting:
                boot_msg = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
                print(f"🤖 Board Output: {boot_msg.strip()}")

            # Test 1: Valid Move (Take 200 steps forward slowly)
            send_move_command(ser, direction=1, steps=200, delay_us=2000)
            
            time.sleep(1) # Pause for a second between movements

            # Test 2: Valid Move (Take 400 steps backward quickly)
            send_move_command(ser, direction=0, steps=400, delay_us=800)
            
    except serial.SerialException as e:
        print(f"❌ Serial Port Error: {e}")
    except KeyboardInterrupt:
        print("\n🛑 Test stopped by user.")

if __name__ == '__main__':
    main()