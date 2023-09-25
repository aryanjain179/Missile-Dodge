// SpaceInvaders.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/12/2022 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// buttons connected to PE0-PE3
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1
// 8*R resistor DAC bit 2 on PB2 
// 4*R resistor DAC bit 3 on PB3
// 2*R resistor DAC bit 4 on PB4
// 1*R resistor DAC bit 5 on PB5 (most significant bit)
// LED on PD1
// LED on PD0


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Print.h"
#include "Random.h"
#include "TExaS.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer1.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

int8_t gameState = -1;         // -1 uninitialzed, 0 active, 1, over
 int8_t fl;                     // flare countdown
 int8_t numF;                   // flare count
 uint32_t tick;                 // framecounter
 uint8_t flag;                  // draw screen
 uint32_t score;                
 uint8_t dmg;                   // dmgFx frameselect
 uint8_t hit;                   // dmgFx toggle
 uint8_t charge;               // shield recharge timer
 uint8_t restock;              // flare recharge time
 uint32_t highScore;             
 #define numEntities 3
 
typedef enum {English, Spanish} Language_t;
Language_t myLanguage=English;
typedef enum {TITLE, DECOY, LANGUAGE, GAMEOVER, HP, HIGHSCORE, NEWHIGHSCORE} phrase_t;
 const char Title_English[] ="  MISSILE DODGE";
 const char Title_Spanish[] ="ESQUIVAR MISILES";

 const char Decoy_English[]=" DECOY";
 const char Decoy_Spanish[]="CIMBEL";

 const char Language_English[]="English";
 const char Language_Spanish[]="Espa\xA4ol";

 const char GameOver_English[]="   GAME OVER";
 const char GameOver_Spanish[]="JUEGO TERMINADO";

 const char HP_English[] = "HP";
 const char HP_Spanish[] = "PV";

 const char HighScore_English[] = " HIGH SCORE: ";
 const char HighScore_Spanish[] = "ALTO PUNTAJE: ";

 const char NewHighScore_English[] = " NEW HIGH SCORE!";
 const char NewHighScore_Spanish[] = " \xADNUEVO RECORD!";

 const char *Phrases[7][2]={
  {Title_English, Title_Spanish},
  {Decoy_English,Decoy_Spanish},
  {Language_English,Language_Spanish},
  {GameOver_English,GameOver_Spanish},
  {HP_English, HP_Spanish},
  {HighScore_English, HighScore_Spanish},
  {NewHighScore_English, NewHighScore_Spanish}
};


