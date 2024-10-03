#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <csignal>   // For handling Ctrl+C to terminate the loop
#include <unistd.h>  // For sleep()

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

bool keepRunning = true;  // Flag to keep the loop running

// Signal handler for Ctrl+C
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received. Exiting program." << std::endl;
    keepRunning = false;
}

int main(int argc, char** argv) {
    int socketHandle;            // Handle to UDP socket used to communicate with Net F/T
    struct sockaddr_in addr;     // Address of Net F/T
    struct hostent* he;          // Host entry for Net F/T
    std::vector<byte> request(8);  // The request data sent to the Net F/T
    Response resp;               // The structured response received from the Net F/T
    std::vector<byte> response(36); // The raw response data received from the Net F/T
    std::string AXES[] = { "Fx", "Fy", "Fz", "Tx", "Ty", "Tz" };  // Names of force and torque axes

    // Catch Ctrl+C signal for graceful termination
    signal(SIGINT, signalHandler);

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " IPADDRESS" << std::endl;
        return -1;
    }

    // Create a socket
    socketHandle = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketHandle == -1) {
        std::cerr << "Error creating socket." << std::endl;
        exit(1);
    }

    // Prepare the request
    *reinterpret_cast<uint16*>(&request[0]) = htons(0x1234);    // Standard header
    *reinterpret_cast<uint16*>(&request[2]) = htons(COMMAND);   // Command code
    *reinterpret_cast<uint32*>(&request[4]) = htonl(NUM_SAMPLES); // Number of samples

    // Resolve the IP address
    he = gethostbyname(argv[1]);
    if (he == nullptr) {
        std::cerr << "Error resolving hostname." << std::endl;
        exit(2);
    }

    std::memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    // Connect to the server
    int err = connect(socketHandle, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    if (err == -1) {
        std::cerr << "Error connecting to the server." << std::endl;
        exit(3);
    }

    // Main loop for continuous data sending
    while (keepRunning) {
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

        // Output the response data
        std::cout << "Status: 0x" << std::hex << resp.status << std::dec << std::endl;
        for (int i = 0; i < 6; i++) {
            std::cout << AXES[i] << ": " << resp.FTData[i] << std::endl;
        }

        // Sleep for 1 second to control data sending frequency (adjust as needed)
        sleep(1);
    }

    // Close the socket before exiting
    close(socketHandle);

    return 0;
}

