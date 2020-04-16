#include "tetris.h"

static struct sigaction act, oact;

char main(){
	bool exit=0;

	// init ncurses 
	initscr();
	curs_set(0);

	
	/*@set colors*/
	start_color();
	//color for block shapes
		init_pair(0, COLOR_WHITE, COLOR_WHITE);
	init_pair(1, COLOR_WHITE, COLOR_MAGENTA);	//글씨색, 배경색
	init_pair(2, COLOR_WHITE, COLOR_CYAN);
	init_pair(3, COLOR_WHITE, COLOR_YELLOW);
	init_pair(4, COLOR_WHITE, COLOR_GREEN);
	init_pair(5, COLOR_WHITE, COLOR_RED);
	init_pair(6, COLOR_WHITE, COLOR_WHITE);
	init_pair(7, COLOR_WHITE, COLOR_BLUE);
	//color for shadow
	init_pair(8, COLOR_WHITE, COLOR_BLACK);
	//color for "GameOver"
	init_pair(9, COLOR_WHITE, COLOR_RED);


	noecho();
	keypad(stdscr, TRUE);	

	srand((unsigned int)time(NULL));

	// 2주차 초기화
	createRankList();	

	// 3주차 초기화
	recRoot=(Node *)malloc(sizeof(Node));
	recRoot->child = (Node **)malloc(sizeof(Node *)*CHILDREN_MAX);
	recRoot->level=-1;
	recRoot->accumulatedScore=0;
	constructRecTree(recRoot);

	// 게임 실행
	while(!exit){
		clear();
		switch(menu()){
			case MENU_PLAY: play(); break;
			case MENU_REC: recommendedPlay(); break;
			case MENU_EXIT: exit=1; break;

		default: break;
		}
	}

	// init ncurses
	curs_set(1);
	attroff(A_REVERSE);
	endwin();
	system("clear");

	return 0;
}

void InitTetris(){
	char i,j;

	// upLim
	upLim = HEIGHT - 1;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			recRoot->recField[j][i] = field[j][i]=0;

	// 블록 정보 초기화
	for(i=0;i<VISIBLE_BLOCKS;++i){
		nextBlock[i]=rand()%7;
	}
	recRoot->recUpLim = upLim;
	recommend(recRoot);
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	// 점수 초기화
	score=0;

	// flag 초기화	
	gameOver=0;
	timed_out=0;

	// 배경 그리기
	DrawOutline();
	DrawField();

	// 블록 그리기
	drawRecommend();
	DrawShadow(blockY,blockX, nextBlock[0], blockRotate);
	DrawBlock(blockY,blockX,nextBlock[0],blockRotate,' ');

	// 부수 정보 출력
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline(){	
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	attron(COLOR_PAIR(1));
	DrawBox(0,0,HEIGHT,WIDTH);
	attroff(COLOR_PAIR(1));

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	attron(COLOR_PAIR(2));
	DrawBox(3,WIDTH+10,4,8);
	attroff(COLOR_PAIR(2));

	/* score를 보여주는 공간의 태두리를 그린다.*/
	//move(9,WIDTH+10);
	move(9,WIDTH+10);
	printw("SCORE");
	attron(COLOR_PAIR(2));
	DrawBox(10,WIDTH+10,1,8);
	attroff(COLOR_PAIR(2));
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	cajMax:
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

char ProcessCommand(int command){
	char ret=1;
	bool drawFlag=0;
	
	char rotMax = block[nextBlock[0]].rotNo;

	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%rotMax,blockY,blockX, upLim)))
			blockRotate=(blockRotate+1)%rotMax;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX, upLim)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1,upLim)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1,upLim)))
			blockX--;
		break;
	case ' ':
		drawFlag = 1;
		spcY = blockY;
		blockY = shdwY;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	char i,j;

	for(j=0;j<HEIGHT;j++){	// 원래 초기화 j = 0
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]){
				attron(A_REVERSE);
				printw(" ");
			}
			else{
				attroff(A_REVERSE);
				printw(".");
			}
		}
	}
}

void DrawField_AddBlock(){
	blockStruct *blk = &(block[nextBlock[0]]);	// block 모양 선택

	rotateStruct *rot = &(blk->rotate[blockRotate]);	// block의 여러 회전형 중 기본형 선택
	char *blkRow;	// block 정의역의 한 행 
	char  bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표

	char i;
	attron(A_REVERSE);
	for( i = 0; i < 4; i++){
		blkRow = rot->shape[i];
		bY = blkRow[0], bX = blkRow[1];	
		if(blockY+bY<0){
			move(bY+1,bX+1);
			printw(" ");
		}
	}
}

void DrawField_DeleteLine(char lineCnt, short int upLim, short int yLand){
	char bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표
	char BY;

	short int upLimMod = upLim - lineCnt;

	for(bY = yLand; bY >= upLimMod; bY-- ){
		BY = bY + 1;
		for(bX = 0; bX < WIDTH; bX++){
			move(BY,bX+1);
			if(field[bY][bX]){
				attron(A_REVERSE);
				printw(" ");
			}
			else{
				attroff(A_REVERSE);
				printw(".");
			}
		}
	}
}


void PrintScore(int score){
	move(11,WIDTH+11);
	attroff(A_REVERSE);
	printw("%8d",score);
}

