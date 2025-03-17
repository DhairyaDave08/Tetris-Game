#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <ctime>

using namespace std;

const int WIDTH = 10;
const int HEIGHT = 20;
const int INITIAL_SPEED = 500;
const int SPEED_INCREMENT = 50;
const int MIN_SPEED = 100;
const int LINES_PER_LEVEL = 5;

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
int currentX, currentY, currentPiece;
vector<vector<int>> currentTetromino;
bool gameOver = false;
int score = 0;
int dropSpeed = INITIAL_SPEED;
int linesCleared = 0;
int level = 1;
DWORD lastFallTime = 0;

void gotoxy(int x, int y) {
    COORD coord = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo = {1, false};
    SetConsoleCursorInfo(console, &cursorInfo);
}

void setColor(int fgColor, int bgColor) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fgColor | (bgColor << 4));
}

int getPieceColor(int piece) {
    int colors[] = {11, 14, 13, 10, 12, 6, 9}; // Vibrant colors
    return colors[piece % 7];
}

bool isValidMove(int x, int y, const vector<vector<int>> &shape) {
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
    }
}

vector<vector<int>> rotate(const vector<vector<int>> &shape) {
    size_t rows = shape.size(), cols = shape[0].size();
    vector<vector<int>> rotated(cols, vector<int>(rows, 0));
    for (size_t i = 0; i < rows; i++)
        for (size_t j = 0; j < cols; j++)
            rotated[j][rows - 1 - i] = shape[i][j];
    return rotated;
}

void spawnPiece() {
    currentPiece = rand() % tetrominoes.size();
    currentTetromino = tetrominoes[currentPiece];
    currentX = WIDTH / 2 - currentTetromino[0].size() / 2;
    currentY = -1;
    if (!isValidMove(currentX, currentY, currentTetromino))
        gameOver = true;
}

void drawBoard() {
    gotoxy(0, 0);
    setColor(0, 7);
    cout << "Score: " << score << "   Level: " << level << "\n";

    for (int i = 0; i < HEIGHT; i++) {
        cout << "|";
        for (int j = 0; j < WIDTH; j++) {
            bool isTetrominoPart = false;
            for (size_t ti = 0; ti < currentTetromino.size(); ti++) {
                for (size_t tj = 0; tj < currentTetromino[ti].size(); tj++) {
                    if (currentTetromino[ti][tj] && i == currentY + ti && j == currentX + tj) {
                        isTetrominoPart = true;
                        setColor(getPieceColor(currentPiece), getPieceColor(currentPiece));
                        cout << "[]";
                        break;
                    }
                }
            }
            if (!isTetrominoPart) {
                if (board[i][j]) {
                    setColor(getPieceColor(board[i][j] - 1), getPieceColor(board[i][j] - 1));
                    cout << "[]";
                } else {
                    setColor(0, 8);
                    cout << " .";
                }
            }
        }
        setColor(7, 0);
        cout << "|\n";
    }
}

void handleInput() {
    if (_kbhit()) {
        int key = _getch();
        if (key == 224) {
            switch (_getch()) {
                case 75:
                    if (isValidMove(currentX - 1, currentY, currentTetromino)) currentX--;
                    break;
                case 77:
                    if (isValidMove(currentX + 1, currentY, currentTetromino)) currentX++;
                    break;
                case 80:
                    if (isValidMove(currentX, currentY + 1, currentTetromino)) currentY++;
                    break;
                case 72:
                    auto rotated = rotate(currentTetromino);
                    if (isValidMove(currentX, currentY, rotated))
                        currentTetromino = rotated;
                    break;
            }
        } else if (key == 'q' || key == 'Q') {
            gameOver = true;
        } else if (key == 32) {
            while (isValidMove(currentX, currentY + 1, currentTetromino)) currentY++;
        }
    }
}

void update() {
    if (GetTickCount() - lastFallTime > dropSpeed) {
        if (isValidMove(currentX, currentY + 1, currentTetromino)) currentY++;
        else {
            placePiece();
            clearLines();
            spawnPiece();
        }
        lastFallTime = GetTickCount();
    }
}

int main() {
    srand(time(0));
    system("cls");
    hideCursor();
    spawnPiece();
    while (!gameOver) {
        handleInput();
        update();
        drawBoard();
    }
    system("cls");
    cout << "\nGAME OVER\nSCORE: " << score << endl;
    return 0;
}
