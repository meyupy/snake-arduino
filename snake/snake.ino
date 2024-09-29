const byte PINS_ROW[8] = {A0, 12, A2, 13, 5, A3, 7, 2};
const byte PINS_COL[8] = {9, 8, 4, A1, 3, 10, 11, 6};
const byte PIN_BUTTON_1 = A4, PIN_BUTTON_2 = A5;

class Game{

private:

  class Snake{

  private:

    void setDirection(int isPushedButton1, int isPushedButton2){
      this->direction += isPushedButton1&&isPushedButton2 ? 0 : (isPushedButton1 ? -1 : (isPushedButton2 ? 1 : 0));
      this->direction = this->direction==0 ? 4 : (this->direction==5 ? 1 : this->direction);
    }

    void arrangeNextPos(){
      byte headX = this->pos[0][0], headY = this->pos[0][1];
      switch(this->direction){
        case 1: headX += 1; break;
        case 2: headY -= 1; break;
        case 3: headX -= 1; break;
        case 4: headY += 1; break;
      }
      this->nextPos[0][0] = headX; this->nextPos[0][1] = headY;
      for(byte i=0; i<this->length-1; i++){
        for(byte j=0; j<2; j++) this->nextPos[i+1][j] = this->pos[i][j];
      }
    }

    bool willEat(byte appleX, byte appleY){
      return appleX==this->nextPos[0][0]&&appleY==this->nextPos[0][1];
    }

    bool willCrushWalls(){
      byte headX = this->nextPos[0][0], headY = this->nextPos[0][1];
      if(headX<0||headX>7||headY<0||headY>7) return true;
      return false;
    }

    bool willCrushTail(){
      byte headX = this->nextPos[0][0], headY = this->nextPos[0][1];
      for(byte i=1; i<63; i++){
        if(headX==this->nextPos[i][0]&&headY==this->nextPos[i][1]) return true;
      }
      return false;
    }

    void move(){
      for(byte i=0; i<64; i++){
        this->pos[i][0] = this->nextPos[i][0];
        this->pos[i][1] = this->nextPos[i][1];
      }
    }

    byte direction;  // 1-4, 0-270 degrees
    byte length;
    byte nextPos[64][2];

  public:

    void update(int isPushedButton1, int isPushedButton2, byte appleX, byte appleY){
      this->setDirection(isPushedButton1, isPushedButton2);
      this->arrangeNextPos();
      if(this->willEat(appleX, appleY)){
        this->nextPos[this->length][0] = this->pos[this->length-1][0];
        this->nextPos[this->length][1] = this->pos[this->length-1][1];
        this->hasEaten = true;
        this->length++;
      }
      if(this->willCrushWalls()||this->willCrushTail()){
        this->hasCrushed = true;
      }
      else{
        this->move();
      }
    }

    void resetValues(){
      this->direction = 2; this->length = 2;
      this->hasCrushed = false; this->hasEaten = false;
      for(byte i=0; i<64; i++){
        this->pos[i][0] = 8; this->pos[i][1] = 8;
        this->nextPos[i][0] = 8; this->nextPos[i][1] = 8;
      }
      this->pos[0][0] = 3; this->pos[0][1] = 6;
      this->pos[1][0] = 3; this->pos[1][1] = 7;
    }

    bool hasCrushed;
    bool hasEaten;
    byte pos[64][2];  // head to tail, empty ones are {8, 8}

  };

  class Apple{

  private:

    bool onSnake(byte snakePos[64][2]){
      for(byte i=0; i<64; i++){
        if(snakePos[i][0]==8) return false;
        if(this->pos[0]==snakePos[i][0]&&this->pos[1]==snakePos[i][1]) return true;
      }
    }

  public:

    void setPos(byte snakePos[64][2]){
      do{
        for(byte i=0; i<2; i++){
          randomSeed(micros());
          this->pos[i] = random(0, 8);
        }
      } while(this->onSnake(snakePos));
    }

    byte pos[2];

  };

  Snake snake;
  Apple apple;

  void update(){
    for(byte x=0; x<8; x++){
      for(byte y=0; y<8; y++) this->state[x][y] = 0;
    }
    for(byte i=0; i<64; i++){
      this->state[this->snake.pos[i][0]][this->snake.pos[i][1]] = 1;
    }
    this->state[this->apple.pos[0]][this->apple.pos[1]] = 1;
  }

  void resetValues(){
    this->snake.resetValues();
    this->apple.setPos(this->snake.pos);
  }

  bool running = false;
  const byte FTW = 12;  // frames to wait
  long int totalFrames = 0;

public:
  
  Game(){
    for(byte i=0; i<8; i++){
      for(byte j=0; j<8; j++){
        this->state[i][j] = i%2==j%2 ? 1 : 0;
      }
    }
  }

  void iterate(int isPushedButton1, int isPushedButton2){
    if(this->totalFrames % this->FTW == 0){
      if(this->running){
        this->snake.update(isPushedButton1, isPushedButton2, this->apple.pos[0], this->apple.pos[1]);
        if(this->snake.hasCrushed) this->running = false;
        if(this->snake.hasEaten){
          this->apple.setPos(this->snake.pos);
          this->snake.hasEaten = false;
        }
        this->update();
      }
      else if(isPushedButton1 && isPushedButton2){
        this->resetValues();
        this->running = true;
        this->update();
      }
    }
    this->totalFrames++;
  }

  byte state[8][8];

};

Game game;

void setup() {
  
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);

  for(byte i=0; i<8; i++){
    pinMode(PINS_ROW[i], OUTPUT);
    pinMode(PINS_COL[i], OUTPUT);
    digitalWrite(PINS_COL[i], HIGH);
  }

}

void loop() {

  game.iterate(!digitalRead(PIN_BUTTON_1), !digitalRead(PIN_BUTTON_2));
  displayMatrix(game.state);

}

void displayMatrix(byte state[8][8]) {

  for(byte r=0; r<8; r++){
    digitalWrite(PINS_ROW[r], HIGH);
    for(byte c=0; c<8; c++){
      const byte pixelState = state[7-r][c];  // vertical
      digitalWrite(PINS_COL[c], !pixelState);
      delayMicroseconds(250);
      if(pixelState) digitalWrite(PINS_COL[c], HIGH);
    }
    digitalWrite(PINS_ROW[r], LOW);
  }

}