import time
from pyfirmata import Arduino, util
import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64

# Replace 'COM3' with your actual Arduino port
ARDUINO_PORT = '/dev/ttyUSB0' # Example: 'COM3' for Windows, '/dev/ttyUSB0' for Linux

# Define the number of steps per revolution for your motor
STEPS = 32

class MotorControl(Node):
    def __init__(self):
	super().__init__('motor_control')

	# Initialize the Arduino board
	self.board = Arduino(ARDUINO_PORT)
	self.get_logger().info(f"Connected to Arduino on port {ARDUINO_PORT}")

	# Define the stepper motor control pins
	self.stepper_pins = [8, 10, 9, 11] # Pins used to control the stepper motor

	# Set up the stepper motor control pins as outputs
	for pin in self.stepper_pins:
	self.board.digital[pin].mode = 1 # Set mode to OUTPUT

	# Initialize the iterator to read from the board
	self.iterator = util.Iterator(self.board)
	self.iterator.start()

	# Set the default speed to 100 RPM as in the Arduino setup
	self.current_speed = 100
	self.set_speed(self.current_speed)

	# Subscriber to receive the speed value
	self.subscription = self.create_subscription(
	Float64,
	'stepper_speed',
	self.speed_callback,
	10
	)

	self.get_logger().info('StepperMotorController node initialized, listening for speed commands.')

    def set_speed(self, rpm):
	"""
	Function to set the speed of the stepper motor.
	Speed is defined in RPM (revolutions per minute).
	"""
	delay = 60.0 / (STEPS * rpm) # Calculate delay based on RPM and steps per revolution
	self.get_logger().info(f"Setting stepper motor speed to: {rpm} RPM with delay {delay:.5f} seconds per step.")
	self.current_speed = rpm
	return delay

    def speed_callback(self, msg):
	"""
	Callback function to handle received speed commands.
	"""
	rpm = msg.data
	self.get_logger().info(f"Received speed command: {rpm} RPM")

	# Set the speed of the stepper motor
	delay = self.set_speed(rpm)

	# Implement the logic to control the motor steps based on the calculated delay
	self.step_motor(delay)

    def step_motor(self, delay):
	"""
	Function to step the motor with a delay.
	"""
	while True:
	    # The stepper motor moves one step
	for pin in self.stepper_pins:
	    self.board.digital[pin].write(1)
	    time.sleep(delay)
	    self.board.digital[pin].write(0)
	    time.sleep(delay)

    def destroy(self):
	# Clean up the board connection when the node is destroyed
	self.board.exit()
	super().destroy()


def main(args=None):
    rclpy.init(args=args)
    motor_controller = MotorControl()
    try:
        rclpy.spin(motor_controller)
    except KeyboardInterrupt:
        motor_controller.get_logger().info('Keyboard Interrupt detected! Stopping node.')
    finally:
        motor_controller.destroy()
        rclpy.shutdown()

if __name__ == '__main__':
main()
