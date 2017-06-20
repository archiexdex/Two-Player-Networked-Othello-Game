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

#define GREEN "\033[0;32;32m"
#define PURPLE "\033[0;35m"

static int width;
static int height;
static int cx = 3;
static int cy = 3;


void restart();
void update(int, int, int);
void modify(int*, int*, int);
bool checkIO(int);
void draw_player_turn(int, int);
bool isOk(int, int, int);

char buf[1024];

int main(int argc, char* argv[]) {

	int isS = false;
	int player = 0, now = PLAYER1;
	char *port = NULL;
	char *ip = NULL;
	if (argc != 3) return 0;
	if ( !strcmp(argv[1],"-s") ){
		player = PLAYER1;
		isS = true;
		port = argv[2];
	}
	else if( !strcmp(argv[1],"-c") ) {
		player = PLAYER2;
		isS = false;
		char *tmp = argv[2];
		char *ptr = strtok(tmp,":");
		ip = ptr;
		ptr = strtok(NULL,":");
		port = ptr;
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

	restart(player, now);

	int moved, ch, isQuit = 0;
	while(true) {			// main loop
		moved = 0;
		if ( checkIO(0) ){
			ch = getch();
			switch(ch) {
				case ' ':
				case 0x0d:
				case 0x0a:
				case KEY_ENTER:
					if ( player == now && isOk(cx, cy, now) ){
						board[cy][cx] = now;
						update(cx, cy, now);
						draw_cursor(cx, cy, 1);
						draw_score();
						sprintf(buf,"x %d %d %d",cx,cy,now);
						send(fd,buf,sizeof(buf), 0);
						now = -now;
						moved++;
					}
					break;
				case 'q':
				case 'Q':
					isQuit = 1;
					break;
				case 'r':
				case 'R':
					restart(player, now);
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
		}
		else if( checkIO(fd) ){
			char op;
			int x ,y, p;
			recv(fd, buf, sizeof(buf), 0);
			switch (buf[0]) {
				case 'x':
					sscanf(buf,"%c%d%d%d",&op, &x,&y,&p);
					board[y][x] = p;
					update(x, y, p);
					draw_cursor(x, y, 1);
					draw_score();
					now = -now;
					moved++;
					break;
				case 'q':
					isQuit = 1; break;
			}
			moved++;
		}
		
		if (isQuit) break;
		if(moved) {
			draw_player_turn(player, now);
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

bool isOk(int x, int y, int player){
	int i = 0;
	for (i = 0 ; i < 8 ; i++){
		if (findPlayer(x,y,i,player))
			return true;
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

void restart(int player, int now) {
	clear();
	cx = cy = 3;
	init_board();
	draw_board();
	draw_cursor(cx, cy, 1);
	draw_score();
	refresh();
	
	draw_player_turn(player, now);
	attron(A_BOLD);
	move(height-1, 0);
	printw("Arrow keys: move; Space: put GREEN; Return: put PURPLE; R: reset; Q: quit");
	attroff(A_BOLD);
	refresh();
}

void draw_player_turn(int player, int now) {
	initscr();
	start_color();
	init_pair(11, COLOR_BLACK, COLOR_GREEN);
	init_pair(22, COLOR_BLACK, COLOR_MAGENTA);
	move(0,0);
	if ( player == PLAYER1 ){
		attron(COLOR_PAIR(11));
		printw("Player #1: %s",(player == now)? "It's my turn    " : "waiting for peer" );
		attroff(COLOR_PAIR(11));
	}
	else {
		attron(COLOR_PAIR(22));
		printw("Player #2: %s",(player == now)? "It's my turn    " : "waiting for peer" );
		attroff(COLOR_PAIR(22));
	}
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

