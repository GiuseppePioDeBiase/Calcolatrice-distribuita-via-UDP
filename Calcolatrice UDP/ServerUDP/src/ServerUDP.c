/*
 ============================================================================
 Name        : ServerUDP.c
 Author      : Giuseppe Pio De Biase
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Server.h"

#if defined WIN32
#include <winsock.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define closesocket close
#endif

// Function declarations for arithmetic operations
void calculateResult(char *msg);
double add(double num1, double num2);
double mult(double num1, double num2);
double sub(double num1, double num2);
double division(double num1, double num2);

// Function to clean up Winsock resources on Windows platforms
void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

// Function to handle errors
void errorhandler(char *errorMessage) {
    printf("%s", errorMessage);
}

// Arithmetic operations functions
double add(double a, double b) {
    return a + b;
}
double mult(double a, double b) {
    return a * b;
}
double sub(double a, double b) {
    return a - b;
}
double division(double a, double b) {
    if (b != 0) {
        return a / b;
    } else {
        return 0.0; // Error handling: Division by zero
    }
}

int main(int argc, char *argv[]) {
    // Windows-specific: Initializing Winsock
    #if defined WIN32
        WSADATA wsa_data;
        int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (result != 0) {
            errorhandler("Error during WSAStartup");
            return 1;
        }
    #endif

    int my_socket;

    /* create a UDP socket */
    if ((my_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        errorhandler("Error creating socket");
        clearwinsock();
        return EXIT_FAILURE;
    }

    // Bind the socket
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr(PROTO_ADDR);
    sad.sin_port = htons(PROTOPORT);

    if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
        errorhandler("bind() failed.");
        closesocket(my_socket);
        clearwinsock();
        return EXIT_FAILURE;
    }

    struct sockaddr_in clientAddr; // Structure for the client's address

    printf("Searching for a client...");

    while(1) {
        // Receive and process data from the client until the client sends "="
        socklen_t client_len = sizeof(clientAddr); // Set the client's size
        memset(msg, 0, BUFFERSIZE); // clean msg

        int BT_RCV = recvfrom(my_socket, msg, BUFFERSIZE, 0, (struct sockaddr*) & clientAddr, &client_len);

        if (BT_RCV <= 0) {
            if (BT_RCV == 0) {
                printf("Client has closed the connection.");
            } else {
                errorhandler("recvfrom() failed or connection closed prematurely");
            }
        } else {
            struct hostent *he;
            he = gethostbyaddr((char *)&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET);

            printf("\nRequest operation '%s' from client %s, IP %s", msg, he->h_name, inet_ntoa(clientAddr.sin_addr));
            calculateResult(msg);

            // Send processed data back to the client
            if (sendto(my_socket, msg, sizeof(msg), 0, (struct sockaddr*) &clientAddr, client_len) != sizeof(msg)) {
                errorhandler("sendto() sent a different number of bytes than expected");
                closesocket(my_socket);
                clearwinsock();
                return 0;
            }
        }
    }
}

// Function to create a socket
int createSocket(int my_socket) {
    if ((my_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        errorhandler("socket() failed.");
        clearwinsock();
        return -1;
    }
    return my_socket;
}

// Function to calculate the result of arithmetic operations
void calculateResult(char *msg) {
    // Extract operator and operands from the input string
    char operator = msg[0];

    // Check if the operator is '=' to terminate the communication
    if (operator == '=') {
        return;
    }

    int numOperands = 0;
    double operands[2];

    // Tokenize the input string and collect the operands
    char *token = strtok(msg + 2, " ");
    while (token != NULL && numOperands < 2) {
        if (sscanf(token, "%lf", &operands[numOperands]) != 1) {
            // Error handling: Invalid operand format
            printf("Invalid operand format: %s", token);
            return; // Or handle the error as necessary
        }
        numOperands++;
        token = strtok(NULL, " ");
    }

    double result = operands[0];
    for (int i = 1; i < numOperands; i++) {
        switch (operator) {
            case '+':
                result = add(result, operands[i]);
                break;
            case '*':
                result = mult(result, operands[i]);
                break;
            case '-':
                result = sub(result, operands[i]);
                break;
            case '/':
                if (operands[i] != 0) {
                    result = division(result, operands[i]);
                } else {
                    // Error handling: Division by zero
                    char *divisionError = "|Error| - Division by zero";
                    strcpy(msg, divisionError);
                    return;
                }
                break;
            default:
                // Error handling: Unknown operator
                printf("Unknown operator: %c", operator);
                return;
        }
    }

    // Convert the result to a string and update the input string
    snprintf(msg, BUFFERSIZE, "%.2f %c %.2f = %.2f", operands[0], operator, operands[1], result);
}
