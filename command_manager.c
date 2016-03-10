#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_STRING_LENGTH 40 //max length of every command entered
#define MAX_COMMAND_NO 13 //max number of commands acceptable
#define MAX_TOKS 5 //max keywords each command may contain

struct coordinates{ //hold data that relate to a player's position
	int col; //column number -- correspondes to D_B board indexes
	int line; //line (row) number
};

struct kind {
  char choice; //wall ('w') or move ('m')
  char extra; //orientation('h' for horizontal or 'v' for vertical) for walls
              //0 for moves (it has no meaning)
};

struct moves { //used to categorize a move
  struct coordinates position;
  int possible; //if the move is possible to be played
  struct kind data;
};

struct listnode{ //used to hold the game state (moves log)
	struct coordinates data;
	char color;
	char kind;
	char w_o;
	struct listnode *previous_state;
}*lh = NULL;

struct coordinates find_position(char);

int list_commands(char c_list[MAX_COMMAND_NO][MAX_STRING_LENGTH]);
int possible_move(struct coordinates, struct coordinates *, int , int);
int Command_Manager();
int parseString(char *, char ***);
int walls(char *);
int playmove(char *, char *);
int playwall(char *, char *, char *, int *, int *);
int boardsize(char *, int*, int* );
int evaluate_move(char );
int undo(int, int *, int *);
int best_move(char , int, int *, int *);

char **input_stream(int *);

void known_command(char *, char c_list[MAX_COMMAND_NO][MAX_STRING_LENGTH]);
void showboard(int , int );
void data_board_initialize(int, int, int);
void name();
void genmove(char*, int*, int*);
void winner();
void clear_board(int *,int *);
void validMoves(struct coordinates , int , char , struct moves *);
void test_cost_board();
void findCost(struct coordinates , struct coordinates , char);

int moves_played = 0, size = 5; //is made global for convenience
int **D_B , **cost_board; //a board that holds data for wall positions and player positions
int min_cost = INT_MAX, valuation = INT_MAX;
struct moves optimal_move;


int main(void){

	Command_Manager(); //control is passed to the Command_Manager

	return 0;
}

//used to control the flow of input and command execution
int Command_Manager() {
	int wwall=10, bwall=10, argc=0;
	//argc counts how many arguments where entered including the command
	char **A; //a board that stores the keywords from input
	char c_list[MAX_COMMAND_NO][MAX_STRING_LENGTH]; //stores all possible commands

	//data initialization -- D_B board
	data_board_initialize(size,0,0);

	//initializing c_list
	strcpy(c_list[0], "name");
	strcpy(c_list[1], "known_command");
	strcpy(c_list[2], "list_commands");
	strcpy(c_list[3], "quit");
	strcpy(c_list[4], "boardsize");
	strcpy(c_list[5], "clear_board");
	strcpy(c_list[6], "walls");
	strcpy(c_list[7], "playmove");
	strcpy(c_list[8], "playwall");
	strcpy(c_list[9], "genmove");
	strcpy(c_list[10], "undo");
	strcpy(c_list[11], "winner");
	strcpy(c_list[12], "showboard");

	while (1) { //endless loop -- terminates only when 'quit' is entered
		showboard(bwall,wwall);
		A = input_stream(&argc);

		if 	(strcmp(A[0],"quit")==0) {

			fflush(stdout); //used after every command is identified in order to clean the buffer

			if(undo(moves_played, &bwall, &wwall))
				printf("= \n\n");

			int i;
			for(i=0;i<2*size-1;i++)
				free(D_B[i]);
			free(D_B);

			break; //breaks endless loop
		}
		else if(strcmp(A[0],"known_command") == 0){
			if(argc >= 2) //if there are at least two keywords entered
				known_command(A[1], c_list);
			else {
				fflush(stdout);
				printf("? false\n\n");
			}
		}
		else if(strcmp(A[0],"clear_board") == 0){
			clear_board(&bwall,&wwall);
			bwall=wwall=10; //reset walls after cleaning wall
		}
		else if(strcmp(A[0],"list_commands") == 0){
			list_commands(c_list);
		}
		else if(strcmp(A[0],"name") == 0){
			name();
		}
		else if(strcmp(A[0],"showboard") == 0){
			showboard(bwall,wwall);
		}
		else if(strcmp(A[0],"walls") == 0 ){
			//if there are at least two keywords entered and second keyword is not a whitespace char
			if(argc >= 2 && strcmp(A[1]," ") && strcmp(A[1], "\t"))
				wwall=bwall=walls(A[1]);
			else {
				fflush(stdout);
				printf("? invalid syntax\n\n");
			}
		}
		else if(strcmp(A[0], "boardsize") == 0){
			if(argc >= 2){ //if there are at least two keywords entered
				size = boardsize(A[1], &bwall, &wwall);
				if(undo(moves_played, &bwall, &wwall))
					printf("= \n\n");
				else
					printf("? cannot undo\n\n");
			}
			else {
				fflush(stdout);
				printf("? invalid syntax\n\n");
			}
		}
		else if(strcmp(A[0], "playmove") == 0){
			if(argc >= 3) {//if there are at least three keywords entered
				if (playmove(A[1],A[2]) == 1)
					printf("= \n\n");
				else if (playmove(A[1],A[2]) == 0)
					printf("? illegal move\n\n");
				else 
					printf("? invalid syntax\n\n");
			}
			else {
				fflush(stdout);
				printf("? invalid syntax\n\n");
			}
		}
		else if(strcmp(A[0], "playwall") == 0){
			if(argc >= 4) //if there are at least four keywords entered
				if (playwall(A[1],A[2],A[3],&bwall,&wwall) == 1)
					printf("= \n\n");
				else if (playwall(A[1],A[2],A[3],&bwall,&wwall) == 0)
					printf("? illegal move\n\n");
				else 
					printf("? invalid syntax\n\n");
			else {
				fflush(stdout);
				printf("? invalid syntax\n\n");
			}
		}
		else if(strcmp(A[0], "undo") == 0){
			if(argc >= 2) {
				int t;
				//converting string to number
				t = atoi(A[1]);

				if(undo(t, &bwall, &wwall))
					printf("= \n\n");
				else
					printf("? cannot undo\n\n");
			}
			else {
				fflush(stdout);
				printf("? invalid syntax\n\n");
			}
		}
		else if(strcmp(A[0], "winner") == 0){
			winner();
		}
		else if(strcmp(A[0], "genmove") == 0){
			if(argc >= 1)
				genmove(A[1], &bwall, &wwall);
			else{
				fflush(stdout);
				printf("? invalid syntax\n\n");
			}
		}
		else if(A[0][0] == '#' || A[0][0] == '\n'){
			//used to discard comments entered
			fflush(stdout);
			continue;
		}
		else if(strcmp(A[0], "cost") == 0) {

			fflush(stdout);
			evaluate_move('w');
			printf("= \n\n");
		}
		else {
			fflush(stdout);
			printf("? unknown command\n\n");
		}

		fflush(stdout);
		free(A);
	}
	return 0;
}

