#include "main.h"

int read_id() {
    char buffer[sizeof(uint32_t) + 2];
    fprintf(stdout, "Enter id of deposit: ");
    fflush(stdout);
    fgets(buffer, sizeof(uint32_t), stdin);
    return atoi(buffer);
}


int read_refill_amount() {
    char buffer[sizeof(uint32_t) + 2];
    fprintf(stdout, "Enter refilling amount: ");
    fflush(stdout);
    fgets(buffer, sizeof(uint32_t) + 2, stdin);
    return atoi(buffer);
}


int read_amount() {
    char buffer[sizeof(uint32_t) + 1];
    fprintf(stdout, "Enter amount of deposit: ");
    fflush(stdout);
    fgets(buffer, sizeof(uint32_t), stdin);

    return atoi(buffer);
}


int get_user_choice() {
    printf("1) Открыть вклад\n");
    printf("2) Посмотреть вклады\n");
    printf("3) Пополнить вклад\n");
    printf("4) Закрыть вклад\n");
    printf("5) Завершить работу\n");
    printf("6) Показать счет банка\n");
    printf("7) Наличислить проценты по вкладам\n");

    char buffer[sizeof(int) + 1];
    fgets(buffer, sizeof(int), stdin); //
    return atoi(buffer);
}


int check_number_of_args(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        return -1;
    } else {
        return 1;
    }
}


//индекс позволяет отличить одинаковые пакеты разных запросов
uint32_t generate_new_index(uint32_t old_index) {
    // перемешивание (1 2 4 3)
//    if (old_index == 2) {
//        return 4;
//    } else if (old_index == 4) {
//        return 3;
//    } else {
//        return old_index + 1 > 65535 ? 1 : old_index + 1;
//    }

    // потеря (1 2 4)
    //return old_index > 1 ? old_index + 2 : old_index + 1;

    return old_index + 1 > 65535 ? 1 : old_index + 1;
}

