
/*
 * Grant D. McVittie
 * PixelPlatforms
 * 
 * v0.1
 */
#include <Arduboy2.h>
#include <ArduboyTones.h>
#include "bitmaps.h"

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);

/*
 * timer
 */
int TIME = 10;
/*
 * game state
 * 0 = intro
 * 1 = play
 * 2 = game over
 */
int GAME_STATE = 0;
/*
 * levels
 */
int CURRENT_LEVEL = 0;
/*
 * player position (starts at 8,8 every level)
 */
int PLAYER_X = 8;
int PLAYER_Y = 8;
bool PLAYER_UP = false;
bool PLAYER_DOWN = true;
bool PLAYER_LEFT = false;
bool PLAYER_RIGHT = false;
bool PLAYER_DIE = false;
int PLAYER_SPEED = 2;
/*
 * bomb
 */
int BOMB_X = 8;
int BOMB_Y = 8;
bool BOMB_DONE = false;
bool BOMB_DROPPED = false;
int BOMBS_LEFT = 3;
/*
 * baddies
 */
int BADDIE1_X = 112;
int BADDIE1_Y = 48;
bool BADDIE1_LEFT = true;
int BADDIE2_X = 112;
int BADDIE2_Y = 32;
bool BADDIE2_LEFT = false;
/*
 * collisions
 */
bool COLLIDE = false;
/*
 * grid positions (first [8,8] is reserved for player)
 */
int X_POSITIONS [] = {8,16,24,32,40,48,56,64,72,80,88,96,104,112};
int Y_POSITIONS [] = {8,16,24,32,40,48,56};
/*
 * bounderies
 */
int MIN_X = 8;
int MAX_X = 112;
int MIN_Y = 8;
int MAX_Y = 48;
/*
 * crates
 */
int MAX_CRATES = 80; //actually half this, but we store both x and y, so double.
int CRATE_POSITIONS[80];
bool CRATES_ADDED = false;
/*
 * door for level complete
 */
int DOOR_X = 112;
int DOOR_Y = 48;

/*
 * ok go
 */
void setup() {
  arduboy.boot();
  arduboy.systemButtons();
  arduboy.audio.begin();
  arduboy.setFrameRate(30); 
  arduboy.initRandomSeed();
}

void loop() {
  if (!(arduboy.nextFrame()))
    return;

  //clean slate
  arduboy.clear();
  
  //build level
  buildLevel();
}



void buildLevel(){
  arduboy.clear();
  switch(GAME_STATE){
    case 0:
      arduboy.drawSlowXYBitmap(0,0,title, 128, 64, WHITE);
    break;
    case 2:
      arduboy.setCursor(12,12);
      arduboy.print( String("YOU HAVE ONE JOB,") );
      arduboy.setCursor(12,20);
      arduboy.print( String("5 BOMBS, AND 10s:") );      
      arduboy.setCursor(12,28);
      arduboy.print( String("GET TO THE DOOR...") );
      arduboy.setCursor(12,36);
      arduboy.print( String("ALIVE.") );
      arduboy.setCursor(64,56);
      arduboy.print( String("GOOD LUCK!") );
      delay(250);
    break;
    case 1:
      if(COLLIDE){
        arduboy.drawSlowXYBitmap(0,0,gameover,128,64,WHITE);
        if(arduboy.everyXFrames(125)){
          CURRENT_LEVEL = 1;
          reset();
        }
      } else {
        arduboy.drawSlowXYBitmap(0,0,background, 128, 64, WHITE);
        //crates
        if(CRATES_ADDED == false){
          generateCratePositions();
        } else {
          drawCrates();     
        }
        //collisions
        collisionDetection();
        //player
        addPlayer();
        //baddies
        addBaddies();
        //add bombs
        addBombs();
        //countdown
        countDown();
      }     
    break;
  }
  //button presses
  delay(20);
  handleButtons();
  
  arduboy.display();
}

void countDown(){
  if(arduboy.everyXFrames(30)){
    if(TIME>1){
      TIME--;
    } else {
      soundGameOver();
      CURRENT_LEVEL = 1;
      reset();
    }
  }

  arduboy.setCursor(68,0);
  arduboy.print( String(CURRENT_LEVEL) );
  
  arduboy.setCursor(108,0);
  arduboy.print( String(TIME) );
  
}