//used in order to initialize D_B's elements
void data_board_initialize(int size, int mode, int n){
	int i,j;

	if (mode){ //if this function was used before, first free previous instance
		for(i=0;i<2*n-1;i++)
			free(D_B[i]);
		free(D_B);
	}

	D_B = malloc((2*size-1)*sizeof(int *));

	if(D_B == NULL){
		fflush(stdout);
		printf("allocation failed");
		exit(-1);
	}

	for(i=0;i<(2*size-1);i++){
		D_B[i]=malloc((2*size-1)*sizeof(int));
		if(D_B[i] == NULL){
			fflush(stdout);
			printf("allocation failed");
			exit(-1);
		}
	}

	//every element initialized to 0
	for(i=0;i<(2*size-1);i++){
		for(j=0;j<(2*size-1);j++)
			D_B[i][j] = 0;
	}

	D_B[0][size-1] = 2; // black at the top
	D_B[2*size-2][size-1] = 3; //white at the bottom
}

//used to change the boardsize and create a new one from scratch
int boardsize(char *new, int *bwall, int *wwall){
	int n;

	n = atoi(new);
	if (n >=3 && n <= 25 && n % 2 == 1) {
			data_board_initialize(n,1,size);
			*bwall = 10;
			*wwall = 10;
			return n;
	}
	fflush(stdout);
	printf("? unacceptable size\n\n");
	return 9; //to set the board to an acceptable size temporarly
}

//used to change the number of walls available to every player
int walls(char *n_w){
	int n;

	n=atoi(n_w);
	fflush(stdout);
	printf("=\n\n");

	return n;
}

//breaks the input line into keywords and returns them in an array
char **input_stream(int *argc){
	char C[MAX_STRING_LENGTH];
	char **A;
	int  n,i;

	//gets with safety the input
	fgets(C,MAX_STRING_LENGTH,stdin);
	//puts a whitespace at the end to cut the last word of the input line
  strcat(C," ");

  *argc = parseString(C, &A);
  n = *argc;

	//if last char of last keyword is a newline char, change it to \0 char
	if(A[n-1][strlen(A[n-1]) - 1] == '\n')
		A[n-1][strlen(A[n-1]) - 1] = '\0';

  return A;
}

//used to implement the input line break down
int parseString(char *input_line, char ***argv) {
	char delimit[]=" \t"; //keywords are separated using space or tab
	char *buffer;
	int argc;

	buffer = malloc(strlen(input_line) * sizeof(char*));

	strcpy(buffer,input_line);

	*argv = malloc(MAX_TOKS * sizeof(char**));

	argc = 0;

	(*argv)[argc++] = strtok(buffer, delimit);
	while ((((*argv)[argc] = strtok(NULL, delimit)) != NULL) && (argc < MAX_TOKS))
  		argc++;

  	return argc;
}

//prints the name of the program
void name(){
	fflush(stdout);
	printf("= ~MLeo_Quoridor~\n\n");
}

//prints the game's board while reading D_B's data
void showboard(int bwall, int wwall){
	fflush(stdout);
	int i, j, letter = 0, col=size;

	for(i = -2; i <= (2*size); i++) {
		for (j = -2; j <= (2*size); j++) {

			if (i == -2 || i == (2*size)) {
				if (abs(j)%2 == 1) {
					printf(" ");
				}
				else {
		      		if (j == -2 || j == 2*size || j == (2*size - 1))
		      		printf("   ");
			        else {
						printf(" %c ", 65+letter);
						letter++;
	          		}
				}
			}
      else if (i == -1 || i == 2*size-1){
			if (j ==-2 || j == 2*size)
		      printf("   ");
		    else if (abs(j)%2 == 1)
		      printf("+");
		    else
		      printf("---");
		  }
			else {
		    if (j == -2 || j == 2*size){
						if (i%2 == 0) {
							if(col > 9)
								printf("%d ",col);
							else
								printf(" %d ",col);
			    			if (j == 2*size) {
								col--;
							if (i == 0)
									printf(" black walls: %d", bwall);
								else if (i == 2)
									printf(" white walls: %d", wwall);
							}
						}
						else
							printf("   ");
	      }
        else if (j == -1 || j == 2*size-1){
					if (i%2 == 0)
						printf("|");
					else
						printf("+");
        }
				else {
					if (i%2 == 0)
					 	if (j%2 == 0) {
							if (D_B[i][j] == 3)
								printf(" W ");
							else if (D_B[i][j] == 2)
								printf(" B ");
							else if (D_B[i][j] == 0)
								printf("   ");
						}
						else {
							if (D_B[i][j] == 0)
								printf("|");
							else
								printf("H");
						}
					else {
						if (j%2 == 0) {
							if (D_B[i][j] == 0)
								printf("---");
							else
							printf("===");
						}
						else {
							if (D_B[i][j] == 0)
								printf("+");
							else {
								if (D_B[i-1][j] == 1 || D_B[i+1][j] == 1)
									printf("H");
								else
									printf("=");
							}
						}
					}
				}
			}
		}
    printf("\n");
    letter=0;
	}
	printf("\n");
}

