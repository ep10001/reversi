/**@file    reversi.c
 * @version 0.0.9
 * @brief   This is the Rebersi Game.
 * 
 * @author  K.A.(ep10001) 2012 original version
 * @section COPYRIGHT
 * This program is free/libre and open-source software; released under
 * CC0 1.0 Universal (CC0 1.0) Public Domain Dedication, and any later version.
 *
 * @todo restruct mainloop for using recursive call.
 *
 * @date
 * 2012-06-06T22:00:00+09:00
 * - version 0.0.9 fix: undefined code
 * 2012-02-08T13:26:00+09:00
 * - version 0.0.8 fix: wrong free
 * 2012-02-02T12:00:00+09:00
 * - version 0.0.7 initial release
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define N 10 //(N-2)*(N-2)の盤面

/** @note Node as tag-named has another namespace from Node as struct-named */
typedef struct Node{
        struct Node *child;
		struct Node *brother;
        char   color;
        int    x,y;
        int    field;
        char   board[N][N];
}Node;

int   Precondition   (Node *head);
int   MainLoop       (Node *head);
int   Postcondition  (Node *head);
int   InitNode       (Node *p   );
int   ClearDTree     (Node *p   );
int   PrintBoard     (char board[N][N]);
int   InitBoard      (char board[N][N]);
int   SetPiece       (char board[N][N], char color, int x, int y);
int   SelectTurn     (void);
int   RevPieces      (char board[N][N], char color, int x, int y);
int   CheckRevPieces (char board[N][N], char color, int x, int y);
int   CountRevPieces (char board[N][N], char color, int x, int y, int bx, int by);
int   ReturnPieces   (char board[N][N], char color, int x, int y, int bx, int by, int count);
int   PC_Turn        (Node *head, int* pass);
int   NPC_Turn       (Node *head, int* pass);
int   InputTurn      (char board[N][N],char color);
bool  IsPassTurn     (char board[N][N],char color);
int   InputBest      (Node *head, char color);
int   CountField     (char board[N][N],char color);
int   MakeChilds     (Node *mother, char color);
Node* SelectNode     (Node *head, char color);
int   LastMessage    (char board[N][N]);

int Precondition(Node *head){
  InitNode  (head);
  PrintBoard(head->board);
  return(0);
}

int SelectTurn(void){
  int result;
  printf("Which turn do you choice? 0:first 1:second\n");
  do{
    scanf("%d",&result);
  }while(!(result==0||result==1));
  printf("You choice %s turn.\n", result?"second":"first");
  return(result);
}

int InitNode(Node *p){
  p->child=NULL;
  p->brother=NULL;
  InitBoard(p->board);
  return(0);
}

int InitBoard(char board[N][N]){
  int turn;
  int center = N/2;
  turn = SelectTurn();
  /* 周囲1マスを含むマスをeで初期化 */
  for(int i=0;i<=N-1;i++){
    for(int j=0;j<=N-1;j++){
      SetPiece(board,'e',i,j);
    }
  }
  SetPiece(board,'W',center  ,center  );
  SetPiece(board,'B',center  ,center-1);
  SetPiece(board,'B',center-1,center  );
  SetPiece(board,'W',center-1,center-1);
  if(turn==1){
    RevPieces(board,'B',center  ,center+1);
  }
  return(0);
}

int CopyBoard(char from[N][N],char to[N][N]){
  /* 周囲1マスを含めてコピー */
  for(int i=0;i<=N-1;i++){
    for(int j=0;j<=N-1;j++){
      SetPiece(to,from[i][j],i,j);
    }
  }
  return(0);
}

int SetPiece(char board[N][N], char color, int x, int y){
  board[x][y]=color;
  return(0);
}

int MainLoop(Node *head){
  int pass   = 0;
  
  printf("start main loop\n");
  while(pass<2){
    PC_Turn (head,&pass);
    NPC_Turn(head,&pass);
  }
  
  LastMessage(head->board);

  return(0);
}

int LastMessage(char board[N][N]){
  int wcount=0;
  int bcount=0;
  for  (int i = 1;i<=N-2;i++){
    for(int j = 1;j<=N-2;j++){
      if(board[i][j]=='W'){
        wcount++;
      }
      else if(board[i][j]=='B'){
        bcount++;
      }
    }
  }
  if(wcount==bcount){
    printf("a drawn\n");
  }else if(wcount>bcount){
    printf("You got win\n");
  }else if(wcount<bcount){
    printf("You got lose\n");
  }
  return(0);
}

int PC_Turn(Node *head, int* pass){
  char color  = 'W';
  if(IsPassTurn(head->board,color)){
    *pass += 1;
    if(*pass<2){
      printf("You got pass\n");
    }
  }
  else{
    *pass = 0;
    InputTurn(head->board,color);
    PrintBoard(head->board);
  }
  return(0);
}

int NPC_Turn(Node *head, int* pass){
  char color  = 'B';

  if(IsPassTurn(head->board,color)){
    *pass += 1;
    if(*pass<2){
      printf("NPC got pass\n");
    }
  }
  else{
    *pass = 0;
    InputBest(head,color);
    PrintBoard(head->board);
  }
  return(0);
}

int InputBest(Node *head, char color){
  Node *BestNode;
  BestNode = SelectNode(head,color);
  RevPieces(head->board,color,BestNode->x,BestNode->y);
  printf("%s set %c on %d,%d\n","NPC",color,BestNode->x,BestNode->y);
  ClearDTree(head->child);
  head->child=NULL;
  return(0);
}

