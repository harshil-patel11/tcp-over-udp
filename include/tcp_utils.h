/**
 * @file tcp_utils.h
 * @brief Function prototypes for creating, sending and receiving TCP segments
 *
 * This header file contains the function prototypes for creating a socket,
 sending a TCP segment, and receiving a TCP segment
 *
 * @author Ritam Singal (ritamsingal)
 * @author Harshil Patel (harshil-patel11)
 * @bug No known bugs
*/

#ifndef TCP_UTILS_H
#define TCP_UTILS_H

#include <sys/time.h>

#define MAX_WINDOW_SIZE 24        // maximum window size for sending packets
#define DEFAULT_TIMEOUT_US 250000 // default timeout for receiving packets

/**
 * @brief Enum representing the different errors in sending and receiving TCP
 */
typedef enum tcp_error {
  SUCCESS = 0,
  CHECKSUM_FAILED = -1,
  TIMEOUT = -2,
  RECV_FAILED = -3,    // failed to receive from socket
  SEND_FAILED = -4,    // failed to send from socket
  UNKNOWN_FAILURE = -5 // unknown failure
} tcp_error_t;

/**
 * @brief Create a socket
 *
 * Create a UDP socket and return the socket descriptor
 *
 * @return int Socket descriptor
 */
int create_socket();

/**
 * @brief Bind a socket
 *
 * Bind a socket to the given server address
 *
 * @param socket_desc Socket descriptor
 * @param server_addr Server address
 * @param udp_port UDP port
 * @param ip_addr IP address
 * @return int 0 if successful, -1 if failed
 */
int bind_socket(int socket_desc, struct sockaddr_in *server_addr,
                unsigned short int udp_port, char *ip_addr);

/**
 * @brief Get the IP address of the host
 *
 * Get the IP address of the host and store it in the given pointer
 *
 * @param ip Pointer to store the IP address
 * @return int 0 if successful, -1 if failed
 */
int get_host_ip(char **ip);

/**
 * @brief Get the IP address of the host by hostname
 *
 * Get the IP address of the host by hostname and store it in the given pointer
 *
 * @param ip Pointer to store the IP address
 * @param host Host name
 * @return int 0 if successful, -1 if failed
 */
int get_host_ip_by_hostname(char **ip, char *host);

/**
 * @brief Send a TCP segment
 *
 * Send a TCP segment to the given server address
 *
 * @param socket_desc Socket descriptor
 * @param send_segment TCP segment to send
 * @param server_addr Server address
 * @return SUCCESS if successful, SEND_FAILED if failed to send from socket
 */
tcp_error_t send_tcp(int socket_desc, tcp_segment_t *send_segment,
                     struct sockaddr_in *server_addr);

/**
 * @brief Receive a TCP segment
 *
 * Receive a TCP segment from the given client address
 *
 * @param socket_desc Socket descriptor
 * @param client_addr Client address
 * @param recv_segment TCP segment to receive
 * @return SUCCESS if successful, CHECKSUM_FAILED if checksums don't match,
 * RECV_FAILED if failed to receive from socket
 */
tcp_error_t recv_tcp(int socket_desc, struct sockaddr_in *client_addr,
                     tcp_segment_t *recv_segment);

/**
 * @brief Receive a TCP segment with timeout
 *
 * Receive a TCP segment from the given client address with a timeout
 *
 * @param socket_desc Socket descriptor
 * @param client_addr Client address
 * @param recv_segment TCP segment to receive
 * @return SUCCESS if successful, CHECKSUM_FAILED if checksums don't match,
 * TIMEOUT if timeout occurs, RECV_FAILED if failed to receive from socket,
 * UNKNOWN_FAILURE if unknown failure
 */
tcp_error_t recv_tcp_with_timeout(int socket_desc,
                                  struct sockaddr_in *client_addr,
                                  tcp_segment_t *recv_segment);

#endif // TCP_UTILS_H