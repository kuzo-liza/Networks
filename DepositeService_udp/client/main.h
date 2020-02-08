#ifndef DEPOSIT_SERVICE_CLIENT_UDP_MAIN_H
#define DEPOSIT_SERVICE_CLIENT_UDP_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <stdint.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>

#define MAX_PACKET_SIZE 519

// size of packet parts
#define SIZE_OF_PACKET_ACK_TYPE 2
#define SIZE_OF_PACKET_TYPE 2
#define SIZE_OF_PACKET_INDEX 4
#define SIZE_OF_PACKET_AMOUNT 4
#define SIZE_OF_ID_DEPOSIT 4
#define SIZE_OF_ACK_NUMBER 4
#define SIZE_OF_REFILL_AMOUNT 4
#define SIZE_OF_CURRENT_AMOUNT 4
#define SIZE_OF_INITIAL_AMOUNT 4
#define SIZE_OF_PACKET_BLOCK_NUMBER 4

//packet types
#define ERROR_PACKET 1
#define ACKNOWLEDGMENT_PACKET 2
#define LIST_OF_DEPOSITS_PACKET 3
#define OPEN_DEPOSIT_PACKET 4
#define REFILL_DEPOSIT_PACKET 5
#define CLOSE_DEPOSIT_PACKET 6
#define GET_LIST_OF_DEPOSITS_PACKET 7
#define GET_BANK_AMOUNT_PACKET 8
#define SHOW_BANK_AMOUNT_PACKET 9
#define PLEASE_ADD_PERCENTS_PACKET 10


// ack types
#define DEPOSIT_WAS_OPENED 1
#define DEPOSIT_WAS_REFILLED 2
#define DEPOSIT_WAS_DELETED 3
#define PACKET_WAS_RECEIVED 4
#define PERCENTS_ADDED 5

//packet creator
void* create_add_deposit_packet(uint32_t* packet_length, uint32_t index, uint32_t initial_amount);
void* create_remove_deposit_packet(uint32_t* packet_size, uint32_t index, uint32_t deposit_id);
void* create_get_list_of_deposits_packet(uint32_t *packet_length, uint32_t index);
void* create_acknowledgment_packet(uint32_t* packet_length, uint16_t ack_type, uint32_t index, uint32_t number);
void* create_refill_deposit_packet(uint32_t* packet_length, uint32_t index, uint32_t deposit_id, uint32_t amount);
void* create_show_bank_amount_packet(uint32_t* packet_size, uint32_t index);
void* create_please_add_percents_packet(uint32_t* packet_length, uint32_t index);

#endif //DEPOSIT_SERVICE_CLIENT_UDP_MAIN_H