void DrawNextBlock(char *nextBlock){
	char i;

	// 기존 block
	rotateStruct *rot = &(block[nextBlock[0]].rotate[0]);	// block의 여러 회전형 중 기본형 선택

	char *blkRow;	// block 정의역의 한 행 
	char bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표

	// 기존 block 지우기
	attroff(A_REVERSE);
	for( i = 0; i < 4; i++){
		blkRow = rot->shape[i];
		bY = blkRow[0], bX = blkRow[1];	
		move(4+bY,WIDTH+13+bX);
		printw(" ");
	}

	// 새로운 block
	rot = &(block[nextBlock[1]].rotate[0]);	// block의 여러 회전형 중 기본형 선택

	// 새로운 block 그리기
	attron(A_REVERSE);
	for( i = 0; i < 4; i++){
		blkRow = rot->shape[i];
		bY = blkRow[0], bX = blkRow[1];	
		move(4+bY,WIDTH+13+bX);
		printw(" ");
	}
}

void DrawBlock(short int y, short int x, char blockID, char blockRotate, char tile){
	char i;

	rotateStruct *rot = &(block[blockID].rotate[blockRotate]);	// block의 여러 회전형 중 선택
	char *blkRow;	// block 정의역의 한 행 
	char bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표

	// block 그리기
	attron(A_REVERSE);
	for( i = 0; i < 4; i++){
		blkRow = rot->shape[i];
		bY = blkRow[0], bX = blkRow[1];	
		if(y+bY<0)
			continue;
		move(bY+y+1,bX+x+1);
		printw("%c",tile);
	}
}

void DeleteBlock(short int y, short int x, char blockID, char blockRotate){
	char i;

	rotateStruct *rot = &(block[blockID].rotate[blockRotate]);	// block의 여러 회전형 중 선택
	char *blkRow;	// block 정의역의 한 행 
	char bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표

	// block 그리기
	attroff(A_REVERSE);
	for( i = 0; i < 4; i++){
		blkRow = rot->shape[i];
		bY = blkRow[0], bX = blkRow[1];	
		if(y+bY<0)
			continue;
		move(bY+y+1,bX+x+1);
		printw(".");
	}
}

void DrawBox(char y, char x, char height, char width){
	char i,j;
	attroff(A_REVERSE);
	move(y,x);
	addch('o');
	//addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
	//	addch(ACS_HLINE);
	//addch(ACS_URCORNER);
	addch('-');
	addch('o');
	for(j=0;j<height;j++){
		move(y+j+1,x);
		//addch(ACS_VLINE);
		addch('|');
		move(y+j+1,x+width+1);
		//addch(ACS_VLINE);
		addch('|');
	}
	move(y+j+1,x);
	//addch(ACS_LLCORNER);
	addch('o');
	for(i=0;i<width;i++)
		addch('-');
	//	addch(ACS_HLINE);
	//addch(ACS_LRCORNER);
	addch('o');
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			attron(COLOR_PAIR(9));
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			attroff(COLOR_PAIR(9));
			move(HEIGHT/2,WIDTH/2-4);
			attroff(A_REVERSE);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);


	alarm(0);
	getch();
	attron(COLOR_PAIR(9));
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	attroff(COLOR_PAIR(9));
	move(HEIGHT/2,WIDTH/2-4);
	attron(COLOR_PAIR(9));
	printw("GameOver!!");
	attroff(COLOR_PAIR(9));
	refresh();
	getch();
	newRank(score);
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

bool CheckToMove_Shadow(char f[HEIGHT][WIDTH], char currentBlock, char blockRotate, short int blockY, short int blockX){
	char i;
	
	rotateStruct *rot = &(block[currentBlock].rotate[blockRotate]);	// block의 여러 회전형 중 선택
	char *blkRow;	// block 정의역의 한 행 
	char bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표

	// 그림자가 필드의 아래끝을 벗어나면 0 반환 
	if(!(blockY + rot->maxY < HEIGHT))
		return 0;
	
	// 그림자가 기존의 블록과 겹치면 0 반환
	for( i = 0; i < 4; i++){
		blkRow = rot->shape[i];
		bY = blkRow[0], bX = blkRow[1];	
		if(f[blockY + bY][blockX + bX])
			return 0;	
	}

	// 결격 사유가 없으면 1 반환
	return 1;
}

