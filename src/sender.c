/**
 * @file sender.c
 *
 * @brief Function definitions for sending data to the receiver
 *
 * This file contains the function definitions for sending data to the receiver
 * and receiving ACKs from the receiver, establishing a connection with the
 * receiver, and closing the connection with the receiver.
 *
 * The main() function runs the sender and sends data to the receiver using a
 * UDP socket.
 *
 * The sender sends data to the receiver using a Go-back-n protocol. The sender
 * sends packets to the receiver and waits for ACKs from the receiver. If the
 * sender does not receive an ACK for a packet, it resends the packet and all
 * subsequent packets.
 *
 * @author Ritam Singal (ritamsingal)
 * @author Harshil Patel (harshil-patel11)
 * @bug No known bugs
 */

#include <arpa/inet.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <pthread.h>
#include <sys/time.h>

#include "../include/sender.h"
#include "../include/tcp_segment.h"
#include "../include/tcp_utils.h"
#include "../include/utils.h"

// https://www.educative.io/answers/how-to-implement-udp-sockets-in-c
void rsend(char *hostname, unsigned short int hostUDPport, char *filename,
           unsigned long long int bytesToTransfer) {

  unsigned long long int numBytesToTransfer = bytesToTransfer;
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    printf("Couldn't open file\n");
    return -1;
  }

  int socket_desc = create_socket();
  if (socket_desc < 0) {
    printf("Error while creating socket\n");
    fclose(file);
    return -1;
  }

  char *server_ip;
  struct sockaddr_in server_addr;

  printf("Hostname: %s\n", hostname);
  if (get_host_ip_by_hostname(&server_ip, hostname) < 0) {
    printf("Couldn't get server IP\n");
    close(socket_desc);
    fclose(file);
    return -1;
  }
  printf("Server IP: %s\n", server_ip);

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(hostUDPport);
  server_addr.sin_addr.s_addr = inet_addr(server_ip);

  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  if (getsockname(socket_desc, (struct sockaddr *)&client_addr,
                  &client_addr_len) < 0) {
    printf("Couldn't get socket name\n");
    close(socket_desc);
    fclose(file);
    return -1;
  }
  in_port_t client_port = ntohs(client_addr.sin_port);

  if (establish_connection_sender(client_port, hostUDPport, socket_desc,
                                  &server_addr, &client_addr) != SUCCESS) {
    printf("Couldn't establish connection\n");
    close(socket_desc);
    fclose(file);
    return -1;
  }

  uint32_t seq_number = 0;
  char buffer[SEGMENT_DATA_SIZE * 1024];
  char *buffer_ptr = buffer;
  size_t bytes_in_buffer = 0;
  int window_size = 1;

  while (1) {
    if (bytes_in_buffer == 0) {
      if (numBytesToTransfer == 0) {
        break;
      }

      memset(buffer, '\0', sizeof(buffer));
      bytes_in_buffer = fread(buffer, 1, sizeof(buffer), file);
      if (bytes_in_buffer == 0) {
        break;
      }
      if (bytes_in_buffer > numBytesToTransfer) {
        bytes_in_buffer = numBytesToTransfer;
        // set extra bytes read to NULL
        memset(buffer + bytes_in_buffer, '\0',
               sizeof(buffer) - bytes_in_buffer);
      }
      numBytesToTransfer -= bytes_in_buffer;
      buffer_ptr = buffer;
    }

    int num_packets_sent = 0;
    int send_and_recv_retval = send_and_recv_packets(
        socket_desc, hostUDPport, client_port, &server_addr, seq_number,
        buffer_ptr, bytes_in_buffer, window_size, &num_packets_sent);

    if (send_and_recv_retval != SUCCESS) {
      if (num_packets_sent == SEND_FAILED || num_packets_sent == RECV_FAILED ||
          num_packets_sent == UNKNOWN_FAILURE) {
        printf("Error while sending/receiving packets\n");
        fclose(file);
        close(socket_desc);
        return -1;
      }
    } else {
      if (num_packets_sent * SEGMENT_DATA_SIZE > bytes_in_buffer) {
        bytes_in_buffer = 0;
      } else {
        bytes_in_buffer -= num_packets_sent * SEGMENT_DATA_SIZE;
      }

      seq_number += num_packets_sent;
      buffer_ptr += num_packets_sent * SEGMENT_DATA_SIZE;
      if (num_packets_sent != window_size) {
        window_size = MAX(1, window_size / 2);
      } else if (window_size < MAX_WINDOW_SIZE) {
        window_size = MIN(MAX_WINDOW_SIZE, window_size + 2);
      }
    }
  }

  close_connection_sender(client_port, hostUDPport, socket_desc, &server_addr,
                          &client_addr);

  fclose(file);
  close(socket_desc);

  return 0;
}

