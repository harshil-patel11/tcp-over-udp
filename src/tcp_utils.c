/**
 * @file tcp_utils.c
 * @brief Function definitions for creating, sending and receiving TCP segments
 *
 * This file contains the function definitions for creating a socket, sending a
 * TCP segment, and receiving a TCP segment
 *
 * @author Ritam Singal (ritamsingal)
 * @author Harshil Patel (harshil-patel11)
 * @bug No known bugs
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "../include/tcp_segment.h"
#include "../include/tcp_utils.h"

int create_socket() {
  int socket_desc;
  socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  return socket_desc;
}

int bind_socket(int socket_desc, struct sockaddr_in *server_addr,
                unsigned short int udp_port, char *ip_addr) {
  server_addr->sin_family = AF_INET;
  server_addr->sin_port = htons(udp_port);
  server_addr->sin_addr.s_addr = inet_addr(ip_addr);

  if (bind(socket_desc, (struct sockaddr *)server_addr, sizeof(*server_addr)) <
      0) {

    return -1;
  }
  return 0;
}

// https://www.tutorialspoint.com/how-to-get-the-ip-address-of-local-computer-using-c-cplusplus
int get_host_ip(char **ip) {
  char host[256];
  struct hostent *host_entry;
  int hostname;

  // To retrieve hostname
  hostname = gethostname(host, sizeof(host));
  if (hostname == -1) {
    return -1;
  }

  // To retrieve host information
  host_entry = gethostbyname(host);
  if (host_entry == NULL) {
    return -1;
  }

  // To convert an Internet network
  // address into ASCII string
  *ip = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
  return 0;
}

int get_host_ip_by_hostname(char **ip, char *host) {
  struct hostent *host_entry;
  int hostname;

  // To retrieve host information
  host_entry = gethostbyname(host);
  if (host_entry == NULL) {
    return -1;
  }

  // To convert an Internet network
  // address into ASCII string
  *ip = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
  return 0;
}

tcp_error_t send_tcp(int socket_desc, tcp_segment_t *send_segment,
                     struct sockaddr_in *server_addr) {

  if (sendto(socket_desc, send_segment, sizeof(*send_segment), 0,
             (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
    return SEND_FAILED;
  }
  return SUCCESS;
}

tcp_error_t recv_tcp(int socket_desc, struct sockaddr_in *client_addr,
                     tcp_segment_t *recv_segment) {
  size_t client_addr_len = sizeof(*client_addr);
  if (recvfrom(socket_desc, recv_segment, sizeof(*recv_segment), 0,
               (struct sockaddr *)client_addr, &client_addr_len) < 0) {
    return RECV_FAILED;
  }
  if (compare_checksum(recv_segment) == 0) {
    return CHECKSUM_FAILED;
  }
  return SUCCESS;
}

tcp_error_t recv_tcp_with_timeout(int socket_desc,
                                  struct sockaddr_in *client_addr,
                                  tcp_segment_t *recv_segment) {

  int recv_retval = 0;

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(socket_desc, &fds);

  struct timeval timeout;
  timeout.tv_sec = 0;                   // Timeout in seconds
  timeout.tv_usec = DEFAULT_TIMEOUT_US; // Timeout in microseconds

  int activity = select(socket_desc + 1, &fds, NULL, NULL, &timeout);
  if (activity == -1) {
    return UNKNOWN_FAILURE;
  } else if (activity == 0) {
    return TIMEOUT;
  } else {
    recv_retval = recv_tcp(socket_desc, client_addr, recv_segment);
  }

  return recv_retval;
}