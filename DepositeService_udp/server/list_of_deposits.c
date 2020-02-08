#include "main.h"

static pthread_mutex_t list_of_deposits_mutex;
static Deposit_info* root = NULL;
static double bank_amount = 0;

//инициализация мьютекса
void init_list_of_deposits_mutex() {
    pthread_mutex_init(&list_of_deposits_mutex, NULL);
}

double list_of_deposits_get_bank_amount() {
    return bank_amount;
}

//создание нового элемента списка
Deposit_info* make_new_deposit(int deposit_id, int initial_amount,int client_sockfd, struct sockaddr_in cliaddr) {
    Deposit_info* new_deposit = (Deposit_info*) malloc(sizeof(Deposit_info));

    new_deposit->client_sockfd = client_sockfd;
    new_deposit->current_amount = (double) initial_amount;
    new_deposit->initial_amount = initial_amount;
    new_deposit->deposit_id = deposit_id;
    new_deposit->cliaddr = cliaddr;
    new_deposit->next = NULL;

    return new_deposit;
}


uint32_t generate_deposit_id() {

    pthread_mutex_lock(&list_of_deposits_mutex);
    uint32_t deposit_id = 1;

    for (Deposit_info* iterator = root; iterator != NULL; iterator = iterator->next) {
        deposit_id = iterator->deposit_id + 1;
    }

    pthread_mutex_unlock(&list_of_deposits_mutex);
    return deposit_id;
}

//добавление элемента в список
void list_of_deposits_add(Deposit_info* new_deposit) {
    pthread_mutex_lock(&list_of_deposits_mutex);

    Deposit_info* iterator = root;

    while (iterator != NULL && iterator->next != NULL) {
        iterator = iterator->next;
    }

    if (iterator == NULL) {
        root = new_deposit;
        pthread_mutex_unlock(&list_of_deposits_mutex);
        return;
    }

    iterator->next = new_deposit;

    pthread_mutex_unlock(&list_of_deposits_mutex);
    //printf("DEPOSIT", ADD_DEPOSIT, id, amount);
}

//удаление
//возвращает накопленную сумму клиента
int32_t list_of_deposits_remove(int port, const char *address, uint32_t deposit_id) {

    pthread_mutex_lock(&list_of_deposits_mutex);
    Deposit_info* iterator = root;
    Deposit_info* previous =  NULL;

    while (iterator != NULL && iterator->deposit_id != deposit_id) {
        previous = iterator;
        iterator = iterator->next;
    }

    if (iterator == NULL || iterator->cliaddr.sin_port != port || inet_ntoa(iterator->cliaddr.sin_addr) != address) {
        pthread_mutex_unlock(&list_of_deposits_mutex);
        return -1;
    }

    //если удаляемый вклад первый в списке
    if (iterator == root){
        Deposit_info* old_root = root;
        root = root->next;
        free(old_root);
        pthread_mutex_unlock(&list_of_deposits_mutex);
        return deposit_id;
    }

    previous->next = iterator->next;
    free(iterator);

    pthread_mutex_unlock(&list_of_deposits_mutex);
    return deposit_id;
}

// добавить проценты всем вкладам
void list_of_deposits_add_percents(){
    pthread_mutex_lock(&list_of_deposits_mutex);

    Deposit_info* iterator = root;

    while (iterator != NULL) {
        bank_amount +=  iterator->current_amount * 0.05;
        iterator->current_amount +=  iterator->current_amount * 0.1;
        iterator = iterator -> next;
    }

    pthread_mutex_unlock(&list_of_deposits_mutex);
}

//подсчёт общей длины следующих параметров всех элементов
//static uint64_t count_list_length(int client_sockfd) {
//    uint64_t result;
//    Deposit_info* iterator;
//
//    for (iterator = root,  result = 0; iterator != NULL ; iterator = iterator->next) {
//        if(client_sockfd == iterator->client_sockfd){
//            result += (SIZE_OF_ID_DEPOSIT + SIZE_OF_INITIAL_AMOUNT + SIZE_OF_CURRENT_AMOUNT);
//        }
//    }
//
//    return result;
//}

// получить вклады определенного клиента
//void list_of_deposits_send(int client_sockfd) {
//
//    uint32_t packet_length = SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + count_list_length(client_sockfd);
//    uint16_t packet_type = LIST_OF_DEPOSITS_PACKET;
//
//    //отправка длины и типа пакета
//    write(client_sockfd, &packet_length, SIZE_OF_PACKET_LENGTH);
//    write(client_sockfd, &packet_type, SIZE_OF_PACKET_TYPE);
//
//    //отправка
//    for (Deposit_info* iterator = root; iterator != NULL; iterator = iterator->next) {
//        if(client_sockfd == iterator->client_sockfd){
//            write(client_sockfd, &(iterator->deposit_id), SIZE_OF_ID_DEPOSIT);
//            write(client_sockfd, &(iterator->initial_amount), SIZE_OF_INITIAL_AMOUNT);
//            write(client_sockfd, &(iterator->current_amount), SIZE_OF_CURRENT_AMOUNT);
//        }
//    }
//}