//prints c_list's elements -- all commands acceptable
int list_commands(char c_list[MAX_COMMAND_NO][MAX_STRING_LENGTH]) {
	int i;

	fflush(stdout);
	printf("=\n");
	for(i=0;i< 13; i++)
		printf("%s\n",c_list[i]);
	printf("\n");
	return 0;
}

//check if a command is acceptable from the program
void known_command(char *command_name, char c_list[MAX_COMMAND_NO][MAX_STRING_LENGTH]) {
	int i, found;

	for (i = 0, found = 0; i < 13 && !found; i++)
		if (strcmp(command_name, c_list[i]) == 0)
			found = 1;
	fflush(stdout);
	if (found)
		printf("= true\n\n");
	else
		printf("? false\n\n");
}

//check if a move or wall placement is a legal move
int possible_move(struct coordinates current_pos, struct coordinates *temp, int mode, int data) { //data represents horizontal-vertical for mode 1(playwall)
	//if mode == 1														//and black or white for mode 0(playmove)
	if(mode){
		struct coordinates wall_co = *temp;
		if (wall_co.line >= 1 && wall_co.line <=2*size-3 && wall_co.col >= 1 && wall_co.col <= 2*size-3){
			if(data){
				if(D_B[wall_co.line][wall_co.col] == 0 && D_B[wall_co.line][wall_co.col-1] == 0 && D_B[wall_co.line][wall_co.col+1] == 0){
					
					return 1;
				}
				else
					return 0;
			}
			else{
				if(D_B[wall_co.line][wall_co.col] == 0 && D_B[wall_co.line-1][wall_co.col] == 0 && D_B[wall_co.line+1][wall_co.col] == 0){
					
					return 1;
				}
				else
					return 0;
			}
		}
	}

	// if mode == 0
	else {
		struct coordinates temp1, temp2;
		int d1,d2,a1,a2,c1,c2;

		//data 0->white player good || data 1->black player good
		//d1,d2 player current position (good)
		//a1,a2 adversary current position (bad)

		temp1 = current_pos;

		if(data == 1)
			temp2 = find_position('w');
		
		else if (data == 0)
			temp2 = find_position('b');

		d1 = temp1.line;
		d2 = temp1.col;

		a1 = temp2.line;
		a2 = temp2.col;

		c1 = temp->line;
		c2 = temp->col;

		//check the 4 possible moves left
		if ((c2 == d2-2) || (c2 == d2-4)) {
			if (c1 == d1) { //same col
				if (d2-1>=0) { //check for boundaries
					if (D_B[d1][d2-1] == 0) {//if there is no wall
						if (a1 == d1 && a2 == d2-2) { //if player is blocked by the other player
							if (d2-3>=0) //check for boundaries
								if (D_B[d1][d2-3] == 0 && c2 == d2-4){ //not wall behind enemy
									return 1;
								}

						}
						else if (c2 == d2-2){ //if not enemy there
							return 1;
						}
					}
				}
			}
			else if (c1 == d1+2 && c2 == d2-2) { //lower left diagonal
				if (d1+3 <= 2*size-2){ //check to not go out of bounds
					if (D_B[d1+1][d2] == 0 && D_B[d1+3][d2] && a1==d1+2 && a2==d2){ //wall behind enemy, enemy behind player and no wall between
							return 1;
					}		
				}
				if(d1+2 <= 2*size-2){
					if (d2+3 <= 2*size-2)  { //check to not go out of bounds
						if (D_B[d1][d2-1] == 0 && D_B[d1][d2+3] && a1==d1 && a2==d2-2){ //wall behind enemy, enemy left of player and no wall between
							return 1;
						}
					}
					if(d2+2 <= 2*size-2){
						if (D_B[d1][d2-1] == 0 && a2==0 && c2==0 && a1==d1 && a2==d2-2){ //end of board behind enemy, enemy left of player and no wall between
							return 1;
						}
						else if(D_B[d1+1][d2] == 0 && a1==2*size-2 && c1==2*size-2 && a1==d1+2 && a2==d2){ //end of board behind enemy, enemy behind player and no wall between
							return 1;
						}
					}
				}
			}
			else if (c1 == d1-2 && c2 == d2-2) { //upper left diagonal
				if (d1-3 >= 0){ //check to not go out of bounds
					if(D_B[d1-1][d2] == 0 && D_B[d1-3][d2] && a1==d1-2 && a2==d2){ //wall behind enemy, enemy in front of player and no wall between
						return 1;
					}					
				}
				if(d1-2 >= 0){
					if (d2-3 >= 0) {//check to not go out of bounds
						if (D_B[d1][d2-1] == 0 && D_B[d1][d2-3] && a1==d1 && a2==d2-2){ //wall behind enemy, enemy left of player and no wall between
							return 1;
						}
					}
					if(d2-2 >= 0){
						if (D_B[d1][d2-1] == 0 && a2==0 && c2==0 && a1==d1 && a2==d2-2 ){ //end of board behind enemy, enemy left of player and no wall between
							return 1;
						}
						else if(D_B[d1-1][d2] == 0 && a1==0 && c1==0 && a1== d1-2 && a2==d2){ //end of board behind enemy, enemy in front of player and no wall between
							return 1;
						}
					}
				}
			}
		}

		//check the 4 possible moves right
		else if ((c2 == d2+2) || (c2 == d2+4)) {
			if (c1 == d1) { //same col
				if (d2+1<=2*size-2) { //check for boundaries
					if (D_B[d1][d2+1] == 0) { //if there is no wall
						if (a1==d1 && a2==d2+2) { //if player is blocked by the other player
							if (d2+3 <=2*size-2) //check for boundaries
								if (D_B[d1][d2+3] == 0 && c2 == d2+4){ //not wall behind enemy
									return 1;
								}
						}
						else if (c2 == d2+2){ //if not enemy there
							return 1;
						}
					}
				}
			}
			else if (c1 == d1+2 && c2 == d2+2) { //lower right diagonal
				if (d1+3 <= 2*size-2){ //check to not go out of bounds
					if (D_B[d1+1][d2] == 0 && D_B[d1+3][d2] && a1==d1+2 && a2==d2){ //wall behind enemy, enemy behind player and no wall between
						return 1;
					}
				}
				if(d1+2 <= 2*size-2){
					if (d2+3 <=2*size-2) {//check to not go out of bounds
						if (D_B[d1][d2+1] == 0 && D_B[d1][d2+3] && a1==d1 && a2==d2+2){ //wall behind enemy, enemy right of player and no wall between
							return 1;
						}
					}
					if(d2+2 <=2*size-2){
						if (D_B[d1][d2+1] == 0 && a2==2*size-2 && c2==2*size-2 && a1==d1 && a2==d2+2){ //end of board behind enemy, enemy right of player and no wall between
							return 1;
						}
						else if(D_B[d1+1][d2] == 0 && a1==2*size-2 && c1==2*size-2 && a1==d1+2 && a2==d2){ //end of board behind enemy, enemy behind player and no wall between
							return 1;
						}
					}
				}
			}
			else if (c1 == d1-2 && c2 == d2+2) { //upper right diagonal
				
				if (d1-3 >= 0){ //check to not go out of bounds
				
					if (D_B[d1-1][d2] == 0 && D_B[d1-3][d2] && a1==d1-2 && a2==d2){ //wall behind enemy, enemy in front of player and no wall between
						return 1;
					}				
				}
				if(d1-2>=0){

					if (d2+3 <= 2*size-2) {//check to not go out of bounds

						if (D_B[d1][d2+1] == 0 && D_B[d1][d2+3] == 1 && a1==d1 && a2==d2+2){ //wall behind enemy, enemy is right of player and no wall between
							
							return 1;
						}
					}
					if(d2+2 <= 2*size-2){

						if (D_B[d1][d2+1] == 0 && a2==2*size-2 && c2==2*size-2 && a1==d1 && a2==d2+2){ //end of board behind enemy, enemy is right of player and no wall between
							
							return 1;
						}
						else if(D_B[d1-1][d2] == 0 && a1==0 && c1==0 && a1==d1-2 && a2==d2){ //end of board behind enemy, enemy in front of player and no wall between
							return 1;
						}
					}
				}
			}
		}

		//check the 4 possible moves up or down
		else if (c2 == d2) {
			if (c1 < d1) { // 2 top moves
				if (d1-1>=0){ //check for boundaries
					if (D_B[d1-1][d2] == 0){ //if there is wall in front of player
						if (a1==d1-2 && a2==d2){ //if there is enemy blocking
							if (d1-3>=0) //check for boundaries
								if (D_B[d1-3][d2] == 0 && c1==d1-4){ //if no wall behind enemy
									return 1;
								}
						}
						else if (c1==d1-2){ //else if not enemy blocking
							return 1;
						}
					}
				}
			}
			else if (c1 > d1) { // 2 bottom moves
				if (d1+1<=2*size-2){ //check for boundaries
					if (D_B[d1+1][d2] == 0){ //if there is wall in front of player
						if (a1==d1+2 && a2==d2){ //if there is enemy blocking
							if (d1+3<=2*size-2) //check for boundaries
								if (D_B[d1+3][d2] == 0 && c1==d1+4){ //if no wall behind enemy
									return 1;
								}
						}
						else if (c1==d1+2){ //else if not enemy blocking
							return 1;
						}
					}
				}
			}
		}
		return 0; //if no condition is satisfied
	}
}

