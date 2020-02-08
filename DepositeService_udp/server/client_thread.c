
#include "main.h"

//поток, работающий с определённым пользователем
void *socket_listening_thread(void *arg) {
    //обработка пакета в зависимости от типа
    void *packet = ((Listening_thread_input *) arg)->packet;
    int sockfd = ((Listening_thread_input *) arg)->sockfd;
    struct sockaddr_in cliaddr = ((Listening_thread_input *) arg)->cliaddr;
    int cliaddr_len = ((Listening_thread_input *) arg)->cliaddr_len;
    uint32_t deposit_id;
    uint32_t packet_size;
    void *answer_packet;
    uint32_t amount;
    Deposit_info *deposit;
    int is_deposit_last;


    switch (*(uint16_t *) packet) {
        case OPEN_DEPOSIT_PACKET:
            //открыть вклад

            // если принятый индекс != ожидаемый -> посылаем старый пакет; return; 0 - нет клиента
            if (list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) != 0 &&
                ((*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                  list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) - 1))) {

                answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                &packet_size);
                if (answer_packet != NULL) {
                    sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                           cliaddr_len);
                    printf("Отправлен пакет с индексом: %d\n",
                           *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                }
            }

            if (*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr))) {

                if ((*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE)) > 2) { // дублирование 1 2 3 3 3 3
                    answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                    &packet_size);
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                } else {
                    deposit_id = generate_deposit_id();
                    list_of_deposits_add(make_new_deposit(deposit_id,
                                                          *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE +
                                                                         SIZE_OF_PACKET_INDEX), sockfd,
                                                          cliaddr));
                    answer_packet = create_acknowledgment_packet(&packet_size,
                                                                 *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE),
                                                                 DEPOSIT_WAS_OPENED, deposit_id);

                    list_of_clients_set_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr), answer_packet,
                                                    packet_size);
                    list_of_clients_set_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)); //++
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                }
            }

            break;

        case CLOSE_DEPOSIT_PACKET:
            // закрыть вклад
            // если принятый индекс != ожидаемый -> посылаем старый пакет;
            if (list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) != 0 &&
                *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) - 1) {

                answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                &packet_size);
                if (answer_packet != NULL) {
                    sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                           cliaddr_len);
                    printf("Отправлен пакет с индексом: %d\n",
                           *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                }
            }

            if (*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr))) {

                if ((*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE)) > 2) {
                    answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                    &packet_size);
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                } else {

                    deposit_id = *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX);
                    double id = list_of_deposits_remove(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr), deposit_id);

                    //отправить подтверждение
                    if (id < 0) {
                        char *error_msg = "ERROR: couldn't close deposit";
                        answer_packet = create_error_packet(&packet_size,
                                                            *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE),
                                                            error_msg, strlen(error_msg));
                    } else {
                        answer_packet = create_acknowledgment_packet(&packet_size,
                                                                     *(uint32_t *) ((char *) packet +
                                                                                    SIZE_OF_PACKET_TYPE),
                                                                     DEPOSIT_WAS_DELETED, id);
                    }

                    list_of_clients_set_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr), answer_packet,
                                                    packet_size);
                    list_of_clients_set_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr));
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                }
            }
            break;

        case GET_LIST_OF_DEPOSITS_PACKET:

            // если принятый индекс != ожидаемый -> посылаем старый пакет;
            if (list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) != 0 &&
                *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) - 1) {

                answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                &packet_size);
                if (answer_packet != NULL) {
                    sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                           cliaddr_len);
                    printf("Отправлен пакет с индексом: %d\n",
                           *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                }
            }

            if (*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr))) {

                if ((*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE)) > 2) {
                    answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                    &packet_size);
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                } else {
                    if ((deposit = list_of_deposits_get_deposit(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr), 1,
                                                                &is_deposit_last)) == NULL) {

                        char *error_msg = "ERROR: you have no deposit opened";
                        answer_packet = create_error_packet(&packet_size,
                                                            *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE),
                                                            error_msg, strlen(error_msg));

                    } else {
                        answer_packet = create_list_of_deposit_packet(
                                &packet_size,
                                *(uint32_t *) (packet + SIZE_OF_PACKET_TYPE),
                                deposit->deposit_id,
                                is_deposit_last == 1 ? 0 : 1,
                                deposit->initial_amount,
                                deposit->current_amount);
                    }

                    list_of_clients_set_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr), answer_packet,
                                                    packet_size);
                    if (is_deposit_last == 1) {
                        list_of_clients_set_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr));
                    }
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                }
            }
            break;

        case ACKNOWLEDGMENT_PACKET:
            // если принятый индекс != ожидаемый -> посылаем старый пакет;
            if (list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) != 0 &&
                *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) - 1) {
                answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                &packet_size);
                if (answer_packet != NULL) {
                    sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                           cliaddr_len);
                    printf("Отправлен пакет с индексом: %d\n",
                           *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                }
            }

            if (*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr))) {

                if ((*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE)) > 2) {
                    answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                    &packet_size);
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                } else {
                    if ((deposit = list_of_deposits_get_deposit(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                (int) (*(uint32_t *) (packet + SIZE_OF_PACKET_TYPE +
                                                                                      SIZE_OF_PACKET_INDEX +
                                                                                      SIZE_OF_PACKET_ACK_TYPE) + 1),
                                                                &is_deposit_last)) == NULL) {

                        char *error_msg = "ERROR: you have no deposit opened";
                        answer_packet = create_error_packet(&packet_size,
                                                            *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE),
                                                            error_msg, strlen(error_msg));

                    } else {
                        answer_packet = create_list_of_deposit_packet(
                                &packet_size,
                                *(uint32_t *) (packet + SIZE_OF_PACKET_TYPE),
                                deposit->deposit_id,
                                is_deposit_last == 1 ? 0 :
                                *(uint32_t *) (packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX +
                                               SIZE_OF_PACKET_ACK_TYPE) + 1,
                                deposit->initial_amount,
                                deposit->current_amount);
                    }

                    list_of_clients_set_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr), answer_packet,
                                                    packet_size);
                    if (is_deposit_last == 1) {
                        list_of_clients_set_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr));
                    }
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                }
            }

            break;

        case REFILL_DEPOSIT_PACKET:
            // если принятый индекс != ожидаемый -> посылаем старый пакет;
            if (list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) != 0 &&
                *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) - 1) {

                answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                &packet_size);
                if (answer_packet != NULL) {
                    sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                           cliaddr_len);
                    printf("Отправлен пакет с индексом: %d\n",
                           *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                }
            }

            if (*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr))) {

                if ((*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE)) > 2) {
                    answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                    &packet_size);
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                } else {
                    deposit_id = *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX);
                    amount = *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_INDEX +
                                            SIZE_OF_ID_DEPOSIT);

                    double deposit_amount;
                    if ((deposit_amount = list_of_deposits_refill_deposit(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                          deposit_id, amount)) == -1) {

                        char *error_msg = "ERROR: couldn't refill deposit";
                        answer_packet = create_error_packet(&packet_size,
                                                            *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE),
                                                            error_msg, strlen(error_msg));

                    } else {
                        answer_packet = create_acknowledgment_packet(&packet_size,
                                                                     *(uint32_t *) ((char *) packet +
                                                                                    SIZE_OF_PACKET_TYPE),
                                                                     DEPOSIT_WAS_REFILLED,
                                                                     (uint32_t) deposit_amount);
                    }

                    list_of_clients_set_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr), answer_packet,
                                                    packet_size);
                    list_of_clients_set_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr));
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                }
            }
            break;

        case GET_BANK_AMOUNT_PACKET:
            // если принятый индекс != ожидаемый -> посылаем старый пакет;
            if (list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) != 0 &&
                *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) - 1) {
                answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                &packet_size);
                if (answer_packet != NULL) {
                    sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                           cliaddr_len);
                    printf("Отправлен пакет с индексом: %d\n",
                           *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                }
            }

            if (*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr))) {

                if ((*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE)) > 2) {
                    answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                    &packet_size);
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                } else {
                    answer_packet = create_show_bank_amount_packet(&packet_size,
                                                                   *(uint32_t *) ((char *) packet +
                                                                                  SIZE_OF_PACKET_TYPE),
                                                                   list_of_deposits_get_bank_amount());

                    list_of_clients_set_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr), answer_packet,
                                                    packet_size);
                    list_of_clients_set_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr));
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                }
            }
            break;

        case PLEASE_ADD_PERCENTS_PACKET:
            // если принятый индекс != ожидаемый -> посылаем старый пакет; return;
            if (list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) != 0 &&
                *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) - 1) {
                answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                &packet_size);
                if (answer_packet != NULL) {
                    sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                           cliaddr_len);
                    printf("Отправлен пакет с индексом: %d\n",
                           *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                }
            }

            if (*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE) ==
                list_of_clients_get_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr))) {

                if ((*(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE)) > 2) {
                    answer_packet = list_of_clients_get_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr),
                                                                    &packet_size);
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                } else {

                    if (add_percents_to_client(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr)) > 0) {
                        answer_packet = create_acknowledgment_packet(&packet_size,
                                                                     *(uint32_t *) ((char *) packet +
                                                                                    SIZE_OF_PACKET_TYPE),
                                                                     PERCENTS_ADDED, 0);
                    } else {
                        char *error_msg = "ERROR: couldn't add percents";
                        answer_packet = create_error_packet(
                                &packet_size,
                                *(uint32_t *) ((char *) packet + SIZE_OF_PACKET_TYPE),
                                error_msg,
                                strlen(error_msg)
                        );
                    }

                    list_of_clients_set_last_answer(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr), answer_packet,
                                                    packet_size);
                    list_of_clients_set_required_index(cliaddr.sin_port, inet_ntoa(cliaddr.sin_addr));
                    if (answer_packet != NULL) {
                        sendto(sockfd, answer_packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &cliaddr,
                               cliaddr_len);
                        printf("Отправлен пакет с индексом: %d\n",
                               *(uint32_t *) ((char *) answer_packet + SIZE_OF_PACKET_TYPE));
                    }
                }
            }
            break;

        default:
            break;
    }

    free(packet);
    return NULL;
}


Listening_thread_input *
init_listening_thread_input_structure(int sockfd, void *packet, struct sockaddr_in *cliaddr,
                                      const int *cliaddr_len) {
    Listening_thread_input *new_input_structure = (Listening_thread_input *) malloc(sizeof(Listening_thread_input));

    new_input_structure->sockfd = sockfd;
    memcpy(&new_input_structure->cliaddr, cliaddr, sizeof(struct sockaddr_in));
    new_input_structure->cliaddr_len = *cliaddr_len;
    new_input_structure->packet = packet;

    return new_input_structure;
}


// создание потока
pthread_t create_listening_thread(int sockfd, void *packet, struct sockaddr_in cliaddr, int cliaddr_len) {
    pthread_t listening_thread;

    Listening_thread_input *listening_thread_input = init_listening_thread_input_structure(sockfd, packet, &cliaddr,
                                                                                           &cliaddr_len);

    if (pthread_create(&listening_thread, NULL, socket_listening_thread, listening_thread_input)) {
        return -1;
    }

    return listening_thread;
}