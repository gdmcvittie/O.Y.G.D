
/*
 * Grant D. McVittie
 * PixelPlatforms
 * 
 * v.0.3
 * 
 * UPDATES:
 * - baddies will never insta-kill you upon respawn
 * - baby mode (4 steps at a time, grid overlay, 5 bombs and 12s)
 * - easy mode (4 steps at a time, 3 bombs and 10s)
 * - hard mode (2 steps at a time, 2 bombs and 8s)
 * - key mode (2 steps at a time, 8 bombs, 16s, but you need to get a key before getting to the door)
 * 
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
 * - added un-bombable blocks
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
 */
bool BABY = false;
bool EASY = false;
bool HARD = false;
bool NEEDKEY = true;

bool HAS_KEY = false;
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
 * key position
 */
int KEY_X = 112;
int KEY_Y = 8;
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
int BADDIE_X_POSITIONS [] = {16,24,32,40,48,56,64,72,80,88,96,104,112};
int BADDIE_X_POS_LEN = 12;
int BADDIE_Y_POSITIONS [] = {16,24,32,48,56};
int BADDIE_Y_POS_LEN = 4;

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
      if(NEEDKEY){
        arduboy.drawBitmap(50,47,needkey,16,16,BLACK);
      }else if(HARD){
        arduboy.drawBitmap(50,47,hard,16,16,BLACK);
      } else if(EASY){
        arduboy.drawBitmap(50,47,easy,16,16,BLACK);
      } else if(BABY){
        arduboy.drawBitmap(50,47,baby,16,16,BLACK);
      }
    break;
    case 2:
      arduboy.setCursor(12,12);
      arduboy.print( String("YOU HAVE ONE JOB,") );
      arduboy.setCursor(12,20);
      if(NEEDKEY){
        arduboy.print( String("8 BOMBS, AND 16s:") );
      }else if(HARD){
        arduboy.print( String("2 BOMBS, AND 8s:") );
      } else if(EASY){
        arduboy.print( String("3 BOMBS, AND 10s:") );
      } else if(BABY){
        arduboy.print( String("5 BOMBS, AND 12s:") );
      }
      if(NEEDKEY){
        arduboy.setCursor(12,28);
        arduboy.print( String("GET THE KEY AND") );
        arduboy.setCursor(12,36);
        arduboy.print( String("GET TO THE DOOR...") );
        arduboy.setCursor(12,48);
        arduboy.print( String("ALIVE.") );
      } else {
        arduboy.setCursor(12,28);
        arduboy.print( String("GET TO THE DOOR...") );
        arduboy.setCursor(12,36);
        arduboy.print( String("ALIVE.") );
      }
      
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

        if(BABY){
          arduboy.drawBitmap(0,0,grid,128,64,BLACK);
        }

        if(NEEDKEY && !HAS_KEY){
          if(!HAS_KEY){
            arduboy.drawBitmap(KEY_X,KEY_Y,key,8,8,BLACK);
          } else {
            arduboy.drawBitmap(26,-1,key,8,8,BLACK);
          }         
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
        if(BABY){
          BABY = false;
          EASY = true;
          HARD = false;
          NEEDKEY = false;
        } else if(EASY){
          BABY = false;
          EASY = false;   
          HARD = true;
          NEEDKEY = false;       
        } else if(HARD){
          BABY = false;
          EASY = false;
          HARD = false;
          NEEDKEY = true;
        } else if(NEEDKEY){
          BABY = true;
          EASY = false;
          HARD = false;
          NEEDKEY = false;
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
      if(BABY || EASY){
        PLAYER_SPEED = 4;
      } else if(HARD || NEEDKEY){
        PLAYER_SPEED = 2;
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
  if(!NEEDKEY){
    arduboy.drawBitmap(DOOR_X,DOOR_Y,door,8,8,BLACK);
  } else{
    if(NEEDKEY && HAS_KEY){
      arduboy.drawBitmap(DOOR_X,DOOR_Y,door,8,8,BLACK);
    }
  }
  
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

  if (PLAYER_X < KEY_X + 7 && PLAYER_X + 7 > KEY_X && PLAYER_Y < KEY_Y + 7 && 7 + PLAYER_Y > KEY_Y) {
    if(NEEDKEY){      
      if(SOUND_ENABLED && !HAS_KEY){
        soundHit();
      }
      HAS_KEY = true;
    }
  }
  
  if (PLAYER_X < DOOR_X + 7 && PLAYER_X + 7 > DOOR_X && PLAYER_Y < DOOR_Y + 7 && 7 + PLAYER_Y > DOOR_Y) {
    if(NEEDKEY){
      if(HAS_KEY){
        if(SOUND_ENABLED){
          soundHit();
        }
        CURRENT_LEVEL++;
        reset();
      }
    } else {
      if(SOUND_ENABLED){
        soundHit();
      }
      CURRENT_LEVEL++;
      reset();
    }
    
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

  int posX1 = rand() % BADDIE_X_POS_LEN;
  BADDIE1_X = BADDIE_X_POSITIONS[posX1];
  int posX2 = rand() % BADDIE_X_POS_LEN;
  BADDIE2_X = BADDIE_X_POSITIONS[posX2];
  int posX3 = rand() % BADDIE_X_POS_LEN;
  if(posX3==0){
    posX3 = 1;
  }
  BADDIE3_X = BADDIE_X_POSITIONS[posX3];
  int posX4 = rand() % BADDIE_X_POS_LEN;
  if(posX4==0){
    posX4 = 1;
  }
  BADDIE4_X = BADDIE_X_POSITIONS[posX4];
  int posY1 = rand() % BADDIE_Y_POS_LEN;
  BADDIE1_Y = BADDIE_Y_POSITIONS[posY1];
  int posY2 = rand() % BADDIE_Y_POS_LEN;
  BADDIE2_Y = BADDIE_Y_POSITIONS[posY2];
  int posY3 = rand() % BADDIE_Y_POS_LEN;
  BADDIE3_Y = BADDIE_Y_POSITIONS[posY3];
  int posY4 = rand() % BADDIE_Y_POS_LEN;
  BADDIE4_Y = BADDIE_Y_POSITIONS[posY4];



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
  

  if(BABY){
    TIME = 12;
    BOMBS_LEFT = 5;
  }else if(EASY){
    TIME = 10;
    BOMBS_LEFT = 3;
  } else if(HARD){
    TIME = 8;
    BOMBS_LEFT = 2;
  } else if(NEEDKEY){
    HAS_KEY = false;
    TIME = 16;
    BOMBS_LEFT = 8;
  }    

  BG = rand() % 3;
  CR8 = rand() % 3;
  
  delay(200); 
}

void trace(String thepraise){
  arduboy.setCursor(0,56);
  arduboy.print(thepraise);
}