bool CheckToMove(char f[HEIGHT][WIDTH], char currentBlock, char blockRotate, short int blockY, short int blockX, short int upLim){
	char i;
	
	rotateStruct *rot = &(block[currentBlock].rotate[blockRotate]);	// block의 여러 회전형 중 선택
	char *blkRow;	// block 정의역의 한 행 
	char bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표
	short int ySum;

	// 블록이 필드의 x범위 밖에 있으면 0 반환
	if(!((-1)*rot->minX <= blockX && blockX < WIDTH - rot->maxX))
		return 0;

	// (블록이 필드의 x 범위 안에 있고)
	// 블록이 빈 공간을 활공하고 있으면 1 반환
	// = 블록이 upLim보다 높이 있을 때
	if((ySum = blockY + rot->maxY) < upLim)
		return 1;

	// 블록이 필드의 아래끝을 벗어나면 0 반환
	if(!(ySum < HEIGHT))
		return 0;
	
	// 블록이 기존 블록과 겹치면 0 반환
	for( i = 0; i < 4; i++){
		blkRow = rot->shape[i];
		bY = blkRow[0], bX = blkRow[1];	
		if(f[blockY + bY][blockX + bX])
			return 0;	
	}
	// 결격 사유가 없으면 1 반환
	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH], int command, char currentBlock, char blockRotate, short int blockY,short int blockX){
	char i;
	short int x, y;
	char r;
	
	blockStruct *blk = &(block[currentBlock]);	// block 모양 선택
	char rotMax = blk->rotNo;		// 선택된 모양의 block의 회전형 개수

	rotateStruct *rot;	// block의 여러 회전형 중 선택
	char *blkRow;	// block 정의역의 한 행 
	char bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표

	y = blockY; x = blockX;	// bX, bY에 block 정의역 위치 할당
	r = blockRotate;		// block의 회전수 할당

	// 키보드 입력에 따라 조정된 block의 x좌표와 y좌표를 원래대로 복구
	switch(command){
	case KEY_DOWN:
		y--; break;
	case KEY_LEFT:
		x++; break;
	case KEY_RIGHT:
		x--; break;
	case KEY_UP:
		r = (r + (rotMax-1)) % rotMax; break;
	case ' ':
		y = spcY; break;
	}

	rot = &(blk->rotate[r]);	// block의 여러 회전형 중 선택

	// 이전 블록과 그림자 지우기
	attroff(A_REVERSE);
	for(i = 0; i < 4; i++){
		// 이전 블록 지우기
		blkRow = rot->shape[i];
		bY = blkRow[0]; bX = blkRow[1];	
		if(y + bY >= 0){
                	move(y + bY + 1, x + bX + 1);
                	printw(".");
		}
		// 이전 그림자 지우기
		if(shdwY + bY >= 0) {
			move(shdwY + bY + 1, x + bX + 1);
			printw(".");
		}
	}

	// 새로운 블록과 그림자 그리기
	
	DrawBlock(recommendY,recommendX,recommendBlk,recommendR,'R');
	DrawShadow(blockY ,blockX, currentBlock, blockRotate);		// 그림자 그리기
	DrawBlock(blockY, blockX, currentBlock, blockRotate, ' ');	// 블록 그리기
}

void BlockDown(int sig){
	char cellCnt = 0, lineCnt = 0;	// 블록이 기존 블록과 닿는 칸 개수, 삭제된 줄 수
	short int yLand;	// 줄이 삭제된 후 채워넣을 줄의 y좌표	
	char i, j;

	// block이 아래로 내려갈수 있다면, 아래로 내리기
        if(CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX, upLim)){
		blockY++;
                DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);	// 갱신된 block을 출력
		timed_out = 0;	// block 이동 활성화
		return;
        }	

        // block이 field를 벗어나면 game over
        if(blockY == -1)
        	gameOver = 1;

	//removeRecommend();
	DeleteBlock(recommendY, recommendX, nextBlock[0], recommendR);	// 블록 그리기
	DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' ');	// 블록 그리기

	// 점수 갱신 및 출력
	cellCnt = AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX, &upLim);	
	lineCnt = DeleteLine(field, nextBlock[0], blockRotate,upLim,blockY, &yLand);
	score += cellCnt * 10 + lineCnt * lineCnt * 100;

	PrintScore(score);

	// next block 갱신 및 출력
	for(i=0;i<VISIBLE_BLOCKS-1;++i){
		nextBlock[i] = nextBlock[i+1];
	}	
	nextBlock[VISIBLE_BLOCKS-1] = rand()%7;

	for(i=MIN(upLim, recRoot->recUpLim); i<HEIGHT; i++)
		for(j=0; j<WIDTH; j++)
			recRoot->recField[i][j] = field[i][j];

	// next block에 맞게 추천

	DrawNextBlock(nextBlock);

        // block 정보 초기화
	blockX = (WIDTH / 2) - 2;
        blockY = -1;		
        blockRotate = 0;	

	//DeleteBlock(prevRecY, prevRecX, prevRecBlk, prevRecR);	// 블록 그리기

        // field를 갱신, next block 출력
        if(lineCnt==0){
		;//DrawField_AddBlock();;
	}
	else
		DrawField_DeleteLine(lineCnt, upLim, yLand);
	
	// upLim다 사용한 후에 갱신
        upLim += lineCnt;

	recRoot->recUpLim = upLim;
	recommend(recRoot);
	// 블록과 그림자 그리기
	drawRecommend();
	DrawShadow(blockY ,blockX, nextBlock[0], blockRotate);
	DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' ');
		
	timed_out = 0;
}

char AddBlockToField(char f[HEIGHT][WIDTH], char currentBlock, char blockRotate, short int blockY, short int blockX, short int *upLim){
	char y, x;		// x는 block 정의역 상 블록의 x좌표, y는 y좌표
	char cellCnt = 0;	// 

	char i;

	rotateStruct *rot = &(block[currentBlock].rotate[blockRotate]);	// block의 여러 회전형 중 선택
	char *blkRow;	// block 정의역의 한 행 
	char bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표

	short int yTop = rot->minY;

        // 쌓여있는 block의 upper bound인 upLim를 blockY의 최솟값으로 갱신
        if (blockY+yTop<*upLim) {
                *upLim = blockY+yTop;
	}		

	// 필드 상 블록의 위치를 1로 set하고 기존 블록과 닿는 칸의 개수 계산
	for(i = 3; i >= 0; i--){
		blkRow = rot->shape[i];
		bY = blkRow[0], bX = blkRow[1];	
       		f[blockY + bY][blockX + bX] = 1;
		if(blockY + bY + 1 == HEIGHT || f[blockY + bY + 1][blockX + bX])
			cellCnt++;
	}

	return cellCnt;
}

