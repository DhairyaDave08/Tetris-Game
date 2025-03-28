/*
  Tetris Game
  BY:-  Data Detective

  contributors:
  Dhairya Dave-202401251
  Manthan Bhatt-202401407
  Hilag Shah-202401469
  Shlok Thakkar-202401203
*/

#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <fstream>

using namespace std;

const int WIDTH = 10;
const int HEIGHT = 20;
const int INITIAL_SPEED = 500;
const int SPEED_INCREMENT = 50;
const int MIN_SPEED = 100;
const int LINES_PER_LEVEL = 5;

// Declaring tetrominoes
vector<vector<vector<int>>> tetrominoes = {
    {{1, 1, 1, 1}},         // I
    {{1, 1}, {1, 1}},       // O
    {{1, 1, 1}, {0, 1, 0}}, // T
    {{0, 1, 1}, {1, 1, 0}}, // S
    {{1, 1, 0}, {0, 1, 1}}, // Z
    {{1, 0, 0}, {1, 1, 1}}, // L
    {{0, 0, 1}, {1, 1, 1}}  // J
};

vector<vector<int>> board(HEIGHT, vector<int>(WIDTH, 0));
int currentX, currentY, currentPiece, nextPiece;
vector<vector<int>> currentTetromino;
bool gameOver = false;
int score = 0;
int dropSpeed = INITIAL_SPEED;
int linesCleared = 0;
int level = 1;
DWORD lastFallTime = 0;
string playerName;
int highScore = 0;
bool smashUsed = false;

// Function prototypes
void gotoxy(int x, int y);
void hideCursor();
void setColor(int fgColor, int bgColor = 0);
int getPieceColor(int piece);
bool isValidMove(int x, int y, const vector<vector<int>>& shape);
void placePiece();
void clearLines();
vector<vector<int>> rotate(const vector<vector<int>>& shape);
void spawnPiece();
void hardDrop();
void drawBoard();
void handleInput();
void loadHighScore();
void saveHighScore();
void gameLoop();
void drawNumber(int number);
void countdown();
void playSound(int frequency, int duration);

// Function definitions
void gotoxy(int x, int y) {
    COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo = { 1, false };
    SetConsoleCursorInfo(console, &cursorInfo);
}

void setColor(int fgColor, int bgColor) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fgColor | (bgColor << 4));
}

int getPieceColor(int piece) {
    int colors[] = { 11, 14, 13, 10, 12, 6, 9 };
    return colors[piece % 7];
}

bool isValidMove(int x, int y, const vector<vector<int>>& shape) {
    for (size_t i = 0; i < shape.size(); i++) {
        for (size_t j = 0; j < shape[i].size(); j++) {
            if (shape[i][j]) {
                int newX = x + j;
                int newY = y + i;
                if (newX < 0 || newX >= WIDTH || newY >= HEIGHT || (newY >= 0 && board[newY][newX])) {
                    return false;
                }
            }
        }
    }
    return true;
}

void placePiece() {
    for (size_t i = 0; i < currentTetromino.size(); i++) {
        for (size_t j = 0; j < currentTetromino[i].size(); j++) {
            if (currentTetromino[i][j]) {
                board[currentY + i][currentX + j] = currentPiece + 1;
            }
        }
    }
}

void clearLines() {
    int cleared = 0;
    for (int i = HEIGHT - 1; i >= 0; i--) {
        bool fullLine = true;
        for (int j = 0; j < WIDTH; j++) {
            if (!board[i][j]) {
                fullLine = false;
                break;
            }
        }

        if (fullLine) {
            cleared++;
            for (int k = i; k > 0; k--) {
                board[k] = board[k - 1];
            }
            board[0] = vector<int>(WIDTH, 0);
            i++;
        }
    }

    if (cleared > 0) {
        score += cleared * 100;
        linesCleared += cleared;

        if (linesCleared >= LINES_PER_LEVEL * level) {
            level++;
            dropSpeed = max(MIN_SPEED, dropSpeed - SPEED_INCREMENT);
        }

        playSound(800, 200);
    }
}

