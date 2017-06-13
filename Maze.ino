#include <SPI.h>
#include <Gamebuino.h>
Gamebuino gb;

#define MAX_WIDTH 50
#define MAX_HEIGHT 50
#define MAX_CANDIDATES 30

byte width;
byte height;
byte hWalls [(MAX_HEIGHT * MAX_WIDTH - 1) / 8 + 1];
byte vWalls [(MAX_HEIGHT * MAX_WIDTH - 1) / 8 + 1];
byte cells [(MAX_HEIGHT * MAX_WIDTH - 1) / 8 + 1];

int candidates [MAX_CANDIDATES];
byte candidateCount;

int posX;
int posY;

int movingX;
int movingY;

const byte logo[] PROGMEM = {64,36,
0x6F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0x6F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0x68,0x82,0x0,0xA0,0x0,0x2A,0x20,0x3,
0x6E,0xEE,0xBE,0xBE,0xFB,0xEA,0xEB,0xFF,
0x60,0x8,0x8A,0x0,0x2A,0x20,0xA,0x8B,
0x7B,0xAE,0xFB,0xAF,0xAF,0xAE,0xEE,0xBB,
0x60,0xA0,0x2A,0xA2,0xA,0x8,0x20,0xB,
0x7B,0xBE,0xEA,0xEF,0xFA,0xEF,0xAE,0xEB,
0x6A,0xA0,0x0,0x28,0x28,0xA2,0xA2,0x83,
0x6A,0xFF,0xBA,0xFB,0xEE,0xBE,0xFF,0xBB,
0x60,0x2,0xAA,0x0,0x2,0x80,0x28,0xB,
0x7B,0xFE,0xEF,0xAE,0xFA,0xBA,0xAE,0xFB,
0x60,0x80,0x0,0xA8,0x82,0x8A,0x82,0x2B,
0x7B,0xBE,0xAB,0xFF,0xFF,0xFB,0xBF,0xEF,
0x60,0x8,0xAB,0xFF,0xFF,0xFA,0x0,0xB,
0x7A,0xEE,0xBB,0x77,0xFF,0xFB,0xAE,0xEB,
0x62,0x22,0x8B,0x24,0xE3,0xB8,0xA2,0x23,
0x7A,0xEA,0xFF,0x56,0x7B,0x5E,0xEE,0xAB,
0x62,0x2A,0x83,0x55,0x77,0x38,0x8A,0xAB,
0x7B,0xFF,0xFB,0x74,0x63,0x9B,0xBB,0xAB,
0x60,0x0,0xA3,0xFF,0xFF,0xF8,0x80,0xAB,
0x7B,0xFA,0xAF,0xFF,0xFF,0xFB,0xFA,0xFB,
0x60,0x22,0x20,0xA0,0x2A,0x20,0xA,0x2B,
0x6F,0xBB,0xFF,0xBF,0xAB,0xBF,0xBA,0xEB,
0x62,0x2A,0x0,0x0,0x2A,0x80,0xAA,0x23,
0x6A,0xAE,0xFB,0xBE,0xEA,0xEF,0xEF,0xFF,
0x6A,0x80,0xA,0x8,0x0,0x2,0x88,0x3,
0x6F,0xEE,0xBF,0xFA,0xEE,0xEE,0xBE,0xAB,
0x60,0x22,0x88,0xA,0x82,0x80,0x0,0xAB,
0x7B,0xEB,0xBB,0xFB,0xAB,0xEE,0xEF,0xAF,
0x68,0x88,0x80,0xA,0xA8,0x22,0x22,0xA3,
0x6E,0xFE,0xEA,0xEE,0xFB,0xBF,0xAA,0xAF,
0x60,0x2,0x8A,0x20,0x22,0x0,0xA8,0xA3,
0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,
0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
};

void setup() {
  gb.begin();
  gb.pickRandomSeed();
  reset();
}