void PortE_Init(){
  SYSCTL_RCGCGPIO_R |= 0x10;
  while((SYSCTL_PRGPIO_R & 0x10) != 0x10){};
 
  GPIO_PORTE_DIR_R &= ~0x7;     //Make PE0-2 inputs
  GPIO_PORTE_DEN_R |= 0x7;
}

 struct Sprite{
      int32_t x,y,dx,dy;          // 2^-12 fixed point
      int16_t maxV;               // max velocity for sprite goes from 0 - 4096
      int8_t acc;                 // acceleration value for sprite
      uint8_t dir, ded;           // determines sprite 
      int16_t hp;                 // only drawn when hp>0
  };
  typedef struct Sprite Sprite_t;
 struct Group{                    // holds a collection of sprite types
      Sprite_t *sprite;
      uint8_t w,h;                // w/h of images
      const uint16_t **imgPt;     // 8 keyframes
      const uint16_t **death;     // Frames for death animation
      uint8_t num;                // number of sprites in group
  };
 typedef struct Group Group_t;

 const uint16_t *ship[8] = {ship0, ship1, ship2, ship3, ship4, ship5, ship6, ship7};
 const uint16_t *flare[8] = {flare0, flare1, flare2, flare3, flare4, flare5, flare6, flare7};
 const uint16_t *missile[8] = {missile0, missile1, missile2, missile3, missile4, missile5, missile6, missile7};
 
 const uint16_t *flaredeath[8] = {flaredeath0, flaredeath0, flaredeath0, flaredeath0, flaredeath0, flaredeath0, flaredeath0, flaredeath0};
 const uint16_t *boom[8] = {explosion0, explosion1, explosion2, explosion3, explosion4, explosion5, explosion6, explosion7};
 const uint16_t *shipfx[8][8] = {
   {ship00, ship01, ship02, ship03, ship04, ship05, ship06, ship07},
   {ship10, ship11, ship12, ship13, ship14, ship15, ship16, ship17},
   {ship20, ship21, ship22, ship23, ship24, ship25, ship26, ship27},
   {ship30, ship31, ship32, ship33, ship34, ship35, ship36, ship37},
   {ship40, ship41, ship42, ship43, ship44, ship45, ship46, ship47},
   {ship50, ship51, ship52, ship53, ship54, ship55, ship56, ship57},
   {ship60, ship61, ship62, ship63, ship64, ship65, ship66, ship67},
   {ship70, ship71, ship72, ship73, ship74, ship75, ship76, ship77} };
 
 Sprite_t player = {50<<12, 50<<12, 0, 0, 4096, 100, 0, 0, 5};
 Sprite_t Flares[16] = {
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1}
 };                                     // values will be initialized when called
 Sprite_t Missiles[16] = {
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1},
   {0, 0, 0, 0, 0, 0, 0, 0, -1}
 };
 
 Group_t entities[3] = {                                  // all drawable entites in here
   {Flares, 5, 5, flare, flaredeath, 16},
   {Missiles, 7, 7, missile, boom, 16},
   {&player, 9, 9, ship, ship, 1}
 };
 
 
 
 void PortF_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x20;   // activate port F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 (built-in LED)
  GPIO_PORTF_PUR_R |= 0x10;
  GPIO_PORTF_DEN_R |=  0x1E;   // enable digital I/O on PF
}
 
 void Init(void){
  DisableInterrupts();
  TExaS_Init(NONE );       // Bus clock is 80 MHz 
  ADC_Init();
  PortE_Init();
  PortF_Init();
  Random_Init(1);
  Output_Init();
  Timer1_Init(80000000/80,2); // 80Hz
  Sound_Init();
  EnableInterrupts();
}
 void gameInit(){
  gameState = -1;         // -1 uninitialzed, 0 active, 1, over
  fl = 0;
  numF = 16;
  tick = 0;
  flag = 0;
  score = 0;
  dmg = 0;
  hit = 0;
  player.x = 50<<12;
  player.dx = 0;
  player.y = 50<<12;
  player.dy = 0;
  player.dir = 0;
  player.hp = 5;
  for(int i = 0; i < 2; i++){                   // Kill all entities
    for(int j = 0; j < entities[i].num; j++){
      entities[i].sprite[j].hp = -1;
    }
  }
  
 }