//plays a move for a given player if the move is legit
int playmove(char *player,char *string_move){
	fflush(stdout);
	struct listnode *new_move;
	struct coordinates temp, pos, *t_p;
	int i, cond1, cond2;

	temp.col = string_move[0]; //take the ASCII code
	temp.line = atoi(&string_move[1]); //make the string number to int

	if (temp.col>=97 && temp.col<=(97+size-1))
		temp.col -= 32; //to convert it to ASCII for upper case

	t_p = &temp;
	cond1 = (temp.col>=65 && temp.col<=(65+size-1)); //if the letter is within accepted limits
	cond2 = (temp.line >=1 && temp.line <= size); //if number is within accepted limits

	//decode coordinates
	temp.line = 2*(size - temp.line);
	temp.col = 2*(temp.col - 65);

	if(cond1 && cond2){

		//player string is case insensitive
		if ((strcasecmp(player,"b") * strcasecmp(player,"black")) == 0){
			if(possible_move(find_position('b'),t_p, 0, 1)){

				//stores the needed data
				new_move=malloc(sizeof(struct listnode));
        		new_move->color = 'b';
				new_move->data = temp;
				new_move->previous_state = lh;
				new_move->kind = 'm';
				new_move->w_o = 'n';

				int d1, d2;
				pos = find_position('b');
				d1 = pos.line;
				d2 = pos.col;

				//erases previous position and create new
				D_B[d1][d2] = 0;
				D_B[temp.line][temp.col] = 2;

				moves_played++;
				lh = new_move;

				return 1;
			}
			else
				return 0;
		}
		else if((strcasecmp(player,"w") * strcasecmp(player,"white") ) == 0){
			if (possible_move(find_position('w'),t_p, 0, 0)) {

				new_move=malloc(sizeof(struct listnode));

        		new_move->color = 'w';
				new_move->data = temp;
				new_move->previous_state = lh;
				new_move->kind = 'm';
				new_move->w_o = 'n';

				int d1, d2;
				pos = find_position('w');
				d1 = pos.line;
				d2 = pos.col;

				D_B[d1][d2] = 0;
				D_B[temp.line][temp.col] = 3;

				lh = new_move;

        		moves_played++;

				return 1;
			}
			else
				return 0;
		}
		else
			return -1;
	}
	else
		return 0;
}

