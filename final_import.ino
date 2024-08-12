#include <SPI.h>

int SW = 12;
int SW_state = 0;
#define LOAD_PIN 10

                   //row, col
int tetrinomeBlock1[2] {0,0};
int tetrinomeBlock2[2] {0,0};
int tetrinomeBlock3[2] {0,0};
int tetrinomeBlock4[2] {0,0};
int spawnObjectFlag = 1;
int currBlock;
int currRotate = 0;

long randArray[7] = {0,0,0,0,0,0,0};
int randArrayIndex = 1  ;
int randArrayIndexMAX = 6;

int xInput = 500;
int yInput = 500;
int rotCnt = 0;
int timeCnt10 = 0;

int score = 0;
int gameOn = 0;
long seed = 1;

int displayTimerFlash = 0;
long colArray[16];
int colAmount;

int ultraCnt = 0;
int echo = 2;
int trig = 3;
int readIn = 0;
long time1Ultra = 0;
long time2Ultra = 0;
long durationUltra;
long distanceUltra;

bool permaBlocks[16][8] {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 0, 0, 1, 1},
  {0, 0, 1, 0, 0, 1, 0, 0},
  {0, 0, 1, 0, 0, 0, 1, 0},
  {0, 0, 1, 0, 0, 0, 0, 1},
  {0, 1, 1, 1, 0, 1, 1, 0},
};

bool spawnBlock[16][8] {
  {0, 1, 1, 1, 0, 1, 1, 1},
  {0, 0, 1, 0, 0, 1, 0, 0},
  {0, 0, 1, 0, 0, 1, 1, 1},
  {0, 0, 1, 0, 0, 1, 0, 0},
  {0, 0, 1, 0, 0, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 0, 1, 1, 0},
  {0, 0, 1, 0, 0, 1, 0, 1},
  {0, 0, 1, 0, 0, 1, 1, 0},
  {0, 0, 1, 0, 0, 1, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
};

typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);

} task;

int delay_gcd;
const unsigned short tasksNum = 5;
task tasks[tasksNum];

enum SM1_States { SM1_INIT, offr, offp, midgame}; //system on and off with joystick button
int SM1_Tick(int state1) {
  switch (state1) { // State transitions
    case SM1_INIT:
      state1 = offr;
      break;
    case offr:
      SW_state = digitalRead(SW);
      if (SW_state == 0) { //if button is pressed in
        //writing to array random values
        seed = millis() % 2147483647;
        for (int i = 0; i < randArrayIndexMAX+1; ++i) {
          seed = PMRng(seed);
          Serial.print("SEED: "); Serial.print(seed);
          randArray[i] = seed % 7;
          Serial.print(" - randarray is "); Serial.println(randArray[i]);
        }
        state1 = offp;
      }
      else {
        state1 = offr;
      }
      break;
    case offp:
      SW_state = digitalRead(SW);
      if (SW_state == 1) { //if button is released
        state1 = midgame;
        gameOn = 1;
      }
      else {
        state1 = offp;
      }
      break;
    case midgame:
      if (gameOn == 0) {
        state1 = offr;
      }
      else {
        state1 = midgame;
      }
      break;
  }
  switch (state1) { // State Action
    case SM1_INIT:
      break;
    case offr:
      break;
    case offp:
      //need to clear the arrays here for spawn and perma
      clearSpawn();
      clearPerma();
      break;
    case midgame:
      break;
  }
  return state1;
}

enum SM2_States { SM2_INIT, displayFlashing, displayActive}; //LED MATRIX
int SM2_Tick(int state2) {
  switch (state2) { // State transitions
    case SM2_INIT:
      state2 = displayFlashing;
      break;
    case displayFlashing:
      
      if (gameOn == 1) {
        state2 = displayActive;
      }
      else {
        state2 = displayFlashing;
      }
      break;
    case displayActive:
      if (gameOn == 0) {
        state2 = displayFlashing;
      }
      else {
        state2 = displayActive;
      }
      
      break;
  }
  switch (state2) { // State Action
    case SM2_INIT:
      break;
    case displayFlashing:
      if (displayTimerFlash < 10) {
        writeToMatrix();            
      }
      else {
        clearMatrix();
      }
      ++displayTimerFlash;
      if (displayTimerFlash >= 20) {
        displayTimerFlash = 0;
      }
      break;
    case displayActive:
      writeToMatrix();
      break;
  }

  return state2;
}

