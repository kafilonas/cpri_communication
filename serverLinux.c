#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>


int receive_data(int socket, unsigned char *buffer, int size)
{
    return recv(socket, buffer, size, 0);
}


int transmit(int socket, unsigned char *buffer, int size)
{
    return send(socket, buffer, size, 0);
}


void *transmit_thread(void *arg)
{
    int socket = *(int *)arg;

    unsigned char buffer[1024];

    for (int i = 0; i < 1024; i++) {
        buffer[i] = i % 256;
    }

    int bytes_sent = transmit(socket, buffer, sizeof(buffer));

    printf("TX thread: Sent %d bytes\n", bytes_sent);

    return NULL;
}


void *receive_thread(void *arg)
{
    int socket = *(int *)arg;

    unsigned char buffer[1024];
    unsigned char expected[1024];

    for (int i = 0; i < 1024; i++) {
        expected[i] = i % 256;
    }

    int bytes_received = receive_data(socket, buffer, sizeof(buffer));

    printf("RX thread: Received %d bytes\n", bytes_received);

    // Integrity Check
    if (bytes_received == 1024 &&
        memcmp(buffer, expected, 1024) == 0)
    {
        printf("RX thread: Integrity: 100%%\n");
    }
    else
    {
        printf("RX thread: Integrity check failed!\n");
    }

    return NULL;
}


int main(void)
{
    int server_socket;
    int client_socket;

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    socklen_t address_length = sizeof(client_address);


    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("socket");
        return 1;
    }


    // Configure server address
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(5000);


    // Bind socket to port 5000
    if (bind(
        server_socket,
        (struct sockaddr *)&server_address,
        sizeof(server_address)) < 0)
    {
        perror("bind");
        close(server_socket);
        return 1;
    }


    // Start listening
    if (listen(server_socket, 1) < 0) {
        perror("listen");
        close(server_socket);
        return 1;
    }

    printf("Waiting for client...\n");


    // Accept connection
    client_socket = accept(
        server_socket,
        (struct sockaddr *)&client_address,
        &address_length
    );

    if (client_socket < 0) {
        perror("accept");
        close(server_socket);
        return 1;
    }

    printf("Client connected!\n");


    // Create threads
    pthread_t tx_thread;
    pthread_t rx_thread;

    pthread_create(
        &tx_thread,
        NULL,
        transmit_thread,
        &client_socket
    );

    pthread_create(
        &rx_thread,
        NULL,
        receive_thread,
        &client_socket
    );


    // Wait for threads
    pthread_join(tx_thread, NULL);
    pthread_join(rx_thread, NULL);


    // Close sockets
    close(client_socket);
    close(server_socket);

    return 0;
}