char DeleteLine(char f[HEIGHT][WIDTH], char currentBlock, char blockRotate,  short int upLim,short int blockY, short int *yLand){
	char y, x;		// x는 block 정의역 상 블록의 x좌표, y는 y좌표
        char lineCnt = 0;	// 지워지는 line 개수 

	short int upLimMod = MAX(upLim , 0);
	short int yBot, yTop;
	short int yTopMod, yBotMod;

	char i;

	rotateStruct *rot = &(block[currentBlock].rotate[blockRotate]);	// block의 여러 회전형 중 선택
	char bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표

	short int yLandMod;

	*yLand = -1;

	yBot = rot->maxY;
	yTop = rot->minY;

	yBotMod = yBot + blockY;
	yTopMod = MAX(blockY + yTop,0);

        for(y = yBotMod; y >= yTopMod; y--){
                for(x = 0; f[y][x] && x < WIDTH; x++);	// 각 line에 block이 있을 때까지 scan
                if(x == WIDTH){				// 해당 line이 block으로 꽉차 있으면
                        if(*yLand==-1)
				*yLand = y;
			f[y][0] = EMPTY;
			lineCnt++;
		}
	}

	// 지울 줄 이 없으면 return
	if(lineCnt==0)
		return lineCnt;

	yLandMod = *yLand;	

	// 아래로 줄을 당길 뿐만 아니라 기존 블록의 상부를 덮어씌우기 위해 y하한 갱신	
	upLimMod = MAX(upLimMod - lineCnt, 0);

	// 삭제된 줄에 블록 채워넣기
	for(y = yLandMod - 1; y >= upLimMod; y--){
		if(f[y][0] != EMPTY){
			for(x = 0; x < WIDTH; x++)
				f[yLandMod][x] = f[y][x];	// 지워지는 line을 한줄 씩 아래로 내리기
			yLandMod--;
		}
	}

	// 예외처리: 블록 더미 위로 lineCnt만큼의 줄이 없어서 내릴 줄이 없다면
	// 그냥 0으로 채워넣기
	if(upLim < 0 || upLimMod < lineCnt)
		for(y = yLandMod; y >= 0; y--)
			for(x = 0; x < WIDTH; x++)
				f[y][x] = 0;	// 지워지는 line을 한줄 씩 아래로 내리기

        return lineCnt;
}

void DrawShadow(short int y, short int x, char blockID, char blockRotate){
	short int yBot = block[blockID].rotate[blockRotate].maxY;
	
	// 블록이 upLim 위에 있다면 블록이 upLim 바로 위에 놓이도록 y좌표 갱신 
	if(y + yBot < upLim - 1)
		y = upLim - yBot;	// 맨 끝에 있던 -1 삭제함

	// 블록이 놓여질 수 없을 때까지 내리기
	for(;CheckToMove_Shadow(field, blockID , blockRotate, y, x);y++);

	// 블록을 내렸다면, 블록이 놓여질 수 있는 마지막 위치로 되돌리기
	if(y != blockY)
		y--;	
	
	// 해당 위치에 그림자 그리기
	DrawBlock(y,x,blockID,blockRotate,'/');

	// DrawChange에서 이전 그림자를 지울 때 필요할 이전 그림자의 y좌표 갱신
	shdwY = y;	
}

void createRankList(){
	// user code
}

void rank(){
	// user code
}

void writeRankFile(){
	// user code
}

void newRank(int score){
	// user code
	curs_set(1);
	curs_set(0);
}

void drawRecommend(){
	if(CheckToMove(field,nextBlock[0],recommendR,recommendY,recommendX,upLim)){
		DrawBlock(recommendY,recommendX,nextBlock[0],recommendR,'R');
//		prevRecY = recommendY; prevRecX = recommendX;
//		prevRecR = recommendR; prevRecBlk = nextBlock[0];
	}
}

/*
void drawRecommend_Down(){
	//attroff(A_REVERSE);
	//DrawBlock(prevRecY,prevRecX,prevRecBlk,prevRecR,'.');
	attron(A_REVERSE);
	DrawBlock(recommendY,recommendX,nextBlock[0],recommendR,'R');
	prevRecY = recommendY; prevRecX = recommendX;
	prevRecR = recommendR; prevRecBlk = nextBlock[0];
}
*/
//Node *newRankNode(){
//	return tempNode;
//}

void constructRecTree(Node *root){
        char i;
        Node **c=root->child;
        for(i=0;i<CHILDREN_MAX;++i){
                c[i]=(Node *)malloc(sizeof(Node));
		c[i]->child = (Node **)malloc(sizeof(Node *)*CHILDREN_MAX);
                c[i]->level=root->level+1;
                if(c[i]->level < VISIBLE_BLOCKS){
                        constructRecTree(c[i]);
                }
        }
}

void destructRecTree(Node *root){
        int i,h;
        Node **c=root->child;
        for(i=0;i<CHILDREN_MAX;++i){
                if(c[i]->level < VISIBLE_BLOCKS){
                        destructRecTree(c[i]);
                }
                free(c[i]);
        }
}