enum SM3_States { SM3_INIT, off3, on3, idle3};  //Spawning Blocks
int SM3_Tick(int state3) {
  switch (state3) { // State transitions
    case SM3_INIT:
      state3 = off3;
      break;
    case off3:
      if (gameOn == 1) {
        state3 = idle3;
      }
      else {
        state3 = off3;
      }
      break;
    case on3:
      spawnObjectFlag = 0;
      state3 = idle3;
      break;
    case idle3:
      if (spawnObjectFlag == 1) {
        state3 = on3;
      }
      else {
        state3 = idle3;
      }
      break;
  }
  switch (state3) { // State Action
    case SM3_INIT:
      break;
    case on3: //spawn state
      Serial.println("Spawning (3) ");
      //spawnBlock[0][3] = 1;
      //spawnBlock[0][4] = 1;
      //spawnBlock[1][3] = 1;
      //spawnBlock[1][4] = 1;
      // First clear the spawn array
      clearSpawn();
      // Next get the tetrinome to fill into the array
      currRotate = 0;
      if (randArray[randArrayIndex] == 0) {
        currBlock = 0;
        spawnBlock[0][2] = 1;
        spawnBlock[0][3] = 1;
        spawnBlock[0][4] = 1;
        spawnBlock[0][5] = 1;
        // [1][2][3][4]
        tetrinomeBlock1[0] = 0;
        tetrinomeBlock1[1] = 2;
        tetrinomeBlock2[0] = 0;
        tetrinomeBlock2[1] = 3;
        tetrinomeBlock3[0] = 0;
        tetrinomeBlock3[1] = 4;
        tetrinomeBlock4[0] = 0;
        tetrinomeBlock4[1] = 5;
      }
      else if (randArray[randArrayIndex] == 1) {
        currBlock = 1;
        spawnBlock[0][2] = 1;
        spawnBlock[1][2] = 1;
        spawnBlock[1][3] = 1;
        spawnBlock[1][4] = 1;
        // [1]
        // [2][3][4]
        tetrinomeBlock1[0] = 0;
        tetrinomeBlock1[1] = 2;
        tetrinomeBlock2[0] = 1;
        tetrinomeBlock2[1] = 2;
        tetrinomeBlock3[0] = 1;
        tetrinomeBlock3[1] = 3;
        tetrinomeBlock4[0] = 1;
        tetrinomeBlock4[1] = 4;
      }
      else if (randArray[randArrayIndex] == 2) {
        currBlock = 2;
        spawnBlock[0][5] = 1;
        spawnBlock[1][5] = 1;
        spawnBlock[1][4] = 1;
        spawnBlock[1][3] = 1;
        //       [1]
        // [4][3][2]
        tetrinomeBlock1[0] = 0;
        tetrinomeBlock1[1] = 5;
        tetrinomeBlock2[0] = 1;
        tetrinomeBlock2[1] = 5;
        tetrinomeBlock3[0] = 1;
        tetrinomeBlock3[1] = 4;
        tetrinomeBlock4[0] = 1;
        tetrinomeBlock4[1] = 3;
      }
      else if (randArray[randArrayIndex] == 3) {
        currBlock = 3;
        spawnBlock[0][3] = 1;
        spawnBlock[0][4] = 1;
        spawnBlock[1][3] = 1;
        spawnBlock[1][4] = 1;
        // [1][2]
        // [3][4]
        tetrinomeBlock1[0] = 0;
        tetrinomeBlock1[1] = 3;
        tetrinomeBlock2[0] = 0;
        tetrinomeBlock2[1] = 4;
        tetrinomeBlock3[0] = 1;
        tetrinomeBlock3[1] = 3;
        tetrinomeBlock4[0] = 1;
        tetrinomeBlock4[1] = 4;
      } 
      else if (randArray[randArrayIndex] == 4) {
        currBlock = 4;
        spawnBlock[0][3] = 1;
        spawnBlock[0][4] = 1;
        spawnBlock[1][2] = 1;
        spawnBlock[1][3] = 1;
        //    [1][2]
        // [3][4]
        tetrinomeBlock1[0] = 0;
        tetrinomeBlock1[1] = 3;
        tetrinomeBlock2[0] = 0;
        tetrinomeBlock2[1] = 4;
        tetrinomeBlock3[0] = 1;
        tetrinomeBlock3[1] = 2;
        tetrinomeBlock4[0] = 1;
        tetrinomeBlock4[1] = 3;
      }
      else if (randArray[randArrayIndex] == 5) {
        currBlock = 5;
        spawnBlock[0][3] = 1;
        spawnBlock[0][4] = 1;
        spawnBlock[1][4] = 1;
        spawnBlock[1][5] = 1;
        // [1][2]
        //    [3][4]
        tetrinomeBlock1[0] = 0;
        tetrinomeBlock1[1] = 3;
        tetrinomeBlock2[0] = 0;
        tetrinomeBlock2[1] = 4;
        tetrinomeBlock3[0] = 1;
        tetrinomeBlock3[1] = 4;
        tetrinomeBlock4[0] = 1;
        tetrinomeBlock4[1] = 5;
      }
      else if (randArray[randArrayIndex] == 6) {
        currBlock = 6;
        spawnBlock[0][4] = 1;
        spawnBlock[1][5] = 1;
        spawnBlock[1][4] = 1;
        spawnBlock[1][3] = 1;
        //    [1]
        // [4][3][2]
        tetrinomeBlock1[0] = 0;
        tetrinomeBlock1[1] = 4;
        tetrinomeBlock2[0] = 1;
        tetrinomeBlock2[1] = 5;
        tetrinomeBlock3[0] = 1;
        tetrinomeBlock3[1] = 4;
        tetrinomeBlock4[0] = 1;
        tetrinomeBlock4[1] = 3;
      }
      else {
        Serial.println("FAIL AT SPAWN");
      }
      //since that we are done with this part of the array, we will randomize for next time
      seed = PMRng(seed);
      randArray[randArrayIndex] = seed % 7;
      //now increment the index
      if (randArrayIndex == randArrayIndexMAX) {
        randArrayIndex = 0;
        Serial.println("Reset random index");
        //helps to show new array
        for (int i = 0; i < randArrayIndexMAX+1; ++i) {
          Serial.print("Randarray is "); Serial.println(randArray[i]);
        }
      }
      else {
        ++randArrayIndex;
      }
      /*
      for (int i = 0; i < 16; ++i) {
        for (int k = 0; k < 8; ++k) {
          if (spawnBlock[i][k]) {
            Serial.print("1 ");
          }
          else {
            Serial.print("0 ");
          }
        }
        Serial.println("");
      }  
      Serial.print(tetrinomeBlock1[0]); Serial.print(tetrinomeBlock1[1]); Serial.print(tetrinomeBlock2[0]); Serial.print(tetrinomeBlock2[1]); Serial.print(tetrinomeBlock3[0]); Serial.print(tetrinomeBlock3[1]), Serial.print(tetrinomeBlock4[0]), Serial.println(tetrinomeBlock4[1]);
      */
      break;
    case idle3:
      //Serial.println("Idle (3) ");
      break;
  }
  return state3;
}

