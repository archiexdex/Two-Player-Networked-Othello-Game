#include "othello.h"

static int width;
static int height;
static int cx = 3;
static int cy = 3;
static const int bx = 8;
static const int by = 8;

void restart();
void update(int y, int x, int player);
void modify(int*, int*, int);


int main() {
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
		//printf("%d \n",ch);
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
			while( 0 <= x && x < bx && 0 <= y && y < by ){
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

