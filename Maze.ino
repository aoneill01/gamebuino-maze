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
0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
0xD1,0x4,0x1,0x40,0x0,0x54,0x40,0x6,
0xDD,0xDD,0x7D,0x7D,0xF7,0xD5,0xD7,0xFE,
0xC0,0x11,0x14,0x0,0x54,0x40,0x15,0x16,
0xF7,0x5D,0xF7,0x5F,0x5F,0x5D,0xDD,0x76,
0xC1,0x40,0x55,0x44,0x14,0x10,0x40,0x16,
0xF7,0x7D,0xD5,0xDF,0xF5,0xDF,0x5D,0xD6,
0xD5,0x40,0x0,0x50,0x51,0x45,0x45,0x6,
0xD5,0xFF,0x75,0xF7,0xDD,0x7D,0xFF,0x76,
0xC0,0x5,0x54,0x0,0x5,0x0,0x50,0x16,
0xF7,0xFD,0xDF,0x5D,0xF5,0x75,0x5D,0xF6,
0xC1,0x0,0x1,0x51,0x5,0x15,0x4,0x56,
0xF7,0x7D,0x57,0xFF,0xFF,0xF7,0x7F,0xDE,
0xC0,0x11,0x57,0xFF,0xFF,0xF4,0x0,0x16,
0xF5,0xDD,0x76,0xEF,0xFF,0xF7,0x5D,0xD6,
0xC4,0x45,0x16,0x49,0xC7,0x71,0x44,0x46,
0xF5,0xD5,0xFE,0xAC,0xF6,0xBD,0xDD,0x56,
0xC4,0x55,0x6,0xAA,0xEE,0x71,0x15,0x56,
0xF7,0xFF,0xF6,0xE8,0xC7,0x37,0x77,0x56,
0xC0,0x1,0x47,0xFF,0xFF,0xF1,0x1,0x56,
0xF7,0xF5,0x5F,0xFF,0xFF,0xF7,0xF5,0xF6,
0xC0,0x44,0x41,0x40,0x54,0x40,0x14,0x56,
0xDF,0x77,0xFF,0x7F,0x57,0x7F,0x75,0xD6,
0xC4,0x54,0x0,0x0,0x55,0x1,0x54,0x46,
0xD5,0x5D,0xF7,0x7D,0xD5,0xDF,0xDF,0xFE,
0xD5,0x0,0x14,0x10,0x0,0x5,0x10,0x6,
0xDF,0xDD,0x7F,0xF5,0xDD,0xDD,0x7D,0x56,
0xC0,0x45,0x10,0x15,0x5,0x0,0x1,0x56,
0xF7,0xD7,0x77,0xF7,0x57,0xDD,0xDF,0x5E,
0xD1,0x11,0x0,0x15,0x50,0x44,0x45,0x46,
0xDD,0xFD,0xD5,0xDD,0xF7,0x7F,0x55,0x5E,
0xC0,0x5,0x14,0x40,0x44,0x1,0x51,0x46,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF6,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF6,
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
      handleMovement();
    }
    else if (posX == width - 2 && posY == height - 2) {
      gb.popup(F("     Good job!"), 60);
      gb.sound.playOK();
      generateMaze(width + 10 < MAX_WIDTH ? width + 10 : MAX_WIDTH, height + 10 < MAX_HEIGHT ? height + 10 : MAX_HEIGHT);
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
      int baseY = (row - posY) * 6 + 20 + movingY;;
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