enum SM4_States { SM4_INIT, off4, idle4, on4}; //Block Movement
// Some things to think about, should tetris only be 14 rows? that way everything spawns right above and there isnt collision
int SM4_Tick(int state4) {
  switch (state4) { // State transitions
    case SM4_INIT:
      state4 = off4;
      break;
    case off4:
      if (gameOn == 1) {
        state4 = idle4;
      }
      else {
        state4 = off4;
      }
      break;
    case idle4:
      if (gameOn == 0) {
        state4 = off4;
      }
      else if (spawnObjectFlag == 0) { //object has successfully spawned
        state4 = on4;
      }
      else {
        state4 = idle4;
      }
      break;
    case on4:
      if (gameOn == 0) {
        state4 = off4;
      }
      else if (spawnObjectFlag == 1) {
        state4 = idle4;
      }
      else {
        state4 = on4;
      }
      break;
  }
  switch (state4) { // State Action
    case SM4_INIT:
      break;
    case idle4:
      break;
    case on4:
      SW_state = digitalRead(SW);
      //first, check for movement while timeCnt4 is <4
      if (timeCnt10 < 10) {
        xInput = analogRead(A1);
        yInput = analogRead(A0);
        //Serial.print("x and y - "); Serial.print(xInput); Serial.print(" "); Serial.println(yInput);
        if (/*yInput > 800*/ distanceUltra < 10) {
          if (rotCnt >= 8) {
            rotateBlock();
            updateSpawn();
            rotCnt = 0;
          }
          else {
            ++rotCnt;
          }
        }
        else if (xInput < 200) {
          //check left boundary
          if (tetrinomeBlock1[1]-1 < 0 || tetrinomeBlock2[1]-1 < 0 || tetrinomeBlock3[1]-1 < 0 || tetrinomeBlock4[1]-1 <0){
            Serial.println("OVERREACH LEFT - BOUNDARY");
          }
          //check left for blocks in way
          else if (permaBlocks[tetrinomeBlock1[0]][tetrinomeBlock1[1]-1] == 1 || permaBlocks[tetrinomeBlock2[0]][tetrinomeBlock2[1]-1] == 1 || permaBlocks[tetrinomeBlock3[0]][tetrinomeBlock3[1]-1] == 1 || permaBlocks[tetrinomeBlock4[0]][tetrinomeBlock4[1]-1] == 1){
            Serial.println("OVERREACH LEFT - BLOCK COLLISION");
          }
          else {
            --tetrinomeBlock1[1];
            --tetrinomeBlock2[1];
            --tetrinomeBlock3[1];
            --tetrinomeBlock4[1];
            updateSpawn();
            for (int i = 0; i < 16; ++i) {
              for (int k = 0; k < 8; ++k) {
                if (spawnBlock[i][k]) {
                  Serial.print("1 ");
                }
                else {
                  Serial.print("0 ");
                }
              }
              Serial.println("");
            }  
          }
        }
        else if (xInput > 800) {
          //check right boundary
          if (tetrinomeBlock1[1]+1 > 7 || tetrinomeBlock2[1]+1 > 7 || tetrinomeBlock3[1]+1 > 7 || tetrinomeBlock4[1]+1 > 7) {
            Serial.println("OVERREACH LEFT - BOUNDARY");            
          }
          //check right for blocks in way
          else if (permaBlocks[tetrinomeBlock1[0]][tetrinomeBlock1[1]+1] == 1 || permaBlocks[tetrinomeBlock2[0]][tetrinomeBlock2[1]+1] == 1 || permaBlocks[tetrinomeBlock3[0]][tetrinomeBlock3[1]+1] == 1 || permaBlocks[tetrinomeBlock4[0]][tetrinomeBlock4[1]+1] == 1){
            Serial.println("OVERREACH RIGHT - BLOCK COLLISION");
          }
          else {
            ++tetrinomeBlock1[1];
            ++tetrinomeBlock2[1];
            ++tetrinomeBlock3[1];
            ++tetrinomeBlock4[1];
            updateSpawn();
            for (int i = 0; i < 16; ++i) {
              for (int k = 0; k < 8; ++k) {
                if (spawnBlock[i][k]) {
                  Serial.print("1 ");
                }
                else {
                  Serial.print("0 ");
                }
              }
              Serial.println("");
            }              
          }
        }
        //if button is pushed in, we want to immediately go down
        else if (SW_state == 0) {
          if (tetrinomeBlock1[0] == 15 || tetrinomeBlock2[0] == 15 || tetrinomeBlock3[0] == 15 || tetrinomeBlock4[0] == 15) {
          saveToPerma();
          tetrisCheck();
          spawnObjectFlag = 1;
         }
         //check if there are blocks below
         else if (permaBlocks[tetrinomeBlock1[0]+1][tetrinomeBlock1[1]]==1 || permaBlocks[tetrinomeBlock2[0]+1][tetrinomeBlock2[1]]==1 || permaBlocks[tetrinomeBlock3[0]+1][tetrinomeBlock3[1]]==1 || permaBlocks[tetrinomeBlock4[0]+1][tetrinomeBlock4[1]]==1) {
          saveToPerma();
          tetrisCheck();
          if (checkGameOver() == false) {
            spawnObjectFlag = 1; 
          }
          else{
            Serial.println("--GAME OVER--");
            gameOn = 0;
          }
         }
         else {
            ++tetrinomeBlock1[0];
            ++tetrinomeBlock2[0];
            ++tetrinomeBlock3[0];
            ++tetrinomeBlock4[0];
            updateSpawn();
            /*for (int i = 0; i < 16; ++i) {
              for (int k = 0; k < 8; ++k) {
                if (spawnBlock[i][k]) {
                  Serial.print("1 ");
                }
                else {
                  Serial.print("0 ");
                }
              }
              Serial.println("");
            }*/
         }
         timeCnt10 = 0;
        }
      }
      //Next,if our count is down (grace period for rotation is over), we need to look below
        //Dont forget to set cnt to 0 at the end!
      else { // If cnt >=4
         //check if at bottom already
         if (tetrinomeBlock1[0] == 15 || tetrinomeBlock2[0] == 15 || tetrinomeBlock3[0] == 15 || tetrinomeBlock4[0] == 15) {
          saveToPerma();
          tetrisCheck();
          spawnObjectFlag = 1;
         }
         //check if there are blocks below
         else if (permaBlocks[tetrinomeBlock1[0]+1][tetrinomeBlock1[1]]==1 || permaBlocks[tetrinomeBlock2[0]+1][tetrinomeBlock2[1]]==1 || permaBlocks[tetrinomeBlock3[0]+1][tetrinomeBlock3[1]]==1 || permaBlocks[tetrinomeBlock4[0]+1][tetrinomeBlock4[1]]==1) {
          saveToPerma();
          tetrisCheck();
          if (checkGameOver() == false) {
            spawnObjectFlag = 1; 
          }
          else{
            Serial.println("--GAME OVER--");
            gameOn = 0;
          }
         }
         else {
            ++tetrinomeBlock1[0];
            ++tetrinomeBlock2[0];
            ++tetrinomeBlock3[0];
            ++tetrinomeBlock4[0];
            updateSpawn();
            /*for (int i = 0; i < 16; ++i) {
              for (int k = 0; k < 8; ++k) {
                if (spawnBlock[i][k]) {
                  Serial.print("1 ");
                }
                else {
                  Serial.print("0 ");
                }
              }
              Serial.println("");
            } */
         }
         timeCnt10 = 0;
      }
      ++timeCnt10;
      break;
  }
  return state4;
}