uint32_t clipHeight(int32_t y, int32_t height, int32_t level){
  
  int32_t top = (y>>12) - height;
  if(top < level){
    height = height + top - level;
  }
  return height;

}
void mainMenu(){
  ST7735_FillScreen(0x0000);
  ST7735_SetCursor(3, 5);
  ST7735_OutString((char *)Phrases[TITLE][myLanguage]);
  ST7735_SetCursor(5, 7);
  ST7735_OutString((char *)Phrases[LANGUAGE][English]);
  ST7735_OutString(" (PE2)");
  ST7735_SetCursor(5, 8);
  ST7735_OutString((char *)Phrases[LANGUAGE][Spanish]);
  ST7735_OutString(" (PE1)");
  while (gameState == -1){
    if (GPIO_PORTE_DATA_R & 0x4){
      gameState = 0;
      myLanguage = English;
    }
    else if (GPIO_PORTE_DATA_R & 0x2){
      gameState = 0;
      myLanguage = Spanish;
    }
  }
  ST7735_FillScreen(0x0000);
}
void HUD(){
  ST7735_SetCursor(6,0);
  ST7735_OutString((char *)Phrases[DECOY][myLanguage]);
  ST7735_SetCursor(17,0);
  ST7735_OutString((char *)Phrases[HP][myLanguage]);
  ST7735_DrawFastHLine(0,8,128,ST7735_Color565(0xFF, 0xFF, 0xFF));
}
void newGame(){
  gameInit();
    mainMenu();
    HUD();
}
void gameOver(){
    if(tick%16 == 0){
      ST7735_SetCursor(3, 5);
      ST7735_OutString((char *)Phrases[GAMEOVER][myLanguage]);
    }
    if(score >= highScore){
      highScore = score;
      if(tick%16 == 4){
        ST7735_SetCursor(3, 6);
        ST7735_OutString((char *)Phrases[NEWHIGHSCORE][myLanguage]);
      }
    }
    if(tick%16 == 8){
      ST7735_SetCursor(2, 7);
      ST7735_OutString((char *)Phrases[HIGHSCORE][myLanguage]);
      LCD_OutFix(highScore, 4, 0);
    }
      if((GPIO_PORTE_DATA_R & 0x1) == 0x1){    
        while((GPIO_PORTE_DATA_R & 0x1)){};
        gameState = -1;
      }
    
}
void draw(){
  for(int i = 0; i < numEntities; i++){                // Iterate through all groups
      for(int j = 0; j < entities[i].num; j++){          // Check each entity in group
        if (entities[i].sprite[j].hp != -1){             // Only draw active sprites
          
          int32_t height = clipHeight(entities[i].sprite[j].y, entities[i].h, 9);
          
          if (entities[i].sprite[j].hp == 0){            // Check for death animation
            entities[i].sprite[j].ded += 4;
            if(entities[i].sprite[j].ded != 0){
              ST7735_DrawBitmap(entities[i].sprite[j].x>>12, entities[i].sprite[j].y>>12, entities[i].death[entities[i].sprite[j].ded>>5], entities[i].w, height);
            }
            else entities[i].sprite[j].hp = -1;
          }                             
          else {
            if(i == 2 && hit){       // Check for damagefx
              dmg += 4;
              if(dmg != 0){
                ST7735_DrawBitmap(entities[i].sprite[j].x>>12, entities[i].sprite[j].y>>12, shipfx[entities[i].sprite[j].dir>>5][dmg>>5], entities[i].w, height);      // ship0 only supported 
              }
              else{
                hit = 0;
                ST7735_DrawBitmap(entities[i].sprite[j].x>>12, entities[i].sprite[j].y>>12, entities[i].imgPt[entities[i].sprite[j].dir>>5], entities[i].w, height);
              }
            }
            else ST7735_DrawBitmap(entities[i].sprite[j].x>>12, entities[i].sprite[j].y>>12, entities[i].imgPt[entities[i].sprite[j].dir>>5], entities[i].w, height);
          }
        }
      }
    }
    
    ST7735_SetCursor(0,0);
    LCD_OutFix(score, 5, 0);
    
    ST7735_SetCursor(13, 0);
    LCD_OutFix(numF, 2, 0);
    
    ST7735_SetCursor(20,0);
    LCD_OutFix(player.hp, 1, 0);
      
}
int main(void){
  Init();
  
  while(1){
    
  if (gameState == -1){
    newGame();
  }
  
  while(flag == 0){};               // wait for new frame  
  draw();

  if (gameState == 1){
    gameOver();
  }
    flag = 0;
  }
  
}
  
