#include "main.h"

int get_user_choice() {
    printf("1) Начислить проценты\n");
    printf("2) Показать счёт банка\n");
    printf("3) Показать все вклады\n");
    printf("4) Завершить работу\n");
    printf("5) Посмотреть список клиентов\n");
    printf("6) Удалить вклады клиента\n");

    char buffer[sizeof(int) + 2];
    fgets(buffer, sizeof(int) + 2, stdin);
    return atoi(buffer);
}

void* console_listening_thread(void* arg) {
    int port;
    char address[17];

    while(1) {

        switch (get_user_choice()){
            case 1:
                list_of_deposits_add_percents();
                break;

            case 2:
                list_of_deposits_export_bank_amount();
                break;

            case 3:
                list_of_deposits_all_deposits(stdout);
                break;

            case 4:
                shutdown(*(int*) arg, SHUT_RDWR);
                close(*(int*) arg);
                return NULL;

            case 5:
                list_of_clients_export(stdout);
                break;

            case 6:
                printf("Введите порт клиента:\n");
                scanf("%d", &port);
                printf("Введите адрес клиента:\n");
                scanf("%s", address);
                if (remove_client_deposits(port, address) < 0){
                    printf("ERROR: can't remove client deposits.");
                } else {
                    printf("Вклады клиента удалены\n");
                }
                break;

            default:
                printf("ERROR: wrong choice\n");
                break;
        }
    }
}

pthread_t create_user_listening_thread(int* initial_socket) {
    pthread_t user_listening_thread;

    if (pthread_create(&user_listening_thread, NULL, console_listening_thread, (void*) initial_socket)) {
        return -1;
    }

    return user_listening_thread;
}