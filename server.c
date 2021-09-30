#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <ncurses.h>

#define MAX_LINE 512
#define BUFSIZE 48

int nFds, nChats = 0;
int player1 = 0;
int player2 = 0;
int listenSocket = 0;
char player1ID[21] =  {'\0',};
char player1Win[5] =  {'\0',};
char player1Lose[5] =  {'\0',};
char player2ID[21] =  {'\0',};
char player2Win[5] =  {'\0',};
char player2Lose[5] =  {'\0',};
bool player1Login = false;
bool player2Login = false;
bool player1Ready = false;
bool player2Ready = false;

void Player1IDReset() {
	for(int i = 0; i < 21; i++)
		player1ID[i] = '\0';
}

void Player1WinReset() {
	player1Win[0] = '0';
	for(int i = 1; i < 5; i++)
		player1Win[i] = '\0';
}

void Player1LoseReset() {
	player1Lose[0] = '0';
	for(int i = 1; i < 5; i++)
		player1Win[i] = '\0';
}

void Player2IDReset() {
	for(int i = 0; i < 21; i++)
		player2ID[i] = '\0';
}

void Player2WinReset() {
	player2Win[0] = '0';
	for(int i = 1; i < 5; i++)
		player2Win[i] = '\0';
}

void Player2LoseReset() {
	player2Lose[0] = '0';
	for(int i = 1; i < 5; i++)
		player2Win[i] = '\0';
}

bool CheckID(char* id) {
	int file;
    char buf[49] = {'\0',};
	file = open("./member.txt", O_CREAT|O_RDONLY, 0644);
	while(read(file, buf, BUFSIZE) > 0) {
        char idTemp[20] =  {'\0',};
		for(int i = 0; i < 20; i++)
			idTemp[i] = buf[i];
		if(strcmp(idTemp, id) == 0) {
			close(file);
			return true;
		}
	}
	close(file);
	return false;
}

bool CheckInformation(char* id, char* pw, int player) {
	int file;
    char idTemp[21] =  {'\0',};
    char pwTemp[21] =  {'\0',};
    char buf[49] = {'\0',};
	file = open("./member.txt", O_CREAT|O_RDONLY, 0644);
	while(read(file, buf, BUFSIZE) > 0) {
		for(int i = 0; i < 20; i++) {
			idTemp[i] = buf[i];
			pwTemp[i] = buf[20 + i];
		}
		if(strcmp(idTemp, id) == 0) {
			if(strcmp(pwTemp, pw) == 0) {
                if(player == 1) {
                    strcpy(player1ID, id);
				    for(int i = 0; i < 4; i++) {
					    player1Win[i] = buf[40 + i];
					    player1Lose[i] = buf[44 + i];
                        }
				} else if(player == 2) {
                    strcpy(player2ID, id);
				    for(int i = 0; i < 4; i++) {
					    player2Win[i] = buf[40 + i];
					    player2Lose[i] = buf[44 + i];
                    }
                }
			    close(file);
			    return true;
            } else {
			    close(file);
			    return false;
            }
		}
	}
	close(file);
	return false;
}

void Join(char* id, char* pw) {
	int file;
    char buf[49] = {'\0',};
	file = open("./member.txt", O_WRONLY|O_APPEND);
	for(int i = 0; i < 20; i++) {
		buf[i] = id[i];
		buf[20 + i] = pw[i];
	}
    buf[40] = '0';
    buf[44] = '0';
	write(file, buf, BUFSIZE);
	close(file);
	return;
}

