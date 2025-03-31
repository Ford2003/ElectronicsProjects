#undef abs
#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#define abs(x) ((x)>0?(x):-(x))

#define UP_BUTTON 2
#define DOWN_BUTTON 3
#define LEFT_BUTTON 4
#define RIGHT_BUTTON 5

const int buttonArray[4] = {UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON};

// Define size maze that can fit on the Arduino UNO R4 WiFi LED Matrix
const int ROWS = 3;
const int COLS = 5;

ArduinoLEDMatrix matrix;

struct Cell {
  bool visited = false;
  bool walls[4] = {true, true, true, true}; // N, E, S, W
};

std::vector<std::vector<Cell>> maze;

std::mt19937 gen;

// LiquidCrystal(rs, e, db4, db5, db6, db7);
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

unsigned long lastTimeButtonStateChanged = 0;
const unsigned long buttonDebounceTime = 100;
byte lastButtonStates[4] = {LOW, LOW, LOW, LOW};

// Player variables
byte playerPosition[2] = {1, 0};
unsigned long lastTimePlayerChanged = 0;
bool playerVisible = false;

// Bitmap frame to be displayed on the 8x12 LED matrix of the arduino UNO R4 WIFI
byte frame[8][12] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
  }; // Initialize 8x12 array with 1s

byte score = 0;
const int highScoreAddr = 0;
byte highScore = 0;


// Text options to display when you escape the maze.
const char* options[] = {
        "    Well Done!    ",
        "    Good Job!    ",
        "    Yaaaaaaay    ",
        "    Splendid    "
    };
// Distribution used to get random indices of the options array
std::uniform_int_distribution<> dis(0, sizeof(options) / sizeof(options[0]) - 1);

// Recursively generates the maze.
void generateMaze(int row, int col) {
  maze[row][col].visited = true;
  std::vector<int> directions = {0, 1, 2, 3}; // N, E, S, W
  std::shuffle(directions.begin(), directions.end(), gen);

  for (int dir : directions) {
    int newRow = row;
    int newCol = col;

    switch (dir) {
      case 0: // North
        if (row > 0 && !maze[row - 1][col].visited) {
          newRow--;
          maze[row][col].walls[0] = false;
          maze[newRow][col].walls[2] = false;
        } else {continue;}
        break;
      case 1: // East
        if (col < COLS - 1 && !maze[row][col + 1].visited) {
          newCol++;
          maze[row][col].walls[1] = false;
          maze[row][newCol].walls[3] = false;
        } else {continue;}
        break;
      case 2: // South
        if (row < ROWS - 1 && !maze[row + 1][col].visited) {
          newRow++;
          maze[row][col].walls[2] = false;
          maze[newRow][col].walls[0] = false;
        } else {continue;}
        break;
      case 3: // West
        if (col > 0 && !maze[row][col - 1].visited) {
          newCol--;
          maze[row][col].walls[3] = false;
          maze[row][newCol].walls[1] = false;
        } else {continue;}
        break;
    }
    generateMaze(newRow, newCol);
  }
}

// Prints the maze to the Serial Monitor
void printMaze() {
  Serial.println("Maze:");
  for (int row = 0; row < ROWS; ++row) {
    for (int col = 0; col < COLS; ++col) {
      Serial.print("+");
      Serial.print(maze[row][col].walls[0] ? "---" : "   "); // North wall
    }
    Serial.println("+"); // Rightmost +
    for (int col = 0; col < COLS; ++col) {
      Serial.print(maze[row][col].walls[3] ? "|" : " "); // West wall
      Serial.print("   "); // Cell content (empty for now)
    }
    Serial.println("|"); // Rightmost wall
  }
  for (int col = 0; col < COLS; ++col) {
    if (col == COLS - 1 && !maze[ROWS - 1][COLS - 1].walls[2]) {
      Serial.print("+   "); // No south wall for the bottom-right cell
    } else {
      Serial.print("+---");
    }
  }
  Serial.println("+");
}

// Sets the frame back to its default state
void resetFrame() {
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 12; j++){
      if (i == 7 or j == 11) {frame[i][j] = 0;}
      else {frame[i][j] = 1;}
    }
  }
}

// Defaults the frame and refills it in using the generated maze, to be displayed as a bitmap on the LED matrix.
void displayMaze(bool print) {
  resetFrame();
  for (int row = 0; row < 2 * ROWS + 1; ++row) {
    for (int col = 0; col < 2 * COLS + 1; ++col) {
      if (row % 2 == 0 and row / 2 < ROWS and col % 2 == 1) {
        frame[row][col] = maze[row / 2][(col - 1) / 2].walls[0];
      }
      if (row / 2 == ROWS and col % 2 == 1) {
        frame[row][col] = maze[(row / 2) - 1][(col - 1) / 2].walls[2];
      }
      if (row % 2 == 1) {
        if (col % 2 == 0 and col / 2 < COLS) {
          frame[row][col] = maze[(row - 1) / 2][col / 2].walls[3];
        }
        if (col / 2 == COLS) {
          frame[row][col] = maze[(row - 1) / 2][(col / 2) - 1].walls[1];
        }
        if (col % 2 == 1){
          frame[row][col] = 0;
        }
      }
    }
  }
  if (print) {
    for (int row = 0; row < 8; row++){
      for (int col = 0; col < 12; col++) {
        Serial.print(frame[row][col]);
      }
      Serial.println("");
    }
  }
  
  matrix.renderBitmap(frame, 8, 12); // Display bitmap
}

