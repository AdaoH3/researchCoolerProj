import serial
import time
import threading
import re

# Config
SERIAL_PORT = "/dev/cu.usbserial-A10LSTIE"  # Adjust for Serial port
BAUD_RATE = 115200
TEMP_LOG_FILE = "temperature_log.csv"

# Open the serial connection
def open_serial():
    try:
        s = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Allow time for Arduino to reset
        print(f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud.")
        return s
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        return None

ser = open_serial()

# Write headers if files are empty
for file_name, header in [(TEMP_LOG_FILE, "Timestamp,Radiator,Room")]:
    try:
        with open(file_name, "a") as f:
            if f.tell() == 0:
                f.write(header + "\n")
    except Exception as e:
        print(f"Error opening {file_name}: {e}")

# Thread: Read and log data
def read_and_log_data():
    global ser
    while True:
        try:
            line = ser.readline().decode().strip()
            if not line:
                continue  # Skip if no data
            
            timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
            
            # Check if this line contains temperature info
            if line.startswith("RADIATOR"):
                # Radiator read initial
                rad_match = re.search(r"RADIATOR[,:]\s*(-?\d+)", line)
                radiator_temp = rad_match.group(1) if rad_match else "?"
                # Temperature read
                room_line = ser.readline().decode().strip()
                print(f"DEBUG: Received room line: '{room_line}'")
                room_match = re.search(r"ROOM[,:]\s*(-?\d+)", room_line)
                room_temp = room_match.group(1) if room_match else "?"
                # Log data into temperature log file
                with open(TEMP_LOG_FILE, "a") as temp_file:
                    temp_file.write(f"{timestamp},{radiator_temp},{room_temp}\n")
                print(f"{timestamp} - Radiator: {radiator_temp}, Room: {room_temp}")
                # Wait 15 seconds before reading again
                time.sleep(15)

        #If serial port closes    
        except serial.SerialException as e:
            print(f"Serial error: {e}")
            if ser:
                ser.close()
            ser = None
            time.sleep(5)
            ser = open_serial()  # Attempt to reconnect
        except Exception as ex:
            print(f"Error: {ex}")
            time.sleep(0.1)

# Thread: Port open for communication toward Arduino Serial Port (Manual Control)
def send_to_arduino():
    global ser
    while True:
        user_input = input("Enter command for Arduino: ").strip()
        if user_input and ser and ser.is_open:
            print(f"Sending to Arduino: {user_input}")
            ser.write((user_input + "\n").encode())
            time.sleep(0.5)

# Starting the threads
read_thread = threading.Thread(target=read_and_log_data, daemon=True)
send_thread = threading.Thread(target=send_to_arduino, daemon=True)

read_thread.start()
send_thread.start()

# Ends program when KeyboardInterrupt is detected (ctrl c)
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Program interrupted. Closing serial connection.")
    if ser and ser.is_open:
        ser.close()
