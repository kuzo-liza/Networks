#include "main.h"

void* create_acknowledgment_packet(uint32_t* packet_size, uint32_t index, uint16_t ack_type, uint32_t number) {
    *packet_size = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_PACKET_ACK_TYPE + SIZE_OF_ACK_NUMBER;
    void* packet = malloc(*packet_size);

    uint16_t packet_type = ACKNOWLEDGMENT_PACKET;
    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX, &ack_type, SIZE_OF_PACKET_ACK_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_PACKET_ACK_TYPE, &number, SIZE_OF_ACK_NUMBER);

    return packet;
}


void* create_error_packet(uint32_t* packet_length, uint32_t index, char* err_text, uint64_t msg_size) {
    *packet_length = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + msg_size;
    void* packet = malloc(*packet_length);

    uint16_t packet_type = ERROR_PACKET;
    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX, err_text, msg_size);

    return packet;
}


void* create_list_of_deposit_packet(uint32_t* packet_length, uint32_t index, uint32_t deposit_id, uint32_t block_number,
        uint32_t initial_amount, double current_amount) {

    *packet_length = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_PACKET_BLOCK_NUMBER +
            SIZE_OF_ID_DEPOSIT + SIZE_OF_INITIAL_AMOUNT + sizeof(double);
    void* packet = malloc(*packet_length);

    uint16_t  packet_type = LIST_OF_DEPOSITS_PACKET;
    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX, &block_number, SIZE_OF_PACKET_BLOCK_NUMBER);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_PACKET_BLOCK_NUMBER, &deposit_id,
           SIZE_OF_ID_DEPOSIT);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_PACKET_BLOCK_NUMBER + SIZE_OF_ID_DEPOSIT,
           &initial_amount, SIZE_OF_INITIAL_AMOUNT);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + SIZE_OF_PACKET_BLOCK_NUMBER + SIZE_OF_ID_DEPOSIT +
           SIZE_OF_INITIAL_AMOUNT, &current_amount, sizeof(double));

    return packet;
}


void* create_show_bank_amount_packet(uint32_t* packet_length, uint32_t index, double bank_amount){
    *packet_length = SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX + sizeof(double);
    void* packet = malloc(*packet_length);
    uint16_t packet_type = SHOW_BANK_AMOUNT_PACKET;

    memcpy(packet, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_TYPE, &index, SIZE_OF_PACKET_INDEX);
    memcpy(packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX, &bank_amount, sizeof(double));
    return packet;
}
