
/*
 * Grant D. McVittie
 * PixelPlatforms
 * 
 * v0.2
 * 
 * UPDATES:
 * - more backgrounds and spike types
 * - added a vertical baddie
 * - drop a bomb with b, explode it with a (use to be timed, but now you have control)
 * - randomized door position
 * - added easy mode
 * - added sound on/off
 * - adjusted hit areas
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
 * MODE
 * 0 = normal
 * 1 = easy
 */
bool EASY = true;
/*
 * audio
 */
bool SOUND_ENABLED = false;
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
int BADDIE3_X = 24;
int BADDIE3_Y = 16;
bool BADDIE3_DOWN = true;
int BADDIE4_X = 24;
int BADDIE4_Y = 16;
bool BADDIE4_DOWN = false;
/*
 * collisions
 */
bool COLLIDE = false;
bool COLLIDE_BLOCK = false;
/*
 * grid positions (first [8,8] is reserved for player)
 */
int X_POSITIONS [] = {8,24,32,40,56,64,72,88,104,112};
int X_POS_LEN = 9;
int Y_POSITIONS [] = {8,24,40,56};
int Y_POS_LEN = 3;
int BLOCK_X_POSITIONS [] = {16,48,80,96};
int BLOCK_X_POS_LEN = 3;
int BLOCK_Y_POSITIONS [] = {16,32,48};
int BLOCK_Y_POS_LEN = 2;
int DOOR_X_POSITIONS [] = {24,32,40,56,64,72,88,104,112};
int DOOR_X_POS_LEN = 8;
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
int MAX_BLOCKS = 20;
int BLOCK_POSITIONS[20];
bool CRATES_ADDED = false;
/*
 * door for level complete
 */
int DOOR_X = 112;
int DOOR_Y = 48;
/*
 * background to use
 */
int BG = 0;
int CR8 = 0;


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
      if(SOUND_ENABLED){
        arduboy.drawBitmap(29,47,sound_on,16,16,BLACK);
      }else{
        arduboy.drawBitmap(29,47,sound_off,16,16,BLACK);
      }
      if(!EASY){
        arduboy.drawBitmap(50,47,hard,16,16,BLACK);
      }else{
        arduboy.drawBitmap(50,47,easy,16,16,BLACK);
      }
    break;
    case 2:
      arduboy.setCursor(12,12);
      arduboy.print( String("YOU HAVE ONE JOB,") );
      arduboy.setCursor(12,20);
      arduboy.print( String("3 BOMBS, AND 10s:") );      
      arduboy.setCursor(12,28);
      arduboy.print( String("GET TO THE DOOR...") );
      arduboy.setCursor(12,36);
      arduboy.print( String("ALIVE.") );
      arduboy.setCursor(64,56);
      arduboy.print( String("GOOD LUCK!") );
      delay(150);
    break;
    case 1:
      if(COLLIDE){
        arduboy.drawSlowXYBitmap(0,0,gameover,128,64,WHITE);
        if(arduboy.everyXFrames(125)){
          CURRENT_LEVEL = 1;
          reset();
        }
      } else {
        
        if(BG == 0){
          arduboy.drawSlowXYBitmap(0,0,background3, 128, 64, WHITE);
        } else if(BG == 1){
          arduboy.drawSlowXYBitmap(0,0,background2, 128, 64, WHITE);
        } else{
          arduboy.drawSlowXYBitmap(0,0,background, 128, 64, WHITE);
        }

        
        
        //crates
        if(CRATES_ADDED == false){
          generateCratePositions();
        } else {
          drawCrates();     
        }
        
        //player
        addPlayer();
        //baddies
        addBaddies();
        //add bombs
        addBombs();
        //countdown
        countDown();
        //collisions
        collisionDetection();
      }     
    break;
  }
  //button presses
  delay(20);
  handleButtons();
  
  arduboy.display();
}

