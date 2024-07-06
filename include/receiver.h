#ifndef RECEIVER_H
#define RECEIVER_H

/**
 * @file receiver.h
 *
 * @brief Function prototypes for the receiver
 *
 * This header file contains the function prototypes for receiving a file from a
 * sender, flushing data to file, sending an ACK to the sender, establishing a
 * connection with the sender, and closing the connection with the sender.
 *
 * @author Ritam Singal (ritamsingal)
 * @author Harshil Patel (harshil-patel11)
 * @bug No known bugs
 */

#include <stdio.h>
#include <stdlib.h>

#include "tcp_segment.h"
#include "tcp_utils.h"

/**
 * @brief Receive a file from a sender
 *
 * Receive a file from a sender given the UDP port, destination file and
 * writeRate.
 *
 * @param myUDPport The UDP port of the receiver
 * @param destinationFile The name of the file to write
 * @param writeRate The rate at which to write the file
 */
void rrecv(unsigned short int myUDPport, char *destinationFile,
           unsigned long long int writeRate);

/**
 * @brief flush data to file
 *
 * Flush data to file based on the file buffer and file buffer sequence
 *
 * @param file The file to write to
 * @param file_buffer The buffer to write to the file
 * @param file_buffer_seq The sequence of the file buffer. This is used to mask
 * the file buffer which is split into multiple segments of SEGMENT_DATA_SIZE
 * size. Only the segments marked with 1 in the file_buffer_seq are written to
 * the file.
 * @return int number of packets (only the data) flushed to file
 */
int flush_packets_to_file(FILE *file, char *file_buffer, int *file_buffer_seq);

/**
 * @brief Send an ACK to sender
 *
 * Send a TCP ACK to the sender.
 *
 * @param socket_desc The socket descriptor
 * @param client_segment The segment received from the client
 * @param client_addr The address of the client
 * @return tcp_error_t
 */
tcp_error_t send_ack(int socket_desc, tcp_segment_t *client_segment,
                     struct sockaddr_in *client_addr);

/**
 * @brief establish a connection with the sender
 *
 * Establish a reliable connection with the sender in order to receive data.
 *
 * @param socket_desc The socket descriptor
 * @param client_addr The address of the sender
 * @return tcp_error_t
 */
tcp_error_t establish_connection_receiver(int socket_desc,
                                          struct sockaddr_in *client_addr);

/**
 * @brief close the connection with the sender
 *
 * Close the connection with the sender.
 *
 * @param socket_desc The socket descriptor
 * @param client_addr The address of the sender
 * @return tcp_error_t
 */
tcp_error_t close_connection_receiver(int socket_desc,
                                      struct sockaddr_in *client_addr);

#endif