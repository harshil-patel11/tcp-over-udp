#ifndef SENDER_H
#define SENDER_H

/**
 * @file sender.h
 *
 * @brief Function prototypes for the sender
 *
 * This header file contains the function prototypes for sending a file to a
 * receiver, sending packets to the receiver and receiving ACKs, establishing a
 * connection with the receiver, and closing the connection with the receiver.
 *
 * @author Ritam Singal (ritamsingal)
 * @author Harshil Patel (harshil-patel11)
 * @bug No known bugs
 */

#include "tcp_segment.h"
#include "tcp_utils.h"

/**
 * @brief Send a file to a receiver
 *
 * Send a file to a receiver given the hostname, hostUDPport, filename and
 * bytesToTransfer.
 * Transfers the first bytesToTransfer bytes of filename to the receiver at
 * hostname:hostUDPport.
 *
 * @param hostname The hostname of the receiver
 * @param host_udp_port The UDP port of the receiver
 * @param filename The name of the file to send
 * @param bytesToTransfer The number of bytes to send
 */
void rsend(char *hostname, unsigned short int hostUDPport, char *filename,
           unsigned long long int bytesToTransfer);

/**
 * @brief send packets to the receiver and receive ACKs
 *
 * Send packets to the receiver and receive ACKs from the receiver for those
 * packets. Ensures reliable data transfer of the packets. The function returns
 * the number of packets sent successfully based on the Go-back-n calculation on
 * unsuccessful packet delivery.
 *
 * @param socket_desc The socket descriptor
 * @param host_udp_port The UDP port of the receiver
 * @param client_port The port of the sender
 * @param server_addr The address of the receiver
 * @param seq_number The sequence number base value of the packets to send
 * @param buffer The buffer containing the dat to send
 * @param bytes_in_buffer The number of bytes in the buffer that are to be sent
 * @param window_size The window size of the TCP connection
 * @param retval A pointer where the function stores the return value. The
 * return value is the number of packets sent successfully
 * @return tcp_error_t
 */
tcp_error_t
send_and_recv_packets(int socket_desc, unsigned short int host_udp_port,
                      in_port_t client_port, struct sockaddr_in *server_addr,
                      uint32_t seq_number, char *buffer, size_t bytes_in_buffer,
                      int window_size, int *retval);

/**
 * @brief establish a connection with the receiver
 *
 * Establish a reliable connection with the receiver in order to transfer data.
 *
 * @param client_port The port of the sender
 * @param server_port The port of the receiver
 * @param socket_desc The socket descriptor
 * @param server_addr The address of the receiver
 * @param client_addr The address of the sender
 * @return tcp_error_t
 */
tcp_error_t establish_connection_sender(int client_port, int server_port,
                                        int socket_desc,
                                        struct sockaddr_in *server_addr,
                                        struct sockaddr_in *client_addr);

/**
 * @brief close the connection with the receiver
 *
 * Close the connection with the receiver.
 *
 * @param client_port The port of the sender
 * @param server_port The port of the receiver
 * @param socket_desc The socket descriptor
 * @param server_addr The address of the receiver
 * @param client_addr The address of the sender
 * @return tcp_error_t
 */
tcp_error_t close_connection_sender(int client_port, int server_port,
                                    int socket_desc,
                                    struct sockaddr_in *server_addr,
                                    struct sockaddr_in *client_addr);

#endif