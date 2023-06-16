#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "quiche.h"
#include "message.pb-c.h"

#define MAX_DATAGRAM_SIZE 1350

int main() {
    // Create a UDP socket
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to a local address and port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // Create a QUIC configuration
    quiche_config *config = quiche_config_new(QUICHE_PROTOCOL_VERSION);

    // Load TLS certificate and private key
    if (quiche_config_load_cert_chain_from_pem_file(config, "cert.pem") != 0) {
        fprintf(stderr, "Failed to load TLS certificate\n");
        exit(EXIT_FAILURE);
    }

    if (quiche_config_load_priv_key_from_pem_file(config, "key.pem") != 0) {
        fprintf(stderr, "Failed to load TLS private key\n");
        exit(EXIT_FAILURE);
    }

    // Accept QUIC connections
    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        uint8_t buf[MAX_DATAGRAM_SIZE];
        ssize_t read = recvfrom(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (read < 0) {
            perror("Failed to receive packet");
            exit(EXIT_FAILURE);
        }

        // Create a QUIC connection
        quiche_conn *conn = quiche_accept(NULL, buf, read, config);
        if (conn == NULL) {
            fprintf(stderr, "Failed to create connection\n");
            exit(EXIT_FAILURE);
        }

        // Process the QUIC connection
        bool done = false;
        while (!done) {
            uint8_t out[MAX_DATAGRAM_SIZE];
            ssize_t written = quiche_conn_send(conn, out, sizeof(out));
            if (written < 0) {
                fprintf(stderr, "Failed to send QUIC packet\n");
                exit(EXIT_FAILURE);
            }

            // Send the QUIC packet to the client
            ssize_t sent = sendto(socket_fd, out, written, 0, (struct sockaddr *)&client_addr, client_addr_len);
            if (sent < 0) {
                perror("Failed to send packet");
                exit(EXIT_FAILURE);
            }

            // Receive QUIC packets from the client
            ssize_t received = recvfrom(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_addr_len);
            if (received < 0) {
                perror("Failed to receive packet");
                exit(EXIT_FAILURE);
            }

            // Process the received QUIC packet
            quiche_recv(conn, buf, received);

            // Check if the QUIC connection is complete
            if (quiche_conn_is_established(conn)) {
                // Create a MyMessage instance
                MyMessage message = MY_MESSAGE__INIT;
                message.key = "example_key";
                message.value.len = 10; // Size of the value byte array
                message.value.data = (uint8_t *)malloc(10 * sizeof(uint8_t));
                memcpy(message.value.data, "example", 10);

                // Get the serialized message size
                size_t serialized_size = my_message__get_packed_size(&message);

                // Allocate memory for the serialized message
                uint8_t *buffer = (uint8_t *)malloc(serialized_size);

                // Serialize the message
                my_message__pack(&message, buffer);

                // Send the serialized message over the QUIC connection
                ssize_t sent_bytes = quiche_conn_stream_send(conn, 0, buffer, serialized_size, true);
                if (sent_bytes < 0) {
                    fprintf(stderr, "Failed to send message over QUIC\n");
                    exit(EXIT_FAILURE);
                }

                // Free allocated memory
                free(message.value.data);
                free(buffer);

                done = true; // Set `done` to true to exit the loop after sending the message
            }
        }

        // Free the QUIC connection
        quiche_conn_free(conn);
    }

    // Free the QUIC configuration
    quiche_config_free(config);

    return 0;
}
