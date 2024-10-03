import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64
import pandas as pd

class SensorReader(Node):
    def __init__(self):
        super().__init__('sensor_reader')

        # Publisher to publish the interpolated average as a float
        self.publisher_ = self.create_publisher(Float64, 'csv_average', 10)

	# Path to your CSV file
	self.csv_file_path = '/home/kwasi/Documents/force_sensor_data.csv'

	# Timer to periodically read the CSV file
	self.timer = self.create_timer(1.0, self.read_csv_and_publish) # Adjust the timer period as needed

	self.get_logger().info('Sensor Reader Node initialized and ready to read CSV file.')

    def read_csv_and_publish(self):
	try:
	    # Read the CSV file
	    df = pd.read_csv(self.csv_file_path)

	    # Get the last row in the dataframe
	    last_row = df.iloc[-1]

	    # Select columns C to I (using 0-based indexing: C is index 2, I is index 8)
	    values = last_row[2:9]

	    # Take the absolute value of each entry and calculate the mean
	    abs_values = values.abs()
	    avg_value = abs_values.mean()

	    # Interpolate the average from the range [8, 22] to the target range [100, 1240]
	    mapped_value = self.map_value(avg_value, 8, 22, 100, 1240)

	    # Create a message and publish the interpolated value
	    msg = Float64()
	    msg.data = mapped_value
	    self.publisher_.publish(msg)

	    self.get_logger().info(f'Published interpolated value: {mapped_value}')

	except Exception as e:
	    self.get_logger().error(f'Error reading CSV file: {e}')

    # Now a regular instance method
    def map_value(self, value, old_min, old_max, new_min, new_max):
	"""Maps a value from one range to another."""
	# Linear interpolation formula
	return new_min + ((value - old_min) / (old_max - old_min)) * (new_max - new_min)

def main(args=None):
    rclpy.init(args=args)
    sensor_reader_node = SensorReader()

    try:
	rclpy.spin(sensor_reader_node)
    except KeyboardInterrupt:
	csv_reader_node.get_logger().info('Node stopped by user.')
    finally:
	sensor_reader_node.destroy_node()
	rclpy.shutdown()

if __name__ == '__main__':
    main()
