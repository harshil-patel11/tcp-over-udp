/**
 * @file tcp_segment.c
 * @brief Function definitions for creating, calculating checksum and comparing
 * checksum of a TCP segment
 *
 * This file contains the function definitions for creating a TCP segment,
 * calculating the checksum of a TCP segment, and comparing the checksum of a
 * TCP segment
 *
 * @author Ritam Singal (ritamsingal)
 * @author Harshil Patel (harshil-patel11)
 * @bug No known bugs
 */

#include <stdlib.h>
#include <string.h>

#include "../include/tcp_segment.h"

void create_tcp_segment(uint16_t source_port, uint16_t dest_port,
                        uint32_t seq_number, uint32_t ack_number, uint8_t flags,
                        unsigned char *data, size_t data_size,
                        tcp_segment_t *segment) {

  segment->source_port = source_port;
  segment->dest_port = dest_port;
  segment->seq_number = seq_number;
  segment->ack_number = ack_number;
  segment->flags = flags;
  memset(segment->data, '\0', sizeof(segment->data));
  memcpy(segment->data, data, data_size);

  segment->head_len = sizeof(segment->source_port) +
                      sizeof(segment->dest_port) + sizeof(segment->seq_number) +
                      sizeof(segment->ack_number) + sizeof(segment->flags) +
                      sizeof(segment->checksum);
  segment->checksum = calculate_checksum(segment);
}

uint16_t calculate_checksum(tcp_segment_t *segment) {
  uint32_t sum = 0;
  sum += segment->source_port;
  sum += segment->dest_port;
  sum += segment->seq_number;
  sum += segment->ack_number;
  sum += segment->head_len;
  sum += segment->flags;

  for (int i = 0; i < SEGMENT_DATA_SIZE; i++) {
    sum += segment->data[i];
  }

  sum = (sum & 0xFFFF) + (sum >> 16);
  sum = ~sum;
  return (uint16_t)sum;
}

int compare_checksum(tcp_segment_t *segment) {
  uint16_t calculated_checksum = calculate_checksum(segment);
  if (segment->checksum == calculated_checksum) {
    return 1;
  }
  return 0;
}