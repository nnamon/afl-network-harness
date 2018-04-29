#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

struct options_struct {
    char login[36];
    char password[36];
    char server_name[36];
    char status[128];
    char secret_password[256];
};

char * banner = "Please pick an option: login, status, secret, boom, exit\n> ";

void read_until_newline(int fd, char * s, size_t len) {
    char current = 0;
    int index = 0;
    while (current != '\n' && index < len) {
        read(fd, &current, 1);
        s[index] = current;
        ++index;
    }
    s[index-1] = 0;
}

void run() {
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    char sendBuff[1025];
    char recvBuff[1025];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5001);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);

    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    int still_running = 1;
    struct options_struct opts;
    strcpy(opts.server_name, "Testing Server v.1.1.0\n");
    strcpy(opts.status, "Idle");
    strcpy(opts.secret_password, "You_Cannot_Guess_Me_I_Am_Secret_and_Sensitive");

    int banner_len = strlen(banner);
    int authenticated = 0;

    while (still_running) {
        write(connfd, banner, banner_len);
        // Read the option
        read_until_newline(connfd, recvBuff, 10);
        if (strcmp(recvBuff, "exit") == 0) {
            still_running = 0;
            write(connfd, "Exiting!\n", 9);
        }
        else if (strcmp(recvBuff, "secret") == 0) {
            if (!authenticated) {
                write(connfd, "Not authorised!\n", 16);
                continue;
            }

            write(connfd, opts.secret_password, strlen(opts.secret_password));
            write(connfd, "\n", 1);
        }
        else if (strcmp(recvBuff, "boom") == 0) {
            if (!authenticated) {
                write(connfd, "Not authorised!\n", 16);
                continue;
            }
            write(connfd, "Crash!\n", 7);
            *(int *)(0) = 0;
        }
        else if (strcmp(recvBuff, "status") == 0) {
            strcpy(sendBuff, "How long of an answer are you expecting?: ");
            write(connfd, sendBuff, strlen(sendBuff));

            read(connfd, recvBuff, sizeof(recvBuff) - 1);
            size_t size_status = atoi(recvBuff);

            write(connfd, "My status is: ", 14);
            write(connfd, opts.status, size_status);
            write(connfd, "\n", 1);
            strcpy(opts.status, "Statusy");
        }
        else if (strcmp(recvBuff, "login") == 0) {
            strcpy(sendBuff, "Username: ");
            write(connfd, sendBuff, strlen(sendBuff));

            read(connfd, recvBuff, sizeof(recvBuff) - 1);
            if (strncmp(recvBuff, "admin", 5) != 0) {
                write(connfd, "Invalid username!\n", 18);
                continue;
            }

            strcpy(sendBuff, "Password: ");
            write(connfd, sendBuff, strlen(sendBuff));

            read(connfd, recvBuff, sizeof(recvBuff) - 1);
            if (strncmp(recvBuff, "adminpass", 9) != 0) {
                write(connfd, "Invalid password!\n", 18);
                continue;
            }

            write(connfd, "Successfully authenticated.\n", 28);
            authenticated = 1;
            strcpy(opts.status, "Authenticated");
        }
        else {
            write(connfd, "Sorry.\n", 7);
        }
    }
}

int main(int argc, char ** argv) {
    run();
}