enum SM5_States { SM5_INIT, allstate}; //Ultrasonic sensor
int SM5_Tick(int state5) {
  switch(state5) {
    case SM5_INIT:
      state5 = allstate;
      break;      
    case allstate:
      state5 = allstate;
      break;
  }
    switch(state5) {
      case SM5_INIT:
        break;
      case allstate:
      //COMPLEX 3 - Ultrasonic sensor\\ 
        digitalWrite(trig, LOW);
        //delayMicroseconds(2);
        digitalWrite(trig, HIGH);
        //delayMicroseconds(10);
        digitalWrite(trig, LOW);

        while (digitalRead(echo) == 0) {
        }
        time1Ultra = micros();
        while (digitalRead(echo) == 1) {
        }
        time2Ultra = micros();
        //durationUltra = pulseIn(echo, HIGH);
        durationUltra = time2Ultra - time1Ultra;
        Serial.print("Duration: "); Serial.print(durationUltra);
        distanceUltra = durationUltra * 0.034 / 2;
                     //speed of light  //send + receive
        Serial.print(" - Distance: "); Serial.println(distanceUltra);

        if(ultraCnt == 1) {
          ultraCnt = 0;
        }
        else {
          ++ultraCnt;
        }
        
        break;
  }
  return state5;
}


void setup() {
  // put your setup code here, to run once:
  unsigned char i = 0;
  tasks[i].state = SM1_INIT;
  tasks[i].period = 100000;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM1_Tick;
  i++;
  tasks[i].state = SM2_INIT;
  tasks[i].period = 50000;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM2_Tick;
  i++;
  tasks[i].state = SM3_INIT;
  tasks[i].period = 500000;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM3_Tick;
  i++;
  tasks[i].state = SM4_INIT;
  tasks[i].period = 50000;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM4_Tick;
  i++;
  tasks[i].state = SM5_INIT;
  tasks[i].period = 60000; //2
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM5_Tick;
  

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  pinMode(LOAD_PIN, OUTPUT);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();
  // Enable mode B
  maxTransfer(0x09, 0xFF);
  // Use lowest intensity
  maxTransfer(0x0A, 0x00);
  // Only scan one digit
  maxTransfer(0x0B, 0x07);
  // Turn on no-decode
  maxTransfer(0x09, 0x00);
  // Turn on chip
  maxTransfer(0x0C, 0x01);
  
  delay_gcd = 1000; // GCD
  Serial.begin(9600);
  pinMode(SW, INPUT_PULLUP); 
}

void loop() {
  unsigned char i;
  for (i = 0; i < tasksNum; ++i) {
    if ( (micros() - tasks[i].elapsedTime) >= tasks[i].period) {
      tasks[i].state = tasks[i].TickFct(tasks[i].state);
      tasks[i].elapsedTime = micros(); // Last time this task was ran
    }
  }
}