// 관통할 수 있게 check to move 새로운 버전 만들기
// upLim override->upLim을 인자로 받기
// 필드 상속 필요 있나?
int recommend(Node *root){
        int r,x,y,rBoundary,lBoundary;
        int h,w;
        int eval;
        int max=0;
        int solR,solY,solX,solBlk;
        int recommended=0;
        int i=0;
	int j;
        int lv=root->level+1;
	Node **c=root->child;

	short int thisUpLim;
	blockStruct *blk = &(block[nextBlock[lv]]);
        char rotMax = blk->rotNo;  
	rotateStruct *rot;

	char cellCnt, lineCnt;
	short int yLand;
	
        for(r=0;r<rotMax;++r){ /* 모든 rotation에 대해 */
		rot = &(blk->rotate[r]);

		lBoundary = (-1)*rot->minX;
		rBoundary = WIDTH - rot->maxX;

                for(x=lBoundary;x<rBoundary;++x,++i){ /* 이동 가능한 모든 x에 대해 */
			thisUpLim = root->recUpLim;
                        /* 먼저 이전 레벨의 field를 복원 */
                        for(h=thisUpLim;h<HEIGHT;++h){
                                for(w=0;w<WIDTH;++w){
                                        c[i]->recField[h][w]=root->recField[h][w];
                                }
                        }

                        /* 현재 (r,x)에 대해 떨어질 수 있는 y를 구하기 */
                        y=thisUpLim-rot->maxY;
                        if(CheckToMove(c[i]->recField,nextBlock[lv],r,y,x,thisUpLim)){
                                while(CheckToMove(c[i]->recField,nextBlock[lv],r,++y,x,thisUpLim));
                                --y;
                        }
                        else{ /* 지금까지 구한 (r,x)에 대해 현재 레벨의 블럭이 놓일 곳 없음 */
                                continue;
                        }

			cellCnt = AddBlockToField(c[i]->recField, nextBlock[lv], r, y, x, &thisUpLim);
			lineCnt = DeleteLine(c[i]->recField,nextBlock[lv],r, thisUpLim, y, &yLand);
			c[i]->accumulatedScore=root->accumulatedScore+cellCnt*10+lineCnt*lineCnt*100;
			c[i]->recUpLim = thisUpLim + lineCnt;

			/* 지금까지 구한 (r,y,x)를 바탕으로,
                           현재 레벨의 블럭을 field에 추가했을 때 얻을 수 있는 점수 */

                        if(lv < VISIBLE_BLOCKS-1){
                                eval=recommend(c[i]);
                        }
                        else{
                                eval=c[i]->accumulatedScore;
                        }
                        if(max<eval){
                                recommended=1;
                                max=eval;
				solBlk=nextBlock[lv];
                                solR=r;
                                solY=y;
                                solX=x;
                        }
                }
        }

        if(lv==0 && recommended){
		recommendBlk=solBlk;
                recommendR=solR;
                recommendY=solY;
                recommendX=solX;
        }

        return max;
}

void removeRecommend(){
	char i;
	
	blockStruct *blk = &(block[prevRecBlk]);	// block 모양 선택
	char rotMax = blk->rotNo;		// 선택된 모양의 block의 회전형 개수

	rotateStruct *rot;	// block의 여러 회전형 중 선택
	char *blkRow;	// block 정의역의 한 행 
	char bY, bX;	// block 정의역(4X4) 상 block 한 칸의 y좌표, x좌표

	rot = &(blk->rotate[prevRecR]);	// block의 여러 회전형 중 선택

	// 이전 블록과 그림자 지우기
	attroff(A_REVERSE);
	for(i = 0; i < 4; i++){
		blkRow = rot->shape[i];
		bY = blkRow[0]; bX = blkRow[1];	
		if(prevRecY + bY >= 0){
                	move(prevRecY + bY + 1, prevRecX + bX + 1);
                	printw(".");
		}
	}
}

void updateField(int sig){
	char cellCnt, lineCnt;
	short int yLand;
        char i, j;
        if(!CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX,upLim)) gameOver=1;
	else{
		// remove previous blck
		//DeleteBlock(recommendY, recommendX, nextBlock[0], recommendR);  // 블록 그리기
		
		DeleteBlock(shdwY, blockX, nextBlock[0], blockRotate);      // 블
		DeleteBlock(blockY, blockX, nextBlock[0], blockRotate);      // 블

		DrawBlock(recommendY, recommendX, nextBlock[0], recommendR, ' ');      // 블

		// update and print score
		cellCnt = AddBlockToField(field, nextBlock[0], recommendR, recommendY, recommendX, &upLim); 
		lineCnt = DeleteLine(field, nextBlock[0], recommendR,upLim,recommendY, &yLand);
		score += cellCnt * 10 + lineCnt * lineCnt * 100;
                PrintScore(score);

		// initialize 이거 필요한가?
		blockY=-1;blockX=(WIDTH/2)-2;blockRotate=0;

		// update next block
                for(i=0;i<VISIBLE_BLOCKS-1;++i){
                        nextBlock[i] = nextBlock[i+1];
                }
                nextBlock[VISIBLE_BLOCKS-1] = rand()%7;
		DrawNextBlock(nextBlock);

		// update Field
		if(lineCnt>0)
			DrawField_DeleteLine(lineCnt, upLim, yLand);

		// recommend 
		for(i=MIN(upLim, recRoot->recUpLim); i<HEIGHT; i++)
			for(j=0; j<WIDTH; j++)
				recRoot->recField[i][j] = field[i][j];
		recRoot->recUpLim = upLim;
		recommend(recRoot);


		// draw block
		drawRecommend();
		DrawShadow(blockY ,blockX, nextBlock[0], blockRotate);
		DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' ');

		timed_out=0;
	}
}

