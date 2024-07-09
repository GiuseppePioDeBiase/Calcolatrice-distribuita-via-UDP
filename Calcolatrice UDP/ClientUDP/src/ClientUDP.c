/*
 ============================================================================
 Name        : ClientUDP.c
 Author      : Giuseppe Pio De Biase
 Author      : Giuseppe Pio De Biase
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Including necessary libraries based on the platform
#if defined _WIN32
#include <winsock.h> // Windows-specific header for socket operations
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define closesocket close // Definition for closing the socket on non-Windows systems
#endif

#include "Client.h" // Including the "Client.h" header file for client-related functions and definitions

// Function declarations
struct sockaddr_in bindSocket(struct sockaddr_in sad, char* server_ip, int port_number);
void inputString(char *msg);
void errorhandler(char *error_message);
void clearwinsock();

// Function to handle errors
void errorhandler(char *error_message) {
    printf("%s", error_message); // Print the error message
}

// Function to clean up Winsock resources on Windows platforms
void clearwinsock() {
#if defined WIN32
    WSACleanup(); // Clean up Winsock resources
#endif
}


int main(int argc, char *argv[]) {

	// Conditional for initializing Winsock on Windows platforms
	#if defined WIN32
	    // Initializing Winsock
	    WSADATA wsa_data;
	    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	    if (result != 0) {
	        errorhandler("Error during WSAStartup");

	    }
	#endif

	// Socket variable
	int c_socket = -1;
	// Create a UDP socket
	if ((c_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
	    errorhandler("Socket creation failed."); // Error message in case socket creation fails
	    closesocket(c_socket); // Close the socket
	    clearwinsock(); // Clean up Winsock resources
	    return -1; // Return an error code
	}

	// Variables for user input and host resolution
	char *H_input[2];
	struct hostent *host;
	struct in_addr *ina;
	unsigned portC;
	char *first;

	// Get input from the user or set default values
	if (argc > 1) {
	    first = strtok(argv[1], ":");
	    H_input[0] = first;
	    H_input[1] = strtok(NULL, "\0");
	} else {
	    H_input[0] = PROTO_ADDR; // Default IP address
	    H_input[1] = PROTOPORT; // Default port
	}

	// Resolve the host name to an IP address
	host = gethostbyname((char*) H_input[0]);
	if (host) {
	    ina = (struct in_addr*) host->h_addr_list[0]; // Get the resolved IP address
	    portC = strtol(H_input[1], NULL, 10); // Get the port as an integer
	    printf("Address resolved: %s:%d", inet_ntoa(*ina), (unsigned) portC); // Print the resolved IP address and port

	} else {
	    printf("Address not resolved."); // Print if the address cannot be resolved

	    closesocket(c_socket); // Close the socket
	    clearwinsock(); // Clean up Winsock resources
	}

	// Configure the sockaddr_in structure for the server
	struct sockaddr_in echoServAddr = bindSocket(echoServAddr, inet_ntoa(*ina), (int) portC);
	struct sockaddr_in fromAddr;


    // Loop for interaction with the server
    while (1) {
        // Clear the message buffer
        memset(msg, 0, BUFFERSIZE);

        // Request to enter commands to send to the server
        printf("\nEnter the commands to send to the server(ex. + 23 45): ");

        // Loop to handle user input and operations
        while (1) {
            // Read input from the user
            fgets(msg, BUFFERSIZE, stdin);

            // Variables for operator and numbers
            char operator;
            double num1, num2;

            // Check if the input matches the format for an arithmetic operation and if the operator is valid
            if (sscanf(msg, "%c %lf %lf", &operator, &num1, &num2) == 3 && strchr("+-*/", operator) != NULL) {
                // If valid, exit the loop to proceed with sending data
                break;
            } else if (sscanf(msg, "%c", &operator) == 1 && operator == '=') {
                // Check if the input indicates to close the connection
                printf("Closing the connection...\n");
                closesocket(c_socket);
                clearwinsock();
                exit(EXIT_SUCCESS);
            } else {
                // If the input format is invalid, show a message to the user and continue the loop
                printf("Invalid input format.\n");
                printf("Enter the commands to send to the server: ");
            }
        }

        // Get the length of the message to send
        int echoStringLen = strlen(msg);

        // Send the formatted message to the server
        if (sendto(c_socket, msg, echoStringLen, 0, (struct sockaddr*) &echoServAddr, sizeof(echoServAddr)) != echoStringLen) {
            errorhandler("sendto() sent a different number of bytes than expected.");
            break;
        }

        // Receive the response from the server
        unsigned int fromSize = sizeof(fromAddr);
        int respStringLen = recvfrom(c_socket, msg, BUFFERSIZE, 0, (struct sockaddr*) &fromAddr, &fromSize);

        // Check the source of the received packet
        if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
            errorhandler("Error: received a packet from an unknown source.");
            exit(EXIT_FAILURE);
        }

        // Terminate the received message with the string termination character
        msg[respStringLen] = '\0';

        // Get host details
        struct hostent *he;
        he = gethostbyaddr((char *)&fromAddr.sin_addr, sizeof(struct in_addr), AF_INET);

        // Show the received result and server information
        printf("\nReceived result from server %s, IP %s: %s", he->h_name, inet_ntoa(fromAddr.sin_addr), msg);

        // Check if the server sends a "Bye" message to close the loop
        char *byeString = "Bye";
        if (strcmp(msg, byeString) == 0) {
            break;
        }
    }

    // Close the socket and clean up Winsock resources before terminating the program
    closesocket(c_socket);
    clearwinsock();
    return 0;
}

// Function to configure and prepare a 'sockaddr_in' structure for socket binding
struct sockaddr_in bindSocket(struct sockaddr_in sad, char* server_ip, int port_number) {
    // Clear the memory of 'sad' structure
    memset(&sad, 0, sizeof(sad));

    // Set the address family to AF_INET (IPv4)
    sad.sin_family = AF_INET;

    // Assign the server IP address to the 'sin_addr' field using 'inet_addr'
    sad.sin_addr.s_addr = inet_addr(server_ip);

    // Set the port number in network byte order using 'htons'
    sad.sin_port = htons(port_number);

    // Return the configured 'sockaddr_in' structure
    return sad;
}


