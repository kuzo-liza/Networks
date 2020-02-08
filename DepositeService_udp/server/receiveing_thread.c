#include "main.h"

void* receiving_thread(void* arg) {
    struct sockaddr_in cliaddr;
    int cliaddr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in servaddr;
    int port = ((Receiving_thread_input*) arg)->port;
    int *sockfd = ((Receiving_thread_input*) arg)-> initial_sockfd;

    while (1) {
        void* packet = malloc(MAX_PACKET_SIZE);
        bzero(packet, MAX_PACKET_SIZE);
        bzero(&cliaddr, sizeof(cliaddr));

        if (recvfrom(*sockfd, packet, (size_t) MAX_PACKET_SIZE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &cliaddr_len) <= 0) {
            free(packet);
            free(arg);
            return NULL;
        }

        int new_sockfd;
        //создание начального сокета
        if ((new_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("ERROR: socket creation failed.\n");
            free(packet);
            free(arg);
            return NULL;
        }

        //заполнение информации сервера
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;

        // make socket reusable
        if (setsockopt(new_sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) {
            fprintf(stderr, "ERROR: setsockopt(SO_REUSEADDR) failed");
        }

        pthread_t listening_thread = create_listening_thread(new_sockfd, packet, cliaddr, cliaddr_len);
        list_of_clients_add(
                make_new_client(new_sockfd, cliaddr.sin_port, strdup(inet_ntoa(cliaddr.sin_addr)), listening_thread));

    }
}


Receiving_thread_input* init_receiving_thread_input_structure(int port, int *initial_sockfd) {
    Receiving_thread_input* new_input_structure = (Receiving_thread_input*) malloc(sizeof(Receiving_thread_input));

    new_input_structure->port = port;
    new_input_structure->initial_sockfd=initial_sockfd;

    return new_input_structure;
}


pthread_t create_receiving_thread(int port, int *initial_sockfd) {
    pthread_t receiving_thread_;

    Receiving_thread_input* receiving_thread_input = init_receiving_thread_input_structure(port, initial_sockfd);

    if( pthread_create(&receiving_thread_, NULL, receiving_thread, receiving_thread_input)) {
        return -1;
    }

    return receiving_thread_;
}