//places a wall for a given player if the placement is legit
int playwall(char *player, char *string_wall, char *orientation, int *bwall, int *wwall){
	fflush(stdout);
	struct listnode *new_move;
	struct coordinates temp, *t_p;
	int i, cond1, cond2, cond3, cond4;

	temp.col = string_wall[0];
	temp.line = atoi(&string_wall[1]);

	if (temp.col>=97 && temp.col<=(97+size-1))
		temp.col -= 32; //to convert it to ASCII for upper case

	t_p = &temp;
	cond1 = (temp.col>=65 && temp.col<(65+size-1));
	cond2 = (temp.line >1 && temp.line <= size);

	temp.line = 2*(size - temp.line) + 1;
	temp.col = 2*(temp.col - 65) + 1;

	//wall orientation is case insensitive
	cond3 = (strcasecmp(orientation,"h") * strcasecmp(orientation,"horizontal"));
	cond4 = (strcasecmp(orientation,"v") * strcasecmp(orientation,"vertical"));

	if(cond1 && cond2){

		if(cond3 * cond4 == 0){

			if ((strcasecmp(player,"b") * strcasecmp(player,"black")) == 0){

				if(possible_move(*t_p,t_p, 1, cond4)){


					new_move=malloc(sizeof(struct listnode));

          			new_move->color = 'b';
					new_move->data = temp;
					new_move->previous_state = lh;
					new_move->kind = 'w';

					//depending on the orientation of the wall, the given wall place and 
					//the two neighbouring wall spots (up and down for vertical/left and right for horizontal) are filled 
					if(cond4) {
						D_B[temp.line][temp.col] = 1;
						D_B[temp.line][temp.col-1] = 1;
						D_B[temp.line][temp.col+1] = 1;
						new_move->w_o = 'h';
					}
					else {
						D_B[temp.line][temp.col] = 1;
						D_B[temp.line-1][temp.col] = 1;
						D_B[temp.line+1][temp.col] = 1;
						new_move->w_o = 'v';
					}

					lh = new_move;
					(*bwall)--;
          			moves_played++;

					return 1;
				}
				else
					return 0;
			}
			else if((strcasecmp(player,"w") * strcasecmp(player,"white")) == 0){

				if (possible_move(*t_p,t_p, 1, cond4)) {

					new_move=malloc(sizeof(struct listnode));

          			new_move->color = 'w'; //w == white
					new_move->data = temp;
					new_move->previous_state = lh;
					new_move->kind = 'w'; // w == wall

					if(cond4) {
						D_B[temp.line][temp.col] = 1;
						D_B[temp.line][temp.col-1] = 1;
						D_B[temp.line][temp.col+1] = 1;
						new_move->w_o = 'h';
					}
					else {
						D_B[temp.line][temp.col] = 1;
						D_B[temp.line-1][temp.col] = 1;
						D_B[temp.line+1][temp.col] = 1;
						new_move->w_o = 'v';
					}

					lh = new_move;
					(*wwall)--;
					moves_played++;

					return 1;
				}
				else
					return 0;
			}
			else
				return -1;
		}
		else
			return -1;
	}
	else{
		return 0;
	}
}

//for a given number of moves to undo, this function removes the move from the move log
int undo(int times, int *bwall, int *wwall) {
	fflush(stdout);

	//check to see if the times asked to undo are legit
	if (times <= moves_played){
		struct listnode *temp;

		//repeat until there are no need to undo 
		while (times) {

			if (lh == NULL) return 0;

			//in case there is only one position played
			if (lh->previous_state == NULL) {

				temp = lh->previous_state;

				free(lh);
				lh = NULL;

				times--;
				moves_played--;
				data_board_initialize(size, 1, size);

				return 1;
			}

			temp = lh->previous_state;

			//if the move to undo is a movement, then erase the current position and reset it
			if (lh->kind == 'm') {
				struct coordinates position;
				char color;

				D_B[lh->data.line][lh->data.col] = 0;

				color = lh->color;
				free(lh);
				lh = temp;

				position = find_position(color); //where it will go

				if (color == 'b')
					D_B[position.line][position.col] = 2;
				else
					D_B[position.line][position.col] = 3;
			}
			//if the move to undo is a wall placement, just free the wall spots
			else if (lh->kind == 'w') {
				if(lh->w_o == 'h'){
					D_B[lh->data.line][lh->data.col] = 0;
					D_B[lh->data.line][lh->data.col+1] = 0;
					D_B[lh->data.line][lh->data.col-1] = 0;
					if (lh->color == 'w')
						(*wwall)++;
					else
						(*bwall)++;
				}
				else{
					D_B[lh->data.line][lh->data.col] = 0;
					D_B[lh->data.line+1][lh->data.col] = 0;
					D_B[lh->data.line-1][lh->data.col] = 0;
					if (lh->color == 'w')
						(*wwall)++;
					else
						(*bwall)++;
				}
				free(lh);
				lh = temp;
			}

			times--;
			moves_played--;
		}
		return 1;
	}
	else //if times > moves_played
		return 0;
}

//check to see if there is a winner -- if there is return the color of the winner
void winner(){
	fflush(stdout);
	struct coordinates temp;

	//check if the position of black or white are at the finishing line
	temp = find_position('b');

	if(temp.line == 2*size-2){
		printf("= true black\n\n");
	}
	else{
		temp = find_position('w');

		if(temp.line == 0)
			printf("= true white\n\n");
		else
			printf("= false\n\n");
	}
}

//for a given player, it runs through the whole move log and finds the latest occurrence of the player
struct coordinates find_position(char player) {
	struct coordinates first_move;
	struct listnode *p;
	p = lh;

	while (p!=NULL) {
		if (p->kind == 'm')
			if (p->color == player)
				return p->data;
		p=p->previous_state;
	}

