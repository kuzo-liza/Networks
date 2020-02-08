
#include "main.h"


int checkArguments(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage %s port \n", argv[0]);
        return -1;
    }
    return 1;
}


int main(int argc, char* argv[]) {
    struct sockaddr_in cliaddr;
    int sockfd;
    int cliaddr_len;
    struct sockaddr_in servaddr;

    //проверка количества аргументов
    if (checkArguments(argc, argv) < 0) {
        return 0;
    }

    const uint16_t port = (uint16_t) atoi(argv[1]);

    //создание начального сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("ERROR: socket creation failed.\n");
        return 0;
    }

    //заполнение информации сервера
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // make socket reusable
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) { //
        fprintf(stderr, "ERROR: setsockopt(SO_REUSEADDR) failed");
    }

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("ERROR: bind failed");
        return 0;
    }

    init_list_of_clients_mutex();
    init_list_of_deposits_mutex();

    pthread_t receiving_thread = create_receiving_thread(port, &sockfd);
    pthread_t user_listening_thread = create_user_listening_thread(&sockfd);
    pthread_join(receiving_thread, NULL);
    pthread_join(user_listening_thread, NULL);

    list_of_clients_remove_all();
    list_of_deposits_remove_all();

}