from threading import Timer
import requests
import time
import logging
import datetime
import json
import os
import serial
import led_matrix


# API endpoint
api_url = "https://roboticsanddesign.herokuapp.com/"

# authorization code to validate the requests to the web server
authorization = "1viZ9QRp9RHiEL7b2NqL"

# Opens the serial port.
ser = serial.Serial('/dev/ttyACM0', 9600)

# Bytes definition for communicating with the arduino over the Serial port.
byte_busy = b'0' #Busy Byte
byte_free = b'1' #Free Byte

# Timer variables
is_timeout = False
t = 60.0

# Initializating logs file.
logging.basicConfig(filename="logs.log", level=logging.ERROR)


# Function to get the current command from the web server.
def get_command():
    data = {"authorization": authorization}
    r = requests.post((api_url + "command"), json=data)
    return r


# Function to update the command's status on the web server.
def update_command(status, command):
    data = {"authorization": authorization, "status": status, "command": command}
    r = requests.post((api_url + "update"), json=data)
    return r


# Plays a .mp3 file by giving it its path.
def play(filepath):
    os.system('play ' + filepath + ' pitch 300 echo 0.8 0.88 15 1 reverb 20')


# Function to get the commands from the web server and execute them.
def run_request():
    try:
        time.sleep(3)
        # Get the current command
        response = get_command()
        if(response.status_code != 200):
            raise Exception("Error occurred while getting command")

        command = json.loads(response.text)["command"]
        if(command != None):
            # Executes the command by playing a .mp3 file.
            file_name = 'question' + command + '.mp3'
            play(os.path.join('assets', file_name))

            # Sets the robot status to free.
            response = update_command("free", None)

            # If the requests terminates with an error, it tryies again for a maximum of 10 times. 
            iterator = 0
            while(response.status_code != 200 and iterator < 10):
                logging.error(str(datetime.datetime.now()) + ' | ' + "Error occurred while updating command")
                response = update_command("free", None)
                iterator += 1
                time.sleep(1)

    except Exception as e:
        logging.error(str(datetime.datetime.now()) + ' | ' + str(e))



# Sets the global variable is_timeout to True
def timeout():
    global is_timeout
    is_timeout = True


# Starts a fixed time timer and start sending requests to the web serever until a 
# timeout occurs.
def listen_for_request():

    global is_timeout
    timer = Timer(t, timeout)
    timer.start()

    # Gets and executes the command until a timeout occurs
    while(not is_timeout):
        run_request()

    is_timeout = False


# Main loop of the phase 1.
def run():
    while True:
        # Reads the serial data
        data = ser.read()

        # It enters if it receives the 'busy' byte from the arduino (scared transition).
        if(data == byte_busy):
            # sets the led matrix's modality to scared.
            led_matrix.set_mode("scared")

            # waits until it receives the 'free' byte from the Arduino on the serial port.
            while True:
                data = ser.read()
                if(data == byte_free): break

            # sets the led matrix's modality to normal.
            led_matrix.set_mode("normal")

            play("assets/intro.mp3")

            # Enables the requests to the web server in order to get and execute the user's commands.
            listen_for_request()

            # Sends the 'free' byte to the Arduino in order to make it start moving again.
            ser.write(byte_free)