void Withdrawal(char* id) {
	int file, temp;
    char buf[49] = {'\0',};
    char idTemp[21] = {'\0',};
	file = open("./member.txt", O_RDONLY);
	temp = open("./temp.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
	while(read(file, buf, BUFSIZE) > 0){
		for(int i = 0; i < 20; i++)
			idTemp[i] = buf[i];
		if(strcmp(idTemp, id) != 0)
			write(temp, buf, BUFSIZE);
	}
	close(file);
	close(temp);
	remove("./member.txt");
	rename("./temp.txt", "./member.txt");
}

int tcpListen(int host, int port, int backlog) {
    int sd;
    struct sockaddr_in clntAddr;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    bzero((char*)&clntAddr, sizeof(clntAddr));
    clntAddr.sin_family = AF_INET;
    clntAddr.sin_addr.s_addr = htonl(host);
    clntAddr.sin_port = htons(port);

    bind(sd, (struct sockaddr *)&clntAddr, sizeof(clntAddr));

    listen(sd, backlog);
    return sd;
}

void PlayGame() {
    while(true) {
        char* buf[48];
        recv(player1, buf, 48, 0);
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in clntAddr;
    char buf[MAX_LINE + 1];
    int nBytes, acceptSocket;
    int addrLen = sizeof(struct sockaddr_in);
    fd_set readFds;

    if(argc != 2) {
        printf("Usage : %s PortNumber\n", argv[0]);
        exit(1);
    }

    listenSocket = tcpListen(INADDR_ANY, atoi(argv[1]), 5);

    while(1) {
        FD_ZERO(&readFds);
        FD_SET(listenSocket, &readFds);
        nFds = listenSocket;
        if(player1 != 0) {
            FD_SET(player1, &readFds);
            if(player1 > nFds) {
                nFds = player1;
            }
        }
        if(player2 != 0) {
            FD_SET(player2, &readFds);
            if(player2 > nFds) {
                nFds = player2;
            }
        }
        nFds++;

        select(nFds, &readFds, NULL, NULL, NULL);

        if(FD_ISSET(listenSocket, &readFds)) {
            if(player1 == 0) {
                player1 = accept(listenSocket, (struct sockaddr*)&clntAddr, &addrLen);
                printf("CONNECT: 1\n");
            } else if(player2 == 0) {
                player2 = accept(listenSocket, (struct sockaddr*)&clntAddr, &addrLen);
                printf("CONNECT: 2\n");
            } else {
                listenSocket = 0;
                printf("CONNECT: Player is full\n");
            }
        }

        if(FD_ISSET(player1, &readFds)) {
            if(recv(player1, buf, 48, 0) == 0) {
                printf("UNCONNECT: 1\n");
                close(player1);
                Player1IDReset();
                Player1WinReset();
                Player1LoseReset();
                player1Login = false;
                player1Ready = false;
                player1 = 0;
            } else {
                printf("PLAYER1: %s\n", buf);
                char* work = strtok(buf, ",");
                if(strcmp(work, "join") == 0) {
                    char* idTemp = strtok(NULL, ",");
                    char* pwTemp = strtok(NULL, ",");
                    char id[21] = {'\0',};
                    char pw[21] = {'\0',};
	                for(int i = 0; i < strlen(idTemp); i++) {
		                id[i] = idTemp[i];
	                }
	                for(int i = 0; i < strlen(pwTemp); i++) {
		                pw[i] = pwTemp[i];
	                }

                    if(CheckID(id)) {
                        send(player1, "fail", sizeof("fail"), 0);
                        printf("PLAYER1 JOIN FAILED: %s\n", id);
                    }
                    else {
                        send(player1, "success", sizeof("success"), 0);
                        Join(id, pw);
                        printf("PLAYER1 JOINED: %s\n", id);
                    }
                } else if(strcmp(work, "login") == 0) {
                    char* idTemp = strtok(NULL, ",");
                    char* pwTemp = strtok(NULL, ",");
                    char id[21] = {'\0',};
                    char pw[21] = {'\0',};
	                for(int i = 0; i < strlen(idTemp); i++) {
		                id[i] = idTemp[i];
	                }
	                for(int i = 0; i < strlen(pwTemp); i++) {
		                pw[i] = pwTemp[i];
	                }

                    if(CheckInformation(id, pw, 1) && (strcmp(id, player2ID) != 0)) {
                        char answer[20];
                        sprintf(answer, "%s%s%s%s%s", "success,", player1Win, ",", player1Lose, ",1");
                        send(player1, answer, sizeof(answer), 0);
                        printf("PLAYER1 LOGIN: %s\n", id);
                        player1Login = true;
                    }
                    else {
                        char answer[20];
                        strcpy(answer, "fail,0,0,1");
                        send(player1, answer, sizeof(answer), 0);
                        printf("PLAYER1 LOGIN FAILED: %s\n", id);
                    }
                } else if(strcmp(work, "logout") == 0) {
                    Player1IDReset();
                    Player1WinReset();
                    Player1LoseReset();
                    printf("PLAYER1 LOGOUT\n");
                    player1Login = false;
                } else if(strcmp(work, "withdrawal") == 0) {
                    char* id = strtok(NULL, ",");
                    Withdrawal(id);
                    printf("WITHDRAWAL: %s\n", id);
                } else if(strcmp(work, "ready") == 0) {
                    player1Ready = true;
                    if(player2Ready) {
                        char answer[35];
                        sprintf(answer, "%s%s%s%s%s", player2ID, ",", player2Lose, ",", player2Lose);
                        send(player1, answer, sizeof(answer), 0);
                        char answe[35];
                        sprintf(answe, "%s%s%s%s%s", player1ID, ",", player1Lose, ",", player1Lose);
                        send(player2, answe, sizeof(answer), 0);
                        printf("GAME START!\n");
                        player1Ready = false;
                        player2Ready = false;
                        PlayGame();
                    }
                }
            }
        }

        if(FD_ISSET(player2, &readFds)) {
            nBytes = recv(player2, buf, MAX_LINE, 0);
            if(nBytes == 0) {
                printf("UNCONNECT: 2\n");
                close(player2);
                Player2IDReset();
                Player2WinReset();
                Player2LoseReset();
                player2Login = false;
                player2Ready = false;
                player2 = 0;
            } else {
                printf("PLAYER2: %s\n", buf);
                char* work = strtok(buf, ",");
                if(strcmp(work, "join") == 0) {
                    char* idTemp = strtok(NULL, ",");
                    char* pwTemp = strtok(NULL, ",");
                    char id[21] = {'\0',};
                    char pw[21] = {'\0',};
	                for(int i = 0; i < strlen(idTemp); i++) {
		                id[i] = idTemp[i];
	                }
	                for(int i = 0; i < strlen(pwTemp); i++) {
		                pw[i] = pwTemp[i];
	                }

                    if(CheckID(id)) {
                        send(player2, "fail", sizeof("fail"), 0);
                        printf("PLAYER2 JOIN FAILED: %s\n", id);
                    }
                    else {
                        send(player2, "success", sizeof("success"), 0);
                        Join(id, pw);
                        printf("PLAYER2 JOINED: %s\n", id);
                    }
                } else if(strcmp(work, "login") == 0) {
                    char* idTemp = strtok(NULL, ",");
                    char* pwTemp = strtok(NULL, ",");
                    char id[21] = {'\0',};
                    char pw[21] = {'\0',};
	                for(int i = 0; i < strlen(idTemp); i++) {
		                id[i] = idTemp[i];
	                }
	                for(int i = 0; i < strlen(pwTemp); i++) {
		                pw[i] = pwTemp[i];
	                }

                    if(CheckInformation(id, pw, 2) && (strcmp(id, player1ID) != 0)) {
                        char answer[20];
                        sprintf(answer, "%s%s%s%s%s", "success,", player2Win, ",", player2Lose, ",2");
                        send(player2, answer, sizeof(answer), 0);
                        printf("PLAYER2 LOGIN: %s\n", id);
                        player2Login = true;
                    }
                    else {
                        char answer[20];
                        strcpy(answer, "fail,0,0,1");
                        send(player2, answer, sizeof(answer), 0);
                        printf("PLAYER2 LOGIN FAILED: %s\n", id);
                    }
                } else if(strcmp(work, "logout") == 0) {
                    Player2IDReset();
                    Player2WinReset();
                    Player2LoseReset();
                    printf("PLAYER2 LOGOUT\n");
                    player2Login = false;
                } else if(strcmp(work, "withdrawal") == 0) {
                    char* id = strtok(NULL, ",");
                    Withdrawal(id);
                    printf("WITHDRAWAL: %s\n", id);
                } else if(strcmp(work, "ready") == 0) {
                    player2Ready = true;
                    if(player1Ready) {
                        char answer[35];
                        sprintf(answer, "%s%s%s%s%s", player2ID, ",", player2Lose, ",", player2Lose);
                        send(player1, answer, sizeof(answer), 0);
                        char answe[35];
                        sprintf(answe, "%s%s%s%s%s", player1ID, ",", player1Lose, ",", player1Lose);
                        send(player2, answe, sizeof(answer), 0);
                        printf("GAME START!\n");
                        player1Ready = false;
                        player2Ready = false;
                        PlayGame();
                    }
                }
            }
        }
    }
    return 0;
}