vector<vector<int>> rotate(const vector<vector<int>>& shape) {
    size_t rows = shape.size(), cols = shape[0].size();
    vector<vector<int>> rotated(cols, vector<int>(rows, 0));
    for (size_t i = 0; i < rows; i++)
        for (size_t j = 0; j < cols; j++)
            rotated[j][rows - 1 - i] = shape[i][j];
    return rotated;
}

void spawnPiece() {
    currentPiece = nextPiece;
    nextPiece = rand() % tetrominoes.size();
    currentTetromino = tetrominoes[currentPiece];
    currentX = WIDTH / 2 - currentTetromino[0].size() / 2;
    currentY = 0;
    if (!isValidMove(currentX, currentY, currentTetromino))
        gameOver = true;
}

void hardDrop() {
    playSound(600, 300); // Play sound at start of hard drop
    while (isValidMove(currentX, currentY + 1, currentTetromino)) {
        currentY++;
    }
    placePiece();
    clearLines();
    spawnPiece();
}

void drawBoard() {
    gotoxy(0, 0);
    setColor(15);
    cout << playerName << "'s Score: " << score << "   Level: " << level << "   High Score: " << highScore << "\n";

    for (int i = 0; i < HEIGHT; i++) {
        cout << "|";
        for (int j = 0; j < WIDTH; j++) {
            bool isTetromino = false;
            for (size_t ti = 0; ti < currentTetromino.size(); ti++) {
                for (size_t tj = 0; tj < currentTetromino[ti].size(); tj++) {
                    if (currentTetromino[ti][tj] && currentY + ti == i && currentX + tj == j) {
                        setColor(getPieceColor(currentPiece));
                        cout << "[]";
                        isTetromino = true;
                        break;
                    }
                }
                if (isTetromino) break;
            }
            if (!isTetromino) {
                if (board[i][j]) {
                    setColor(getPieceColor(board[i][j] - 1));
                    cout << "[]";
                }
                else {
                    setColor(8);
                    cout << " .";
                }
            }
        }
        setColor(15);
        cout << "|\n";
    }

    for (int i = 0; i < 4; i++) {
        gotoxy(WIDTH * 2 + 5, 3 + i);
        cout << "        ";
    }

    setColor(14);
    gotoxy(WIDTH * 2 + 5, 2);
    cout << "Next Piece:";
    for (size_t i = 0; i < tetrominoes[nextPiece].size(); i++) {
        gotoxy(WIDTH * 2 + 5, 3 + i);
        for (size_t j = 0; j < tetrominoes[nextPiece][i].size(); j++) {
            if (tetrominoes[nextPiece][i][j]) {
                setColor(getPieceColor(nextPiece));
                cout << "[]";
            }
            else {
                cout << "  ";
            }
        }
    }

    setColor(14);
    gotoxy(WIDTH * 2 + 5, 15);
    cout << "RULE BOOK:";
    gotoxy(WIDTH * 2 + 5, 16);
    cout << "Left Arrow  - Move Left";
    gotoxy(WIDTH * 2 + 5, 17);
    cout << "Right Arrow - Move Right";
    gotoxy(WIDTH * 2 + 5, 18);
    cout << "Up Arrow    - Rotate";
    gotoxy(WIDTH * 2 + 5, 19);
    cout << "Down Arrow  - Soft Drop";
    gotoxy(WIDTH * 2 + 5, 20);
    cout << "Spacebar    - Hard Drop";
    gotoxy(WIDTH * 2 + 5, 21);
    cout << "S           - Smash (Once per game, requires 500 points)";
    gotoxy(WIDTH * 2 + 5, 22);
    cout << "Q           - Quit";
}