/*
 * button handling
 */
 void handleButtons(){
  switch(GAME_STATE){
    //intro
    case 0:
      if ( arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON) ){
        //reset();
        GAME_STATE = 2; 
      }
    break;
    case 2:
      if ( arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON) ){
        reset(); 
      }
    break;
    //playing
    case 1:
      if(!COLLIDE){
        if ( arduboy.pressed(RIGHT_BUTTON) ){
          if(PLAYER_X < MAX_X){
            PLAYER_X = PLAYER_X + PLAYER_SPEED;
            PLAYER_UP = false;
            PLAYER_DOWN = false;
            PLAYER_LEFT = true;
            PLAYER_RIGHT = false;
            soundMove();
          }
        }
        if ( arduboy.pressed(LEFT_BUTTON) ){
          if(PLAYER_X > MIN_X){
            PLAYER_X = PLAYER_X - PLAYER_SPEED;          
            PLAYER_UP = false;
            PLAYER_DOWN = false;
            PLAYER_LEFT = false;
            PLAYER_RIGHT = true;
            soundMove();
          }
        }
        if ( arduboy.pressed(UP_BUTTON) ){
          if(PLAYER_Y > MIN_Y){
            PLAYER_Y = PLAYER_Y - PLAYER_SPEED;
            PLAYER_UP = true;
            PLAYER_DOWN = false;
            PLAYER_LEFT = false;
            PLAYER_RIGHT = false;
            soundMove();
          }
        }
        if ( arduboy.pressed(DOWN_BUTTON) ){
          if(PLAYER_Y < MAX_Y){
            PLAYER_Y = PLAYER_Y + PLAYER_SPEED;
            PLAYER_UP = false;
            PLAYER_DOWN = true;
            PLAYER_LEFT = false;
            PLAYER_RIGHT = false;
            soundMove();
          }
        }
        if( arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON) ){
          dropBomb();
        }     
        
      } else {
        if ( arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON) ){
          CURRENT_LEVEL = 1;
          reset(); 
        }   
      }
    break;
  }
 }




/*
 * crates
 */
void generateCratePositions(){
  arduboy.initRandomSeed();
  int i;
  for(i=0;i<MAX_CRATES;i=i+2){
    int posX = rand() % 13;
    int posY = rand() % 5;
    //no crates in the player start position (8,8)
    if(posX == 0 && posY == 0){
      posX = 1;
      posY = 2;
    }
    CRATE_POSITIONS[i] = X_POSITIONS[posX];
    CRATE_POSITIONS[i+1] = Y_POSITIONS[posY];
  }
  CRATES_ADDED = true;  
}
void drawCrates(){
  int i;
  for(i=0;i<MAX_CRATES;i=i+2){
    arduboy.drawBitmap(CRATE_POSITIONS[i],CRATE_POSITIONS[i+1],crate,8,8,BLACK);    
  }
  //add door
  arduboy.drawBitmap(DOOR_X,DOOR_Y,door,8,8,BLACK);
}

/*
 * player
 */
void addPlayer(){
  if(PLAYER_UP){
    arduboy.drawBitmap(PLAYER_X,PLAYER_Y,player_up,8,8,BLACK);
  }
  if(PLAYER_DOWN){
    arduboy.drawBitmap(PLAYER_X,PLAYER_Y,player_down,8,8,BLACK);
  }
  if(PLAYER_LEFT){
    arduboy.drawBitmap(PLAYER_X,PLAYER_Y,player_left,8,8,BLACK);
  }
  if(PLAYER_RIGHT){
    arduboy.drawBitmap(PLAYER_X,PLAYER_Y,player_right,8,8,BLACK);
  }
  
}

/*
 * baddies
 */
void addBaddies(){
  if(BADDIE1_LEFT){
    if(BADDIE1_X>MIN_X){
      BADDIE1_X--;
    } else {
      BADDIE1_LEFT = false;
    }
  } else {
    if(BADDIE1_X<MAX_X){
      BADDIE1_X++;
    } else {
      BADDIE1_LEFT = true;
    }
  }
  if(arduboy.everyXFrames(6)){
    arduboy.drawBitmap(BADDIE1_X,BADDIE1_Y,baddie1b,8,8,BLACK);
  }else{
    arduboy.drawBitmap(BADDIE1_X,BADDIE1_Y,baddie1a,8,8,BLACK);
  }
  //baddie 2
  if(BADDIE2_LEFT){
    if(BADDIE2_X>MIN_X){
      BADDIE2_X--;
    } else {
      BADDIE2_LEFT = false;
    }
  } else {
    if(BADDIE2_X<MAX_X){
      BADDIE2_X++;
    } else {
      BADDIE2_LEFT = true;
    }
  }
  if(arduboy.everyXFrames(4)){
    arduboy.drawBitmap(BADDIE2_X,BADDIE2_Y,baddie2b,8,8,BLACK);
  }else{
    arduboy.drawBitmap(BADDIE2_X,BADDIE2_Y,baddie2a,8,8,BLACK);
  }
}