const int16_t xcomp[8] = {256 , 181, 0, -181, -256, -181, 0, 181};
const int16_t ycomp[8] = {0, -181, -256, -181, 0, 181, 256, 181};
void checkHit(void);
void checkSpeed(uint8_t, uint8_t);
uint32_t getDist(Sprite_t , Sprite_t );
uint8_t debug;
void Timer1A_Handler(void){ // can be used to perform tasks in background
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;          // acknowledge TIMER1A timeout
  entities[2].sprite[0].maxV = ADC_In();      // throttle input from slide
  if (gameState == -1 || flag == 1) return;
// -----------------------------------PLAYER UPDATE--------------------------------------------------------------
   if(player.hp > 0){
    debug = 0;
     if(player.x>>12 < 0){         // Checks for hitting edge EDIT: JANK FIX BUT I THINK IT WORKS
      player.dx = -player.dx;
      player.x = player.x + (1<<12);
      debug = 1;
    } 
    if(((player.x>>12) + entities[2].w) > 128){
      player.dx = -player.dx;
      player.x = player.x - (1<<12);
      debug = 1;
    }
    if(((player.y>>12) - entities[2].h) < 0){
      player.dy = -player.dy;
      player.y = player.y + (1<<12);
      debug = 1;
    }
    if (player.y>>12 > 160){
      player.dy = -player.dy;
      player.y = player.y - (1<<12);
      debug = 1;
    }
    if(debug != 1){
      player.x += player.dx;                                                  // Update position
      player.y += player.dy;
    }
      player.dx += ((entities[2].sprite[0].acc*xcomp[player.dir>>5])>>8);     // Update velocities
      player.dy += ((entities[2].sprite[0].acc*ycomp[player.dir>>5])>>8);
  
    checkSpeed(2,0);                                                        // Check for max speed
    if(tick%80==0){
      score += tick>>6;
    }
   charge++;
   if(player.hp < 5 && charge == 0){
     player.hp++;
   }
// -----------------------------------IO CHECKS-----------------------------------------------------------------
  
    if(GPIO_PORTE_DATA_R & 0x4){        // Rotate player ship 
      player.dir += 2;
    } 
  
    if(GPIO_PORTE_DATA_R & 0x2){
      player.dir -= 2;
    } 

    if(GPIO_PORTE_DATA_R & 0x1){        // Flare check
      
      if(fl%16 == 0 && numF > 0){ 
        for (int i = 0; i < entities[0].num; i++){ 
          if(Flares[i].hp < 0){
            numF--;
            Sound_Shoot();
            Flares[i].x = player.x;    // Spawn flare on player in opposite direction
            Flares[i].y = player.y;
            Flares[i].dx = -(player.dx>>1);
            Flares[i].dy = -(player.dy>>1);
            Flares[i].hp = 80*5; 
          break;
          }
        }
      }
      fl++;
    } 
  }
  else{
    gameState = 1;                      // If health = 0, end game
  }

// -----------------------------------FLARE UPDATE--------------------------------------------------------------
  if((restock == 0) && numF < entities[0].num){
        numF++;
      }
  restock++;
  for(int i = 0; i < entities[0].num; i++){                                   // Flare update, position and health
    if (Flares[i].hp > 0){
      Flares[i].x += Flares[i].dx;
      Flares[i].y += Flares[i].dy;
      if(tick%10 == 0){
        Flares[i].dir = Random();
      }        
      Flares[i].hp--;
    }
  }


// -----------------------------------MISSILE UPDATE------------------------------------------------------------
  if(tick%(25) == 0 && gameState == 0){                                                             // Spawn new missile every second
    static int32_t active = 0;
    if ((score>>7)+1 < entities[1].num){
      active = (score>>7)+1;
    }

    for(int i = 0; i < active; i++){
      if(Missiles[i].hp < 0){
        Missiles[i].x = (-64 + (int)Random())<<12;
        Missiles[i].y = (-48 + (int)Random())<<12;
        Missiles[i].dx = 0;
        Missiles[i].dy = 0;
        Missiles[i].maxV = 2048;
        Missiles[i].acc =  100;
        Missiles[i].hp = 80*5;                                                  // 5 seconds flight time
        break;
      }
    }
      
  }
  for(int i = 0; i < entities[1].num; i++){                                     // Update active missile physics                                
    if (Missiles[i].hp > 0){
      Missiles[i].x += Missiles[i].dx;                                          // Update Position
      Missiles[i].y += Missiles[i].dy;
      
      uint32_t closestDist = 0xFFFFFFFF;
      int32_t closestType = 2;
      int32_t closestEnt = 0;
      
      for(int j = 0; j < 3; j+=2){                                              // Find closest Trackable (Flares or Ship) FIXED BUG THAT BROKE GAME WHEN NO TRACKABLES AVAILABLE, BEHAVIOR IN THIS CASE MAY WANT TO BE MODIFIED CURRENTLY WILL TRACK LAST FLARE
        for(int k = 0; k < entities[j].num; k++){
          if(entities[j].sprite[k].hp > 0){                                     // Check only alive entities
            uint32_t current = getDist(Missiles[i], entities[j].sprite[k]);
            if(current < closestDist){
              closestDist = current;
              closestType = j;
              closestEnt = k;
            }
          }            
        }
        
      }
      
      if ((Missiles[i].x - entities[closestType].sprite[closestEnt].x) < 0){    // Accelerate in direction of closest entity
        Missiles[i].dx += Missiles[i].acc;
      }
      else {Missiles[i].dx -= Missiles[i].acc;}
      if ((Missiles[i].y - entities[closestType].sprite[closestEnt].y) < 0){
        Missiles[i].dy += Missiles[i].acc;
      }
      else {Missiles[i].dy -= Missiles[i].acc;}
      checkSpeed(1, i);                                                         // Check speed of current missile           
      
      Missiles[i].dir += 2;                                                     // Animate Sprite
      Missiles[i].hp--;                                                         // Decrement fuel
    }
  }
  checkHit();

// -----------------------------------SET FLAGS--------------------------------------------------------------
  GPIO_PORTF_DATA_R ^= tick&8;
  tick++;
  flag = 1;
}