	if (p == NULL) {
		if (player == 'b') {
			first_move.col = size-1;
			first_move.line = 0;
		}
		else if (player == 'w'){
			first_move.col = size-1;
			first_move.line = 2*size -2;
		}
		return first_move;
	}
	return first_move; //to silence warning messages
}

//calls the undo function to remove all the moves played from the move log and then initialized the board
void clear_board(int *bwall,int *wwall) {
	fflush(stdout);
	if(undo(moves_played,bwall,wwall))
		printf("= \n\n");
	else
		printf("? cannot undo\n\n");

	data_board_initialize(size,1,size);
}

//returns for a current player either all the movements possible from a given position 
//or all the movements and wall placements possible 
void validMoves(struct coordinates place, int mode, char player, struct moves *VM) {
	fflush(stdout);
	int i, j, k;

	//depending on the use needed it helps define the needed size of the totalMoves array
	//if mode == 1, then we need both movements and wall placements
	//else if mode == 0, then we need only movements possible
	if (mode) 
		k = 12+(2*(size-1)*(2*size-3));
	else
		k = 12;

	struct moves totalMoves[k];

	if (mode) {
		for (i = 0; i < 12+(2*(size-1)*(2*size-3)); i++) { //initializing array
			totalMoves[i].position.col = 0;
	   		totalMoves[i].position.line = 0;
	   		totalMoves[i].data.choice = 0;
	   		totalMoves[i].data.extra = 0;
			totalMoves[i].possible = 0;
		}
	}
	else {
		for (i = 0; i < 12; i++) { //initializing array
			totalMoves[i].position.col = 0;
	   		totalMoves[i].position.line = 0;
	   		totalMoves[i].data.choice = 0;
	   		totalMoves[i].data.extra = 0;
			totalMoves[i].possible = 0;
		}
	}

	//the first 12 position of totalMoves array are all valid movements
  	for (i=0; i<12; i++) {
	    struct coordinates temp, *t_p;

	    t_p = &temp;
	    totalMoves[i].position = temp = place;
	    totalMoves[i].data.choice = 'm';
	    totalMoves[i].data.extra = '0';

	    switch (i) {
	      
	      case 0:

	      	if(player == 'w'){
	        	totalMoves[i].position.line = temp.line = place.line-4;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	        }
	        else{
	        	totalMoves[i].position.line = temp.line = place.line+4;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	        }
	        break;

	      case 1:

	      	if(player == 'w'){
	        	totalMoves[i].position.line = temp.line = place.line-2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	        }
	        else{
	        	totalMoves[i].position.line = temp.line = place.line+2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	        }
	        break;

	      case 2:

	      	if(player == 'w'){
	      		totalMoves[i].position.col = temp.col = place.col-2;
	        	totalMoves[i].position.line = temp.line = place.line-2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	        }
	        else{
	        	totalMoves[i].position.col = temp.col = place.col+2;
	        	totalMoves[i].position.line = temp.line = place.line+2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	        }
	        break;

	      	
	      case 3:

	      	if(player == 'w'){
	        	totalMoves[i].position.col = temp.col = place.col+2;
	        	totalMoves[i].position.col = temp.line = place.line-2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 0);
			}
			else {
				totalMoves[i].position.col = temp.col = place.col-2;
	        	totalMoves[i].position.col = temp.line = place.line+2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
			}
	        break;

	      case 4:

	      	if(player == 'w'){
	      		totalMoves[i].position.col = temp.col = place.col-4;
	      		totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	      	}
	      	else{
	        	totalMoves[i].position.col = temp.col = place.col+4;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	        }
	        break;

	      case 5:

	      	if(player == 'w'){
	      		totalMoves[i].position.col = temp.col = place.col-2;
	      		totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	      	}
	      	else{
	        	totalMoves[i].position.col = temp.col = place.col+2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	        }
	        break;

	      case 6:

	      	if(player == 'w'){
	      		totalMoves[i].position.col = temp.col = place.col+2;
	      		totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	      	}
	      	else{
	        	totalMoves[i].position.col = temp.col = place.col-2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	        }
	        break;

	      case 7:

	      	if(player == 'w'){
	      		totalMoves[i].position.col = temp.col = place.col+4;
	      		totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	      	}
	      	else{
	        	totalMoves[i].position.col = temp.col = place.col-4;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	        }
	        break;

	      case 8:

	      	if(player == 'w'){
	      		totalMoves[i].position.col = temp.col = place.col-2;
	        	totalMoves[i].position.line = temp.line = place.line+2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	        }
	        else{
	        	totalMoves[i].position.col = temp.col = place.col+2;
	        	totalMoves[i].position.line = temp.line = place.line-2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	        }
	        break;

	      case 9:

	      	if(player == 'w'){
	        	totalMoves[i].position.line = temp.line = place.line+2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	        }
	        else{
	    		totalMoves[i].position.line = temp.line = place.line-2;
	    		totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	    	}
	        break;

	      case 10:

	      	if(player == 'w'){
	      		totalMoves[i].position.col = temp.col = place.col+2;
	        	totalMoves[i].position.line = temp.line = place.line+2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	        }
	        else{
	        	totalMoves[i].position.col = temp.col = place.col-2;
	        	totalMoves[i].position.line = temp.line = place.line-2;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	        }
	        break;

	      case 11:

	      	if(player == 'w'){
	        	totalMoves[i].position.line = temp.line = place.line+4;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 0);
	        }
	        else{
	        	totalMoves[i].position.line = temp.line = place.line-4;
	        	totalMoves[i].possible = possible_move(place,t_p, 0, 1);
	        }
	        break;
	    }
	}
  	if (mode) { //mode is 0 if validMoves is used by pathFinder function
		struct coordinates temp, *t_p;
		int v; //to hold previous i value and to continue as the new index
	    v = i; // i equals 12 for the previous for loop
	    //check for every position if a wall can be placed
		t_p = &temp;
		//
	    for (i = 0; i < size; i++) {
	      	for (j = 65; j < size + 65; j++) {
	      		if (v < k) {

	      			temp.line = 2*(size - i) + 1;
					temp.col = 2*(j - 65) + 1;

			        totalMoves[v].position.col = temp.col = j;
			        totalMoves[v].position.line = temp.line = i;
			        totalMoves[v].possible = possible_move(*t_p,t_p, 1, 1);
			        totalMoves[v].data.choice = 'w';
			        totalMoves[v].data.extra = 'h';
			        v++;
					totalMoves[v].position.col = temp.col = j;
			        totalMoves[v].position.line = temp.line = i;
			        totalMoves[v].possible = possible_move(*t_p,t_p, 1, 0);
			        totalMoves[v].data.choice = 'w';
			        totalMoves[v].data.extra = 'v';
			        v++;
			    }
			    else
			    	break;
	      	}
	      	if (v >= k)
	      		break;
	    }
  	}
  	if (mode)
  		for (i = 0; i < k; i++)
  			VM[i] = totalMoves[i];
  	else 
  		for (i = 0; i < 12; i++) {
  			VM[i] = totalMoves[i];
  		}
}

