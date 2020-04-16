#ifndef _TETRIS_H_
#define _TETRIS_H_


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

#define WIDTH	10
#define HEIGHT	22
#define NOTHING	0
#define QUIT	'q'
#define NUM_OF_SHAPE	7
#define NUM_OF_ROTATE	4
#define BLOCK_HEIGHT	4
#define BLOCK_WIDTH	4
#define BLOCK_NUM	2

// menu number
#define MENU_PLAY '1'
#define MENU_REC '3'
#define MENU_EXIT '4'

// 사용자 이름의 길이
#define NAMELEN 16

#define CHILDREN_MAX 36

// 제가 추가한 매크로 함수입니다.
#define MAX(x,y) ((x)>(y))?(x):(y)
#define MIN(x,y) ((x)<(y))?(x):(y)

//rankAlert errCOde
#define CLEAR_NO	0
#define CLEAR_YES	1

#define RANK_MENU	0
#define RANK_NUMBER	1
#define RANK_ORDER	2
#define NAME_NOT_FOUND	3
#define NO_RANK		4

#define RETURN_TO_RANK	0
#define RETURN_TO_MENU	1
#define RETURN_TO_BOTH	2

//newRankAlert errcode
#define NAME_LENGTH	0
#define RANK_MAX	1

// 사용자 이름의 길이
#define NAMELEN 16
#define SCORELEN 10
#define RANKMAX 32
#define CHILDREN_MAX 36


#define RETURN_TO_NEWRANK	0


#define EMPTY 2
#define VISIBLE_BLOCKS 2

typedef struct _Node {
	//must-have elements	
	int accumulatedScore;
	char recField[HEIGHT][WIDTH];
	struct _Node **child;
	//optional element
	char curBlockID;
	int recBlockX, recBlockY;
	char recBlockRotate;
	struct _Node *parent;
	short int recUpLim;
	char level;
} Node;


typedef struct _RecNode{
	char *name;
	int score;
	struct _RecNode *prev;
	struct _RecNode *next;
} RecNode;

// block의 정보를 저장할 새로운 구조체

typedef struct _rotateStruct{
	char shape[4][3];

	char minY;
	char maxY;
	char minX;
	char maxX;
}rotateStruct;

typedef struct _blockStruct{
	struct _rotateStruct rotate[NUM_OF_ROTATE];
	char rotNo;
}blockStruct;

void debug(void);


blockStruct block[NUM_OF_SHAPE] = {
{
		// rotate[4] 단위 ok
	{
		{
			{ { 1, 0 }, { 1,1 }, { 1,2 }, { 1,3 } },
			1,
			1,
			0,
			3
		},
		{
			 { { 3,1 },{ 2,1 },{ 1,1 },{ 0,1 } } ,
			0 ,
			3 ,
			1 ,
			 1
		}
	},2
},
	//===============id=1================
	{
		{
			{
				{{ 3, 3 }, { 2,1 }, { 2,2 }, { 2,3 } },
				2,
				3,
				1,
				3
			},
			{
				{ { 3,2 },{ 1,3 },{ 2,2 },{ 1,2 } } ,
				 1 ,
				3 ,
				2 ,
				3
			},
			{
				{ { 2,1 },{ 2,2 },{ 2,3 },{ 1,1 } } ,
				1 ,
				2 ,
				1 ,
				3
			},
			{
				{ { 3,1 },{ 3,2 },{ 2,2 },{ 1,2 } } ,
				1 ,
				3 ,
				1 ,
				2
			}
		},4
	},
	//===============id=2================
	{
		{
			{
				{ { 3, 1 }, { 2,2 }, { 2,3 }, { 2,1 } },
				2,
				3,
				1,
				3
			},
			{
				{ { 3,2 },{ 3,3 },{ 2,2 },{ 1,2 } } ,
				1 ,
				3 ,
				2 ,
				3
			},
			{
				{ { 2,1 },{ 2,2 },{ 2,3 },{ 1,3 } } ,
				1 ,
				 2 ,
				 1 ,
				3
			},
			{
				{ { 3,2 },{ 1,1 },{ 2,2 },{ 1,2 } } ,
				1 ,
				3 ,
				1 ,
				2
			}
		},4
	},
	//===============id=3================
	{
		{
			{
				{ { 2, 0 }, { 2,1 }, { 2,2 }, { 1,1 } },
				1,
				2,
				0,
				2
			},
			{
				{ { 3,1 },{ 2,0 },{ 2,1 },{ 1,1 } } ,
				1 ,
				3 ,
				0 ,
				1
			},
			{
				{ { 3,1 },{ 2,0 },{ 2,2 },{ 2,1 } } ,
				2 ,
				3 ,
				0 ,
				2
			},
			{
				{ { 3,1 },{ 2,2 },{ 2,1 },{ 1,1 } } ,
				1 ,
				3 ,
				1 ,
				2
			}
		},4
	},
	//===============id=4================
	{
		{
			{
				{ { 3, 1 }, { 3,2 }, { 2,1 }, { 2,2 } },
				2,
				3,
				1,
				2
			}
		},1
	},
	//===============id=5================
	{
		{
			{
				{ { 2, 1 }, { 2,2 }, { 1,3 }, { 1,2 } },
				1,
				2,
				1,
				3
			},
			{
				{ { 2,1 },{ 3,2 },{ 2,2 },{ 1,1 } } ,
				1 ,
				3 ,
				1 ,
				2
			}
		},2
	},
	//===============id=6================
	{
		{
			{
				{ { 3, 2 }, { 3,3 }, { 2,1 }, { 2,2 } },
				2,
				3,
				1,
				3
			},
			{
				 { { 3,1 },{ 2,2 },{ 2,1 },{ 1,2 } } ,
				1 ,
				3 ,
				1 ,
				2
			}
		},2
	}
};