void checkHit(void){
    for(int i = 0; i < entities[1].num; i++){
      if(Missiles[i].hp > 0){
        for(int j = 0; j < 3; j+=2){                                                  // Check for collision against all Trackables
          for(int k = 0; k < entities[j].num; k++){
          
            if(entities[j].sprite[k].hp > 0){                                         // Check only alive entities
              if((Missiles[i].x>>12)                 < (entities[j].sprite[k].x >> 12) + entities[j].w  && 
                 (Missiles[i].x>>12) + entities[1].w > (entities[j].sprite[k].x >> 12)                  &&
                 (Missiles[i].y>>12) - entities[i].h < (entities[j].sprite[k].y >> 12)                  &&
                 (Missiles[i].y>>12)                 > (entities[j].sprite[k].y >> 12) - entities[j].h    ){
                      if(j == 2){
                        hit = 1;                                                      // Restart Shield animation
                        dmg=0;
                        charge = 0;                                                   // Restart Shield Charge
                      }
                      Sound_Explosion();
                      if(gameState == 0){
                      score += 100;
                      }
                      Missiles[i].hp = 0;
                      entities[j].sprite[k].hp -= 1;
                      entities[j].sprite[k].dx = (entities[j].sprite[k].dx + Missiles[i].dx)>>1;    // Conservation of momentum inelastic
                      entities[j].sprite[k].dy = (entities[j].sprite[k].dx + Missiles[i].dy)>>1;
              } 
            }
          
          }            
        }
      }
    }
}

void checkSpeed(uint8_t type, uint8_t index){
  if (entities[type].sprite[index].dx > entities[type].sprite[index].maxV){                             // Check for max speed
    entities[type].sprite[index].dx = entities[type].sprite[index].maxV;
  }
  else if (entities[type].sprite[index].dx < -entities[type].sprite[index].maxV){                             // Check for max speed
    entities[type].sprite[index].dx = -entities[type].sprite[index].maxV;
  }
  if (entities[type].sprite[index].dy > entities[type].sprite[index].maxV){                             // Check for max speed
    entities[type].sprite[index].dy = entities[type].sprite[index].maxV;
  }
  else if (entities[type].sprite[index].dy < -entities[type].sprite[index].maxV){                             // Check for max speed
    entities[type].sprite[index].dy = -entities[type].sprite[index].maxV;
  }
}

uint32_t getDist(Sprite_t missile, Sprite_t trackable){         // Scales with distance
  int32_t delx = (missile.x>>12) - (trackable.x>>12);
  int32_t dely = (missile.y>>12) - (trackable.y>>12);
  int32_t answer = delx*delx + dely*dely;
  return answer;
}

  