void recommendedPlay(){
        int command;
        clear();
        act.sa_handler = updateField;
        sigaction(SIGALRM,&act,&oact);
        InitTetris();
        do{
                if(timed_out==0){
                        alarm(1);
                        timed_out=1;
                }
                command = GetCommand();
                if(command=='q' || command=='Q'){
                        alarm(0);
                        DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
                        move(HEIGHT/2,WIDTH/2-4);
                        printw("Good-bye!!");
                        refresh();
                        getch();

                        return;
                }
        }while(!gameOver);

        alarm(0);
        getch();
        DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
        move(HEIGHT/2,WIDTH/2-4);
        printw("GameOver!!");
        refresh();
        getch();
}


void createRankList(){
	FILE *fp = fopen("rank.txt", "r");
	int i;
	char *tempName;

	if((fscanf(fp,"%d",&recNum)==EOF)||recNum==0)	//빈 파일일 경우
		recNum = 0;
	scoreRoot = (recNum>0)?(RecNode*)malloc(recNum*sizeof(RecNode)):NULL;
	for(i=0;i<recNum;i++){
		tempName = (char*)malloc(NAMELEN*sizeof(char));
		fscanf(fp,"%s",tempName);
		fscanf(fp,"%d",&scoreRoot[i].score);
		scoreRoot[i].name = tempName;
		scoreRoot[i].prev = (i>0)?&scoreRoot[i-1]:NULL;
		scoreRoot[i].next = (i<recNum-1)?&scoreRoot[i+1]:NULL;
	}
	fclose(fp);
}

void rank(){
	int i, j;
	RecNode *tempRec;
	int starti = 0, endi = 0;
	char start[10], end[10];
	int startcnt, endcnt;
	char scoreArray[10];
	char tempName[NAMELEN + 1];
	int tempScore;
	int nameCnt;
	int rank;
	clear();
	noecho();
	if(recNum==0)
		rankAlert(NO_RANK,-1,RETURN_TO_RANK);
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");
	printw("\n\nPress q of any case to return to main menu");
	switch(wgetch(stdscr)){
	case '1':
		echo();
		move(3,0);
		printw("X: ");
		scanw("%d", &starti);
		/*
		for(i=0;isdigit(start[i]=wgetch(stdscr))&&i<10;i++){
			if(i==0){
				move(5,0);
				printw("\n");
				move(3,3);
			}
			if(i==9)
				break;
			starti += starti*10 + start[i] - '0';
		}
		
		if(!isdigit(start[i])&&start[i]!='\n'){
			move(3,0);
			rankAlert(CLEAR_NO,RANK_NUMBER,RETURN_TO_RANK);	
			return;
		}
		if(starti)
			printw("X: %d",starti);
		else
			printw("X:");
		*/
		//move(4,0);
		printw("Y: ");
		scanw("%d", &endi);
		/*
		for(i=0;isdigit(end[i]=wgetch(stdscr))&&i<10;i++){
			if(i==9)
				break;
			endi += endi*10 + end[i] - '0';
		}
		if(!isdigit(end[i])&&end[i]!='\n'){
			move(4,0);
			rankAlert(CLEAR_NO,RANK_NUMBER,RETURN_TO_RANK);
			return;
		}
		if(endi)
			printw("Y: %d",endi);
		else
			printw("Y:");
		*/
		printw("\n");
		if(starti>0)starti--;
		if(endi>0)endi--;
		break;
	case '2':
		move(3,0);
		echo();
		printw("input the name: ");
		wgetnstr(stdscr, tempName, NAMELEN);
		printw("\tname\t\t|\tscore\n");	
		printw("------------------------------------------\n");
		for(nameCnt=0, tempRec=scoreRoot; tempRec!=NULL;tempRec=tempRec->next){
			if(strcmp(tempName,tempRec->name))
				continue;
			nameCnt++;
			printw("%s",tempRec->name);
			for(j=0;j<NAMELEN-strlen(tempRec->name);j++)
				printw(" ");
			printw("\t| ");
			printw("%d",tempRec->score);
			printw("\n");		
		}
		wgetch(stdscr);
		if(nameCnt==0)
			rankAlert(CLEAR_NO,NAME_NOT_FOUND,RETURN_TO_RANK);
			return;
		rankAlert(CLEAR_NO,-1,RETURN_TO_RANK);
		return;
	case '3':
		printw("\n\tname\t\t|\tscore\n");	
		printw("------------------------------------------\n");
		for(tempRec=scoreRoot; tempRec!=NULL; tempRec=tempRec->next){
			printw("%s",tempRec->name);
			for(j=0;j<NAMELEN-strlen(tempRec->name);j++)
				printw(" ");
			printw("\t| ");
			printw("%d",tempRec->score);
			printw("\n");		
		}
		printw("input the rank: ");
		scanw("%d", &rank);	
		for(i=0, tempRec=scoreRoot; i<rank-1&&tempRec!=NULL; i++, tempRec=tempRec->next);
		//OK
		if(i<recNum-1) tempRec->next->prev=tempRec->prev;
		if(i>0) tempRec->prev->next=tempRec->next;
		printw("success\n");
		wgetch(stdscr);
		//OK
		//free(tempRec);	//왜 안되는지 모르겠당 ㅠㅜ
		recNum--;
		printw("\n\tname\t\t|\tscore\n");	
		printw("------------------------------------------\n");
		for(tempRec=scoreRoot; tempRec!=NULL; tempRec=tempRec->next){
			printw("%s",tempRec->name);
			for(j=0;j<NAMELEN-strlen(tempRec->name);j++)
				printw(" ");
			printw("\t| ");
			printw("%d",tempRec->score);
			printw("\n");		
		}
		rankAlert(CLEAR_NO,-1,RETURN_TO_RANK);
		return;	
	case 'q':
	case 'Q':
		return;
	default:
		move(5,0);
		rankAlert(CLEAR_NO,-1,RETURN_TO_MENU);
		return;
	}
		
	if(endi>recNum-1) endi=recNum-1;	//if endi exceeds recNum assign recNum-1
	if(starti==0)starti=0;			//if starti 
	if(endi==0)endi=recNum-1;		//if value for endi was not input, assign recNum-1
	if(starti>endi)
		rankAlert(CLEAR_NO,RANK_ORDER,RETURN_TO_RANK);
	if(!(0<=starti&&starti<recNum)||!(0<=endi&&endi<recNum)){
		rankAlert(CLEAR_NO,RANK_MENU,RETURN_TO_RANK);
		return;
	}
	move(5,0);
	printw("\tname\t\t|\tscore\n");	
	printw("------------------------------------------\n");
	/*
	for(i=starti;i<=endi;i++){
		printw(scoreRoot[i].name);
		printw("\t\t| ");
		sprintf(scoreArray,"%d",scoreRoot[i].score);
//		for(i=0,tempScore=scoreRoot[i].score;tempScore>0;i++,tempScore/=10)
//			scoreArray[i]=tempScore%10 + '0';
//		scoreArray[i] = '\0';
		printw("%s",scoreArray);
		printw("\n");	
	}	
	rankAlert(CLEAR_NO,-1,RETURN_TO_BOTH);
	return;
	*/
	for(tempRec=scoreRoot, i=0; tempRec!=NULL; tempRec=tempRec->next, i++){
		if(i<starti)
			continue;
		if(i>endi)
			break;
		printw("%s",tempRec->name);
		for(j=0;j<NAMELEN-strlen(tempRec->name);j++)
			printw(" ");
		printw("\t| ");
		sprintf(scoreArray,"%d",tempRec->score);
		printw("%s",scoreArray);
		printw("\n");		
	}
	rankAlert(CLEAR_NO,-1,RETURN_TO_BOTH);
	return;

/*	
	printw("X:\n");
	wgetch(stdscr);
	printw("Y:\n");
	wgetch(stdscr);
	switch(menu()){
	case MENU_PLAY: play(); break;
	case MENU_RANK: rank(); break;
	case MENU_AUTO: ; break;
	case MENU_EXIT: exit=1; break;
	default: break*/
}