char field[HEIGHT][WIDTH];	/* 테트리스의 메인 게임 화면 */
char nextBlock[BLOCK_NUM];	/* 현재 블럭의 ID와 다음 블럭의 ID들을 저장; [0]: 현재 블럭; [1]: 다음 블럭 */
char blockRotate;
short int blockY,blockX;	/* 현재 블럭의 회전, 블럭의 Y 좌표, 블럭의 X 좌표*/
int score;			/* 점수가 저장*/
bool gameOver=0;			/* 게임이 종료되면 1로 setting된다.*/
bool timed_out;
char recommendR, recommendBlk;
short int recommendY,recommendX; // 추천 블럭 배치 정보. 차례대로 회전, Y 좌표, X 좌표
char prevRecR, prevRecBlk;
short int prevRecY, prevRecX;

// 제가 추가한 전역변수입니다.
short int upLim;// = HEIGHT - 4;	// 쌓인 block의 upperBound
bool enableInit = 1;
//short int prevID, prevRotate, prevY, prevX;
short int shdwY;
short int spcY;
recNode *scoreRoot;
Node *recRoot;

/***********************************************************
 *	테트리스의 모든  global 변수를 초기화 해준다.
 *	input	: none
 *	return	: none
 ***********************************************************/
void InitTetris();

/***********************************************************
 *	테트리스의 모든  interface를 그려준다.
 *	input	: none
 *	return	: none
 ***********************************************************/
void DrawOutline();

/***********************************************************
 *	테트리스와 관련된 키입력을 받는다.
 *	input	: none
 *	return	: (int) 입력받은 command
 *		  KEY_DOWN : 방향키 아래
 *		  KEY_UP   : 방향키 위
 *		  KEY_RIGHT: 방향키 오른쪽
 *		  KEY_LEFT : 방향키 왼쪽
 *		  ' '	   : Space bar
 *		  'q'/'Q'  : quit
 ***********************************************************/
int GetCommand();

/***********************************************************
 *	GetCommand로 입력받은 command에 대한 동작을 수행한다.
 *	input	: (int) GetCommand로 받은 command
 *	return	: (int) quit에 대한 입력을 받을 경우 0,
 * 		  그외의 경우 1을 return한다.
 ***********************************************************/
char ProcessCommand(int command);

void DrawDefeatedField();



/***********************************************************
 *	블럭이 일정 시간(1초)마다 내려가도록 호출되는 함수
 *	더이상 내릴수 없을 경우,
 *		블럭을 field에 합친다.
 *		완전이 채워진 line을 지운다.
 *		next block을 current block으로 바꿔주고
 *		block의 좌표를 초기화 한다.
 *		다음 블럭을 화면에 그리고 갱신된 score를 
 *		화면에 display한다.
 *	input	: (int) sig
 *	return	: none
 ***********************************************************/
void BlockDown(int sig);

/***********************************************************
 *	입력된 움직임이 가능한지를 판단해주는 함수.
 *	input	: (char[][]) 블럭의 움직임을 확인할 필드
 *		  (int) 현재 블럭의 모양 ID
 *		  (int) 블럭의 회전 횟수
 *		  (int) 블럭의 Y좌표
 *		  (int) 블럭의 X좌표
 *	return	: (int) 입력에 대한 블럭 움직임이 가능하면 1
 *		  가능하지 않으면 0을 return 한다.
 ***********************************************************/
bool CheckToMove(char f[HEIGHT][WIDTH], char currentBlock, char blockRotate,short int blockY,short int blockX, short int upLim);
bool CheckToMove_Shadow(char f[HEIGHT][WIDTH], char currentBlock, char blockRotate, short int blockY, short int blockX);

/***********************************************************
 *	테트리스에서 command에 의해 바뀐 부분만 다시 그려준다.
 *	input	: (char[][]) command의해 바뀐 블럭을 확인할 필드
 *		  (int) 바뀌기 전 모양을 알기 위해 command를 입력으로 받는다.
 *		  (int) 현재 블럭의 모양 ID
 *		  (int) 블럭의 회전 횟수
 *		  (int) 블럭의 Y좌표
 *		  (int) 블럭의 X좌표
 *	return	: none
 ***********************************************************/
void DrawChange(char f[HEIGHT][WIDTH], int command, char currentBlock, char blockRotate, short int blockY,short int blockX);

/***********************************************************
 *	테트리스의 블럭이 쌓이는 field를 그려준다.
 *	input	: none
 *	return	: none
 ***********************************************************/