void clearSpawn() {
      for (int i = 0; i < 16; ++i) {
        for (int k = 0; k < 8; ++k) {
          spawnBlock[i][k] = 0;
        }
      }
}

void clearPerma() {
  for (int i = 0; i < 16; ++i) {
    for (int k = 0; k < 8; ++k) {
      permaBlocks[i][k] = 0;
    }
  }
}

void updateSpawn() {
   clearSpawn();
   for (int i = 0; i < 16; ++i) {
     for (int k = 0; k < 8; ++k) {
       if (i == tetrinomeBlock1[0] && k == tetrinomeBlock1[1]) {
         spawnBlock[i][k] = 1;
       }     
       else if (i == tetrinomeBlock2[0] && k == tetrinomeBlock2[1]) {
         spawnBlock[i][k] = 1;
       }
       else if (i == tetrinomeBlock3[0] && k == tetrinomeBlock3[1]) {
         spawnBlock[i][k] = 1;
       }
       else if (i == tetrinomeBlock4[0] && k == tetrinomeBlock4[1]) {
         spawnBlock[i][k] = 1;
       }
       else {
        spawnBlock[i][k] = 0;
       }
     }
   }
}



void saveToPerma() { //also needs to do the funny tetris move and clear lanes
  score = score + 1;
   for (int i = 0; i < 16; ++i) {
     for (int k = 0; k < 8; ++k) {
      permaBlocks[i][k] = permaBlocks[i][k] || spawnBlock[i][k];
     }
   }
   Serial.println("///////////PERMA///////////");
      for (int i = 0; i < 16; ++i) {
        for (int k = 0; k < 8; ++k) {
                if (permaBlocks[i][k]) {
                  Serial.print("1 ");
                }
                else {
                  Serial.print("0 ");
                }
              }
              Serial.println("");
            }
   Serial.println("///////////PERMA///////////");
}
bool checkGameOver() {
  int checkBlockOverTop = 0;
  for (int i = 0; i < 2; ++i) {
        for (int k = 0; k < 8; ++k) {
                if (permaBlocks[i][k]) {
                  checkBlockOverTop = 1;
                }
        }
  }
  if (checkBlockOverTop == 1) {
    return true;
  }
  else {
    return false;
  }
}
void tetrisCheck() {
  int tetris = 1;
  int row = 2;
  Serial.println("From the top of the array: ");
  for (row = 2; row < 16; ++row) {
     for (int k = 0; k < 8; ++k) {
      if (!(permaBlocks[row][k])) {
        tetris = 0; //no tetris :(
      }
     }
     if (tetris == 1) { //If the row is full - do the Tetris thing!
      Serial.print(row); Serial.println(" - Tetris is real.");
      score = score + 20;
      //move everything down one
      if (row == 2) {
        //if tetris is at the top
        Serial.println(" - now popping off of top of array - ");
        for (int k = 0; k < 8; ++k) {
          permaBlocks[row][k] = 0;
        }
      }
      else {
        //for every other row
        Serial.print(" - shifting everything down from row "); Serial.print(row); Serial.println(" - ");
        for (int a = row; a > 2; --a) {
          for (int k = 0; k < 8; ++k) {
            permaBlocks[a][k] = permaBlocks[a-1][k];
          }
        }
      }
     }
     else {
      tetris = 1;
      Serial.println(row);
     }
  }
     


 
  Serial.println("///////////TETRIS///////////");
      for (int i = 0; i < 16; ++i) {
        for (int k = 0; k < 8; ++k) {
                if (permaBlocks[i][k]) {
                  Serial.print("1 ");
                }
                else {
                  Serial.print("0 ");
                }
              }
              Serial.println("");
            }
   Serial.println("///////////TETRIS///////////");
   
}

