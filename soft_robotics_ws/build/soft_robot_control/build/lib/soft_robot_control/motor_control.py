import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64

import pip 
pip.main(['install', 'pymata4'])
from pymata4 import pymata4

# Constants for the stepper motor
STEPS_PER_REV = 32  # Number of steps per revolution of the stepper motor
ARDUINO_PINS = [8, 9, 10, 11]  # Arduino pins connected to the stepper motor
ARDUINO_PORT = '/dev/ttyACM0'  # Port where the Arduino is connected

class MotorControl(Node):
    def __init__(self):
        super().__init__('motor_controller')

        # Initialize the pymata4 board with the correct port
        #self.board = pymata4.Pymata4(com_port=ARDUINO_PORT, baud_rate=57600)
        self.board = pymata4.Pymata4()
        self.get_logger().info(f"Connecting to Arduino on {ARDUINO_PORT}...")

        # Initialize the stepper motor
        self.setup_stepper(STEPS_PER_REV, ARDUINO_PINS)
        
        # Subscriber to the 'csv_average' topic
        self.subscription = self.create_subscription(
            Float64,
            'csv_average',
            self.csv_average_callback,
            10
        )
        self.get_logger().info("Stepper Control Node initialized and subscribed to 'csv_average'.")

    def setup_stepper(self, steps_per_rev, pins):
        """
        Configure the board for stepper motor control.
        """
        self.board.set_pin_mode_stepper(steps_per_rev, pins)
        self.get_logger().info("Stepper motor initialized.")
    
    def csv_average_callback(self, msg):
        """
        Callback function to handle incoming messages from the 'csv_average' topic.
        Uses the received speed value to control the stepper motor.
        """
        motor_speed = msg.data
        motor_speed = int(motor_speed)

        # Set the number of steps; positive for forward, negative for reverse
        steps = 32  # or any other value you want to rotate

        # Move the stepper motor
        self.control_stepper(motor_speed, steps)

    def control_stepper(self, motor_speed, steps):
        """
        Rotate the stepper motor.
        """
        self.board.stepper_write(motor_speed, steps)
        self.get_logger().info(f"Stepper motor moved {steps} steps at speed {motor_speed}.")

    def destroy(self):
        """
        Clean up and shut down the board.
        """
        self.board.shutdown()
        self.get_logger().info("Shutting down the board...")

def main(args=None):
    rclpy.init(args=args)
    stepper_node = MotorControl()

    try:
        rclpy.spin(stepper_node)
    except KeyboardInterrupt:
        stepper_node.get_logger().info('Node stopped by user.')
    finally:
        stepper_node.destroy()
        stepper_node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