// Display the player on the LED matrix
void showPlayer() {
  frame[playerPosition[1]][playerPosition[0]] = 1;
  matrix.renderBitmap(frame, 8, 12);
}

// Hide the player from the LED matrix
void hidePlayer() {
  frame[playerPosition[1]][playerPosition[0]] = 0;
  matrix.renderBitmap(frame, 8, 12);
}

// Prints the frame to the Serial Monitor
void printFrame() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 12; j++) {
      Serial.print(frame[i][j]);
    }
    Serial.println("");
  }
}

// Displays a single digit number on the 7-segment display
void displayScore(byte highScore, byte score) {
  lcd.clear();
  lcd.print("Your Score: ");
  lcd.print(score);
  lcd.setCursor(0,1); // Move cursor to the 2nd row
  lcd.print("High Score: ");
  lcd.print(highScore);
}

// Displays a randomly chosen scrolling text on the LED matrix from the array of options.
void scrollText() {
  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(40);

  // add the text
  int randomIndex = dis(gen);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(options[randomIndex]);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();
}

void setup() {
  Serial.begin(9600);
  Serial.flush();
  lcd.begin(16,2);
  matrix.begin();
  maze.assign(ROWS, std::vector<Cell>(COLS));
  randomSeed(analogRead(0));
  gen.seed(analogRead(0));
  // Generate maze and make starting and ending holes.
  generateMaze(0, 0);
  maze[0][0].walls[0] = false;
  maze[ROWS-1][COLS-1].walls[2] = false;

  // Set pinmodes on the control buttons
  pinMode(UP_BUTTON, INPUT);
  pinMode(DOWN_BUTTON, INPUT);
  pinMode(LEFT_BUTTON, INPUT);
  pinMode(RIGHT_BUTTON, INPUT);

  
  
  highScore = EEPROM.read(highScoreAddr);
  Serial.println("Saved Highscore");
  Serial.print(highScore);
  Serial.println("");
  // If highest score has been reached or never been saved then write 0 as highscore
  if (highScore == 255) { 
    EEPROM.write(highScoreAddr, 0);
    highScore = 0;
  }

  // Display initial score of 0 and saved highscore.
  displayScore(highScore, score);

  // Print the maze to the Serial Monitor
  printMaze();
  displayMaze(false);
  Serial.println("");
  printFrame();
}

void loop() {
  // Handle blinking the player.
  if (millis() - lastTimePlayerChanged > 500) {
    if (playerVisible) {
      hidePlayer();
    }
    else {
      showPlayer();
    }
    playerVisible = !playerVisible;
    lastTimePlayerChanged = millis();
  }

  if (millis() - lastTimeButtonStateChanged > buttonDebounceTime) {
    for (int i = 0; i < 4; i++) {
      byte buttonState = digitalRead(buttonArray[i]);
      if (buttonState == lastButtonStates[i]) {continue;}
      lastTimeButtonStateChanged = millis();
      lastButtonStates[i] = buttonState;
      if (buttonState == HIGH) {
        if (buttonArray[i] == DOWN_BUTTON and playerPosition[1] < 7 and !frame[playerPosition[1]+1][playerPosition[0]]) { // Pressed down button and not on the bottom of the maze and the next position down is a 0.
          hidePlayer();
          playerPosition[1]++;
        }
        else if (buttonArray[i] == UP_BUTTON and playerPosition[1] > 0 and !frame[playerPosition[1]-1][playerPosition[0]]) { // Pressed up button and not on the top of the maze and the next position up is a 0.
          hidePlayer();
          playerPosition[1]--;
        }
        else if (buttonArray[i] == LEFT_BUTTON and playerPosition[0] > 0 and !frame[playerPosition[1]][playerPosition[0]-1]) { // Pressed left button and not on the left of the maze and the next position left is a 0.
          hidePlayer();
          playerPosition[0]--;
        }
        else if (buttonArray[i] == RIGHT_BUTTON and playerPosition[0] < 11 and !frame[playerPosition[1]][playerPosition[0]+1]) { // Pressed right button and not on the right of the maze and the next position right is a 0.
          hidePlayer();
          playerPosition[0]++;
        }
        if (playerPosition[0] == 11 or playerPosition[1] == 7) { // Player escaped the maze
          score = (score + 1) % 255;
          Serial.println("YOU WIN!");
          Serial.print("Current Score: ");
          Serial.print(score);
          Serial.println("");
          if (score > highScore) {
            highScore = score;
            EEPROM.write(highScoreAddr, highScore);
            delay(50);
            Serial.println("New High Score: ");
            Serial.print(highScore);
            Serial.println("");
          }
          // Show the score on the 7-Seg display.
          displayScore(highScore, score);
          scrollText();
          // Generate new maze
          maze.assign(ROWS, std::vector<Cell>(COLS));
          generateMaze(0, 0);
          maze[0][0].walls[0] = false;
          maze[ROWS-1][COLS-1].walls[2] = false;
          playerPosition[0] = 1;
          playerPosition[1] = 0;
          displayMaze(false);
        }
      }
    }
  }
}