void rotateBlock(){
  int t1_0 = tetrinomeBlock1[0]; //0 is y coor
  int t1_1 = tetrinomeBlock1[1]; //1 is x coor
  int t2_0 = tetrinomeBlock2[0];
  int t2_1 = tetrinomeBlock2[1];
  int t3_0 = tetrinomeBlock3[0];
  int t3_1 = tetrinomeBlock3[1];
  int t4_0 = tetrinomeBlock4[0];
  int t4_1 = tetrinomeBlock4[1];

  //+1 to y is down
  //+1 to x is right

  //rotation is ALWAYS COUNTER-CLOCKWISE!
  
  if (currBlock == 0) { //CLEAR
        // [1][2][3][4]
    if (currRotate == 0) {
      //check for wall collisions
      if ((t1_0 + 1 < 16 && t1_1 + 1 < 8)&&(t3_0 - 1 >= 0 && t3_1 - 1 >= 0)&&(t4_0 - 2 >=0 && t4_1 - 2 >=0)) {
        //t2 is our root 
        //check for permaBlock collisions
        if (!permaBlocks[t1_0 + 1][t1_1 + 1]&&!permaBlocks[t3_0 - 1][t3_1 - 1]&&!permaBlocks[t4_0 - 2][t4_1 - 2]){
          tetrinomeBlock1[0] = tetrinomeBlock1[0] + 1;
          tetrinomeBlock1[1] = tetrinomeBlock1[1] + 1;
          tetrinomeBlock3[0] = tetrinomeBlock3[0] - 1;
          tetrinomeBlock3[1] = tetrinomeBlock3[1] - 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] - 2;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] - 2;
          currRotate = 1;
        }
      }
    }
    else {
      if ((t1_0 - 1 >=0 && t1_1 - 1 >=0)&&(t3_0 + 1 < 16 && t3_1 + 1 < 8)&&(t4_0 + 2 < 16 && t4_1 + 2 < 8)) {
        if (!permaBlocks[t1_0 - 1][t1_1 - 1]&&!permaBlocks[t3_0 + 1][t3_1 + 1]&&!permaBlocks[t4_0 + 2][t4_1 + 2]){
          tetrinomeBlock1[0] = tetrinomeBlock1[0] - 1;
          tetrinomeBlock1[1] = tetrinomeBlock1[1] - 1;
          tetrinomeBlock3[0] = tetrinomeBlock3[0] + 1;
          tetrinomeBlock3[1] = tetrinomeBlock3[1] + 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] + 2;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] + 2;
          currRotate = 0;
        }
      }
    }
  }
  
  else if (currBlock == 1) { //CLEAR
        // [1]
        // [2][3][4]
    //root is 3
    if (currRotate == 0) {
      if ((t1_0 + 2 < 16)&&(t2_0 + 1 < 16 && t2_1 + 1 < 8)&&(t4_0 - 1 >=0 && t4_1 - 1 >=0)) {
        if (!permaBlocks[t1_0 + 2][t1_1]&&!permaBlocks[t2_0 + 1][t2_1 + 1]&&!permaBlocks[t4_0 - 1][t4_1 - 1]){
            tetrinomeBlock1[0] = tetrinomeBlock1[0] + 2;
            tetrinomeBlock2[0] = tetrinomeBlock2[0] + 1;
            tetrinomeBlock2[1] = tetrinomeBlock2[1] + 1;
            tetrinomeBlock4[0] = tetrinomeBlock4[0] - 1;
            tetrinomeBlock4[1] = tetrinomeBlock4[1] - 1;
            currRotate = 1; 
        }
      }
    }
    else if (currRotate == 1) {
      //   [4]
      //   [3]
      //[1][2]
      if ((t1_1 + 2 < 8)&&(t2_0 - 1 >=0 && t2_1 + 1 < 8)&&(t4_0 + 1 < 16 && t4_1 - 1 >=0)) {
        if (!permaBlocks[t1_0][t1_1 + 2]&&!permaBlocks[t2_0 - 1][t2_1 + 1]&&!permaBlocks[t4_0 + 1][t4_1 - 1]) {
          tetrinomeBlock1[1] = tetrinomeBlock1[1] + 2;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] - 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] + 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] + 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] - 1;
          currRotate = 2;
        }
      }
    }
    else if (currRotate == 2) { 
      //[4][3][2]
      //      [1]
      if ((t1_0 - 2 >=0)&&(t2_0 - 1 >= 0 && t2_1 - 1 >= 0)&&(t4_0 + 1 < 16 && t4_1 + 1 < 8)) {
        if (!permaBlocks[t1_0 - 2][t1_1]&&!permaBlocks[t2_0 - 1][t2_1 - 1]&&!permaBlocks[t4_0 + 1][t4_1 + 1]) {
          tetrinomeBlock1[0] = tetrinomeBlock1[0] - 2;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] - 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] - 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] + 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] + 1;
          currRotate = 3;
        }
      }
    }
    else {
      //[2][1]
      //[3]
      //[4]
      if ((t1_1 - 2 >= 0)&&(t2_0 + 1 < 16 && t2_1 - 1 >= 0)&&(t4_0 - 1 >= 0 && t4_1 + 1 < 8)) {
        if (!permaBlocks[t1_0][t1_1 - 2]&&!permaBlocks[t2_0 + 1][t2_1 - 1]&&!permaBlocks[t4_0 - 1][t4_1 + 1]) {
          tetrinomeBlock1[1] = tetrinomeBlock1[1] - 2;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] + 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] - 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] - 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] + 1;
          currRotate = 0;
        }
      }
    }
  }
  else if (currBlock == 2) { 
        //       [1]
        // [4][3][2]
        //root is 3
    if (currRotate == 0) {
      if ((t1_1 - 2 >=0)&&(t2_0 - 1 >=0 && t2_1 -1 >=0)&&(t4_0 + 1 < 16 && t4_1 + 1 < 8)) {
        if (!permaBlocks[t1_0][t1_1 - 2]&&!permaBlocks[t2_0 - 1][t2_0 - 1]&&!permaBlocks[t4_0 + 1][t4_1 + 1]){
          tetrinomeBlock1[1] = tetrinomeBlock1[1] - 2;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] - 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] - 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] + 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] + 1;           
            currRotate = 1; 
        }
      }
    }
    else if (currRotate == 1) {
      //[1][2]
      //   [3]
      //   [4]
      if ((t1_0 + 2 < 16)&&(t2_0 + 1 < 16 && t2_1 - 1 >=0)&&(t4_0 - 1 >=0 && t4_1 + 1 < 8)) {
        if (!permaBlocks[t1_0 + 2][t1_1]&&!permaBlocks[t2_0 + 1][t2_1 - 1]&&!permaBlocks[t4_0 - 1][t4_1 + 1]) {
          tetrinomeBlock1[0] = tetrinomeBlock1[0] + 2;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] + 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] - 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] - 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] + 1;     
          currRotate = 2; 
        }
      }
    }
    else if (currRotate == 2) { //ERROR
      //[2][3][4]
      //[1]
      if ((t1_1 + 2 < 8)&&(t2_0 + 1 < 16 && t2_1 + 1 < 8)&&(t4_0 - 1 >=0 && t4_1 - 1 >=0)) {
        if (!permaBlocks[t1_0][t1_1 + 2]&&!permaBlocks[t2_0 + 1][t2_1 + 1]&&!permaBlocks[t4_0 - 1][t4_1 - 1]) {
          tetrinomeBlock1[1] = tetrinomeBlock1[1] + 2;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] + 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] + 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] - 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] - 1;     
          currRotate = 3; 
        }
      }
    }
    else {
      //[4]
      //[3]
      //[2][1]
      if((t1_0 - 2 >=0)&&(t2_0 - 1 >=0 && t2_1 + 1 < 8)&&(t4_0 + 1 < 16 && t4_1 - 1 >=0)) {
        if (!permaBlocks[t1_0 - 2][t1_1]&&!permaBlocks[t2_0 - 1][t2_1 + 1]&&!permaBlocks[t4_0 + 1][t4_1 - 1]) {
          tetrinomeBlock1[0] = tetrinomeBlock1[0] - 2;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] - 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] + 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] + 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] - 1; 
          currRotate = 0; 
        }
      }
    }
  }
  else if (currBlock == 3) {
        // [1][2]
        // [3][4]    
  }
  else if (currBlock == 4) {
        //    [1][2]
        // [3][4]    
        //root is 1
    if (currRotate == 0) {
      if ((t2_0 - 1 >= 0 && t2_1 - 1 >= 0)&&(t3_1 + 2 < 8)&&(t4_0 - 1 >= 0 && t4_1 + 1 < 8)) {
        if (!permaBlocks[t2_0 - 1][t2_1 - 1]&&!permaBlocks[t3_0][t3_1 + 2]&&!permaBlocks[t4_0 - 1][t4_1 + 1]) {
          tetrinomeBlock2[0] = tetrinomeBlock2[0] - 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] - 1;
          tetrinomeBlock3[1] = tetrinomeBlock3[1] + 2;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] - 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] + 1;
          currRotate = 1;
        }
      }
    }
    else {
      if ((t2_0 + 1 <16 && t2_1 + 1 < 8)&&(t3_1 - 2 >=0)&&(t4_0 + 1 < 16 && t4_1 - 1 >=0)) {
        if (!permaBlocks[t2_0 + 1][t2_1 + 1]&&!permaBlocks[t3_0][t3_1 - 2]&&!permaBlocks[t4_0 + 1][t4_1 - 1]) {
          tetrinomeBlock2[0] = tetrinomeBlock2[0] + 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] + 1;
          tetrinomeBlock3[1] = tetrinomeBlock3[1] - 2;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] + 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] - 1;
          currRotate = 0;
        }
      }
    }
  }
  else if (currBlock == 5) {
        // [1][2]
        //    [3][4]  
        //root is 2
    if (currRotate == 0) {
      if ((t1_0 + 1 < 16 && t1_1 + 1 < 8)&&(t3_0 - 1 >=0 && t3_1 + 1 < 8)&&(t4_0 - 2 >=0)) {
        if (!permaBlocks[t1_0 + 1][t1_1 + 1]&&!permaBlocks[t3_0 - 1][t3_1 + 1]&&!permaBlocks[t4_0-2][t4_1]) {
          tetrinomeBlock1[0] = tetrinomeBlock1[0] + 1;
          tetrinomeBlock1[1] = tetrinomeBlock1[1] + 1;
          tetrinomeBlock3[0] = tetrinomeBlock1[0] - 1;
          tetrinomeBlock3[1] = tetrinomeBlock1[1] + 1;
          tetrinomeBlock4[0] = tetrinomeBlock1[0] - 2;
          currRotate = 1;
        }
      }
    }
    else {
      //   [4]
      //[2][3]
      //[1]
      if ((t1_0 - 1 >= 0 && t1_1 - 1 >= 0)&&(t3_0 + 1 < 16 && t3_1 - 1 >= 0)&&(t4_1 + 2 < 16)) {
        if (!permaBlocks[t1_0 - 1][t1_1 - 1]&&!permaBlocks[t3_0 + 1][t3_1 - 1]&&!permaBlocks[t4_0 + 2][t4_1]) {
          tetrinomeBlock1[0] = tetrinomeBlock1[0] - 1;
          tetrinomeBlock1[1] = tetrinomeBlock1[1] - 1;
          tetrinomeBlock3[0] = tetrinomeBlock1[0] + 1;
          tetrinomeBlock3[1] = tetrinomeBlock1[1] - 1;
          tetrinomeBlock4[0] = tetrinomeBlock1[0] + 2;
          currRotate = 0;
        }
      }
    }
  }
  else if (currBlock == 6) {
        //    [1]
        // [4][3][2]    
         //root is 3
    if (currRotate == 0) {
      if ((t1_0 + 1 < 16 && t1_1 - 1 >=0)&&(t2_0-1 >=0 && t2_1-1 >=0)&&(t4_0+1 < 16 && t4_1+1 < 8)) {
        if (!permaBlocks[t1_0 + 1][t1_1 - 1]&&!permaBlocks[t2_0-1][t2_1-1]&&!permaBlocks[t4_0+1][t4_1+1]) {
          tetrinomeBlock1[0] = tetrinomeBlock1[0] + 1;
          tetrinomeBlock1[1] = tetrinomeBlock1[1] - 1;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] - 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] - 1 ;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] + 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] + 1;
          currRotate = 1;
        }
      }
    }
    else if (currRotate == 1) {
      //   [2]
      //[1][3]
      //   [4]
      if ((t1_0+1 < 16 && t1_1+1 < 8)&&(t2_0+1 < 16 && t2_1-1 >= 0)&&(t4_0-1 >= 0 && t4_1 + 1 < 8)) {
        if (!permaBlocks[t1_0+1][t1_1+1]&&!permaBlocks[t2_0+1][t2_1-1]&&!permaBlocks[t4_0-1][t4_1+1]) {
          tetrinomeBlock1[0] = tetrinomeBlock1[0] + 1;
          tetrinomeBlock1[1] = tetrinomeBlock1[1] + 1;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] + 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] - 1 ;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] - 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] + 1;
          currRotate = 2;
        }
      }
    }
    else if (currRotate == 2) {
      //[2][3][4]
      //   [1]
      if ((t1_0 - 1 >=0 && t1_1 + 1 < 8)&&(t2_0 + 1 < 16 && t2_1 + 1 < 8)&&(t4_0 - 1 >= 0 && t4_1 - 1 >=0)){
        if (!permaBlocks[t1_0 - 1][t1_1 + 1]&&!permaBlocks[t2_0 + 1][t2_1 + 1]&&!permaBlocks[t4_0 - 1][t4_1 - 1]){
          tetrinomeBlock1[0] = tetrinomeBlock1[0] - 1;
          tetrinomeBlock1[1] = tetrinomeBlock1[1] + 1;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] + 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] + 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] - 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] - 1;
          currRotate = 3;
        }
      }
    }
    else { 
      //[4]
      //[3][1]
      //[2]
      if ((t1_0 - 1 >=0 && t1_1 - 1 >= 0)&&(t2_0 - 1 >= 0 && t2_1 + 1 < 8)&&(t4_0 + 1 < 16 && t4_1 - 1 >= 0)) {
        if(!permaBlocks[t1_0-1][t1_1-1]&&!permaBlocks[t2_0-1][t2_1+1]&&!permaBlocks[t4_0+1][t4_1-1]) {
          tetrinomeBlock1[0] = tetrinomeBlock1[0] - 1;
          tetrinomeBlock1[1] = tetrinomeBlock1[1] - 1;
          tetrinomeBlock2[0] = tetrinomeBlock2[0] - 1;
          tetrinomeBlock2[1] = tetrinomeBlock2[1] + 1;
          tetrinomeBlock4[0] = tetrinomeBlock4[0] + 1;
          tetrinomeBlock4[1] = tetrinomeBlock4[1] - 1;
          currRotate = 0;
        }
      }
    }
  }
}

