/**
 * @file tcp_segment.h
 * @brief Function prototypes for creating, calculating checksum and comparing
 * checksum of a TCP segment
 *
 * This header file contains the function prototypes for creating a TCP segment,
 * calculating the checksum of a TCP segment, and comparing the checksum of a
 * TCP segment
 *
 * @author Ritam Singal (ritamsingal)
 * @author Harshil Patel (harshil-patel11)
 * @bug No known bugs
 */

#ifndef TCP_SEGMENT_H
#define TCP_SEGMENT_H

#include <stdint.h>
#include <stdlib.h>

// maximum size of the data field within the segment
#define SEGMENT_DATA_SIZE 512

/**
 * @brief Structure representing a TCP segment
 */
typedef struct tcp_segment {
  uint16_t source_port;
  uint16_t dest_port;
  uint32_t seq_number;
  uint32_t ack_number;
  uint8_t head_len;
  uint8_t flags;
  uint16_t checksum;
  char data[SEGMENT_DATA_SIZE];
} tcp_segment_t;

/**
 * @brief Enum representing the different flags in a TCP segment
 */
typedef enum tcp_flags {
  FIN = 0x01,
  SYN = 0x02,
  RST = 0x04,
  PSH = 0x08,
  ACK = 0x10,
  URG = 0x20,
  ECE = 0x40,
  CWR = 0x80
} tcp_flags_t;

/**
 * @brief Create a TCP segment
 *
 * Create a TCP segment with the given parameters
 *
 * @param source_port Source port
 * @param dest_port Destination port
 * @param seq_number Sequence number
 * @param ack_number Acknowledgement number
 * @param flags Flags
 * @param data Data
 * @param data_size Size of the data
 * @param segment Pointer to the segment to be created
 */
void create_tcp_segment(uint16_t source_port, uint16_t dest_port,
                        uint32_t seq_number, uint32_t ack_number, uint8_t flags,
                        unsigned char *data, size_t data_size,
                        tcp_segment_t *segment);

/**
 * @brief Calculate the checksum of a TCP segment
 *
 * Calculate the checksum of a TCP segment
 *
 * @param segment Pointer to the segment
 * @return calculated checksum

*/
uint16_t calculate_checksum(tcp_segment_t *segment);

/**
 * @brief Compare the checksum of a TCP segment
 *
 * Compares the checksum of a TCP segment received over a socket with
 * a locally calculated checksum
 *
 * @param segment Pointer to the TCP segment
 *
 * @return 1 if the checksum is correct, 0 otherwise
 */
int compare_checksum(tcp_segment_t *segment);

#endif