/*
RecNode* findRank(int score){
	int i;
	int lower = -1, upper = -1;
	RecNode* tempRec = scoreRoot;

	if(i = binSearch(score, &lower, &upper))
		tempRec = scoreRoot[i];
	if(lower==-1) lower = 0;
	if(upper==-1) upper = recNum - 1;
	
	for(tempRec = scoreRoot[upper];tempRec!=scoreRoot[lower].prev;tempRec=tempRec->prev){
		if(tempRec->score == DELETED)	//삭제된 등수는 skip한다.
			continue;
		if(score<=tempRec->score)	//
			break;
	}

	if(tempRec->prev!=NULL)
		tempRec = tempRec->prev;
	else
		return NULL;
	return tempRec;
}


int binSearch(int score, int *lower, int *upper){
	RecNode* tempRec = scoreRoot;
	int left = 0, right = recNum - 1;
	int mid;
	int i;
	while(left<=right){
		mid = (left + right)/2;
		for(i=mid;tempRec[i].score==DELETED&&i<recNum;i++);	//삭제된 점수라면 오른쪽 방향으로 삭제되지 않은 원소 탐색
		if(i==recNum==-1)	//탐색 결과가 없다면 왼쪽 방향으로 탐색
			for(i=mid;tempRec[i].score==DELETED&&i>=0;i--);
		mid = i;
		if(mid==-1)	//양쪽으로 탐색했는데도 삭제되지 않은 원소를 찾지 못함
			return -1;
		switch(COMPARE(score,tempRec[mid].score)){
		case 1:
			left = mid + 1;
			*lower = mid;
			break;
		case 0:
			return mid;
		case -1:
			right = mid - 1;
			*upper = mid;
		}
	}
	return -1;
}
*/

void rankAlert(int clear, int errCode, int destination){
	switch(clear){
	case CLEAR_YES:
		clear();
		break;
	case CLEAR_NO:
		;
	}
	switch(errCode){
	case RANK_MENU:
		printw("Enter number of menu or q\n");
		break;
	case RANK_NUMBER:
		printw("Enter number of item in rank list\n");
		break;
	case RANK_ORDER:
		printw("search failure: no rank in the list\n");	
		break;
	case NAME_NOT_FOUND:
		printw("search failure: no information on the list\n");
	}
	switch(destination){
	case RETURN_TO_BOTH:
	case RETURN_TO_RANK:
		printw("\nPress any key to return to rank menu\n");
	case RETURN_TO_MENU:
		if(!RETURN_TO_RANK)
			printw("Press q of any case to return to main menu\n");	
	}
	switch(wgetch(stdscr)){
	case 'q':
	case 'Q':
		if(destination!=RETURN_TO_RANK){
			return;
		}
	default:
		rank();
		return; 
	}
}		