/*
 * bomb
 */
 void dropBomb(){
  if(!BOMB_DONE && !BOMB_DROPPED && BOMBS_LEFT>0){
    soundGood();
    BOMB_DROPPED = true;
    BOMB_X = PLAYER_X+2;
    BOMB_Y = PLAYER_Y+2;
    BOMBS_LEFT--;    
  }
 }


 void addBombs(){
  arduboy.setCursor(16,0);
  arduboy.print( String(BOMBS_LEFT) );
  
  if(BOMB_DROPPED && !BOMB_DONE){
    arduboy.drawBitmap(BOMB_X,BOMB_Y,bomb,4,4,BLACK);
  }
  
  if(BOMB_DROPPED && BOMB_DONE){
    arduboy.drawBitmap(BOMB_X,BOMB_Y,bomb_explode,4,4,BLACK);
    //get 'real' bomb position
    int realX = BOMB_X-2;
    int realY = BOMB_Y-2;
    //crates surrounding bomb
    int UP[2]={realX,realY-8};
    int DOWN[2]={realX,realY+8};
    int LEFT[2]={realX-8,realY};
    int RIGHT[2]={realX+8,realY};
    int i;
    int tolerance = 6;
    for(i=0;i<MAX_CRATES;i=i+2){
      int CRATE[2]={CRATE_POSITIONS[i],CRATE_POSITIONS[i+1]};
      bool hit = false;
      
      if(CRATE[0] == UP[0] && CRATE[1] == UP[1] ){
         hit = true;
      }      
      if(CRATE[0] == DOWN[0] && CRATE[1] == DOWN[1]){
         hit = true;
      }
      if(CRATE[0] == LEFT[0] && CRATE[1] == LEFT[1]){
         hit = true;
      }
      if(CRATE[0] == RIGHT[0] && CRATE[1] == RIGHT[1]){
         hit = true;
      }
      
      if(hit){
        CRATE_POSITIONS[i] = -64;
        CRATE_POSITIONS[i+1] = -64;
      }
    }
    BOMB_DROPPED = false;
    BOMB_DONE = false;
  }
  if(arduboy.everyXFrames(60)){
    if(BOMB_DROPPED && !BOMB_DONE){
      BOMB_DONE = true;
      soundEgg();
    }
  }
 }

/*
 * collisions
 */
void collisionDetection(){
  COLLIDE = false;
  int i;
  for(i=0;i<MAX_CRATES;i=i+2){
    int CRATE[2]={CRATE_POSITIONS[i],CRATE_POSITIONS[i+1]};
    if (PLAYER_X < CRATE[0] + 7 && PLAYER_X + 7 > CRATE[0] && PLAYER_Y < CRATE[1] + 7 && 7 + PLAYER_Y > CRATE[1]) {
      COLLIDE = true;
      soundBad();
    }
  } 

  if (PLAYER_X < BADDIE1_X + 7 && PLAYER_X + 7 > BADDIE1_X && PLAYER_Y < BADDIE1_Y + 7 && 7 + PLAYER_Y > BADDIE1_Y) {
    COLLIDE = true;
    soundBad();
  }

  if (PLAYER_X < BADDIE2_X + 7 && PLAYER_X + 7 > BADDIE2_X && PLAYER_Y < BADDIE2_Y + 7 && 7 + PLAYER_Y > BADDIE2_Y) {
    COLLIDE = true;
    soundBad();
  }

  if (PLAYER_X < DOOR_X + 7 && PLAYER_X + 7 > DOOR_X && PLAYER_Y < DOOR_Y + 7 && 7 + PLAYER_Y > DOOR_Y) {
    soundHit();
    CURRENT_LEVEL++;
    reset();
  }
}



/*
 * reset game
 */
void reset(){
  PLAYER_X = 8;
  PLAYER_Y = 8;
  PLAYER_UP = false;
  PLAYER_DOWN = true;
  PLAYER_LEFT = false;
  PLAYER_RIGHT = false; 
  PLAYER_DIE = false;
  CRATES_ADDED = false;
  COLLIDE = false;
  GAME_STATE = 1;

  int posX1 = rand() % 14;
  BADDIE1_X = X_POSITIONS[posX1];
  int posX2 = rand() % 14;
  BADDIE2_X = X_POSITIONS[posX2];
  int posY1 = rand() % 6;
  BADDIE1_Y = Y_POSITIONS[posY1];
  int posY2 = rand() % 6;
  BADDIE2_Y = Y_POSITIONS[posY2];

  BOMB_DROPPED = false;
  BOMB_DONE = false;
  BOMB_X = PLAYER_X+2;
  BOMB_Y = PLAYER_Y+2;
  BOMBS_LEFT = 5;

  TIME = 10;
  
  delay(100); 
}

void trace(String thepraise){
  arduboy.setCursor(0,56);
  arduboy.print(thepraise);
}