void loop() {
  if(gb.update()) {
    if(gb.buttons.pressed(BTN_C)) {
      reset();
    }

    if (movingX == 0 && movingY == 0) {
      if (posX == width - 2 && posY == height - 2) {
        gb.popup(F("     Good job!"), 60);
        gb.sound.playOK();
        generateMaze(width + 10 < MAX_WIDTH ? width + 10 : MAX_WIDTH, height + 10 < MAX_HEIGHT ? height + 10 : MAX_HEIGHT);
      }
      
      handleMovement();
    }

    drawMaze();

    if (movingX < 0) movingX += 1;
    if (movingY < 0) movingY += 1;
    if (movingX > 0) movingX -= 1;
    if (movingY > 0) movingY -= 1;
  }
}

void reset() {
  gb.titleScreen(F(""), logo);
  gb.battery.show = false;
  
  generateMaze(10, 10);
}

void handleMovement() {
  if (gb.buttons.repeat(BTN_UP, 1) && posY > 0 && !isSet(hWalls, posY, posX)) 
  {
    if (isSet(cells, posY - 1, posX)) gridSet(cells, posY, posX, false);
    posY--;
    movingY = -6;
    gridSet(cells, posY, posX, true);
  }
  else if (gb.buttons.repeat(BTN_DOWN, 1) && posY < height - 2 && !isSet(hWalls, posY + 1, posX)) 
  {
    if (isSet(cells, posY + 1, posX)) gridSet(cells, posY, posX, false);
    posY++;
    movingY = 6;
    gridSet(cells, posY, posX, true);
  }
  else if (gb.buttons.repeat(BTN_LEFT, 1) && posX > 0 && !isSet(vWalls, posY, posX)) 
  {
    if (isSet(cells, posY, posX - 1)) gridSet(cells, posY, posX, false);
    posX--;
    movingX = -6;
    gridSet(cells, posY, posX, true);
  }
  else if (gb.buttons.repeat(BTN_RIGHT, 1) && posX < width - 2 && !isSet(vWalls, posY, posX + 1)) 
  {
    if (isSet(cells, posY, posX + 1)) gridSet(cells, posY, posX, false);
    posX++;
    movingX = 6;
    gridSet(cells, posY, posX, true);
  }
}

void generateMaze(byte w, byte h) {
  width = w;
  height = h;

  for (int i = 0; i < sizeof(hWalls); i++) hWalls[i] = 0xFF;
  for (int i = 0; i < sizeof(vWalls); i++) vWalls[i] = 0xFF;
  for (int i = 0; i < sizeof(cells); i++) cells[i] = 0xFF;

  candidateCount = 0;
  moveToCell(0, 0);

  while (candidateCount > 0) {
    int randIndex = random(candidateCount);
    int choice = removeCandidate(randIndex);
    byte row = (byte)((choice >> 8) & 0xff);
    byte col = (byte)(choice & 0xff);

    bool foundWall = false;
    while (!foundWall) {
      switch (random(4)) {
        case 0:
          if (col > 0 && !isSet(cells, row, (byte)(col - 1))) {
            gridSet(vWalls, row, col, false);
            foundWall = true;
          }
          break;
        case 1:
          if (col < width - 2 && !isSet(cells, row, (byte)(col + 1))) {
            gridSet(vWalls, row, (byte)(col + 1), false);
            foundWall = true;
          }
          break;
        case 2:
          if (row > 0 && !isSet(cells, (byte)(row - 1), col)) {
            gridSet(hWalls, row, col, false);
            foundWall = true;
          }
          break;
        case 3:
          if (row < height - 2 && !isSet(cells, (byte)(row + 1), col)) {
            gridSet(hWalls, (byte)(row + 1), col, false);
            foundWall = true;
          }

          break;
      }
    }

    moveToCell(row, col);

    if (candidateCount == 0) {
      for (byte r = 0; r < height - 1; r++) {
        for (byte c = 0; c < width - 1; c++) {
          if (!isSet(cells, r, c)) moveToCell(r, c);
        }
      }
    }
  }

  gridSet(hWalls, 0, 0, false);
  gridSet(hWalls, (byte)(height - 1), (byte)(width - 2), false);
  posX = posY = movingX = movingY = 0;
  gridSet(cells, posX, posY, true);
}

