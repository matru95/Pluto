from threading import Thread, Timer
import requests
import time
import asyncio
import logging
import datetime
import json
import os
import serial
import led_matrix


# Main Loop devoted to play a song continuously.
def run():
    os.system("play assets/epilogue.mp3")
    while(True):
        time.sleep(20)
