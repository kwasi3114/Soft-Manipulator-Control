#include "rclcpp/rclcpp.hpp"  // ROS 2 core
#include "std_msgs/msg/float64.hpp"  // ROS 2 message type for publishing the average
#include <chrono>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>

// Create a class that inherits from rclcpp::Node
class ForceSensorDataReader : public rclcpp::Node
{
public:
    ForceSensorDataReader()
    : Node("force_sensor_data_reader")  // Initialize the node with the name
    {
        // Publisher for the sensor average
        average_pub_ = this->create_publisher<std_msgs::msg::Float64>("sensor_average", 10);

        // Set up a timer to call the sensor data publishing function at 1 Hz
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&ForceSensorDataReader::publish_sensor_average, this)
        );

        // Seed the random number generator
        std::srand(std::time(nullptr));
    }

private:
    void publish_sensor_average()
    {
        // Simulated force/torque values: Fx, Fy, Fz, Tx, Ty, Tz
        std::vector<double> sensor_data(6);

        for (auto &value : sensor_data)
        {
            value = (std::rand() % 200 - 100) / 10.0;  // Random value between -10.0 and +10.0
        }

        // Take the absolute values and compute the average
        double sum = 0.0;
        for (const auto &value : sensor_data)
        {
            sum += std::abs(value);
        }
        double average = sum / sensor_data.size();

        // Create a message and publish the average
        auto avg_msg = std_msgs::msg::Float64();
        avg_msg.data = average;
        RCLCPP_INFO(this->get_logger(), "Publishing sensor average: %.2f", average);
        average_pub_->publish(avg_msg);
    }

    // ROS 2 publisher and timer
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr average_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[])
{
    // Initialize ROS 2
    rclcpp::init(argc, argv);

    // Create and spin the node
    rclcpp::spin(std::make_shared<ForceSensorDataReader>());

    // Shutdown ROS 2
    rclcpp::shutdown();
    return 0;
}