void moveToCell(byte row, byte col) {
  gridSet(cells, row, col, false);
  
  if (col < width - 2 && isSet(cells, row, (byte)(col + 1))) {
    addCandidate((row << 8) + (col + 1));
  }
  if (col > 0 && isSet(cells, row, (byte)(col - 1))) {
    addCandidate((row << 8) + (col - 1));
  }
  if (row < height - 2 && isSet(cells, (byte)(row + 1), col)) {
    addCandidate(((row + 1) << 8) + col);
  }
  if (row > 0 && isSet(cells, (byte)(row - 1), col)) {
    addCandidate(((row - 1) << 8) + col);
  }
}

void addCandidate(int val) {
  if (candidateCount >= MAX_CANDIDATES) return;

  for (int i = 0; i < candidateCount; i++) {
    if (candidates[i] == val) return;
  }

  candidates[candidateCount] = val;
  candidateCount++;
}

int removeCandidate(int index) {
  int tmp = candidates[index];
  candidates[index] = candidates[candidateCount - 1];
  candidateCount--;
  return tmp;
}

void drawMaze() {
  gb.display.setColor(BLACK, BLACK);
  
  for (int row = posY >= 6 ? posY - 6 : 0; row < height && row < posY + 6; row++) {
    for (int col = posX >= 9 ? posX - 9 : 0; col < width && col < posX + 9; col++) {
      int baseX = (col - posX) * 6 + 38 + movingX;
      int baseY = (row - posY) * 6 + 20 + movingY;
      if (col < width - 1) {
        if (isSet(hWalls, row, col)) { 
          gb.display.drawFastHLine(baseX, baseY, 7);
        }

        if (row < height - 1) {
          if (isSet(cells, row, col)) {
            gb.display.fillRect(baseX + 2, baseY + 2, 3, 3);
            if (row > 0 && isSet(cells, row - 1, col) && !isSet(hWalls, row, col)) {
              gb.display.fillRect(baseX + 2, baseY - 1, 3, 3);
            }
            if (col > 0 && isSet(cells, row, col - 1) && !isSet(vWalls, row, col)) {
              gb.display.fillRect(baseX - 1, baseY + 2, 3, 3);
            }
          }
        }
      }

      if (row < height - 1) {
        if (isSet(vWalls, row, col)) {
          gb.display.drawFastVLine(baseX, baseY, 7);
        }
      }
    }
  }

  if (movingX > 0) {
    if (posX > 0 && isSet(cells, posY, posX - 1)/* && !isSet(vWalls, posY, posX) */) {
      gb.display.setColor(WHITE, WHITE);
      gb.display.fillRect(43, 22, movingX, 3);
    }
    else {
      gb.display.setColor(BLACK, BLACK);
      gb.display.fillRect(40, 22, movingX, 3);
    }
  }
  if (movingX < 0) {
    if (posX < width - 2 && isSet(cells, posY, posX + 1)/*  && !isSet(vWalls, posY, posX + 1)*/) {
      gb.display.setColor(WHITE, WHITE);
      gb.display.fillRect(40 + movingX, 22, -movingX, 3);
    }
    else {
      gb.display.setColor(BLACK, BLACK);
      gb.display.fillRect(43 + movingX, 22, -movingX, 3);
    }
  }

  if (movingY > 0) {
    if (posY > 0 && isSet(cells, posY - 1, posX)) {
      gb.display.setColor(WHITE, WHITE);
      gb.display.fillRect(40, 25, 3, movingY);
    }
    else {
      gb.display.setColor(BLACK, BLACK);
      gb.display.fillRect(40, 22, 3, movingY);
    }
  }
  if (movingY < 0) {
    if (posY < height - 2 && isSet(cells, posY + 1, posX)) {
      gb.display.setColor(WHITE, WHITE);
      gb.display.fillRect(40, 22 + movingY, 3, -movingY);
    }
    else {
      gb.display.setColor(BLACK, BLACK);
      gb.display.fillRect(40, 25 + movingY, 3, -movingY);
    }
  }
}

bool isSet(byte* grid, byte row, byte col) {
  int offset = row * width + col;
  return (grid[offset / 8] & (1 << (7 - (offset % 8)))) != 0;
}

void gridSet(byte* grid, byte row, byte col, bool on) {
  int offset = row * width + col;
  if (on) grid[offset / 8] |= (byte)(1 << (7 - (offset % 8)));
  else grid[offset / 8] &= (byte)~(1 << (7 - (offset % 8)));
}
