/**
 * @file receiver.c
 *
 * @brief Function definitions for receiving data from the sender
 *
 * This file contains the function definitions for receiving data from the
 * sender and writing it to a file, flushing data to file, sending an ACK to the
 * sender, establishing a connection with the sender, and closing the connection
 * with the sender.
 *
 * The main() function runs the receiver and listens for incoming packets from
 * the sender. It then writes the data to a file.
 *
 * @author Ritam Singal (ritamsingal)
 * @author Harshil Patel (harshil-patel11)
 * @bug No known bugs
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <pthread.h>

#include "../include/receiver.h"
#include "../include/tcp_segment.h"
#include "../include/tcp_utils.h"
#include "../include/utils.h"

// https://www.educative.io/answers/how-to-implement-udp-sockets-in-c
void rrecv(unsigned short int myUDPport, char *destinationFile,
           unsigned long long int writeRate) {

  char *ip;
  if (get_host_ip(&ip) < 0) {
    printf("Error while getting host IP\n");
    return -1;
  }
  printf("Host IP: %s\n", ip);

  FILE *output_file = fopen(destinationFile, "w");
  if (output_file == NULL) {
    printf("Couldn't open file\n");
    return -1;
  }

  int socket_desc = create_socket();
  if (socket_desc < 0) {
    printf("Error while creating socket\n");
    fclose(output_file);
    return -1;
  }

  struct sockaddr_in server_addr;
  if (bind_socket(socket_desc, &server_addr, myUDPport, ip) < 0) {
    printf("Unable to bind socket\n");
    close(socket_desc);
    fclose(output_file);
    return -1;
  }
  printf("Done with binding socket address to socket descriptor\n");

  uint32_t last_flushed_seq = 0;
  int file_buffer_seq[MAX_WINDOW_SIZE];
  memset(file_buffer_seq, 0, sizeof(file_buffer_seq));
  char file_buffer[SEGMENT_DATA_SIZE * MAX_WINDOW_SIZE];
  memset(file_buffer, '\0', sizeof(file_buffer));

  while (1) {
    tcp_segment_t client_segment;
    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);

    int recv_retval = recv_tcp(socket_desc, &client_addr, &client_segment);
    if (recv_retval == RECV_FAILED) {
      printf("Unable to receive packet\n");
      close(socket_desc);
      fclose(output_file);
      return -1;
    }

    if (client_segment.flags == SYN) {
      printf("Received SYN\n");
      if (establish_connection_receiver(socket_desc, &client_addr) != SUCCESS) {
        printf("Unable to send SYN-ACK\n");
        close(socket_desc);
        fclose(output_file);
        return -1;
      }
    } else if (client_segment.flags == FIN) {
      printf("Received FIN\n");
      if (close_connection_receiver(socket_desc, &client_addr) != SUCCESS) {
        printf("Unable to send FIN-ACK\n");
        close(socket_desc);
        fclose(output_file);
        return -1;
      }
      flush_packets_to_file(output_file, file_buffer, file_buffer_seq);
      break;
    } else {
      int flush = process_data(socket_desc, &client_segment, &client_addr,
                               file_buffer, file_buffer_seq, last_flushed_seq);
      if (flush == 1) {
        int num_packets_flushed =
            flush_packets_to_file(output_file, file_buffer, file_buffer_seq);
        last_flushed_seq += num_packets_flushed;
      }

      // send ACK only if we can buffer the data
      if (client_segment.seq_number < last_flushed_seq + MAX_WINDOW_SIZE) {
        if (send_ack(socket_desc, &client_segment, &client_addr) != SUCCESS) {
          printf("Unable to send ACK\n");
          close(socket_desc);
          fclose(output_file);
          return -1;
        }
      }
    }
  }

  close(socket_desc);
  fclose(output_file);
  return 0;
}

int process_data(int socket_desc, tcp_segment_t *client_segment,
                 struct sockaddr_in *client_addr, char *file_buffer,
                 int *file_buffer_seq, uint32_t last_flushed_seq) {

  if (client_segment->seq_number < last_flushed_seq + MAX_WINDOW_SIZE &&
      client_segment->seq_number >= last_flushed_seq) {
    memcpy(file_buffer + SEGMENT_DATA_SIZE *
                             (client_segment->seq_number - last_flushed_seq),
           client_segment->data, SEGMENT_DATA_SIZE);

    file_buffer_seq[client_segment->seq_number - last_flushed_seq] = 1;
  }

  // check if we can flush to file
  for (int i = 0; i < MAX_WINDOW_SIZE; i++) {
    if (file_buffer_seq[i] == 0) {
      return 0;
    }
  }

  return 1;
}

int flush_packets_to_file(FILE *file, char *file_buffer, int *file_buffer_seq) {
  int num_packets_to_flush = 0;
  for (int i = 0; i < MAX_WINDOW_SIZE; i++) {
    if (file_buffer_seq[i] == 0) {
      break;
    }
    num_packets_to_flush++;
  }

  int trailing_nulls = 0;
  for (int i = SEGMENT_DATA_SIZE - 1; i >= 0; i--) {
    if (file_buffer[(num_packets_to_flush - 1) * SEGMENT_DATA_SIZE + i] ==
        '\0') {
      trailing_nulls++;
    } else {
      break;
    }
  }

  size_t bytes_written =
      fwrite(file_buffer, 1,
             num_packets_to_flush * SEGMENT_DATA_SIZE - trailing_nulls, file);

  memset(file_buffer_seq, 0, sizeof(int) * MAX_WINDOW_SIZE);
  memset(file_buffer, '\0', sizeof(char) * SEGMENT_DATA_SIZE * MAX_WINDOW_SIZE);

  return num_packets_to_flush;
}

tcp_error_t send_ack(int socket_desc, tcp_segment_t *client_segment,
                     struct sockaddr_in *client_addr) {

  tcp_segment_t send_segment;
  char server_message[10];
  sprintf(server_message, "ACK: %d", client_segment->seq_number);
  create_tcp_segment(ntohs(client_addr->sin_port), ntohs(client_addr->sin_port),
                     0, client_segment->seq_number, ACK, server_message,
                     strlen(server_message), &send_segment);

  return send_tcp(socket_desc, &send_segment, client_addr);
}

tcp_error_t establish_connection_receiver(int socket_desc,
                                          struct sockaddr_in *client_addr) {

  tcp_segment_t send_segment;
  char *server_message = "SYN-ACK";
  create_tcp_segment(ntohs(client_addr->sin_port), ntohs(client_addr->sin_port),
                     0, 0, SYN | ACK, server_message, strlen(server_message),
                     &send_segment);

  printf("Receiver sending SYN-ACK\n");
  return send_tcp(socket_desc, &send_segment, client_addr);
}

tcp_error_t close_connection_receiver(int socket_desc,
                                      struct sockaddr_in *client_addr) {

  tcp_segment_t send_segment;
  char *server_message = "FIN-ACK";
  create_tcp_segment(ntohs(client_addr->sin_port), ntohs(client_addr->sin_port),
                     0, 0, FIN | ACK, server_message, strlen(server_message),
                     &send_segment);

  printf("Receiver sending FIN-ACK\n");
  return send_tcp(socket_desc, &send_segment, client_addr);
}

int main(int argc, char **argv) {
  unsigned short int udp_port;
  char *filename_to_write = NULL;

  if (argc != 3) {
    fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
    exit(1);
  }

  udp_port = (unsigned short int)atoi(argv[1]);
  filename_to_write = argv[2];

  rrecv(udp_port, filename_to_write, 0);
  return (EXIT_SUCCESS);
}
