#ifndef SERVER_MAIN_H
#define SERVER_MAIN_H

#define MAX_PACKET_SIZE 516

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
  int port;
  int *initial_sockfd;
} Receiving_thread_input;

typedef struct {
  struct sockaddr_in cliaddr;
  int sockfd;
  int cliaddr_len;
  void *packet;
} Listening_thread_input;

typedef struct deposit_info {
  uint32_t deposit_id;
  double current_amount;
  int client_sockfd;
  uint32_t initial_amount;
  struct sockaddr_in cliaddr;
  struct deposit_info *next;
} Deposit_info;

typedef struct User_info {
  char *address;
  int port;
  int sockfd;
  pthread_t client_thread;
  struct sockaddr_in cliaddr;
  uint32_t required_index;
  void* last_answer;
  uint32_t packet_size;
  struct User_info *next;
} User_info;

// ack types
#define DEPOSIT_WAS_OPENED 1
#define DEPOSIT_WAS_REFILLED 2
#define DEPOSIT_WAS_DELETED 3
#define PACKET_WAS_RECEIVED 4
#define PERCENTS_ADDED 5

// packet types
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

// size of packet parts
#define SIZE_OF_PACKET_ACK_TYPE 2
#define SIZE_OF_PACKET_TYPE 2
#define SIZE_OF_PACKET_AMOUNT 4
#define SIZE_OF_PACKET_INDEX 4
#define SIZE_OF_ID_DEPOSIT 4
#define SIZE_OF_ACK_NUMBER 4
#define SIZE_OF_ERR_TYPE 2
#define SIZE_OF_REFILL_AMOUNT 4
#define SIZE_OF_CURRENT_AMOUNT 4
#define SIZE_OF_INITIAL_AMOUNT 4
#define SIZE_OF_PACKET_BLOCK_NUMBER 4

// list of clients
void list_of_clients_add(User_info *new_client);
User_info *make_new_client(int sockfd, int port, char *address, pthread_t client_thread);
int list_of_clients_remove(int sockfd);
void list_of_clients_remove_all();
void init_list_of_clients_mutex();
pthread_t create_user_listening_thread(int *initial_socket);
void list_of_clients_export(FILE *dst_fd);
uint32_t list_of_clients_get_client_index(int port, char *address);
void list_of_clients_set_client_index(int port, char *address, uint32_t index);
void list_of_clients_set_required_index(int port, char* address);
void list_of_clients_set_last_answer(int port, char* address, void *answer, uint32_t packet_size);
void* list_of_clients_get_last_answer(int port, char* address, uint32_t *packet_size);
uint32_t list_of_clients_get_required_index(int port, char* address);

// list of deposits
void list_of_deposits_remove_all();
void init_list_of_deposits_mutex();
double list_of_deposits_refill_deposit(int port, const char *address,
                                       uint32_t deposit_id,
                                       uint32_t added_amount);
int32_t list_of_deposits_remove(int port, const char *address, uint32_t deposit_id);
void list_of_deposits_send(int client_sockfd);
void list_of_deposits_add(Deposit_info *new_deposit);
void list_of_deposits_add_percents(void);
uint32_t generate_deposit_id(void);
void list_of_deposits_export_bank_amount();
void list_of_deposits_all_deposits(FILE *output_file);
Deposit_info *make_new_deposit(int deposit_id, int initial_amount,
                               int client_sockfd, struct sockaddr_in cliaddr);
Deposit_info* list_of_deposits_get_deposit(int port, const char *address, int number, int *is_deposit_last);


void *create_add_deposit_packet(uint32_t *packet_length, uint32_t index,
                                uint32_t initial_amount);
void *create_remove_deposit_packet(uint32_t *packet_size, uint32_t index,
                                   uint32_t deposit_id);
void *create_get_list_of_deposits_packet(uint32_t index);
void *create_acknowledgment_packet(uint32_t *packet_size, uint32_t index,
                                   uint16_t ack_type, uint32_t number);
void *create_error_packet(uint32_t *packet_length, uint32_t index,
                          char *err_text, uint64_t msg_size);
void *create_refill_deposit_packet(uint32_t *packet_length, uint32_t index,
                                   uint32_t deposit_id, uint32_t amount);
void *create_list_of_deposit_packet(uint32_t *packet_length, uint32_t index, uint32_t deposit_id, uint32_t block_number,
                                    uint32_t initial_amount, double current_amount);
double list_of_deposits_get_bank_amount(void);

        pthread_t create_receiving_thread(int port, int *initial_sockfd);

pthread_t create_listening_thread(int sockfd, void *packet, struct sockaddr_in cliaddr, int cliaddr_len);

void* create_show_bank_amount_packet(uint32_t* packet_length, uint32_t index, double bank_amount);
int add_percents_to_client(int port, const char *address);
int remove_client_deposits(int port, const char *address);
#endif // SERVER_MAIN_H