void rank1(){}

void rank2(){}

void rank3(){}


void writeRankFile(){
	FILE *fp = fopen("rank.txt", "w");
	RecNode *tempRec;
	
	fprintf(fp, "%d\n", recNum);
	for(tempRec=scoreRoot;tempRec;tempRec=tempRec->next)
		fprintf(fp, "%s\t%d\n", tempRec->name, tempRec->score); 
	
	
	fclose(fp);
}

void newRank(int score){
	char ch;
	//char* tempName;
	RecNode *tempRec;
	char* nameMem;
	int nameLen;
	char tempName[NAMELEN+1];
	char scoreArray[SCORELEN+1];
	int tempScore;
	int i;
	
	clear();
	echo();

	//몇등인지 알아내기	
	place = -1;
	if(recNum>0)
		for(tempRec=scoreRoot, i=0; tempRec!=NULL; tempRec=tempRec->next, i++){
			if(score>tempRec->score){
				place = i+1;	
				break;
			}
			if(tempRec->next==NULL){
				place = i+2;
				break;
			}
		}
	else if(recNum==0)
		place = 1;
	//NOT OK
	
	if(place>RANKMAX){
		newRankAlert(score,RANK_MAX,RETURN_TO_MENU);
		return;	
	}

	
	newRec = (RecNode*)malloc(sizeof(RecNode));
	//tempName = (char*)malloc((NAMELEN+1)*sizeof(char));
	//getstr 이용
	
	printw("your name: ");
	//wgetch(stdscr);
	wgetnstr(stdscr,tempName,NAMELEN);
	if((nameLen=strlen(tempName))>=NAMELEN){
		//free(tempName);
		newRankAlert(score, NAME_LENGTH, RETURN_TO_NEWRANK);
		return;// newRec;
	}
	nameMem = (char*)malloc((nameLen+2)*sizeof(char));
	strcpy(nameMem,tempName);
	newRec->name = nameMem;
		
	//이름 입력받고 할당 후  출력
	/*
	printw("your name: ");
	for(i=0;!isspace(tempName[i]=wgetch(stdscr));i++){
		if(i>=NAMELEN){
			newRankAlert(score, NAME_LENGTH, RETURN_TO_NEWRANK);
			return newRec;
		}	
	}
	tempName[i]='\0';
	printw("%s\n\n",tempName);
	newRec->name = (char*)malloc((i+1)*sizeof(char));
	newRec->name[0] = '\0';
	strcpy(newRec->name,tempName);
	*/	

	//점수 출력
	printw("\nyour score: %d\n\n",score);
	/*
	for(i=0;!isspace(scoreArray[i]=wgetch(stdscr));i++){
		if(i>=SCORELEN){
			free(tempName); free(scoreArray);
			newRankAlert(score, SCORE_LENGTH, RETURN_TO_NEWRANK);
			return;
		}	
		if(!isdigit(tempName[i])){
			free(tempName); free(ScoreArray);	
			newRankAlert(score, SCORE_NUMBER, RETURN_TO_NEWRANK);
			return;
		}
	}
	scoreArray[i]='\0';
	printw("%s\n",scoreArray);
	*/
	//점수 구조체 생성
	
	//이름을 구조체에 입력
//	newRec->name = tempName;
	
	//점수를 구조체에 입력
	newRec->score = score;

	if(recNum==0){
		scoreRoot = newRec;	
		scoreRoot->next = NULL;
		scoreRoot->prev = NULL;	
	}
	else if(place>recNum){
		tempRec->next = newRec;
		newRec->prev = tempRec;
		newRec->next = NULL;
	}
	else {
		if(recNum>=2)
			tempRec->prev->next = newRec;
		newRec->prev = tempRec->prev;
			
		tempRec->prev = newRec;
		newRec->next = tempRec;
	}
	if(place==1)
		scoreRoot = newRec;

	recNum++;
	
	newRankAlert(score,-1,RETURN_TO_MENU);
//	scoreRoot

	//strcpy(newRec->score,score);
}

void newRankAlert(int score, int errCode, int destination){

	noecho();
	switch(errCode){
	case NAME_LENGTH:
		printw("Enter name of length less than %d",NAMELEN);
		newRank(score);
		return;
	case RANK_MAX:
		printw("You didn't make it to the top %d\n",RANKMAX);
		newRank(score);
		return;
	/*
	case SCORE_LENGTH:
		printw("Enter score of length less than %d",SCORELEN);
		newRank(score);
		return;
	case SCORE_NUMBER:
		printw("Enter score in decimal digits\n");
		newRank(score);	
		return;
	*/
	}
	switch(destination){
	case RETURN_TO_NEWRANK:
		printw("Press any key to enter again\n");
		wgetch(stdscr);
		newRank(score);
		return;
	case RETURN_TO_MENU:
		printw("Press any key to return to menu\n");
		wgetch(stdscr);
		return;
	}
	

}

void commingSoon(int destination){
	noecho();
	clear();
	printw("Sorry! Opted feature is not availabe at the moment.\n\n");
	switch(destination){
	case RETURN_TO_RANK:
		printw("Press any key to return to rank menu\n");
		wgetch(stdscr);
		rank();
		return;
	case RETURN_TO_MENU:
		printw("Press any key to return to main menunn");
		wgetch(stdscr);
		return;
	}
}


#include "tetris.h"

