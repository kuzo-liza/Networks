#include "main.h"

static pthread_mutex_t list_of_clients_mutex;
static User_info* root = NULL;

//инициализация мьютекса
void init_list_of_clients_mutex() {
    pthread_mutex_init(&list_of_clients_mutex, NULL);
}

//создание нового элемента списка
User_info* make_new_client(int sockfd, int port, char* address, pthread_t client_thread) {
    User_info* new_client = (User_info*) malloc(sizeof(User_info));

    new_client -> port = port;
    new_client -> address = strdup(address);
    new_client -> sockfd = sockfd;
    new_client -> client_thread = client_thread;
    new_client -> next = NULL;
    new_client -> required_index = 1;
    new_client -> last_answer = NULL;
    new_client -> packet_size = 0;

    return new_client;
}


uint32_t list_of_clients_get_required_index(int port, char* address) {

    pthread_mutex_lock(&list_of_clients_mutex);
    User_info *iterator = root;

    while (iterator != NULL && (iterator->port != port || strcmp(iterator->address, address) != 0) ) {
        iterator = iterator->next;
    }

    if (iterator != NULL) {
        pthread_mutex_unlock(&list_of_clients_mutex);
        return iterator->required_index;
    }

    pthread_mutex_unlock(&list_of_clients_mutex);
    return 0;
}


void list_of_clients_set_required_index(int port, char* address) {

    pthread_mutex_lock(&list_of_clients_mutex);
    User_info *iterator = root;

    while (iterator != NULL && (iterator->port != port || strcmp(iterator->address, address) != 0) ) {
        iterator = iterator->next;
    }

    if (iterator != NULL) {
        iterator->required_index = (iterator->required_index + 1 == 65535) ? 1 : iterator->required_index + 1;
    }

    pthread_mutex_unlock(&list_of_clients_mutex);
}


void list_of_clients_set_last_answer(int port, char *address, void *answer, uint32_t packet_size) {

    pthread_mutex_lock(&list_of_clients_mutex);
    User_info *iterator = root;

    while (iterator != NULL && (iterator->port != port || strcmp(iterator->address, address) != 0)) {
        iterator = iterator->next;
    }

    if (iterator != NULL) {
        //очищаем старую память
        free(iterator->last_answer);
        iterator->packet_size = packet_size;
        iterator->last_answer = answer;
    }
    pthread_mutex_unlock(&list_of_clients_mutex);
}


void* list_of_clients_get_last_answer(int port, char* address, uint32_t *packet_size) {

    pthread_mutex_lock(&list_of_clients_mutex);
    User_info *iterator = root;

    while (iterator != NULL && (iterator->port != port || strcmp(iterator->address, address) != 0) ) {
        iterator = iterator->next;
    }

    if (iterator != NULL) {
        *packet_size = iterator->packet_size;
        pthread_mutex_unlock(&list_of_clients_mutex);
        return iterator->last_answer;
    }

    pthread_mutex_unlock(&list_of_clients_mutex);
    return NULL;
}


//удаление элемента списка
//int list_of_clients_remove(int sockfd) {
//    pthread_mutex_lock(&list_of_clients_mutex);
//
//    if (root != NULL) {
//        User_info* iterator = root;
//        User_info* prev = NULL;
//        //поиск элемента
//        while (iterator != NULL && iterator->sockfd != sockfd) {
//            prev = iterator;
//            iterator = iterator->next;
//        }
//        //если элемента нет
//        if (iterator == NULL) {
//            pthread_mutex_unlock(&list_of_clients_mutex);
//            return -1;
//        }
//        //если элемент первый в списке
//        if (prev == NULL) {
//            shutdown(root -> sockfd, SHUT_RDWR);
//            close(root -> sockfd);
//            pthread_join(root->client_thread, NULL);
//
//            User_info* old_root = root;
//            root = root -> next;
//            free(old_root->last_answer);
//            free(old_root);
//
//            pthread_mutex_unlock(&list_of_clients_mutex);
//            return 1;
//        }
//
//        prev -> next = iterator -> next;
//        shutdown(iterator -> sockfd, SHUT_RDWR);
//        close(iterator -> sockfd);
//        pthread_join(iterator->client_thread, NULL);
//        free(iterator->last_answer);
//        free(iterator);
//
//        pthread_mutex_unlock(&list_of_clients_mutex);
//        return 1;
//    }
//
//    pthread_mutex_unlock(&list_of_clients_mutex);
//    return -1;
//}

//добавление нового элемента списка
void list_of_clients_add(User_info* new_client) {
    pthread_mutex_lock(&list_of_clients_mutex);

    if (root == NULL) {
        root = new_client;
        pthread_mutex_unlock(&list_of_clients_mutex);
        return;
    }

    User_info *iterator;
    User_info *iterator_prev = root;

    for (iterator = root; iterator->next != NULL && (new_client->port != iterator->port ||
                          strcmp(new_client->address, iterator->address) != 0);
                          iterator_prev = iterator, iterator = iterator->next) {
    }

    if (iterator != NULL) {
        pthread_mutex_unlock(&list_of_clients_mutex);
        return;
    }

    iterator_prev->next = new_client;

    pthread_mutex_unlock(&list_of_clients_mutex);
}

//экспорт списка
void list_of_clients_export(FILE* dst_fd) {

    pthread_mutex_lock(&list_of_clients_mutex);

    if (root == NULL) {
        fprintf(dst_fd, "No clients\n");
        pthread_mutex_unlock(&list_of_clients_mutex);
        return;
    }
    User_info* iterator = root;

    fprintf(dst_fd,"%5s %8s %16s\n", "index", "port", "address");

    for (int index = 1; iterator != NULL; index++, iterator = iterator->next) {
        fprintf(dst_fd, "%5d %8d %16s\n", index++, iterator->port, iterator->address);
    }

    pthread_mutex_unlock(&list_of_clients_mutex);
}

//очистка всего списка
void list_of_clients_remove_all() {
    pthread_mutex_lock(&list_of_clients_mutex);

    User_info* iterator = root;
    User_info* iterator_next;

    while(iterator != NULL) {
        iterator_next = iterator->next;

        shutdown(iterator -> sockfd, SHUT_RDWR);
        close(iterator -> sockfd);
        pthread_join(iterator->client_thread, NULL); //
        free(iterator->last_answer);
        free(iterator);

        iterator = iterator_next;
    }

    pthread_mutex_unlock(&list_of_clients_mutex);
}