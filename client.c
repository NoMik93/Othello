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

int server;
int mode = 0, cursor = 0, key = 0, curX = 0, curY = 0;
int board[6][6]; //-1:X 0:none 1:O
int playerNumber = 0;
int blackNum = 2;
int whiteNum = 2;
char id[21] = {'\0',}; //id, pw 20 limit
char pw[21] = {'\0',};
char pwTemp[21] = {'\0',};
char win[5] = {'\0',}; //win, lose 4 limit
char lose[5] = {'\0',};
char buf[49] = {'\0',}; //id + pw + win + lose
char otherPlayerId[21] = {'\0',};
char otherPlayerwin[5] = "0";
char otherPlayerlose[5] = "0";
bool end  = false, game = false, turn = false;
WINDOW *w1;
WINDOW *w2;

int tcpConnect(int af, char* serverIp, unsigned short port) {
    struct sockaddr_in serverAddr;
    int s = socket(af, SOCK_STREAM, 0);

    bzero((char*)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = af;
    inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    connect(s, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    return s;
}

void BoardReset() {
	for(int i = 0; i < 6; i++)
		for(int j = 0; j < 6; j++)
			board[i][j] = 0;
	board[2][2] = 1;
	board[2][3] = -1;
	board[3][2] = -1;
	board[3][3] = 1;
	curX = 2;
	curY = 2;
}

void IDReset() {
	for(int i = 0; i < 21; i++)
		id[i] = '\0';
}

void PWReset() {
	for(int i = 0; i < 21; i++)
		pw[i] = '\0';
}

void PWTempReset() {
	for(int i = 0; i < 21; i++)
		pwTemp[i] = '\0';
}

void WinReset() {
	win[0] = '0';
	for(int i = 1; i < 5; i++)
		win[i] = '\0';
}

void LoseReset() {
	lose[0] = '0';
	for(int i = 1; i < 5; i++)
		lose[i] = '\0';
}

void BufReset() {
	for(int i = 0; i < 49; i++)
		buf[i] = '\0';
}

void KeyEvent() {
	key = getch();
	switch(mode) {
		case 0:
			switch(key) {
				case KEY_LEFT:
					if(cursor == 0)
						cursor = 2;
					else
						cursor--;
					break;
				case KEY_RIGHT:
					if(cursor == 2)
						cursor = 0;
					else
						cursor++;
					break;
				case 10:
					switch(cursor) {
						case 0:
							mode = 2;
							break;
						case 1:
							mode = 1;
							break;
						case 2:
							end = true;
							break;
					}
					cursor = 0;
					break;
			}
			break;
		case 1:
			switch(key) {
				case KEY_LEFT:
					if(cursor == 2)
						cursor = 3;
					else if(cursor == 3)
						cursor =2;
					break;
				case KEY_RIGHT:
					if(cursor == 2)
						cursor = 3;
					else if(cursor == 3)
						cursor = 2;
					break;
				case 10:
					switch(cursor) {
						case 0:
							cursor = 1;
							break;
						case 1:
							cursor = 2;
							break;
						case 2: {
                            char buf[48];
                            sprintf(buf, "%s%s%s%s", "join,", id, ",", pw);
                            send(server, buf, 48, 0);
							char answer[20] = "";
							recv(server, answer, 20, 0);
							if(strcmp(answer, "fail") == 0) {
								attron(COLOR_PAIR(2));
								move(23, 0);
								printw(">>> ");
								move(23, 4);
								printw(id);
								move(23, 4 + strlen(id));
								printw(" has already exist in DB! (Press any key...)");

								getch();
								IDReset();
								PWReset();
								cursor = 0;
								attrset(A_NORMAL);
							}
							else if(strcmp(answer, "success") == 0) {
								attron(COLOR_PAIR(2));
								move(23, 0);
								printw(">>> Welcon to OTHELLO World! (Press any key...)");

								getch();
								IDReset();
								PWReset();
								mode = 0;
								cursor = 0;
								attrset(A_NORMAL);
							}
							break;
                        }
						case 3:
							IDReset();
							PWReset();
							mode = 0;
							cursor = 0;
							break;
					}
				case 127:
					if(cursor == 0)
						id[strlen(id)-1] = '\0';
					else if(cursor == 1)
						pw[strlen(pw)-1] = '\0';
					break;
				default:
					if(key > 47 && key < 123 && strlen(id) < 20 && strlen(pw) < 20) {
						if(cursor == 0) {
							id[strlen(id)] = (char)key;
							id[strlen(id)] = '\0';
						}
						else if(cursor == 1) {
							pw[strlen(pw)] = (char)key;
							pw[strlen(pw)] = '\0';
						}
					}
					break;
			}
			break;
		case 2:
			switch(key) {
				case KEY_LEFT:
					if(cursor == 2)
						cursor = 3;
					else if(cursor == 3)
						cursor =2;
					break;
				case KEY_RIGHT:
					if(cursor == 2)
						cursor = 3;
					else if(cursor == 3)
						cursor = 2;
					break;
				case 10:
					switch(cursor) {
						case 0:
							cursor = 1;
							break;
						case 1:
							cursor = 2;
							break;
						case 2: {
                            char buf[48];
                            sprintf(buf, "%s%s%s%s", "login,", id, ",", pw);
                            send(server, buf, 48, 0);
							char buffer[20] = "";
							recv(server, buffer, 20, 0);
							char* answer = strtok(buffer, ",");
							char* winTemp = strtok(NULL, ",");
							char* loseTemp = strtok(NULL, ",");
							char* number = strtok(NULL, ",");
							playerNumber = atoi(number);
							if(playerNumber == 1) {
								turn = true;
							}
							if (strcmp(answer, "success") == 0) {
								for(int i = 0; i < strlen(winTemp); i++) {
									win[i] = winTemp[i];
								}
								for(int i = 0; i < strlen(loseTemp); i++) {
									lose[i] = loseTemp[i];
								}
								mode = 3;
								cursor = 0;
							}
							else if(strcmp(answer, "fail") == 0) {
								attron(COLOR_PAIR(2));
								move(23, 0);
								printw(">>> It is Wrong ID or Password! else has been Logined ID (Press any key...)");
								getch();
								IDReset();
								PWReset();
								cursor = 0;
								attrset(A_NORMAL);
							}
							break;
						}
						case 3:
							IDReset();
							PWReset();
							mode = 0;
							cursor = 0;
							break;
					}
					break;
				case 127:
					if(cursor == 0)
						id[strlen(id)-1] = '\0';
					else if(cursor == 1)
						pw[strlen(pw)-1] = '\0';
					break;
				default:
					if(key > 47 && key < 123 && strlen(id) < 20 && strlen(pw) < 20) {
						if(cursor == 0) {
							id[strlen(id)] = (char)key;
						}
						else if(cursor == 1) {
							pw[strlen(pw)] = (char)key;
						}
					}
					break;
			}
			break;
		case 3:
			switch(key) {
				case KEY_LEFT:
					if(cursor == 0)
						cursor = 3;
					else
						cursor--;
					break;
				case KEY_RIGHT:
					if(cursor == 2)
						cursor = 0;
					else
						cursor++;
					break;
				case 10:
					switch(cursor) {
						case 0:
							mode = 5;
							cursor = 0;
							break;
						case 1: {
                            char buf[48];
                            sprintf(buf, "%s%s%s%s", "logout,", id, ",", pw);
                            send(server, buf, 48, 0);
							IDReset();
							PWReset();
							mode = 2;
							cursor = 0;
							break;
						}
						case 2:
							mode = 4;
							cursor = 0;
							break;
					}
			}
			break;
		case 4:
			switch(key) {
				case KEY_LEFT:
					if(cursor == 1)
						cursor = 2;
					else if(cursor == 2)
						cursor =1;
					break;
				case KEY_RIGHT:
					if(cursor == 1)
						cursor = 2;
					else if(cursor == 2)
						cursor = 1;
					break;
				case 10:
					switch(cursor) {
						case 0:
							cursor = 1;
							break;
						case 1: {
							if(strcmp(pw, pwTemp) == 0) {
                                char *buf;
                                sprintf(buf, "%s%s", "withdrawal,", id);
                                send(server, buf, 30, 0);
								attron(COLOR_PAIR(2));
								move(23, 0);
								printw(">>> Withdrawal Success! Good Bye! (Press any key...)");
								getch();

								IDReset();
								PWReset();
								PWTempReset();
								mode = 0;
								cursor = 0;
							}
							else {
								attron(COLOR_PAIR(2));
								move(23, 0);
								printw(">>> It is Wrong Password! (Press any key...)");
								getch();

								PWTempReset();
								cursor = 0;
							}
							break;
							}
						case 2:
							mode = 3;
							cursor = 0;
							break;
					}
				case 127:
					if(cursor == 0)
						pwTemp[strlen(pwTemp)-1] = '\0';
					break;
				default:
					if(key > 47 && key < 123 && strlen(pwTemp) < 20) {
						if(cursor == 0) {
							pwTemp[strlen(pwTemp)] = (char)key;
							pwTemp[strlen(pwTemp)] = '\0';
						}
					}
					break;
			}
			break;
		case 5:
			switch(key) {
				case 10:
					mode = 6;
					cursor = 0;
					game = true;
					BoardReset();
			}
			break;
		case 6:
			if(game) {
				switch(key) {
					case KEY_LEFT:
						if(curX == 0)
							curX = 5;
						else
							curX--;
						break;
					case KEY_UP:
						if(curY == 0)
							curY = 5;
						else
							curY--;
						break;
					case KEY_DOWN:
						if(curY == 5)
							curY = 0;
						else
							curY++;
						break;
					case KEY_RIGHT:
						if(curX == 5)
							curX = 0;
						else
							curX++;
						break;
					case 78:
					case 110:
						game = false;
						cursor = 0;
						break;
					case 82:
					case 114:
						game = false;
						cursor = 1;
						break;
					case 88:
					case 120:
						game = false;
						cursor = 2;
						break;
				}
			}
			else {
				switch(key) {
					case KEY_UP:
						if(cursor == 0)
							cursor = 2;
						else
							cursor--;
						break;
					case KEY_DOWN:
						if(cursor == 2)
							cursor = 0;
						else
							cursor++;
						break;
					case 103:
					case 71:
						game = true;
						break;
					case 10:
						switch(cursor) {
							case 0:
								turn = !turn;
								break;
							case 1:
								BoardReset();
								break;
							case 2:
								mode = 3;
								cursor = 0;
								break;
						}
						break;
				}
			}
			break;
	}
}

void SetDisplayVer() {
	w1 = newwin(18, 80, 0, 0);
	w2 = newwin(6, 80, 18, 0);

	wbkgd(w1, COLOR_PAIR(1));
	wbkgd(w2, COLOR_PAIR(2));

	keypad(w1, TRUE);
	keypad(w2, TRUE);
}

void SetDisplayHor() {
	w1 = newwin(24, 60, 0, 0);
	w2 = newwin(24, 20, 0, 60);

	wbkgd(w1, COLOR_PAIR(1));
	wbkgd(w2, COLOR_PAIR(2));

	keypad(w1, TRUE);
	keypad(w2, TRUE);
}

void Display() {
	SetDisplayVer();
	switch(mode) {
		case 0:
			mvwprintw(w1, 8, 28,"System Software Practice");
			attron(COLOR_PAIR(3));
			move(10, 36);
			printw("OTHELLO");
			mvwprintw(w1, 14, 69, "2013726048");
			mvwprintw(w1, 16, 69, "JaeWon Kim");

			mvwprintw(w2, 3, 15, "SIGN IN");
			mvwprintw(w2, 3, 35, "SIGN UP");
			mvwprintw(w2, 3, 57, "EXIT");

			attron(COLOR_PAIR(2)|A_UNDERLINE|A_REVERSE);
			switch(cursor) {
				case 0:
					move(21, 15);
					printw("SIGN IN");
					break;
				case 1:
					move(21, 35);
					printw("SIGN UP");
					break;
				case 2:
					move(21, 57);
					printw("EXIT");
					break;
			}
			break;
		case 1:
			attron(COLOR_PAIR(3));
			move(8, 36);
			printw("SIGN UP");

			mvwprintw(w1, 10, 31, "ID : ");
			mvwprintw(w1, 10, 36, id);
			mvwprintw(w1, 12, 25, "PASSWORD : ");
			for(int i = 0; i < strlen(pw); i++)
				mvwprintw(w1, 12, 36 + i, "*");

			mvwprintw(w2, 3, 16, "SIGN UP");
			mvwprintw(w2, 3, 57, "BACK");

			attron(COLOR_PAIR(2));
			switch(cursor) {
				case 0:
					move(10, 36 + strlen(id));
					printw(" ");
					break;
				case 1:
					move(12, 36 + strlen(pw));
					printw(" ");
					break;
				case 2:
					attron(A_UNDERLINE|A_REVERSE);
					move(21, 16);
					printw("SIGN UP");
					break;
				case 3:
					attron(A_UNDERLINE|A_REVERSE);
					move(21, 57);
					printw("BACK");
					break;
			}
			break;
		case 2:
			attron(COLOR_PAIR(3));
			move(8, 36);
			printw("SIGN IN");

			mvwprintw(w1, 10, 31, "ID : ");
			mvwprintw(w1, 10, 36, id);
			mvwprintw(w1, 12, 25, "PASSWORD : ");
			for(int i = 0; i < strlen(pw); i++)
				mvwprintw(w1, 12, 36 + i, "*");

			mvwprintw(w2, 3, 16, "SIGN IN");
			mvwprintw(w2, 3, 57, "BACK");

			attron(COLOR_PAIR(2));
			switch(cursor) {
				case 0:
					move(10, 36 + strlen(id));
					printw(" ");
					break;
				case 1:
					move(12, 36 + strlen(pw));
					printw(" ");
					break;
				case 2:
					attron(A_UNDERLINE|A_REVERSE);
					move(21, 16);
					printw("SIGN IN");
					break;
				case 3:
					attron(A_UNDERLINE|A_REVERSE);
					move(21, 57);
					printw("BACK");
					break;
			}
			break;
		case 3:
			mvwprintw(w1, 8, 29, "PLAYER ID : ");
			mvwprintw(w1, 8, 41, id);

			mvwprintw(w2, 3, 17, "PLAY");
			mvwprintw(w2, 3, 35, "SIGN OUT");
			mvwprintw(w2, 3, 55, "WITHDRAWAL");

			attron(COLOR_PAIR(2)|A_UNDERLINE|A_REVERSE);
			switch(cursor) {
				case 0:
					move(21, 17);
					printw("PLAY");
					break;
				case 1:
					move(21, 35);
					printw("SIGN OUT");
					break;
				case 2:
					move(21, 55);
					printw("WITHDRAWAL");
					break;
			}
			break;
		case 4:
			attron(COLOR_PAIR(3));
			move(8, 34);
			printw("WITHDRAWAL");

			mvwprintw(w1, 10, 31, "ID : ");
			mvwprintw(w1, 10, 36, id);
			mvwprintw(w1, 12, 25, "PASSWORD : ");
			mvwprintw(w1, 12, 36, pwTemp);

			mvwprintw(w2, 3, 16, "WITHDRAWAL");
			mvwprintw(w2, 3, 57, "BACK");

			attron(COLOR_PAIR(2));
			switch(cursor) {
				case 0:
					move(12, 36 + strlen(pwTemp));
					printw(" ");
					break;
				case 1:
					attron(A_UNDERLINE|A_REVERSE);
					move(21, 16);
					printw("WITHDRAWAL");
					break;
				case 2:
					attron(A_UNDERLINE|A_REVERSE);
					move(21, 57);
					printw("BACK");
					break;
			}
			break;
		case 5:
			if(playerNumber == 1) {
				attron(COLOR_PAIR(3));
				mvwprintw(w1, 6, 7, "PLAYER1 ID : ");
				mvwprintw(w1, 6, 20, id);
				mvwprintw(w1, 8, 15, "STATISTICS");
				mvwprintw(w1, 10, 3, "WIN : ");
				mvwprintw(w1, 10, 9, win);
				mvwprintw(w1, 10, 13, " / LOSE : ");
				mvwprintw(w1, 10, 23, lose);
				double winRate = 100;
				if(((int)atoi(win) + (int)atoi(lose)) > 0)
					winRate = (double)atoi(win) / (double)(atoi(win) + atoi(lose)) * 100;
				char rate[10];
				sprintf(rate, "(%.3f)", winRate);
				mvwprintw(w1, 10, 27, rate);
				mvwprintw(w1, 6, 47, "PLAYER2 ID : ");
				mvwprintw(w1, 6, 60, otherPlayerId);
				mvwprintw(w1, 8, 55, "STATISTICS");
				mvwprintw(w1, 10, 43, "WIN : ");
				mvwprintw(w1, 10, 49, otherPlayerwin);
				mvwprintw(w1, 10, 53, " / LOSE : ");
				mvwprintw(w1, 10, 63, otherPlayerlose);
				winRate = 100;
				if(((int)atoi(otherPlayerwin) + (int)atoi(otherPlayerlose)) > 0)
					winRate = (double)atoi(otherPlayerwin) / (double)(atoi(otherPlayerwin) + atoi(otherPlayerlose)) * 100;
				sprintf(rate, "(%.3f)", winRate);
				mvwprintw(w1, 10, 67, rate);
			} else if(playerNumber == 2) {
				attron(COLOR_PAIR(3));
				mvwprintw(w1, 6, 7, "PLAYER1 ID : ");
				mvwprintw(w1, 6, 20, otherPlayerId);
				mvwprintw(w1, 8, 15, "STATISTICS");
				mvwprintw(w1, 10, 3, "WIN : ");
				mvwprintw(w1, 10, 9, otherPlayerwin);
				mvwprintw(w1, 10, 13, " / LOSE : ");
				mvwprintw(w1, 10, 23, otherPlayerlose);
				double winRate = 100;
				if(((int)atoi(otherPlayerwin) + (int)atoi(otherPlayerlose)) > 0)
					winRate = (double)atoi(otherPlayerwin) / (double)(atoi(otherPlayerwin) + atoi(otherPlayerlose)) * 100;
				char rate[10];
				sprintf(rate, "(%.3f)", winRate);
				mvwprintw(w1, 6, 47, "PLAYER2 ID : ");
				mvwprintw(w1, 6, 60, id);
				mvwprintw(w1, 8, 55, "STATISTICS");
				mvwprintw(w1, 10, 43, "WIN : ");
				mvwprintw(w1, 10, 49, win);
				mvwprintw(w1, 10, 53, " / LOSE : ");
				mvwprintw(w1, 10, 63, lose);
				winRate = 100;
				if(((int)atoi(win) + (int)atoi(lose)) > 0)
					winRate = (double)atoi(win) / (double)(atoi(win) + atoi(lose)) * 100;
				sprintf(rate, "(%.3f)", winRate);
				mvwprintw(w1, 10, 67, rate);
			}
			mvwprintw(w2, 3, 36, "WAITING");
			
			wrefresh(w1);
			wrefresh(w2);
			attrset(A_NORMAL);

            char buf[20] = "ready";
            send(server, buf, 20, 0);
			char buffer[35] = "";
			recv(server, buffer, 35, 0);
			char* idBuf = strtok(buffer, ",");
			char* winBuf = strtok(NULL, ",");
			char* loseBuf = strtok(NULL, ",");
			for(int i = 0; i < strlen(idBuf); i++) {
				otherPlayerId[i] = idBuf[i];
			}
			for(int i = 0; i < strlen(winBuf); i++) {
				otherPlayerwin[i] = winBuf[i];
			}
			for(int i = 0; i < strlen(loseBuf); i++) {
				otherPlayerlose[i] = loseBuf[i];
			}
			
			SetDisplayVer();
			if(playerNumber == 1) {
				attron(COLOR_PAIR(3));
				mvwprintw(w1, 6, 7, "PLAYER1 ID : ");
				mvwprintw(w1, 6, 20, id);
				mvwprintw(w1, 8, 15, "STATISTICS");
				mvwprintw(w1, 10, 3, "WIN : ");
				mvwprintw(w1, 10, 9, win);
				mvwprintw(w1, 10, 13, " / LOSE : ");
				mvwprintw(w1, 10, 23, lose);
				double winRate = 100;
				if(((int)atoi(win) + (int)atoi(lose)) > 0)
					winRate = (double)atoi(win) / (double)(atoi(win) + atoi(lose)) * 100;
				char rate[10];
				sprintf(rate, "(%.3f)", winRate);
				mvwprintw(w1, 10, 27, rate);
				mvwprintw(w1, 6, 47, "PLAYER2 ID : ");
				mvwprintw(w1, 6, 60, otherPlayerId);
				mvwprintw(w1, 8, 55, "STATISTICS");
				mvwprintw(w1, 10, 43, "WIN : ");
				mvwprintw(w1, 10, 49, otherPlayerwin);
				mvwprintw(w1, 10, 53, " / LOSE : ");
				mvwprintw(w1, 10, 63, otherPlayerlose);
				winRate = 100;
				if(((int)atoi(otherPlayerwin) + (int)atoi(otherPlayerlose)) > 0)
					winRate = (double)atoi(otherPlayerwin) / (double)(atoi(otherPlayerwin) + atoi(otherPlayerlose)) * 100;
				sprintf(rate, "(%.3f)", winRate);
				mvwprintw(w1, 10, 67, rate);
			} else if(playerNumber == 2) {
				attron(COLOR_PAIR(3));
				mvwprintw(w1, 6, 7, "PLAYER1 ID : ");
				mvwprintw(w1, 6, 20, otherPlayerId);
				mvwprintw(w1, 8, 15, "STATISTICS");
				mvwprintw(w1, 10, 3, "WIN : ");
				mvwprintw(w1, 10, 9, otherPlayerwin);
				mvwprintw(w1, 10, 13, " / LOSE : ");
				mvwprintw(w1, 10, 23, otherPlayerlose);
				double winRate = 100;
				if(((int)atoi(otherPlayerwin) + (int)atoi(otherPlayerlose)) > 0)
					winRate = (double)atoi(otherPlayerwin) / (double)(atoi(otherPlayerwin) + atoi(otherPlayerlose)) * 100;
				char rate[10];
				sprintf(rate, "(%.3f)", winRate);
				mvwprintw(w1, 6, 47, "PLAYER2 ID : ");
				mvwprintw(w1, 6, 60, id);
				mvwprintw(w1, 8, 55, "STATISTICS");
				mvwprintw(w1, 10, 43, "WIN : ");
				mvwprintw(w1, 10, 49, win);
				mvwprintw(w1, 10, 53, " / LOSE : ");
				mvwprintw(w1, 10, 63, lose);
				winRate = 100;
				if(((int)atoi(win) + (int)atoi(lose)) > 0)
					winRate = (double)atoi(win) / (double)(atoi(win) + atoi(lose)) * 100;
				sprintf(rate, "(%.3f)", winRate);
				mvwprintw(w1, 10, 67, rate);
			}
			attron(COLOR_PAIR(2));
			switch(cursor) {
				case 0:
					attron(A_UNDERLINE|A_REVERSE);
					move(21, 39);
					printw("OK");
					break;
			}
			break;
		case 6:
			SetDisplayHor();
			attron(COLOR_PAIR(1));
			for(int i = 0; i < 7; i++) {
				move(5 + (2 * i), 17);
				printw("+---+---+---+---+---+---+");
			}
			for(int i = 0; i < 6; i++) {
				move(6 + (2 * i), 17);
				printw("|   |   |   |   |   |   |");
			}
			for(int i = 0; i < 6; i++)
				for(int j = 0; j < 6; j++) {
					if(board[i][j] < 0) {
						move(6 + (2 * j), 19 + (4 * i));
						printw("X");
					}
					else if(board[i][j] > 0) {
						move(6 + (2 * j), 19 + (4 * i));
						printw("O");
					}
				}
			attron(A_REVERSE);
			move(6 + (2 * curY), 18 + (4 * curX));
			if(board[curX][curY] < 0)
				printw(" X ");
			else if(board[curX][curY] == 0)
				printw("   ");
			else if(board[curX][curY] > 0)
				printw(" O ");
			attroff(A_REVERSE);
			
			if(playerNumber == 1) {
				mvwprintw(w2, 6, 3, id);
				mvwprintw(w2, 6, 3 + strlen(id), "(O) : ");
				char num[3];
				sprintf(num, "%d", blackNum);
				mvwprintw(w2, 6, 9 + strlen(id), num);
				mvwprintw(w2, 7, 3, otherPlayerId);
				mvwprintw(w2, 7, 3 + strlen(otherPlayerId), "(X) : ");
				sprintf(num, "%d", whiteNum);
				mvwprintw(w2, 7, 9 + strlen(otherPlayerId), num);
			} else if(playerNumber == 2) {
				mvwprintw(w2, 6, 3, otherPlayerId);
				mvwprintw(w2, 6, 3 + strlen(otherPlayerId), "(O) : ");
				char num[3];
				sprintf(num, "%d", blackNum);
				mvwprintw(w2, 6, 9 + strlen(otherPlayerId), num);
				mvwprintw(w2, 7, 3, id);
				mvwprintw(w2, 7, 3 + strlen(id), "(X) : ");
				sprintf(num, "%d", whiteNum);
				mvwprintw(w2, 7, 9 + strlen(id), num);
			} 

			if(playerNumber == 1 && turn) {
				attron(COLOR_PAIR(2)|A_REVERSE);
				move(6, 63);
				printw(id);
				move(6, 63 + strlen(id));
				printw("(O)");
			} else if(playerNumber == 1 && !turn) {
				attron(COLOR_PAIR(2)|A_REVERSE);
				move(7, 60);
				move(7, 63);
				printw(otherPlayerId);
				move(7, 63 + strlen(otherPlayerId));
				printw("(X)");
			} else if(playerNumber == 2 && turn) {
				attron(COLOR_PAIR(2)|A_REVERSE);
				move(7, 60);
				move(7, 63);
				printw(id);
				move(7, 63 + strlen(id));
				printw("(X)");
			} else if(playerNumber == 2 && !turn) {
				attron(COLOR_PAIR(2)|A_REVERSE);
				move(6, 60);
				move(6, 63);
				printw(otherPlayerId);
				move(6, 63 + strlen(otherPlayerId));
				printw("(O)");
			}

			attrset(A_NORMAL);
			attrset(COLOR_PAIR(2)|A_UNDERLINE);
			mvwprintw(w2, 12, 7, " EXT TURN");
			mvwprintw(w2, 14, 7, " EGAME");
			mvwprintw(w2, 16, 7, "E IT");
			mvaddch(12, 67, 'N');
			move(14, 67);
			addch('R'|A_UNDERLINE);
			move(16, 68);
			addch('X'|A_UNDERLINE);

			if(!game) {
				attrset(A_NORMAL);
				attron(COLOR_PAIR(2)|A_UNDERLINE|A_REVERSE);
				if(cursor == 0) {
					move(12, 67);
					printw("NEXT TURN");
				} else if(cursor == 1) {
					move(14, 67);
					printw("REGAME");
				} else if(cursor == 2) {
					move(16, 67);
					printw("EXIT");
				}
			}
			break;
	}
	wrefresh(w1);
	wrefresh(w2);
	attrset(A_NORMAL);
}

int main(int argc, char *argv[])
{
    if(argc != 3){
        printf("Usage : %s IPAddress PortNumber", argv[0]);
        exit(1);
    }

	IDReset();
	PWReset();
	PWTempReset();
	WinReset();
	LoseReset();
	BufReset();

    server = tcpConnect(AF_INET, argv[1], atoi(argv[2]));

	initscr();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);

	start_color();
	init_color(COLOR_BLUE, 25, 400, 720);
	init_color(COLOR_WHITE, 992, 1000, 956);
	init_color(COLOR_CYAN, 200, 15, 720);
	init_pair(1, COLOR_BLUE, COLOR_WHITE);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	init_pair(3, COLOR_CYAN, COLOR_WHITE);

	refresh();


	while(!end) {
		Display();
		KeyEvent();
	}

	endwin();
    close(server);
    return 0;
}
