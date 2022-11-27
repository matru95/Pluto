import json
import threading
import random
import time
import os
from luma.led_matrix.device import max7219
from luma.core.interface.serial import spi, noop
from luma.core.render import canvas

# create matrix device
cwd = os.getcwd()
serial = spi(port=0, device=1, gpio=noop())
device = max7219(serial, cascaded=2)



# Opens the and read the JSON file containing the led matrix's expressions.
def load_expressions():
    with open(os.path.join(cwd, 'assets/expressions.json'), 'r') as json_file:
        data = json.load(json_file)
        json_file.close()
        return data

# Default modality
mode = "normal"

# Default expression 
expressions = load_expressions()

# expressions pool (only for the normal modaliity)
normal_expressions = ["left", "right", "up", "down", "down_left", "down_right", "up_left", "up_right"]


# Sets one of the different modalities of the Robot's Led matrix.
def set_mode(value):
    global mode
    mode = value


# Given an expression under the form of a Bidimensional Array, 
# draws it on the Led matrix by switching on the corresponding leds.
def show_expression(expression):
    expression = expressions[expression]
    with canvas(device) as draw:
        for i in range(len(expression)):
            for j in range(len(expression[i])):
                fill = "white" if expression[i][j] == 1 else "black"
                draw.point((j,i), fill=fill)


# Main Loop of the Led Matrix that shows the animations corresponding to the selected
# modality (normal or scared).
def init():
    index = 0
    while True:
        if(mode == "scared"):
            show_expression("scared")
            time.sleep(0.05)
            show_expression("scared_right")
            time.sleep(1)
            show_expression("scared")
            time.sleep(0.05)
            show_expression("scared_left")
            time.sleep(1)
        else:
            show_expression("normal")
            time.sleep(0.05)
            index = random.choice([i for i in range(len(normal_expressions) - 1) if i != index])
            show_expression(normal_expressions[index])
            time.sleep(0.8)