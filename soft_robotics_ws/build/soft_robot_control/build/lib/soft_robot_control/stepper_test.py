import sys
import time
import pip 
pip.main(['install', 'pymata4'])
from pymata4 import pymata4

"""
This example demonstrates running a stepper motor
"""
NUM_STEPS = 8
ARDUINO_PINS = [8, 9, 10, 11]


def stepper(my_board, steps_per_rev, pins):
    """
    Set the motor control control pins to stepper mode.
    Rotate the motor.

    :param my_board: pymata4
    :param steps_per_rev: Number of steps per motor revolution
    :param pins: A list of the motor control pins
    """

    my_board.set_pin_mode_stepper(steps_per_rev, pins)
    time.sleep(1)
    my_board.stepper_write(500, 500)


board = pymata4.Pymata4()
try:
    stepper(board, NUM_STEPS, ARDUINO_PINS)
    board.shutdown()
except KeyboardInterrupt:
    board.shutdown()
    sys.exit(0)
