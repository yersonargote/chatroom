#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048
#define NAME_LEN 32

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

// Client structure 
typedef struct {
    struct sockaddr_in address;
    int sock_fd;
    int uid;
    char name[NAME_LEN];
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg);

void queue_add(client_t *c);
void queue_remove(int uid);

void send_message(char *s, int uid);

void str_trim_lf(char* arr, int length);
void str_overwrite_stdout(void);
void print_ip_addr(struct sockaddr_in addr);

int main(int argc, char ** argv) {
    if (argc != 2 ) {
        printf("Usando: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char * ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

    pthread_t tid;

    // Confugurando el socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    // Signals
    signal(SIGPIPE, SIG_IGN);

    if ( setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), 
                (char*)&option, sizeof(option)) < 0 ) {
        printf("Error: setsockopt\n");
        return EXIT_FAILURE;
    }

    // Bind
    if ( bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 ) {
        printf("Error: bind\n");
        return EXIT_FAILURE;
    }

    // listen
    if ( listen(listenfd, 10) < 0 ) {
        printf("Error: listen\n");
        return EXIT_FAILURE;
    }

    printf("Welcome\n");

    while(1) {
        socklen_t cli_len = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_len);

        // Check for MAX_CLIENTS
        if ( (cli_count + 1) == MAX_CLIENTS ) {
            printf("Maximo numero de cientes conectados. Conexion rechazada\n");
            print_ip_addr(cli_addr);
            close(connfd);
            continue;
        }

        // Configurando el cliente
        client_t *client = (client_t*) malloc(sizeof(client_t));
        client->address = cli_addr;
        client->sock_fd = connfd;
        client->uid = uid++;

        // Agregar el cliente a la cola
        queue_add(client);
        pthread_create(&tid, NULL, &handle_client, (void*) client);

        // Reducir el uso de la CPU
        sleep(1);
    }

    return EXIT_SUCCESS;
}

void *handle_client(void *arg) {
    char buffer[BUFFER_SIZE];
    char name[NAME_LEN];
    int leave_flag = 0;
    cli_count++;

    client_t *client = (client_t*) arg;

    if (recv(client->sock_fd, name, NAME_LEN, 0) <= 0 
            || strlen(name) < 2 
            || strlen(name) >= NAME_LEN - 1) {
        printf("Digite el nombre correctamene\n");
        leave_flag = 1;
    } else {
        strcpy(client->name, name);
        sprintf(buffer, "%s se ha unido\n", client->name);
        printf("%s", buffer);
        send_message(buffer, client->uid);
    }

    bzero(buffer, BUFFER_SIZE);

    while(1) {
        if(leave_flag) {
            break;
        }

        int receive = recv(client->sock_fd, buffer, BUFFER_SIZE, 0);

        if (receive > 0) {
            if (strlen(buffer) > 0) {
                send_message(buffer, client->uid);
                str_trim_lf(buffer, strlen(buffer));
                printf("%s -> %s\n", buffer, client->name);
            }
        } else if (receive == 0 || strcmp(buffer, "exit") == 0) {
            sprintf(buffer, "%s ha salido\n", client->name);
            printf("%s", buffer);
            send_message(buffer, client->uid);
        } else {
            printf("Error: -1\n");
            leave_flag = 1;
        }
        bzero(buffer, BUFFER_SIZE);
    }
    close(client->sock_fd);
    queue_remove(client->uid);
    free(client);
    cli_count--;
    pthread_detach(pthread_self());
    return NULL;
}

void print_ip_addr(struct sockaddr_in addr) {
    printf("%d.%d.%d.%d", 
            addr.sin_addr.s_addr & 0xff,
            (addr.sin_addr.s_addr & 0xff00) >> 8,
            (addr.sin_addr.s_addr & 0xff0000) >> 16,
            (addr.sin_addr.s_addr & 0xff000000) >> 24
            );
}

void str_overwrite_stdout(void) {
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void queue_add(client_t *client) {
    int i;
    pthread_mutex_lock(&clients_mutex);

    for (i = 0; i < MAX_CLIENTS; i++) {
        if(!clients[i]) {
            clients[i] = client;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void queue_remove(int uid) {
    int i;
    pthread_mutex_lock(&clients_mutex);
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->uid == uid) {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_message(char *s, int uid) {
    int i;
    pthread_mutex_lock(&clients_mutex);
    
    for (i=0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->uid != uid) {
                if (write(clients[i]->sock_fd, s, strlen(s)) < 0) {
                    printf("Error: Write to descriptor failed\n");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}
