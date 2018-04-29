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

void run() {
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    char sendBuff[1025];
    char recvBuff[1025];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);

    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    strcpy(sendBuff, "Username: ");
    write(connfd, sendBuff, strlen(sendBuff));

    read(connfd, recvBuff, sizeof(recvBuff) - 1);
    if (strncmp(recvBuff, "admin", 5) != 0) {
        write(connfd, "Invalid username!\n", 18);
	return;
    }

    strcpy(sendBuff, "Password: ");
    write(connfd, sendBuff, strlen(sendBuff));

    read(connfd, recvBuff, sizeof(recvBuff) - 1);
    if (strncmp(recvBuff, "adminpass", 9) != 0) {
        write(connfd, "Invalid password!\n", 18);
	return;
    }

    write(connfd, "Successfully authenticated.\n", 28);

    read(connfd, recvBuff, 4);

    if (recvBuff[0] == 'B') {
        if (recvBuff[1] == 'O') {
            if (recvBuff[2] == 'O') {
                if (recvBuff[3] == 'M') {
                    *(int *)(0) = 0;
                }
            }
        }
    }

    write(connfd, "Closed connection\n", 18);

    close(connfd);
}

int main(int argc, char ** argv) {
    run();
}