//очистка всего списка
void list_of_deposits_remove_all() {
    Deposit_info* iterator;
    Deposit_info* iterator_next;

    pthread_mutex_lock(&list_of_deposits_mutex);

    for (iterator = root; iterator != NULL ; iterator = iterator_next) {
        iterator_next = iterator->next;
        free(iterator);
    }

    pthread_mutex_unlock(&list_of_deposits_mutex);
}

void list_of_deposits_all_deposits(FILE* output_file){
    Deposit_info* iterator;

    pthread_mutex_lock(&list_of_deposits_mutex);

    if (root == NULL) {
        fprintf(output_file, "No deposits\n");
        pthread_mutex_unlock(&list_of_deposits_mutex);
        return;
    }

    fprintf(output_file, "%17s %6s %10s %14s %14s\n", "address", "port", "deposit_id", "initial_amount",
            "current_amount");

    for (iterator = root; iterator != NULL; iterator = iterator->next) {
        fprintf(output_file, "%17s %6d %10d %14d %14f\n", inet_ntoa(iterator->cliaddr.sin_addr),
                iterator->cliaddr.sin_port, iterator->deposit_id,
                iterator->initial_amount, iterator->current_amount);
    }

    pthread_mutex_unlock(&list_of_deposits_mutex);
}

void list_of_deposits_export_bank_amount() {
    printf("Bank amount: %f\n", bank_amount);
}

//пополнение вклада
double list_of_deposits_refill_deposit(int port, const char *address, uint32_t deposit_id, uint32_t added_amount) {

    pthread_mutex_lock(&list_of_deposits_mutex);

    Deposit_info *iterator = root;

    while (iterator != NULL && iterator->deposit_id != deposit_id) {
        iterator = iterator->next;
    }

    if (iterator == NULL || iterator->cliaddr.sin_port != port || inet_ntoa(iterator->cliaddr.sin_addr) != address) {
        pthread_mutex_unlock(&list_of_deposits_mutex);
        return -1;
    }

    iterator->current_amount += added_amount;

    pthread_mutex_unlock(&list_of_deposits_mutex);
    return iterator->current_amount;
}

int add_percents_to_client(int port, const char *address){
    pthread_mutex_lock(&list_of_deposits_mutex);

    int client_has_deposits = -1;
    Deposit_info* iterator = root;

    while (iterator != NULL) {
        if (iterator->cliaddr.sin_port == port && strcmp(inet_ntoa(iterator->cliaddr.sin_addr), address) == 0 ){
            bank_amount +=  iterator->current_amount * 0.05;
            iterator->current_amount +=  iterator->current_amount * 0.1;
            client_has_deposits = 1;
        }
        iterator = iterator->next;
    }
    pthread_mutex_unlock(&list_of_deposits_mutex);
    return client_has_deposits;
}

int remove_client_deposits(int port, const char *address){

    pthread_mutex_lock(&list_of_deposits_mutex);
    Deposit_info* iterator = root;
    Deposit_info* previous =  NULL;

    while (iterator != NULL) {
        if (iterator->cliaddr.sin_port == port && strcmp(inet_ntoa(iterator->cliaddr.sin_addr), address) == 0){
            if (iterator == root){
                Deposit_info* old_root = root;
                root = root->next;
                free(old_root);
            } else {
                previous->next = iterator->next;
                free(iterator);
            }

        }

        previous = iterator;
        iterator = iterator->next;
    }

    pthread_mutex_unlock(&list_of_deposits_mutex);
    return 1;
}

// у данного пользователя есть депозит
Deposit_info *list_of_deposits_get_deposit(int port, const char *address, int number, int *is_deposit_last) {
    pthread_mutex_lock(&list_of_deposits_mutex);

    Deposit_info *iterator = root;
    Deposit_info *result;

    while (iterator != NULL && number != 0) {
        if (iterator->cliaddr.sin_port == port && inet_ntoa(iterator->cliaddr.sin_addr) == address) {
            number--;
        }

        if (number != 0) {
            iterator = iterator->next;
        }
    }

    result = iterator;

    // определение, является ли депозит последним
    *is_deposit_last = 1;
    while (iterator != NULL) {
        iterator = iterator->next;
        if (iterator != NULL && iterator->cliaddr.sin_port == port && inet_ntoa(iterator->cliaddr.sin_addr) == address) {
            *is_deposit_last = -1;
        }
    }


    pthread_mutex_unlock(&list_of_deposits_mutex);
    return result;
}