void DrawField();
void DrawField_AddBlock();
void DrawField_DeleteLine(char lineCnt, short int upLim, short int yLand);
/***********************************************************
 *	떨어지는 블럭을 field에 더해준다.
 *	input	: (char[][]) 블럭을 쌓을 필드
 *		  (int) 현재 블럭의 모양 ID
 *		  (int) 블럭의 회전 횟수
 *		  (int) 블럭의 Y좌표
 *		  (int) 블럭의 X좌표
 *	return	: none
 ***********************************************************/
char AddBlockToField(char f[HEIGHT][WIDTH], char currentBlock, char blockRotate, short int blockY, short int blockX, short int *upLim);

/***********************************************************
 *	완전히 채워진 Line을 삭제하고 점수를 매겨준다.
 *	input	: (char[][]) 완전히 채워진 line을 확인할 필드
 *	return	: (int) 삭제된 라인의 갯수에 대한 점수
 ***********************************************************/
char DeleteLine(char f[HEIGHT][WIDTH], char currentBlock, char blockRotate, short int upLim, short int blockY, short int *yLand);

/***********************************************************
 *	커서의 위치를 입력된 x, y의 위치로 옮겨주는 역할을 한다.
 *	input	: (int) 커서의 이동할 y 좌표
 *		  (int) 커서의 이동할 x 좌표
 *	return	: none
 ***********************************************************/
void gotoyx(int y, int x);

/***********************************************************
 *	테트리스의 화면 오른쪽상단에 다음 나올 블럭을 그려준다..
 *	input	: (int*) 블럭의 모양에 대한 ID 배열
 *	return	: none
 ***********************************************************/
void DrawNextBlock(char *nextBlock);

/***********************************************************
 *	테트리스의 화면 오른쪽 하단에 Score를 출력한다.
 *	input	: (int) 출력할 점수
 *	return	: none
 ***********************************************************/
void PrintScore(int score);

/***********************************************************
 *	해당 좌표(y,x)에 원하는 크기(height,width)의 box를 그린다.
 *	input	: (int) 그리고자 하는 박스의 왼쪽 상단모서리의 y 좌표
 *		  (int) 왼쪽 상단 모서리의 x 좌표
 *		  (int) 박스의 높이
 *		  (int) 박스의 넓이
 *	return	: none
 ***********************************************************/
void DrawBox(char y, char x, char height, char width);

/***********************************************************
 *	해당 좌표(y,x)에 원하는 모양의 블록을 그린다.
 *	input	: (int) 그리고자 하는 박스의 왼쪽 상단모서리의 y 좌표
 *		  (int) 왼쪽 상단 모서리의 x 좌표
 *		  (int) 블록의 모양
 *		  (int) 블록의 회전 횟수
 *		  (char) 블록을 그릴 패턴 모양
 *	return	: none
 ***********************************************************/
void DrawBlock(short int y, short int x, char blockID, char blockRotate, char tile);
void DeleteBlock(short int y, short int x, char blockID, char blockRotate);

/***********************************************************
 *	블록이 떨어질 위치를 미리 보여준다.
 *	input	: (int) 그림자를 보여줄 블록의 왼쪽 상단모서리의 y 좌표
 *		  (int) 왼쪽 상단 모서리의 x 좌표
 *		  (int) 블록의 모양
 *		  (int) 블록의 회전 횟수
 *	return	: none
 ***********************************************************/
void DrawShadow(short int y, short int x, char blockID, char blockRotate);

/***********************************************************
 *	테트리스 게임을 시작한다.
 *	input	: none
 *	return	: none
 ***********************************************************/
void play();

/***********************************************************
 *	메뉴를 보여준다.
 *	input	: none
 *	return	: 사용자가 입력한 메뉴 번호
 ***********************************************************/
char menu();

/***********************************************************
 *	rank file로부터 랭킹 정보를 읽어와 랭킹 목록을 구성한다.
 *	input	: none
 *	return	: none
 ***********************************************************/
void createRankList();

/***********************************************************
 *	화면에 랭킹 기록들을 보여준다.
 *	input	: none
 *	return	: none
 ***********************************************************/
void rank();

/***********************************************************
 *	rank file을 생성한다.
 *	input	: none
 *	return	: none
 ***********************************************************/
void writeRankFile();

/***********************************************************
 *	새로운 랭킹 정보를 추가한다.
 *	input	: (int) 새로운 랭킹의 점수
 *	return	: none
 ***********************************************************/
void newRank(int score);

/***********************************************************
 *	추천 블럭 배치를 구한다.
 *	input	: (RecNode*) 추천 트리의 루트
 *	return	: (int) 추천 블럭 배치를 따를 때 얻어지는 예상 스코어
 ***********************************************************/
int recommend(Node *root);

/***********************************************************
 *	추천 기능에 따라 블럭을 배치하여 진행하는 게임을 시작한다.
 *	input	: none
 *	return	: none
 ***********************************************************/
void recommendedPlay();
void constructRecTree(Node *root);

void destructRecTree(Node *root);
void drawRecommend();
void drawRecommend_Change();
void removeRecommend();


#endif