void genmove(char *player, int *bwall , int *wwall){
	int i;
	struct coordinates pos; //to hold current position of player
	struct listnode *new_move; //to store new move made to move log
	struct moves move_to_make; //to store data for the move that is about to be made

	if ((strcasecmp(player,"b") * strcasecmp(player,"black")) == 0){

		valuation = INT_MAX;
		best_move('b', 4, bwall, wwall); //get the best move
		move_to_make = optimal_move;
		pos = find_position('b'); //find where the black player is

		if (move_to_make.data.choice == 'm') { //if it is a movement
			D_B[pos.line][pos.col] = 0; //erase previous position
			new_move=malloc(sizeof(struct listnode));

			//make necessary data entries
	        new_move->color = 'b';
			new_move->data.line = move_to_make.position.line;
			new_move->data.col = move_to_make.position.col;
			new_move->previous_state = lh;
			new_move->kind = 'm';
			new_move->w_o = 'n';

			D_B[move_to_make.position.line][move_to_make.position.col] = 2; //move to new position

			lh = new_move;

			moves_played++;
			printf("moves_played: %d\n", moves_played);

			printf("= %c%d\n\n",(new_move->data.col)/2 + 65,abs(new_move->data.line/2 - size)); //print what the move was
			return ;
		}
		else { //if it is a wall placement
			new_move=malloc(sizeof(struct listnode));

			//make necessary data entries
          	new_move->color = 'b';
			new_move->data.line = move_to_make.position.line;
			new_move->data.col = move_to_make.position.col;
			new_move->previous_state = lh;
			new_move->kind = 'w';

			if(move_to_make.data.extra == 'h') { //place the wall horizontally
				D_B[move_to_make.position.line][move_to_make.position.col] = 1;
				D_B[move_to_make.position.line][move_to_make.position.col-1] = 1;
				D_B[move_to_make.position.line][move_to_make.position.col+1] = 1;
				new_move->w_o = 'h';
			}
			else { //place the wall vertically
				D_B[move_to_make.position.line][move_to_make.position.col] = 1;
				D_B[move_to_make.position.line-1][move_to_make.position.col] = 1;
				D_B[move_to_make.position.line+1][move_to_make.position.col] = 1;
				new_move->w_o = 'v';
			}

			lh = new_move;
			(*bwall)--;
          	moves_played++;

			printf("= %c%d %c\n\n", (new_move->data.col-1)/2 + 65, size - (new_move->data.line-1)/2, new_move->w_o); //print what the move was
		}
	}
	else if((strcasecmp(player,"w") * strcasecmp(player,"white")) == 0){
		
		valuation = INT_MAX;
		best_move('w', 2, bwall, wwall); //get the best move
		move_to_make = optimal_move;
		pos = find_position('w'); //find where the white player is
		
		if (move_to_make.data.choice == 'm') { //if it is a movement
			D_B[pos.line][pos.col] = 0; //erase previous position
			new_move = malloc(sizeof(struct listnode));

			//make necessary data entries
	        new_move->color = 'w';
			new_move->data.line = move_to_make.position.line;
			new_move->data.col = move_to_make.position.col;
			new_move->previous_state = lh;
			new_move->kind = 'm';
			new_move->w_o = 'n';

			D_B[move_to_make.position.line][move_to_make.position.col] = 3; //move to new position

			lh = new_move;

	       	moves_played++;
	       	printf("moves_played: %d\n", moves_played);

			printf("= %c%d\n\n",(new_move->data.col)/2 + 65,abs(new_move->data.line/2 - size)); //print what the move was
			return ;
		}
		else { //if it is a wall placement
			new_move=malloc(sizeof(struct listnode));

			//make necessary data entries
          	new_move->color = 'w';
			new_move->data.line = move_to_make.position.line;
			new_move->data.col = move_to_make.position.col;
			new_move->previous_state = lh;
			new_move->kind = 'w';

			if(move_to_make.data.extra == 'h') { //place the wall horizontally
				D_B[move_to_make.position.line][move_to_make.position.col] = 1;
				D_B[move_to_make.position.line][move_to_make.position.col-1] = 1;
				D_B[move_to_make.position.line][move_to_make.position.col+1] = 1;
				new_move->w_o = 'h';
			}
			else { //place the wall vertically
				D_B[move_to_make.position.line][move_to_make.position.col] = 1;
				D_B[move_to_make.position.line-1][move_to_make.position.col] = 1;
				D_B[move_to_make.position.line+1][move_to_make.position.col] = 1;
				new_move->w_o = 'v';
			}

			lh = new_move;
			(*bwall)--;
          	moves_played++;

			printf("= %c%d %c\n\n", (new_move->data.col-1)/2 + 65, size - (new_move->data.line-1)/2, new_move->w_o); //print what the move was
		}
	}
	else
		printf("? invalid syntax\n\n");
}

