from gpiozero import Button
from time import sleep
from threading import Thread
import os
import serial
import led_matrix
import phase0
import phase1
import phase2


# Bytes definition for communicating with the arduino over the Serial port.
byte_phase1 = b'a' #Phase1 Byte
byte_phase2 = b'b' #Phase2 Byte
byte_phase0 = b'c' #Phase0 Byte


# Opens the serial port.
ser = serial.Serial('/dev/ttyACM0', 9600)


# Buttons Definition
btn1 = Button(26)
btn2 = Button(19)


if __name__ == '__main__':
    # starts the led matrix loop on a different thread.
    Thread(target = led_matrix.init).start()

    # plays the init.mp3 file
    os.system("play assets/init.mp3")

    # selects the phase to run according to the pressed button and sends it to the Arduino 
    # over the serial port.
    if btn1.is_pressed:
        ser.write(byte_phase1)
        phase1.run()
    elif btn2.is_pressed:
        ser.write(byte_phase2)
        phase2.run()
    else:
        ser.write(byte_phase0)
        phase0.run()