void handleInput() {
    if (_kbhit()) {
        int key = _getch();
        if (key == 'q' || key == 'Q') gameOver = true;
        if (key == 32) hardDrop();
        if (key == 's' || key == 'S') {
            if (score >= 500 && !smashUsed) {
                for (int i = 0; i < HEIGHT; i++) {
                    for (int j = 0; j < WIDTH; j++) {
                        board[i][j] = 0;
                    }
                }
                smashUsed = true;
                playSound(1000, 300);
            }
        }
        if (key == 224) {
            key = _getch();
            if (key == 75 && isValidMove(currentX - 1, currentY, currentTetromino)) {
                currentX--;
            }
            if (key == 77 && isValidMove(currentX + 1, currentY, currentTetromino)) {
                currentX++;
            }
            if (key == 80 && isValidMove(currentX, currentY + 1, currentTetromino)) {
                currentY++;
            }
            if (key == 72) {
                vector<vector<int>> rotated = rotate(currentTetromino);
                if (isValidMove(currentX, currentY, rotated)) {
                    currentTetromino = rotated;
                }
            }
        }
    }
}

void loadHighScore() {
    ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
}

void saveHighScore() {
    ofstream file("highscore.txt");
    if (file.is_open()) {
        file << highScore;
        file.close();
    }
}

void playSound(int frequency, int duration) {
    Beep(frequency, duration);
    Sleep(50);
}

void drawNumber(int number) {
    vector<string> num3 = {
        "*****",
        "    *",
        "*****",
        "    *",
        "*****"
    };
    vector<string> num2 = {
        "*****",
        "    *",
        "****",
        "*    ",
        "*****"
    };
    vector<string> num1 = {
        "  *  ",
        "  *  ",
        "  *  ",
        "  *  ",
        "  *  "
    };
    vector<string> start = {
        "*****  *****  ***  ****  *****",
        "*        *   *   * *   *   *  ",
        "*****    *   ***** ****    *  ",
        "    *    *   *   * *  *    *  ",
        "*****    *   *   * *   *   *  "
    };

    vector<string> design;
    switch (number) {
    case 3: design = num3; break;
    case 2: design = num2; break;
    case 1: design = num1; break;
    case 0: design = start; break;
    }

    int startY = HEIGHT / 2 - 2;
    int startX = WIDTH * 2 + 5;

    for (size_t i = 0; i < design.size(); i++) {
        gotoxy(startX, startY + i);
        cout << design[i];
    }
}

void countdown() {
    system("cls");
    setColor(14);
    for (int i = 3; i >= 1; i--) {
        drawNumber(i);
        Sleep(1000);
        system("cls");
    }
    drawNumber(0);
    Sleep(500);
    system("cls");
}

void gameLoop() {
    lastFallTime = GetTickCount();
    while (!gameOver) {
        handleInput();
        if (GetTickCount() - lastFallTime >= dropSpeed) {
            if (isValidMove(currentX, currentY + 1, currentTetromino)) {
                currentY++;
            }
            else {
                placePiece();
                clearLines();
                spawnPiece();
            }
            lastFallTime = GetTickCount();
        }
        drawBoard();
        Sleep(10);
    }

    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }

    playSound(200, 500);
}

int main() {
    loadHighScore();

    system("cls");
    setColor(14);
    cout << "===== TETRIS GAME =====\n";
    setColor(15);
    cout << "Enter player name: ";
    cin >> playerName;

    while (true) {
        hideCursor();
        srand(time(0));
        nextPiece = rand() % tetrominoes.size();
        countdown();
        system("cls");
        spawnPiece();
        gameLoop();

        setColor(12);
        cout << "\nGAME OVER. Final Score: " << score << "\n";
        setColor(15);
        cout << "Play Again? (Y/N): ";
        char c;
        cin >> c;
        if (c != 'y' && c != 'Y') break;
        gameOver = false;
        score = 0;
        level = 1;
        linesCleared = 0;
        smashUsed = false;
        board = vector<vector<int>>(HEIGHT, vector<int>(WIDTH, 0));
    }
}