int handle_received_packet(int number_received, const char received_packet[MAX_PACKET_SIZE], uint32_t cur_index,
                            int deposit_id, uint32_t *expected_block_number) {

    if ((number_received > 0) /*&& (*(uint32_t *) (received_packet + SIZE_OF_PACKET_TYPE) == cur_index)*/) {

        if (*(uint16_t *) received_packet == ACKNOWLEDGMENT_PACKET &&
            *(uint16_t *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX) == DEPOSIT_WAS_OPENED) {
            //обработка пакета с подтверждением открытия вклада
            printf("Вклад открыт, id: %d\n",
                   *(uint32_t *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX +
                                  SIZE_OF_PACKET_ACK_TYPE));
            return 1;
        }

        if (*(uint16_t *) received_packet == LIST_OF_DEPOSITS_PACKET &&
            (*(uint16_t *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX) == *expected_block_number ||
             *(uint16_t *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX) == 0)) {
            // обработка пакета с данными
            printf("-----------------\n");
            printf("id of deposit: %d\n",
                   *(uint32_t *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX +
                                  SIZE_OF_PACKET_BLOCK_NUMBER));
            printf("accrued amount: %d\n",
                   *(uint32_t *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX +
                                  SIZE_OF_PACKET_BLOCK_NUMBER + SIZE_OF_ID_DEPOSIT));
            printf("amount: %f\n",
                   *(double *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX +
                                SIZE_OF_PACKET_BLOCK_NUMBER + SIZE_OF_ID_DEPOSIT +
                                SIZE_OF_INITIAL_AMOUNT));

            *expected_block_number =
                    *(uint32_t *) (received_packet + SIZE_OF_PACKET_TYPE +
                                   SIZE_OF_PACKET_INDEX) ==
                    0 ? 0 : *expected_block_number + 1;
            return 1;
        }

        if (*(uint16_t *) received_packet == ACKNOWLEDGMENT_PACKET &&
                *(uint16_t *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX) == DEPOSIT_WAS_REFILLED) {
            //обработка пакета с подтверждением пополнения вклада
            printf("You refilled deposit with id %d and amount is %f.\n", deposit_id,
                   (double) (*(uint32_t *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX +
                                            SIZE_OF_PACKET_ACK_TYPE)));
            return 1;

        }

        if (*(uint16_t *) received_packet == ACKNOWLEDGMENT_PACKET &&
            *(uint16_t *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX) == DEPOSIT_WAS_DELETED) {
            //обработка пакета с подтверждением закрытия вклада
            printf("Вклад удалён, id: %d\n", *(uint32_t *) (received_packet + SIZE_OF_PACKET_TYPE +
                                                            SIZE_OF_PACKET_INDEX + SIZE_OF_PACKET_ACK_TYPE));
            return 1;

        }

        if (*(uint16_t *) received_packet == SHOW_BANK_AMOUNT_PACKET) {
            //обработка пакета с показом счета банка
            printf("Счет банка: %f\n", *(double *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX));
            return 1;
        }

        if (*(uint16_t *) received_packet == ACKNOWLEDGMENT_PACKET &&
                   *(uint16_t *) (received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX) == PERCENTS_ADDED) {
            //обработка пакета с подтверждением начисления процентов
            printf("Проценты по вкладам начислены.\n");
            return 1;
        }

        if (*(uint16_t *) received_packet == ERROR_PACKET) {
            //обработка пакета с ошибкой
            printf("ERROR. error message: %s.\n",
                   received_packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX);
            *expected_block_number = 0;
            return 1;
        }
    }

    return -1;
}

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //проверка количества аргументов
    if (check_number_of_args(argc, argv) < 0) {
        return 0;
    }

    //определение номера порта
    portno = (uint16_t) atoi(argv[2]);

    // создание сокета
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); //

    //проверка созданного сокета
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return 0;
    }

    // получение адреса сервера
    server = gethostbyname(argv[1]); //

    // проверка полученного адреса сервера
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return 0;
    }

    // настройка параметров сервера
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    void *packet;
    char received_packet[MAX_PACKET_SIZE];
    uint32_t amount;
    uint32_t packet_size;
    struct sockaddr_in received_addr;
    int received_addr_len;
    int deposit_id = 0;
    int refill_amount;
    uint32_t cur_index = 0;
    uint32_t expected_block_number;
    int number_received;
    //для таймаута
    int result;
    fd_set inputs;
    struct timeval timeout;
    int i = 0;

    while (1) {
        switch (get_user_choice()) {
            case 1:

                // чтение начальной суммы нового депозита
                if ((amount = read_amount()) < 0) {
                    printf("ERROR: invalid value\n");
                    continue;
                }

                //генерация нового индекса
                cur_index = generate_new_index(cur_index);
                printf("index: %d\n", cur_index);

                // создание пакета с определённым индексом
                packet = create_add_deposit_packet(&packet_size, cur_index, amount);

                //это всё для таймаута
                FD_ZERO(&inputs);
                FD_SET(sockfd, &inputs);

                // несколько раз отправляем сообщение и ждём ответа в течение какого-то времени
                for (i = 0; i < 3; ++i) {
                    //отправка пакета

                    sendto(sockfd, packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &serv_addr,
                           sizeof(serv_addr)); //

                    //ждём ответа в течение таймаута
                    timeout.tv_sec = 5;  //секунды
                    timeout.tv_usec = 0; //микросекунды
                    result = select(FD_SETSIZE, &inputs, NULL, NULL, &timeout); //

                    //если таймаут вышел, а данных нет
                    if (result == 0) {
                        continue;
                    }

                    //принимаем сообщение
                    bzero(&received_addr, sizeof(received_addr));
                    number_received = recvfrom(sockfd, received_packet, MAX_PACKET_SIZE, MSG_WAITALL,
                                               (struct sockaddr *) &received_addr,
                                               (socklen_t * ) & received_addr_len);

                    expected_block_number = 0;
                    if (handle_received_packet(number_received, received_packet, cur_index, deposit_id,
                                               &expected_block_number) == 1) {
                        break;
                    }
                    //обработка принятого пакета с нужным индексом
                }

                // если за все попытки не удалось получить ответ
                if (i == 3) {
                    fprintf(stdout, "Server is not responding\n");
                }

                free(packet);
                break;

            case 2:
                //просмотр вкладов
                cur_index = generate_new_index(cur_index);
                printf("index: %d\n", cur_index);

                expected_block_number = 1;
                while (expected_block_number != 0) {
                    // создаём пакет запроса вкладов или подтверждения
                    if (expected_block_number == 1) {
                        packet = create_get_list_of_deposits_packet(&packet_size, cur_index);
                    } else {
                        packet = create_acknowledgment_packet(&packet_size, PACKET_WAS_RECEIVED, cur_index,
                                                              expected_block_number - 1);
                    }

                    //это всё для таймаута
                    FD_ZERO(&inputs);
                    FD_SET(sockfd, &inputs);

                    //несколько раз пытаемся отправить пакет, пока не получим ответ
                    for (i = 0; i < 3; ++i) {
                        //отправляем ответ
                        sendto(sockfd, packet, packet_size, MSG_WAITALL,
                               (const struct sockaddr *) &serv_addr, sizeof(serv_addr)); //

                        //ждём ответа в течение таймаута
                        timeout.tv_sec = 5;  //секунды
                        timeout.tv_usec = 0; //микросекунды
                        result = select(FD_SETSIZE, &inputs, NULL, NULL, &timeout);

                        //если таймаут вышел, а данных нет
                        if (result == 0) {
                            continue;
                        }

                        // принимаем ответ
                        bzero(&received_addr, sizeof(received_addr));
                        number_received = recvfrom(sockfd, received_packet, MAX_PACKET_SIZE, MSG_WAITALL,
                                                   (struct sockaddr *) &received_addr,
                                                   (socklen_t *) &received_addr_len);

                        if (handle_received_packet(number_received, received_packet, cur_index, deposit_id,
                                               &expected_block_number) == 1) {
                            break;
                        }
                    }

                    // если исчерпали все попытки принять пакет, но так ничего и не получили
                    if (i == 3) {
                        fprintf(stdout, "Server is not responding\n");
                        free(packet);
                        break;
                    }
                }
                break;

            case 3:
                //пополнить вклад

                if ((deposit_id = read_id()) < 0) {
                    printf("ERROR: invalid value\n");
                    continue;
                }

                if ((refill_amount = read_refill_amount()) < 0) {
                    printf("ERROR: invalid value\n");
                    continue;
                }

                //генерация нового индекса
                cur_index = generate_new_index(cur_index);
                printf("index: %d\n", cur_index);

                //создание пакета пополнения вклада
                packet = create_refill_deposit_packet(&packet_size, cur_index, deposit_id, refill_amount);

                //это всё для таймаута
                FD_ZERO(&inputs);
                FD_SET(sockfd, &inputs);

                for (i = 0; i < 3; ++i) {
                    sendto(sockfd, packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &serv_addr,
                           sizeof(serv_addr));

                    //ждём ответа в течение таймаута
                    timeout.tv_sec = 5;  //секунды
                    timeout.tv_usec = 0; //микросекунды
                    result = select(FD_SETSIZE, &inputs, NULL, NULL, &timeout);

                    //если таймаут вышел, а данных нет
                    if (result == 0) {
                        continue;
                    }

                    bzero(&received_addr, sizeof(received_addr));
                    number_received = recvfrom(sockfd, received_packet, MAX_PACKET_SIZE, MSG_WAITALL,
                                               (struct sockaddr *) &received_addr,
                                               (socklen_t *) &received_addr_len);

                    expected_block_number = 0;
                    if (handle_received_packet(number_received, received_packet, cur_index, deposit_id,
                                           &expected_block_number) == 1) {
                        break;
                    }
                }

                // если за все попытки не удалось получить ответ
                if (i == 3) {
                    fprintf(stdout, "Server is not responding\n");
                }

                free(packet);
                break;

            case 4:
                // закрытие вклада
                if ((deposit_id = read_id()) < 0) {
                    printf("ERROR: invalid value\n");
                    continue;
                }

                //генерация нового индекса
                cur_index = generate_new_index(cur_index);
                printf("index: %d\n", cur_index);

                //создание пакета для закрытия вклада
                packet = create_remove_deposit_packet(&packet_size, cur_index, deposit_id);

                FD_ZERO(&inputs);
                FD_SET(sockfd, &inputs);

                for (i = 0; i < 3; ++i) {
                    sendto(sockfd, packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &serv_addr,
                           sizeof(serv_addr));

                    //ждём ответа в течение таймаута
                    timeout.tv_sec = 5;
                    timeout.tv_usec = 0;
                    result = select(FD_SETSIZE, &inputs, NULL, NULL, &timeout);

                    //если таймаут вышел, а данных нет
                    if (result == 0) {
                        continue;
                    }

                    bzero(&received_addr, sizeof(received_addr));
                    number_received = recvfrom(sockfd, received_packet, MAX_PACKET_SIZE, MSG_WAITALL,
                                               (struct sockaddr *) &received_addr,
                                               (socklen_t *) &received_addr_len);

                    expected_block_number = 0;
                    //обработка принятого пакета с нужным индексом
                    if (handle_received_packet(number_received, received_packet, cur_index, deposit_id,
                                           &expected_block_number) == 1) {
                        break;
                    }
                }

                // если за все попытки не удалось получить ответ
                if (i == 3) {
                    fprintf(stdout, "Server is not responding\n");
                }

                free(packet);
                break;

            case 5:
                shutdown(sockfd, SHUT_RDWR);
                close(sockfd);
                return 0;

            case 6:
                //генерация нового индекса
                cur_index = generate_new_index(cur_index);
                printf("index: %d\n", cur_index);

                //создание пакета для закрытия вклада
                packet = create_show_bank_amount_packet(&packet_size, cur_index);

                FD_ZERO(&inputs);
                FD_SET(sockfd, &inputs);

                for (i = 0; i < 3; ++i) {
                    sendto(sockfd, packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &serv_addr,
                           sizeof(serv_addr));

                    //ждём ответа в течение таймаута
                    timeout.tv_sec = 5;
                    timeout.tv_usec = 0;
                    result = select(FD_SETSIZE, &inputs, NULL, NULL, &timeout);

                    //если таймаут вышел, а данных нет
                    if (result == 0) {
                        continue;
                    }

                    bzero(&received_addr, sizeof(received_addr));
                    number_received = recvfrom(sockfd, received_packet, MAX_PACKET_SIZE, MSG_WAITALL,
                                               (struct sockaddr *) &received_addr,
                                               (socklen_t *) &received_addr_len);

                    expected_block_number = 0;
                    //обработка принятого пакета с нужным индексом
                    if (handle_received_packet(number_received, received_packet, cur_index, deposit_id,
                                           &expected_block_number) == 1) {
                        break;
                    }
                }

                // если за все попытки не удалось получить ответ
                if (i == 3) {
                    fprintf(stdout, "Server is not responding\n");
                }

                free(packet);
                break;

            case 7:
                //генерация нового индекса
                cur_index = generate_new_index(cur_index);
                printf("index: %d\n", cur_index);

                //создание пакета для закрытия вклада
                packet = create_please_add_percents_packet(&packet_size, cur_index);

                FD_ZERO(&inputs);
                FD_SET(sockfd, &inputs);

                for (i = 0; i < 3; ++i) {
                    sendto(sockfd, packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &serv_addr,
                           sizeof(serv_addr));

                    //ждём ответа в течение таймаута
                    timeout.tv_sec = 5;
                    timeout.tv_usec = 0;
                    result = select(FD_SETSIZE, &inputs, NULL, NULL, &timeout);

                    //если таймаут вышел, а данных нет
                    if (result == 0) {
                        continue;
                    }

                    bzero(&received_addr, sizeof(received_addr));
                    number_received = recvfrom(sockfd, received_packet, MAX_PACKET_SIZE, MSG_WAITALL,
                                               (struct sockaddr *) &received_addr,
                                               (socklen_t *) &received_addr_len);

                    expected_block_number = 0;
                    //обработка принятого пакета с нужным индексом
                    if (handle_received_packet(number_received, received_packet, cur_index, deposit_id,
                                           &expected_block_number) == 1) {
                        break;
                    }
                }
                // если за все попытки не удалось получить ответ
                if (i == 3) {
                    fprintf(stdout, "Server is not responding\n");
                }

                free(packet);
                break;

            default:
                printf("ERROR: wrong choice\n");
                break;
        }
    }
}