void findCost(struct coordinates current_pos, struct coordinates prev_pos, char color) {
	struct moves VM[12];
	int i;

	validMoves(current_pos, 0, color, VM);

	for(i = 0; i <= 11; i++){

		if(VM[i].possible){

			if(VM[i].position.line != prev_pos.line || VM[i].position.col != prev_pos.col){
				
				if(cost_board[VM[i].position.line/2][VM[i].position.col/2] == 0){

					if(cost_board[current_pos.line/2][current_pos.col/2] == -1){

						cost_board[VM[i].position.line/2][VM[i].position.col/2] = 1;

						if(cost_board[VM[i].position.line/2][VM[i].position.col/2] >= min_cost) {
							break;
						}

						if(color == 'b'){
							if(VM[i].position.line/2 == size - 1){
								min_cost = cost_board[VM[i].position.line/2][VM[i].position.col/2];
								break;
							}
						}
						else{
							if(VM[i].position.line == 0){
								min_cost = cost_board[VM[i].position.line/2][VM[i].position.col/2];
								break;
							}
						}
						findCost(VM[i].position,current_pos,color);
					}
					else{

						cost_board[VM[i].position.line/2][VM[i].position.col/2] = cost_board[current_pos.line/2][current_pos.col/2] + 1;

						if(cost_board[VM[i].position.line/2][VM[i].position.col/2] >= min_cost) {
							break;
						}

						if(color == 'b'){

							if(VM[i].position.line/2 == size - 1){
								min_cost = cost_board[VM[i].position.line/2][VM[i].position.col/2];
								break;
							}
						}
						else{

							if(VM[i].position.line == 0){
								min_cost = cost_board[VM[i].position.line/2][VM[i].position.col/2];
								break;
							}
						}

						findCost(VM[i].position,current_pos,color);
					}
				}
				else{
					if(cost_board[VM[i].position.line/2][VM[i].position.col/2] > cost_board[current_pos.line/2][current_pos.col/2] + 1){

						cost_board[VM[i].position.line/2][VM[i].position.col/2] = cost_board[current_pos.line/2][current_pos.col/2] + 1;

						if(cost_board[VM[i].position.line/2][VM[i].position.col/2] >= min_cost) {
							break;
						}

						if(color == 'b'){

							if(VM[i].position.line/2 == size - 1){
								min_cost = cost_board[VM[i].position.line/2][VM[i].position.col/2];
								break;
							}
						}
						else{
							if(VM[i].position.line == 0){
								min_cost = cost_board[VM[i].position.line/2][VM[i].position.col/2];
								break;
							}
						}

						findCost(VM[i].position,current_pos,color);
					}
				}
			}
		}
	}

	return;
}

void test_cost_board() {
	int i, j;

	printf("%d\n", min_cost);
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			printf("%d", cost_board[i][j]);
		}
		printf("\n");
	}
}

int evaluate_move(char color) {
	char other_color;
 	int i, j, difference;

 	cost_board = malloc(size * sizeof(int *));

	for (i = 0; i < size; i++) {
		cost_board[i] = malloc(size * sizeof(int));
		for (j = 0; j < size; j++)
			cost_board[i][j] = 0;
	}

	cost_board[find_position(color).line/2][find_position(color).col/2] = -1;

	if (color == 'b')
		other_color = 'w';
	else
    	other_color = 'b';

    min_cost = INT_MAX;

    //test_cost_board(); -- for debugging

    findCost(find_position(color), find_position(other_color), color);
	difference = min_cost;

	min_cost = INT_MAX;
	for (i = 0; i < size; i++) {
		cost_board[i] = malloc(size * sizeof(int));
		for (j = 0; j < size; j++)
			cost_board[i][j] = 0;
	}
	cost_board[find_position(other_color).line/2][find_position(other_color).col/2] = -1;

	findCost(find_position(other_color), find_position(color), other_color);
	difference -= min_cost;

	for(i = 0; i < size; i++)
    	free(cost_board[i]);
    free(cost_board);

	return difference;
}

int best_move(char color, int depth, int *bwall, int *wwall){
	int i, temp_cost, k=12 + 2*(size - 1) * (2*size - 3);

	if (depth > 0){

		struct coordinates current_pos = find_position(color);
		struct moves VM[k];

		validMoves(current_pos,color,1,VM);

		for(i=0; i<k; i++){

			if(VM[i].possible){

				if(i<12){

					char player[7], move_to_play[5];

					if (color == 'b') 
		            	strcpy(player, "black");
		          	else
		            	strcpy(player, "white");

		          	sprintf(move_to_play, "%c%d", (VM[i].position.col/2) + 65, abs(VM[i].position.line/2 - size));
		          	
		          	playmove(player, move_to_play);

		          	char other_color;

		          	if (color == 'b')
						other_color = 'w';
					else
    					other_color = 'b';

					temp_cost  = best_move(other_color, depth-1, bwall, wwall);

					if(temp_cost < valuation){

						valuation = temp_cost;
						optimal_move = VM[i];
					}
					undo(1,bwall,wwall);
				}
				else{

					char player[7], wall_to_play[5], orient_to_play[3];

					if (color == 'b') 
			    	    strcpy(player, "black");
			        else
			            strcpy(player, "white");

			        sprintf(wall_to_play, "%c%d", (VM[i].position.col-1)/2 + 65, size - (VM[i].position.line-1)/2);

			        if (VM[i].data.extra == 'v') 
			            strcpy(orient_to_play,"v");
			        else
			            strcpy(orient_to_play, "h");

			        playwall(player, wall_to_play, orient_to_play, bwall, wwall);

			        char other_color;
		          	
		          	if (color == 'b')
						other_color = 'w';
					else
    					other_color = 'b';

					temp_cost  = best_move(other_color, depth-1, bwall, wwall);

					if(temp_cost < min_cost){

						valuation = temp_cost;
						optimal_move = VM[i];
					}
					undo(1,bwall,wwall);
				}
			}
		}
	}
	else
		return evaluate_move(color);
}