Node* SelectNode(Node *head, char color){
  //Min-Max法で2手先(?)を読みます　評価対象＝置けるマスの数
  int min = N*N;
  int max = 0;
  Node *p,*q;
  Node *result=NULL;

  //再帰が使いこなせるひとはもっと格好よく書けるんだろうけれど……
  MakeChilds(head,color);
  for(p=head->child;p!=NULL;p=p->brother){
    min = N*N;
    MakeChilds(p,(color=='W'?'B':'W'));
    for(q=p->child;q!=NULL;q=q->brother){
      if(q->field<=min){
        min = q->field;
      }
    }
    if(max <= min){
      max = min;
      result = p;
    }
  }
  return(result);
}

int InputTurn(char board[N][N],char color){
  int  x,y;
  for(;;){
    printf("please input x and y\n");
    scanf("%d %d",&x,&y);
    if(CheckRevPieces(board,color,x,y)>0){
      RevPieces(board,color,x,y);
      break;
    }else{
      fprintf(stderr,"You cannot set piece on %d,%d\n",x,y);
      continue;
    }
  }
  printf("%s set %c on %d,%d\n","You",color,x,y);
  return(0);
}

bool IsPassTurn(char board[N][N],char color){
  bool result = true;
  for  (int i=1;result&&(i<=N-2);i++){
    for(int j=1;result&&(j<=N-2);j++){
      result  = (CheckRevPieces(board,color,i,j)==0);
    }
  }
  return(result);
}

int CheckRevPieces(char board[N][N], char color, int x, int y){
  int  result  = 0;

  if((board[x][y]=='e')&&(1<=x&&x<=N-2)&&(1<=y&&y<=N-2)){
    for   (int i=-1;i<=1;i++){
      for (int j=-1;j<=1;j++){
        result += CountRevPieces(board,color,x,y,i,j);
      }
    }
  }
  return(result);
}

int CountRevPieces(char board[N][N],char color,int x,int y,int bx,int by){
  int result   = 0;
  for(int i=1; (board[x+i*bx][y+i*by]!=color)&&((bx!=0)||(by!=0)); i++){
    if(board[x+i*bx][y+i*by]=='e'){
      result = 0;
      break;
    }
    result++;
  }
  return(result);
}

int RevPieces(char board[N][N], char color, int x, int y){
  int  result  = 0;
  int  count;

  if((board[x][y]=='e')&&(1<=x&&x<=N-2)&&(1<=y&&y<=N-2)){
    for  (int i=-1;i<=1;i++){
      for(int j=-1;j<=1;j++){
        count  = CountRevPieces(board,color,x,y,i,j);
        ReturnPieces(board,color,x,y,i,j,count);
      }
    }
  }
  return(result);
}

int MakeChilds(Node *mother,char color){
  Node *p;
//  printf("check %c \n",color);
//  PrintBoard(mother->board);
  if(IsPassTurn(mother->board,color)){
    mother->child = (Node *)malloc(sizeof(Node));
    mother->child->color   = color;
    mother->child->child   = NULL;
    mother->child->brother = NULL;
    CopyBoard(mother->board,mother->child->board);
    mother->child->field   = CountField(mother->child->board,(color=='W'?'B':'W'));
  }
//  printf("check1\n");
  for(int i=1;i<=N-1;i++){
    for(int j=1;j<=N-1;j++){
      if(CheckRevPieces(mother->board,color,i,j)>0){
          p                      = mother->child;
          mother->child          = (Node *)malloc(sizeof(Node));
          mother->child->color   = color;
          mother->child->x       = i;
          mother->child->y       = j;
          mother->child->child   = NULL;
          mother->child->brother = p;
          CopyBoard(mother->board,mother->child->board);
          RevPieces(mother->child->board,color,i,j);
          mother->child->field   = CountField(mother->child->board,(color=='W'?'B':'W'));
      }
    }
  }
//  printf("check2\n");
  return(0);
}

int CountField(char board[N][N],char color){
  int result = 0;
  for  (int i=1;i<=N-2;i++){
    for(int j=1;j<=N-2;j++){
      if(CheckRevPieces(board,color,i,j)>0){
        result++;
      }
    }
  }
  return(result);
}

int ReturnPieces(char board[N][N],char color,int x,int y,int bx,int by,int count){
  for(int i=0; i<=count; i++){
    SetPiece(board,color,x+i*bx,y+i*by);
  }
  return(0);
}

int Postcondition(Node *head){
  ClearDTree(head->child);
  exit(EXIT_SUCCESS);
}

int ClearDTree(Node *p){
  if(p!=NULL){
    ClearDTree(p->child);
    ClearDTree(p->brother);
    free(p);
  }
  return(0);
}

int PrintBoard(char board[N][N]){
  printf("0");
  for(int i=1;i<=N-2;i++){
    printf("%d",i);
  }
  printf("\n");
  for(int i=1;i<=N-2;i++){
    printf("%d",i);
    for(int j=1;j<=N-2;j++){
      printf("%c", board[i][j]);
    }
    printf("\n");
  }
  printf("^x y->\n");
  return(0);
}

int main(void){
  Node head;

  Precondition(&head);
  MainLoop(&head);
  Postcondition(&head);

  exit(EXIT_FAILURE);
}
/* need newline at end of file */