void countDown(){
  int frm = 30;
  if(EASY){
    frm = 15;
  }
  if(arduboy.everyXFrames(frm)){
    if(TIME>1){
      TIME--;
    } else {
      if(SOUND_ENABLED){
        soundGameOver();
      }
      
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
      if ( arduboy.pressed(A_BUTTON) ){
        if(SOUND_ENABLED){
          SOUND_ENABLED = false;
        } else {
          SOUND_ENABLED = true;
        }
      }
      if ( arduboy.pressed(B_BUTTON) ){
        if(EASY){
          EASY = false;
        } else {
          EASY = true;
        }
      }
      
      if ( arduboy.pressed(RIGHT_BUTTON) || arduboy.pressed(LEFT_BUTTON) || arduboy.pressed(UP_BUTTON) || arduboy.pressed(DOWN_BUTTON) ){
        GAME_STATE = 2; 
      }
      
      delay(60);
    break;
    case 2:
      if ( arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON) ){
        reset(); 
      }
    break;
    //playing
    case 1:
      if(EASY){
        PLAYER_SPEED = 8;
      }
      if(!COLLIDE){
        if ( arduboy.pressed(RIGHT_BUTTON) ){
          if(PLAYER_X < MAX_X){
            PLAYER_X = PLAYER_X + PLAYER_SPEED;
            PLAYER_UP = false;
            PLAYER_DOWN = false;
            PLAYER_LEFT = true;
            PLAYER_RIGHT = false;
            if(SOUND_ENABLED){
              soundMove();
            }
            
          }
        }
        if ( arduboy.pressed(LEFT_BUTTON) ){
          if(PLAYER_X > MIN_X){
            PLAYER_X = PLAYER_X - PLAYER_SPEED;          
            PLAYER_UP = false;
            PLAYER_DOWN = false;
            PLAYER_LEFT = false;
            PLAYER_RIGHT = true;
            if(SOUND_ENABLED){
              soundMove();
            }
          }
        }
        if ( arduboy.pressed(UP_BUTTON) ){
          if(PLAYER_Y > MIN_Y){
            PLAYER_Y = PLAYER_Y - PLAYER_SPEED;
            PLAYER_UP = true;
            PLAYER_DOWN = false;
            PLAYER_LEFT = false;
            PLAYER_RIGHT = false;
            if(SOUND_ENABLED){
              soundMove();
            }
          }
        }
        if ( arduboy.pressed(DOWN_BUTTON) ){
          if(PLAYER_Y < MAX_Y){
            PLAYER_Y = PLAYER_Y + PLAYER_SPEED;
            PLAYER_UP = false;
            PLAYER_DOWN = true;
            PLAYER_LEFT = false;
            PLAYER_RIGHT = false;
            if(SOUND_ENABLED){
              soundMove();
            }
          }
        }
        if( arduboy.pressed(B_BUTTON) ){
          dropBomb();
        }     
        if( arduboy.pressed(A_BUTTON) ){
          if(BOMB_DROPPED && !BOMB_DONE){
            BOMB_DONE = true;
            if(SOUND_ENABLED){
              soundEgg();
            }
          }
        }
      } else {
        if ( arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON) ){
          CURRENT_LEVEL = 1;
          reset(); 
        }   
      }
      if(EASY){
        delay(30);
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
    int posX = rand() % X_POS_LEN;
    int posY = rand() % Y_POS_LEN;
    //no crates in the player start position (8,8)
    if(posX == 0 && posY == 0){
      posX = 1;
      posY = 2;
    }       
    CRATE_POSITIONS[i] = X_POSITIONS[posX];
    CRATE_POSITIONS[i+1] = Y_POSITIONS[posY];
  }

  for(i=0;i<MAX_BLOCKS;i=i+2){
    int posX = rand() % BLOCK_X_POS_LEN;
    int posY = rand() % BLOCK_Y_POS_LEN;
    BLOCK_POSITIONS[i] = BLOCK_X_POSITIONS[posX];
    BLOCK_POSITIONS[i+1] = BLOCK_Y_POSITIONS[posY];
  }

  int doorX = rand() % DOOR_X_POS_LEN;
  DOOR_X = DOOR_X_POSITIONS[doorX];
  CRATES_ADDED = true;  
}
void drawCrates(){
  int i;
  int j;
  for(i=0;i<MAX_CRATES;i=i+2){
    bool failed = true;
    for(j=0;j<MAX_BLOCKS;j=j+2){
      if(BLOCK_POSITIONS[j] != CRATE_POSITIONS[i] && BLOCK_POSITIONS[j+1] != CRATE_POSITIONS[i+1]){
       failed = false;  
      }
    }
    if(!failed){
      if(CR8 == 0){
        arduboy.drawBitmap(CRATE_POSITIONS[i],CRATE_POSITIONS[i+1],crate2,8,8,BLACK);  
      } else if(CR8 == 1){
        arduboy.drawBitmap(CRATE_POSITIONS[i],CRATE_POSITIONS[i+1],crate1,8,8,BLACK); 
      } else {
        arduboy.drawBitmap(CRATE_POSITIONS[i],CRATE_POSITIONS[i+1],crate,8,8,BLACK); 
      }
    }      
  }
  for(i=0;i<MAX_BLOCKS;i=i+2){
    if(BLOCK_POSITIONS[i] && BLOCK_POSITIONS[i] != 0 && BLOCK_POSITIONS[i] != 8){
      if(BLOCK_POSITIONS[i+1] && BLOCK_POSITIONS[i+1] != 0 && BLOCK_POSITIONS[i+1] != 8){
        arduboy.drawBitmap(BLOCK_POSITIONS[i],BLOCK_POSITIONS[i+1],block,8,8,BLACK);
      }
    }
    
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

  //baddie 3
  if(BADDIE3_DOWN){
    if(BADDIE3_Y<MAX_Y){
      BADDIE3_Y++;
    } else {
      BADDIE3_DOWN = false;
    }
  } else {
    if(BADDIE3_Y>MIN_Y){
      BADDIE3_Y--;
    } else {
      BADDIE3_DOWN = true;
    }
  }
  if(arduboy.everyXFrames(4)){
    arduboy.drawBitmap(BADDIE3_X,BADDIE3_Y,baddie3b,8,8,BLACK);
  }else{
    arduboy.drawBitmap(BADDIE3_X,BADDIE3_Y,baddie3a,8,8,BLACK);
  }

  //baddie 4
  if(BADDIE4_DOWN){
    if(BADDIE4_Y<MAX_Y){
      BADDIE4_Y++;
    } else {
      BADDIE4_DOWN = false;
    }
  } else {
    if(BADDIE4_Y>MIN_Y){
      BADDIE4_Y--;
    } else {
      BADDIE4_DOWN = true;
    }
  }
  if(arduboy.everyXFrames(4)){
    arduboy.drawBitmap(BADDIE4_X,BADDIE4_Y,baddie3a,8,8,BLACK);
  }else{
    arduboy.drawBitmap(BADDIE4_X,BADDIE4_Y,baddie3b,8,8,BLACK);
  }
}



/*
 * bomb
 */
 void dropBomb(){
  if(!BOMB_DONE && !BOMB_DROPPED && BOMBS_LEFT>0){
    if(SOUND_ENABLED){
      soundGood();
    }
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
 }

/*
 * collisions
 */
void collisionDetection(){
  COLLIDE = false;
  int i;
  for(i=0;i<MAX_CRATES;i=i+2){
    int CRATE[2]={CRATE_POSITIONS[i],CRATE_POSITIONS[i+1]};
    if (PLAYER_X < CRATE[0] + 6 && PLAYER_X + 6 > CRATE[0] && PLAYER_Y < CRATE[1] + 6 && 6 + PLAYER_Y > CRATE[1]) {
      COLLIDE = true;
      if(SOUND_ENABLED){
        soundBad();
      }
    }
  } 

  for(i=0;i<MAX_BLOCKS;i=i+2){
    int BLOCK[2]={BLOCK_POSITIONS[i],BLOCK_POSITIONS[i+1]};
    if (PLAYER_X < BLOCK[0] + 6 && PLAYER_X + 6 > BLOCK[0] && PLAYER_Y < BLOCK[1] + 6 && 6 + PLAYER_Y > BLOCK[1]) {
      COLLIDE = true;
      if(SOUND_ENABLED){
        soundBad();
      }
    }
  }

  if (PLAYER_X < BADDIE1_X + 7 && PLAYER_X + 7 > BADDIE1_X && PLAYER_Y < BADDIE1_Y + 7 && 7 + PLAYER_Y > BADDIE1_Y) {
    COLLIDE = true;
    if(SOUND_ENABLED){
      soundBad();
    }
  }

  if (PLAYER_X < BADDIE2_X + 7 && PLAYER_X + 7 > BADDIE2_X && PLAYER_Y < BADDIE2_Y + 7 && 7 + PLAYER_Y > BADDIE2_Y) {
    COLLIDE = true;
    if(SOUND_ENABLED){
      soundBad();
    }
  }

  if (PLAYER_X < BADDIE3_X + 7 && PLAYER_X + 7 > BADDIE3_X && PLAYER_Y < BADDIE3_Y + 7 && 7 + PLAYER_Y > BADDIE3_Y) {
    COLLIDE = true;
    if(SOUND_ENABLED){
      soundBad();
    }
  }

  if (PLAYER_X < BADDIE4_X + 7 && PLAYER_X + 7 > BADDIE4_X && PLAYER_Y < BADDIE4_Y + 7 && 7 + PLAYER_Y > BADDIE4_Y) {
    COLLIDE = true;
    if(SOUND_ENABLED){
      soundBad();
    }
  }
  
  if (PLAYER_X < DOOR_X + 7 && PLAYER_X + 7 > DOOR_X && PLAYER_Y < DOOR_Y + 7 && 7 + PLAYER_Y > DOOR_Y) {
    if(SOUND_ENABLED){
      soundHit();
    }
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

  int posX1 = rand() % X_POS_LEN;
  BADDIE1_X = X_POSITIONS[posX1];
  int posX2 = rand() % X_POS_LEN;
  BADDIE2_X = X_POSITIONS[posX2];
  int posX3 = rand() % X_POS_LEN;
  if(posX3==0){
    posX3 = 1;
  }
  BADDIE3_X = X_POSITIONS[posX3];
  int posX4 = rand() % X_POS_LEN;
  if(posX4==0){
    posX4 = 1;
  }
  BADDIE4_X = X_POSITIONS[posX4];
  int posY1 = rand() % Y_POS_LEN;
  BADDIE1_Y = Y_POSITIONS[posY1];
  int posY2 = rand() % Y_POS_LEN;
  BADDIE2_Y = Y_POSITIONS[posY2];
  int posY3 = rand() % Y_POS_LEN;
  BADDIE3_Y = Y_POSITIONS[posY3];
  int posY4 = rand() % Y_POS_LEN;
  BADDIE4_Y = Y_POSITIONS[posY4];


  if(BADDIE1_X<24 && BADDIE1_LEFT){
    BADDIE1_LEFT = false;
  }
  if(BADDIE2_X<24 && BADDIE2_LEFT){
    BADDIE2_LEFT = false;
  }
  BADDIE3_DOWN = false;
  BADDIE4_DOWN = true;

  BOMB_DROPPED = false;
  BOMB_DONE = false;
  BOMB_X = PLAYER_X+2;
  BOMB_Y = PLAYER_Y+2;
  BOMBS_LEFT = 3;
  
  TIME = 10;

  BG = rand() % 3;
  CR8 = rand() % 3;
  
  delay(200); 
}

void trace(String thepraise){
  arduboy.setCursor(0,56);
  arduboy.print(thepraise);
}
