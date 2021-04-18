
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

volatile sig_atomic_t flag = 0;
int sock_fd = -1;
char name[NAME_LEN];

void str_trim_lf(char* arr, int length);
void str_overwrite_stdout(void);

void catch_ctrl_c_and_exit(int sig);

void recv_msg_thread();
void send_msg_handler();

int main(int argc, char ** argv) {
    if (argc != 2 ) {
        printf("Usando: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char * ip = "127.0.0.1";
    int port = atoi(argv[1]);

	signal(SIGINT, catch_ctrl_c_and_exit);
	printf("Digite su nombre: ");
	fgets(name, NAME_LEN, stdin);
	str_trim_lf(name, strlen(name));

	if (strlen(name) > NAME_LEN || strlen(name) < 2) {
		printf("Digite el nombre correctamente\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in serv_addr;

    // Confugurando el socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

	// Conectarse al servidor
	int err = connect(sock_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	if (err == -1) {
		printf("Error: connect\n");
		return EXIT_FAILURE;
	}

	// Enviar el nombre
	send(sock_fd, name, NAME_LEN, 0);
    printf("Welcome\n");

	pthread_t send_msg_thread;
	if (pthread_create(&send_msg_thread, NULL, (void*) send_msg_handler, NULL) != 0) {
		printf("Error: pthread\n");
		return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
	if (pthread_create(&recv_msg_thread, NULL, (void*) recv_msg_thread, NULL) != 0) {
		printf("Error: pthread\n");
		return EXIT_FAILURE;
	}

	while(1) {
		if (flag) {
			printf("\nBye\n");
			break;
		}
	}

	close(sock_fd);
	return EXIT_SUCCESS;
}

void recv_msg_thread() {
	char message[BUFFER_SIZE] = {};
	while(1) {
		int receive = recv(sock_fd, message, BUFFER_SIZE, 0);
		if (receive > 0) {
			printf("%s", message);
			str_overwrite_stdout();
		} else if (receive == 0) {
			break;
		}
		bzero(message, BUFFER_SIZE);
	}
}

void send_msg_handler() {
	char message[BUFFER_SIZE] = {};
	char buffer[BUFFER_SIZE + NAME_LEN + 2] = {};
	
	while(1) {
		str_overwrite_stdout();
		fgets(message, BUFFER_SIZE, stdin);
		str_trim_lf(message, BUFFER_SIZE);

		if (strcmp(message, "exit") == 0) {
			break;
		} else {
			sprintf(buffer, "%s: %s\n", name, message);
			send(sock_fd, buffer, strlen(buffer), 0);
		}
		bzero(buffer, BUFFER_SIZE + NAME_LEN);
		bzero(message, BUFFER_SIZE);
	}
	catch_ctrl_c_and_exit(2);
}

void str_overwrite_stdout(void) {
    printf("%s", "> ");
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

void catch_ctrl_c_and_exit(int sig) {
	flag = 1;
}
