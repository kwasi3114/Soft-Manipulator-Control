// ROS node to communicate with Net F/T and publish the average of the data array [Fx, Fy, Fz, Tx, Ty, Tz]

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <cmath>  // For abs()

#define PORT 49152      // Port the Net F/T always uses
#define COMMAND 2       // Command code 2 starts streaming
#define NUM_SAMPLES 1   // Will send 1 sample before stopping

// Typedefs used so integer sizes are more explicit
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char byte;

struct Response {
    uint32 rdt_sequence;
    uint32 ft_sequence;
    uint32 status;
    int32 FTData[6];
};

double calculate_average_abs(const int32 FTData[6]) {
    double sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += std::abs(FTData[i]);  // Calculate absolute value
    }
    return sum / 6.0;  // Return average
}

int main(int argc, char** argv) {
    // Initialize ROS node
    ros::init(argc, argv, "netft_average_publisher");
    ros::NodeHandle nh;
    
    // ROS publisher
    ros::Publisher avg_pub = nh.advertise<std_msgs::Float64>("netft_average", 10);

    int socketHandle;            // Handle to UDP socket used to communicate with Net F/T
    struct sockaddr_in addr;     // Address of Net F/T
    struct hostent* he;          // Host entry for Net F/T
    std::vector<byte> request(8);  // The request data sent to the Net F/T
    Response resp;               // The structured response received from the Net F/T
    std::vector<byte> response(36); // The raw response data received from the Net F/T

    // Check if IP address argument is provided
    if (argc < 2) {
        ROS_ERROR("Usage: %s IPADDRESS", argv[0]);
        return -1;
    }

    // Create a socket
    socketHandle = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketHandle == -1) {
        ROS_ERROR("Error creating socket.");
        exit(1);
    }

    // Prepare the request
    *reinterpret_cast<uint16*>(&request[0]) = htons(0x1234);    // Standard header
    *reinterpret_cast<uint16*>(&request[2]) = htons(COMMAND);   // Command code
    *reinterpret_cast<uint32*>(&request[4]) = htonl(NUM_SAMPLES); // Number of samples

    // Resolve the IP address
    he = gethostbyname(argv[1]);
    if (he == nullptr) {
        ROS_ERROR("Error resolving hostname.");
        exit(2);
    }

    std::memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    // Connect to the server
    int err = connect(socketHandle, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    if (err == -1) {
        ROS_ERROR("Error connecting to the server.");
        exit(3);
    }

    ros::Rate loop_rate(10);  // Set a 10 Hz loop rate

    while (ros::ok()) {
        // Send the request
        send(socketHandle, request.data(), request.size(), 0);

        // Receive the response
        recv(socketHandle, response.data(), response.size(), 0);
        resp.rdt_sequence = ntohl(*reinterpret_cast<uint32*>(&response[0]));
        resp.ft_sequence = ntohl(*reinterpret_cast<uint32*>(&response[4]));
        resp.status = ntohl(*reinterpret_cast<uint32*>(&response[8]));
        for (int i = 0; i < 6; i++) {
            resp.FTData[i] = ntohl(*reinterpret_cast<int32*>(&response[12 + i * 4]));
        }

        // Calculate the average of absolute values
        double avg_abs = calculate_average_abs(resp.FTData);

        // Create ROS message
        std_msgs::Float64 msg;
        msg.data = avg_abs;

        // Publish the message
        avg_pub.publish(msg);

        // Log the average for debugging purposes
        ROS_INFO("Published average of absolute FTData: %f", avg_abs);

        // Spin and sleep for the next loop iteration
        ros::spinOnce();
        loop_rate.sleep();
    }

    return 0;
}