tcp_error_t
send_and_recv_packets(int socket_desc, unsigned short int host_udp_port,
                      in_port_t client_port, struct sockaddr_in *server_addr,
                      uint32_t seq_number, char *buffer, size_t bytes_in_buffer,
                      int window_size, int *retval) {

  int num_packets =
      MIN(window_size, CEIL(bytes_in_buffer / (float)SEGMENT_DATA_SIZE));
  int seq_number_acks[num_packets];
  memset(seq_number_acks, 0, sizeof(seq_number_acks));

  // send packets
  for (int i = 0; i < num_packets; i++) {
    tcp_segment_t send_segment;
    create_tcp_segment(client_port, host_udp_port, seq_number + i, 0, 0, buffer,
                       SEGMENT_DATA_SIZE, &send_segment);
    int send_retval = send_tcp(socket_desc, &send_segment, server_addr);
    if (send_retval == SEND_FAILED) {
      printf("Couldn't send packet with seq number %d\n", seq_number + i);
      return SEND_FAILED;
    }
    buffer += SEGMENT_DATA_SIZE;
  }

  // receive acks
  for (int i = 0; i < num_packets; i++) {
    tcp_segment_t recv_segment;
    int recv_result =
        recv_tcp_with_timeout(socket_desc, server_addr, &recv_segment);
    if (recv_result == TIMEOUT) {
      break;
    } else if (recv_result == RECV_FAILED || recv_result == UNKNOWN_FAILURE) {
      return recv_result;
    } else if (recv_result == SUCCESS) {
      uint32_t local_seq_number = recv_segment.ack_number - seq_number;
      if (local_seq_number >= 0 && local_seq_number < num_packets) {
        seq_number_acks[local_seq_number] = 1;
      } else {
        i--;
      }
    }
  }

  // find lowest seq number packet that was acked
  int lowest_seq_number_packet = -1;

  for (int i = 0; i < num_packets; i++) {
    if (seq_number_acks[i] == 0) {
      break;
    } else {
      lowest_seq_number_packet = i;
    }
  }

  *retval = lowest_seq_number_packet + 1;
  return SUCCESS;
}

tcp_error_t establish_connection_sender(int client_port, int server_port,
                                        int socket_desc,
                                        struct sockaddr_in *server_addr,
                                        struct sockaddr_in *client_addr) {

  char *client_message = "establishing connection";
  tcp_segment_t send_segment;
  create_tcp_segment(client_port, server_port, 0, 0, SYN, client_message,
                     strlen(client_message), &send_segment);

  while (1) {
    int send_retval = send_tcp(socket_desc, &send_segment, server_addr);
    if (send_retval != SUCCESS) {
      return send_retval;
    }

    tcp_segment_t recv_segment;
    int recv_retval =
        recv_tcp_with_timeout(socket_desc, client_addr, &recv_segment);
    if (recv_retval == RECV_FAILED || recv_retval == UNKNOWN_FAILURE) {
      return recv_retval;
    } else if (recv_retval == SUCCESS && recv_segment.flags == (SYN | ACK)) {
      printf("Connection established\n");
      break;
    }
  }

  return SUCCESS;
}

tcp_error_t close_connection_sender(int client_port, int server_port,
                                    int socket_desc,
                                    struct sockaddr_in *server_addr,
                                    struct sockaddr_in *client_addr) {

  char *client_message = "close connection";
  tcp_segment_t send_segment;
  create_tcp_segment(client_port, server_port, 0, 0, FIN, client_message,
                     strlen(client_message), &send_segment);

  // retry closing connection upto 10 times before giving up
  for (int i = 0; i < 10; i++) {
    int send_retval = send_tcp(socket_desc, &send_segment, server_addr);
    if (send_retval != SUCCESS) {
      return send_retval;
    }

    tcp_segment_t recv_segment;
    int recv_retval =
        recv_tcp_with_timeout(socket_desc, client_addr, &recv_segment);
    if (recv_retval == RECV_FAILED || recv_retval == UNKNOWN_FAILURE) {
      return recv_retval;
    } else if (recv_retval == SUCCESS && recv_segment.flags == (FIN | ACK)) {
      printf("Connection closed\n");
      break;
    }
  }

  return SUCCESS;
}

int main(int argc, char **argv) {
  int host_udp_port;
  char *hostname = NULL;
  char *filename_to_xfer = NULL;
  unsigned long long int bytes_to_xfer;

  if (argc != 5) {
    fprintf(stderr,
            "usage: %s receiver_hostname receiver_port filename_to_xfer "
            "bytes_to_xfer\n\n",
            argv[0]);
    exit(1);
  }

  hostname = argv[1];
  host_udp_port = (unsigned short int)atoi(argv[2]);
  filename_to_xfer = argv[3];
  bytes_to_xfer = atoll(argv[4]);

  rsend(hostname, host_udp_port, filename_to_xfer, bytes_to_xfer);
  return (EXIT_SUCCESS);
}
