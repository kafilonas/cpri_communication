#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>


int receive(SOCKET socket, unsigned char *buffer, int size)
{
    return recv(socket, buffer, size, 0);
}


int transmit(SOCKET socket, unsigned char *buffer, int size)
{
    return send(socket, buffer, size, 0);
}


DWORD WINAPI transmit_thread(LPVOID arg)
{
    SOCKET socket = *(SOCKET *)arg;
    
    unsigned char buffer[1024];
    for (int i = 0; i < 1024; i++) {
        buffer[i] = i % 256;
    }
    
    int bytes_sent = transmit(socket, buffer, sizeof(buffer));
    
    printf("TX thread: Sent %d bytes\n", bytes_sent);
    
    return 0;
}


DWORD WINAPI receive_thread(LPVOID arg)
{
    SOCKET socket = *(SOCKET *)arg;
    
    unsigned char buffer[1024];
    unsigned char expected[1024];
    
    for (int i = 0; i < 1024; i++) {
        expected[i] = i % 256;
    }
    
    int bytes_received = receive(socket, buffer, sizeof(buffer));
    
    printf("RX thread: Received %d bytes\n", bytes_received);

    //Integrity Check
    if (bytes_received == 1024 && memcmp(buffer, expected, 1024) == 0)
    {
        printf("RX thread: Integrity: 100%%\n");
    }
    else
    {
        printf("RX thread: Integrity check failed!\n");
    }
    
    return 0;
}


int main(void)
{
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_address;

    // Initialize Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(5000);

    //Connect to server
    connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    printf("Connected to server!\n");


    HANDLE tx_thread;
    HANDLE rx_thread;
    tx_thread = CreateThread(NULL, 0, transmit_thread, &client_socket, 0, NULL);
    rx_thread = CreateThread(NULL, 0, receive_thread, &client_socket, 0, NULL);
    
    WaitForSingleObject(tx_thread, INFINITE);
    WaitForSingleObject(rx_thread, INFINITE);

    //Close socket
    closesocket(client_socket);
    WSACleanup();

    return 0;
}






