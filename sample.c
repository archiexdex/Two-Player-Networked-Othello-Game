#include "othello.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>


static int width;
static int height;
static int cx = 3;
static int cy = 3;


void restart();
void update(int, int, int);
void modify(int*, int*, int);
bool checkIO(int);


char buf[1024];

int main(int argc, char* argv[]) {

	int isS = false;
	char *port = NULL;
	char *ip = NULL;
	if (argc != 3) return 0;
	if ( !strcmp(argv[1],"-s") ){
		isS = true;
		port = argv[2];
	}
	else if( !strcmp(argv[1],"-c") ) {
		isS = false;
		char *tmp = argv[2];
		char *ptr = strtok(tmp,":");
		ip = ptr;
		ptr = strtok(NULL,":");
		port = ptr;
		printf("IP %s, port %s\n",ip, port);
	}
	

	int sockfd, clientfd, fd;
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( sockfd < 0 ){
		printf("create socket fail\n");
		return 0;
	}

	struct sockaddr_in dest, client;
	bzero(&dest,sizeof(dest));
	dest.sin_family = PF_INET;
	dest.sin_port = htons(atoi(port) );
	dest.sin_addr.s_addr = isS? INADDR_ANY : inet_addr(ip);

	if (isS) {
		if ( bind(sockfd, (struct sockaddr *)&dest, sizeof(dest)) < 0 ) {
			printf("can't bind server. QQ\n");
			return 0;
		}
		listen(sockfd, 5);
		printf("server listen on port %s\n", port);
		socklen_t client_len = sizeof( client );
		clientfd = accept(sockfd, (struct sockaddr*)&client, &client_len );

		if ( clientfd < 0 ){
			printf("No client connect. OAO\n");
			return 0;
		}
		
	}
	else{
		int c = connect(sockfd, (struct sockaddr*)&dest, sizeof(dest) );
		if ( c < 0 ) {
			printf("No server to connect. QAQ\n");
			return 0;
		}
	}
	
	fd = isS? clientfd : sockfd;

	initscr();			// start curses mode 
	getmaxyx(stdscr, height, width);// get screen size

	cbreak();			// disable buffering
						// - use raw() to disable Ctrl-Z and Ctrl-C as well,
	halfdelay(1);		// non-blocking getch after n * 1/10 seconds
	noecho();			// disable echo
	keypad(stdscr, TRUE);		// enable function keys and arrow keys
	curs_set(0);			// hide the cursor

	init_colors();

	restart();

	int moved, ch, isQuit = 0;
	while(true) {			// main loop
		ch = getch();
		moved = 0;
		if ( check_fd(0) == true ){
			
		}
		switch(ch) {
			case ' ':
				board[cy][cx] = PLAYER1;
				draw_cursor(cx, cy, 1);
				update(cx, cy, PLAYER1);
				draw_score();
				break;
			case 0x0d:
			case 0x0a:
			case KEY_ENTER:
				board[cy][cx] = PLAYER2;
				update(cx, cy, PLAYER2);
				draw_cursor(cx, cy, 1);
				draw_score();
				break;
			case 'q':
			case 'Q':
				isQuit = 1;
				break;
			case 'r':
			case 'R':
				restart();
				break;
			case 'k':
			case KEY_UP:
				draw_cursor(cx, cy, 0);
				cy = (cy-1+BOARDSZ) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
			case 'j':
			case KEY_DOWN:
				draw_cursor(cx, cy, 0);
				cy = (cy+1) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
			case 'h':
			case KEY_LEFT:
				draw_cursor(cx, cy, 0);
				cx = (cx-1+BOARDSZ) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
			case 'l':
			case KEY_RIGHT:
				draw_cursor(cx, cy, 0);
				cx = (cx+1) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
		}
		
		if (isQuit) break;
		if(moved) {
			refresh();
			moved = 0;
		}
		
		if (isQuit) break;
		napms(1);		// sleep for 1ms
	}

	
	endwin();			// end curses mode

	return 0;
}

bool checkIO(int fd) {

	struct timeval tv;
	fd_set readfds;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	// don't care about writefds and exceptfds:
	if ( select(fd+1, &readfds, NULL, NULL, &tv) < 0 ) {
		printf("select error. QAQ\n");
	}

	return FD_ISSET(fd, &readfds);

	// if (FD_ISSET(fd, &readfds))
	// 	printf("A key was pressed!\n");
	// else
	// 	printf("Timed out.\n");
}

bool findPlayer(int x, int y, int direct, int player) {
	// 0  1  2  3  4  5  6  7 
	// E  EN N  WN W  WS S  ES
	while( 0 <= y && y < 8 && 0 <= x && x < 8 ){
		modify(&x,&y,direct);
		if (board[y][x] == player) return true;
		else if ( board[y][x] == 0 ) return false;
	}
	return false;
}

void update(int cx, int cy, int player) {
	int i = 0, x = 0 , y = 0;
	for(i = 0 ; i < 8 ; ++i){
		//printf("%d %d %d %d \n", cx, cy, i, findPlayer(cx, cy, i, player));
		if( findPlayer(cx, cy, i, player) ){
			x = cx, y = cy;
			while( 0 <= x && x < BOARDSZ && 0 <= y && y < BOARDSZ ){
				modify(&x,&y,i);
				//printf("\rx %d y %d i %d\n", x,y,i);
				if ( board[y][x] == 0-player ) {
					board[y][x] = player;
					draw_cursor(x,y,0);
				}
				else if ( board[y][x] == player ) break;
			}
			//scanf("%*s");
		}

	}
	
}

void restart() {
	clear();
	cx = cy = 3;
	init_board();
	draw_board();
	draw_cursor(cx, cy, 1);
	draw_score();
	refresh();

	attron(A_BOLD);
	move(height-1, 0);
	printw("Arrow keys: move; Space: put GREEN; Return: put PURPLE; R: reset; Q: quit");
	attroff(A_BOLD);
}

void modify(int *x, int *y, int direct){
	switch(direct) {
			case 0:
				*x-=1;break;
			case 1:
				*x-=1;
				*y-=1;break;
			case 2:
				*y-=1;break;
			case 3:
				*x+=1;
				*y-=1;break;
			case 4:
				*x+=1;break;
			case 5:
				*x+=1;
				*y+=1;break;
			case 6:
				*y+=1;break;
			case 7:
				*x-=1;
				*y+=1;break;
	}
}