//COMPLEX 1 - Park Miller RNG\\

long PMRng(long seedval) {
    long a = 16807;
    long m = 2147483647;
    long q = 127773;
    long r = 2836;
    long hi = seedval / q;
    long lo = seedval - q * hi;
    long test = a * lo - r * hi;
    if (test > 0) {
      seedval = test; 
    }
    else {
      seedval = test + m;
    }
    Serial.print("PMRng function gives: ");Serial.println(seedval);
  return seedval;
}




void maxTransfer(uint8_t address, uint8_t value) {
  digitalWrite(LOAD_PIN, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(LOAD_PIN, HIGH);
} 

void clearMatrix() {
  for (int i = 1; i <= 8; ++i) {
    maxTransfer(i,0);
    maxTransfer(i,0);
    maxTransfer(i,0);
    maxTransfer(i,0);
  }
}

//COMPLEX 2 - MAX7219\\

void calculateColumn() {
    //Serial.println("--- HIGH MATRIX ---");
    for (int a = 7; a >= 0; --a) {
    colAmount = 0;
      for (int b = 7; b >= 0; --b) {
        //Serial.print(permaBlocks[i][k] | spawnBlock[i][k]);
                           //column values (7)
        if(permaBlocks[b][a] | spawnBlock[b][a]) {
          colAmount = colAmount + round(pow(2, 7-b));
          //Serial.print("*");
        }
      }
      abs(colAmount);
      //Serial.print(" - ");Serial.println(colAmount);
      colArray[7-a] = colAmount;
    }
    
    //Serial.println("--- LOW MATRIX ---");
    for (int c = 7; c >= 0; --c) {
    colAmount = 0;
      for (int d = 15; d >= 8; --d) {
        //Serial.print(permaBlocks[i][k] | spawnBlock[i][k]);
                           //column values (7)
        if(permaBlocks[d][c] | spawnBlock[d][c]) {
          colAmount = colAmount + round(pow(2, 15-d));
          //Serial.print("*");
        }
      }
      abs(colAmount);
      //Serial.print(" - ");Serial.println(colAmount);
      colArray[15-c] = colAmount;
    }
}

void writeToMatrix() {
  //clearMatrix();
  calculateColumn();
  for (int i = 0; i < 8; ++i) {

    digitalWrite(LOAD_PIN, LOW);
    SPI.transfer(i+1);
    SPI.transfer(colArray[i]);
    SPI.transfer(0x0F);
    SPI.transfer(0x00);
    digitalWrite(LOAD_PIN, HIGH);
    digitalWrite(LOAD_PIN, LOW);
    SPI.transfer(i+1);
    SPI.transfer(colArray[i+8]);
    digitalWrite(LOAD_PIN, HIGH);
  }
}
