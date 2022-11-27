from mfrc522 import SimpleMFRC522
import RPi.GPIO as GPIO
import time
import json
import os
import serial


# Creates an instance of the MFRC522 (RFID) Reader.
reader = SimpleMFRC522()

# Opens the serial port.
ser = serial.Serial('/dev/ttyACM0', 9600)

# Opens and loads the JSON file with the planets information
file = open("assets/planets.json", "r").read()
planets = json.loads(file)

# Byte definition for communicating with the arduino over the Serial port.
byte_free = b'1'


# Plays a .mp3 file by giving it its path.
def play(filepath):
    os.system('play ' + filepath + ' pitch 300 echo 0.8 0.88 15 1 reverb 20')


# Main loop of the phase 1.
def run():
    while(True):
        try:
            # Reads and loads the data from the RFID sensor.
            id, planet_name = reader.read()
            planet_name = planet_name.strip()

            # checks if a planet has been reached by comparing the RFID's data with the planets' information.
            if(planet_name in planets.keys()):
                # Gets the reached planet and sends an information on it to the arduino over the serial port.
                current_planet = planets[planet_name]
                planet_byte = bytes(current_planet[0], 'utf-8')
                ser.write(planet_byte)

                # play the .mp3 file containing the curiosities of the reached planet.
                play(os.path.join('assets', current_planet[1]))
                time.sleep(2)

                # Sends the 'free' state to the arduino in order to make it start moving again.
                ser.write(byte_free)
                time.sleep(5)

        except Exception as e:
            print(e)
