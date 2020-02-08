#include "main.h"

void* create_add_deposit_packet(uint32_t* packet_length, uint32_t index, uint32_t initial_amount) {
    *packet_length = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_PACKET_AMOUNT;
    void* packet = malloc(*packet_length);

    uint16_t packet_type = OPEN_DEPOSIT_PACKET;
    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX, &initial_amount, SIZE_OF_INITIAL_AMOUNT);

    return packet;
}


void* create_remove_deposit_packet(uint32_t* packet_size, uint32_t index, uint32_t deposit_id) {
    *packet_size = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_ID_DEPOSIT;
    void* packet = malloc(*packet_size);
    uint16_t packet_type = CLOSE_DEPOSIT_PACKET;

    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX, &deposit_id, SIZE_OF_ID_DEPOSIT);

    return packet;
}

void* create_show_bank_amount_packet(uint32_t* packet_size, uint32_t index){
    *packet_size = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX;
    void* packet = malloc(*packet_size);
    uint16_t packet_type = GET_BANK_AMOUNT_PACKET;

    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
    return packet;
}

void* create_get_list_of_deposits_packet(uint32_t * packet_length, uint32_t index) {
    *packet_length = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX;
    void* packet = malloc(*packet_length);

    uint16_t packet_type = GET_LIST_OF_DEPOSITS_PACKET;
    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);

    return packet;
}


void* create_acknowledgment_packet(uint32_t* packet_length, uint16_t ack_type, uint32_t index, uint32_t number) {
    *packet_length = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_PACKET_ACK_TYPE + SIZE_OF_ACK_NUMBER;
    void* packet = malloc(*packet_length);

    uint16_t packet_type = ACKNOWLEDGMENT_PACKET;
    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX, &ack_type, SIZE_OF_PACKET_ACK_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_PACKET_ACK_TYPE, &number, SIZE_OF_ACK_NUMBER);

    return packet;
}


//void* create_error_packet(uint32_t index, uint16_t err_type, char* err_text, uint64_t msg_size) {
//    uint32_t packet_length = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_ERR_TYPE + msg_size;
//    void* packet = malloc(packet_length);
//    uint16_t packet_type = ERROR_PACKET;
//
//    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
//    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
//    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX, &err_type, SIZE_OF_ERR_TYPE);
//    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_ERR_TYPE, err_text, msg_size);
//
//    return packet;
//}


void* create_refill_deposit_packet(uint32_t* packet_length, uint32_t index, uint32_t deposit_id, uint32_t amount) {
    *packet_length = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_ID_DEPOSIT + SIZE_OF_PACKET_AMOUNT;
    void* packet = malloc(*packet_length);
    uint16_t packet_type = REFILL_DEPOSIT_PACKET;

    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX, &deposit_id, SIZE_OF_ID_DEPOSIT);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_ID_DEPOSIT, &amount, SIZE_OF_PACKET_AMOUNT);

  return packet;
}

void* create_please_add_percents_packet(uint32_t* packet_length, uint32_t index){
    *packet_length = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX;
    void* packet = malloc(*packet_length);
    uint16_t packet_type = PLEASE_ADD_PERCENTS_PACKET;
    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
    return packet;
}