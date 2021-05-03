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

#define BUFFER_SIZE 2048
#define NAME_LEN 32

volatile sig_atomic_t flag = 0;
int sock_fd = 0;
char name[NAME_LEN];

void str_trim_lf(char* arr, int length);
void str_overwrite_stdout(void);

void catch_ctrl_c_and_exit(int sig);

void recv_msg_handler();
void send_msg_handler();

int main(int argc, char ** argv) {

    char * ip;
    int port;
	struct sockaddr_in serv_addr;

    if (argc < 2) {
        fprintf(stderr, "Por lo menos debe especificar la ip del servidor\n");
        exit(EXIT_FAILURE);
    }

    else if (argc > 3) {
        fprintf(stderr, "Demasiados argumentos pasados por consola\n");
        exit(EXIT_FAILURE);
    }

    else if (argc == 2) {
        // puerto por defecto
        port = 12345;
    }

    else if (argc == 3) {
        port = atoi(argv[2]);
    }
   
	signal(SIGINT, catch_ctrl_c_and_exit);
	printf("Digite su nombre: ");
	fgets(name, NAME_LEN, stdin);
	str_trim_lf(name, strlen(name));

	if (strlen(name) > NAME_LEN || strlen(name) < 2) {
		fprintf(stderr, "Digite el nombre correctamente\n");
        exit(EXIT_FAILURE);
	}

    // Confugurando el socket
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error: socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(struct sockaddr_in));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    ip = argv[1];
    if (inet_aton(ip, &serv_addr.sin_addr) == 0) {
        fprintf(stderr, "Direccion NO valida\n");
        exit(EXIT_FAILURE);
    }

	// Conectarse al servidor
	int err = connect(sock_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	if (err == -1) {
        perror("Error: connet");
        exit(EXIT_FAILURE);
	}

	// Enviar el nombre
	send(sock_fd, name, NAME_LEN, 0);
    printf("Bienvenido\n");

	pthread_t send_msg_thread;
	if (pthread_create(&send_msg_thread, NULL, (void*) send_msg_handler, NULL) != 0) {
        perror("Error: pthread send_msg_thread");
        exit(EXIT_FAILURE);
	}

	pthread_t recv_msg_thread;
	if (pthread_create(&recv_msg_thread, NULL, (void*) recv_msg_handler, NULL) != 0) {
        perror("Error: pthread recv_msg_thread");
		return EXIT_FAILURE;
	}

	while(1) {
		if (flag) {
			printf("Adios\n");
			break;
		}
	}

	close(sock_fd);
    exit(EXIT_SUCCESS);
}

void recv_msg_handler() {
	char message[BUFFER_SIZE] = {};
	while(1) {
		int receive = recv(sock_fd, message, BUFFER_SIZE, 0);
		if (receive > 0) {
            if (strncmp(message, "/exit", 5) == 0) {
                printf("Servidor finalizado\n");
                catch_ctrl_c_and_exit(2);
                break;
            }
			printf("%s", message);
			str_overwrite_stdout();
		} else if (receive == 0) {
			break;
		}
        memset(message, 0, BUFFER_SIZE);
	}
}

void send_msg_handler() {
	char message[BUFFER_SIZE] = {};
	char buffer[BUFFER_SIZE + NAME_LEN + 2] = {};
	
	while(1) {
		str_overwrite_stdout();
		fgets(message, BUFFER_SIZE, stdin);
		str_trim_lf(message, BUFFER_SIZE);

		if (strncmp(message, "/exit", 5) == 0) {
			break;
		} else {
			sprintf(buffer, "%s: %s\n", name, message);
			send(sock_fd, buffer, strlen(buffer), 0);
		}
        memset(buffer, 0, BUFFER_SIZE + NAME_LEN + 2);
        memset(message, 0, BUFFER_SIZE);
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
