#include <Windows.h>
#include <ctime>
#include <vector>
#include <sstream>
#include <string>

// prototype function
void drawRect(HDC hdc, RECT rect, int color);
void loadImages();

// Constants for screen and paddle settings
constexpr int SCREEN_H = 720;
constexpr int SCREEN_W = 1280;
constexpr int PADDLE_WIDTH = 125;
constexpr int PADDLE_HEIGHT = 25;
constexpr int BALL_SIZE = 15;
constexpr int STEP = 8;
constexpr int B_STEP = 4;
constexpr int FRAME_DELAY = 16;
bool GAME_START = false;
bool GAME_OVER = false;
int lives = 3;
int winloss = -1; // -1 : game still going, 0 : game lost, 1 : game won

// Paddle position and movement states
int paddleX = (SCREEN_W / 2) - (PADDLE_WIDTH / 2);
int paddleY = SCREEN_H - 100;
bool p_movingLeft = false;
bool p_movingRight = false;

// Ball position and movement states
int ballX = (SCREEN_W / 2) - (BALL_SIZE / 2);
int ballY = SCREEN_H - 300;
bool b_movingLeft = false;
bool b_movingRight = false;
bool b_movingUp = true;
bool b_movingDown = false;

// Brick params
constexpr int BRICK_WIDTH = 125;
constexpr int BRICK_HEIGHT = 30;
constexpr int BRICK_COLUMNS = 8;
constexpr int BRICK_ROWS = 5;
constexpr int BRICK_SPACING = 31;

class Brick {
    public:
    int x, y;
    int width, height;
    bool hit;
    int color;
    RECT rect;

    Brick(int xPos, int yPos, int w, int h, int c) {
        x = xPos; y = yPos;
        width = w; height = h;
        hit = false;
        rect = { x, y, x + width, y + height };
        color = c;
    }

    void draw(HDC hdc) const {
        if (!hit) {
            drawRect(hdc, rect, color);
        }
    }

    bool isTouchingBall() const {
        return !(ballX + BALL_SIZE < x ||
            ballX > x + width ||
            ballY + BALL_SIZE < y ||
            ballY > y + height);
    }
};

// Update paddle position based on user input
void updatePaddle() {
    if (p_movingLeft && paddleX > 0) {
        paddleX -= STEP;
    }
    if (p_movingRight && paddleX + PADDLE_WIDTH < SCREEN_W) {
        paddleX += STEP;
    }

    // Ensure paddle stays within screen boundaries
    if (paddleX + PADDLE_WIDTH >= SCREEN_W) {
        paddleX = SCREEN_W - PADDLE_WIDTH;
    } else if (paddleX < 0) {
        paddleX = 0;
    }
}

// Update ball position per frame
void updateBall() {

    // Paddle collision
    bool hitsPaddle = ballY + BALL_SIZE >= paddleY &&
        ballY + BALL_SIZE <= paddleY + PADDLE_HEIGHT &&
        ballX + BALL_SIZE >= paddleX &&
        ballX <= paddleX + PADDLE_WIDTH;

    if (b_movingLeft && ballX > 0) {
        ballX -= B_STEP;
    } else {
        b_movingLeft = false;
        b_movingRight = true;
    }
    if (b_movingRight && ballX + BALL_SIZE < SCREEN_W) {
        ballX += B_STEP;
    } else {
        b_movingLeft = true;
        b_movingRight = false;
    }

    if (b_movingUp && ballY > 0) {
        ballY -= B_STEP;
    } else {
        b_movingUp = false;
        b_movingDown = true;
    }
    if (b_movingDown && ballY + BALL_SIZE < SCREEN_H) {
        ballY += B_STEP;
    } else {
        b_movingUp = true;
        b_movingDown = false;
    }

    // Ensure ball stays within screen boundaries
    if (ballX + BALL_SIZE >= SCREEN_W) {
        ballX = SCREEN_W - BALL_SIZE;
    } else if (ballX < 0) {
        ballX = 0;
    }

    if (hitsPaddle) {
        b_movingUp = true;
        b_movingDown = false;

        if (p_movingLeft) {
            b_movingLeft = true;
            b_movingRight = false;
        } else if (p_movingRight) {
            b_movingLeft = false;
            b_movingRight = true;
        }

    } else if (ballY + BALL_SIZE >= SCREEN_H) {
        // Ball missed the paddle — reset position or bounce back
        ballX = (SCREEN_W / 2) - BALL_SIZE / 2;
        ballY = SCREEN_H / 2;
    
        b_movingUp = true;
        b_movingDown = false;
        lives--;

        if (lives == 0) {
            GAME_OVER = true;
            winloss = 0; // Ensure winloss is set to 0 when lives run out
            return;
        }

    } else if (ballY < 0) {
        ballY = 0;
        b_movingUp = false;
        b_movingDown = true;
    }
}

// Brick vector
std::vector<Brick> bricks;

// Initialize the bricks vector
void initBricks() {
    for (int row = 0; row < BRICK_ROWS; ++row) {
        for (int col = 0; col < BRICK_COLUMNS; ++col) {
            int x = BRICK_SPACING + col * (BRICK_WIDTH + BRICK_SPACING);
            int y = 50 + row * (BRICK_HEIGHT + BRICK_SPACING);
            bricks.emplace_back(x, y, BRICK_WIDTH, BRICK_HEIGHT, row + 2);
        }
    }
}

// Draw a rectangle with a specified color
void drawRect(HDC hdc, RECT rect, int color) {
    HBRUSH brush = nullptr;

    switch (color) {
        case 0: // Black
            brush = CreateSolidBrush(RGB(0, 0, 0));
            break;
        case 1: // White
            brush = CreateSolidBrush(RGB(255, 255, 255));
            break;
        case 2: // Red
            brush = CreateSolidBrush(RGB(255, 0, 0));
            break;
        case 3: // Orange
            brush = CreateSolidBrush(RGB(255, 165, 0));
            break;
        case 4: // Yellow
            brush = CreateSolidBrush(RGB(255, 255, 0));
            break;
        case 5: // Green
            brush = CreateSolidBrush(RGB(0, 128, 0));
            break;
        case 6: // Blue
            brush = CreateSolidBrush(RGB(0, 0, 255));
            break;
    }

    if (brush) {
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }
}

// win/loss images
HBITMAP yourdidit = nullptr;
HBITMAP betterluck = nullptr;
HWND hYourdidit = nullptr;
HWND hBetterluck = nullptr;

void loadImages() {
    // returns the type of the handle that needs to be cast to an HBITMAP
    yourdidit = (HBITMAP)LoadImageW(NULL, L"bmps\\yourdidit.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    betterluck = (HBITMAP)LoadImageW(NULL, L"bmps\\betterluck.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}


// Window procedure for handling messages
LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            loadImages();
            initBricks();
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // Create a memory device context compatible with the screen
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, SCREEN_W, SCREEN_H);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            // Clear screen with black on memory DC
            RECT screenRect = { 0, 0, SCREEN_W, SCREEN_H };
            drawRect(memDC, screenRect, 0);

            // Draw the paddle, ball, and bricks to the memory hdc
            if (!GAME_OVER) {
                RECT paddle = { paddleX, paddleY, paddleX + PADDLE_WIDTH, paddleY + PADDLE_HEIGHT };
                RECT ball = { ballX, ballY, ballX + BALL_SIZE, ballY + BALL_SIZE };
                drawRect(memDC, paddle, 1);
                drawRect(memDC, ball, 1);
        
                for (Brick& brick : bricks) {
                    if (!brick.hit) {
                        brick.draw(memDC);
                    }
                }
            }

            // Draw text
            std::string liveCounter;
            std::stringstream ss;
            ss << "Lives: " << lives;
            liveCounter = ss.str();

            HFONT hFont = CreateFont(
                30,                 // nHeight (desired height in logical units)
                0,                  // nWidth (0 for default)
                0,                  // nEscapement (angle of escapement in tenths of a degree)
                0,                  // nOrientation (angle of orientation in tenths of a degree)
                FW_BOLD,            // nWeight (e.g., FW_NORMAL, FW_BOLD)
                FALSE,              // not italic
                FALSE,              // not underlined
                FALSE,              // not struck out
                DEFAULT_CHARSET,    // fdwCharSet
                OUT_DEFAULT_PRECIS, // fdwOutputPrecision
                CLIP_DEFAULT_PRECIS, // fdwClipPrecision
                DEFAULT_QUALITY,    // fdwQuality
                DEFAULT_PITCH | FF_SWISS, // fdwPitchAndFamily
                "Consolas"             // use Consolas font
            );

            if (hFont) {
                HFONT hOldFont = (HFONT)SelectObject(memDC, hFont);
                SetTextColor(memDC, RGB(255, 255, 255));
                SetBkMode(memDC, TRANSPARENT);
                TextOut(memDC, 30, 10, liveCounter.c_str(), liveCounter.length());
                SelectObject(memDC, hOldFont);
                DeleteObject(hFont);
            }

            // Copy the back buffer to the front buffer in one operation
            BitBlt(hdc, 0, 0, SCREEN_W, SCREEN_H, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            if (GAME_OVER) {
                if (winloss == 1) {
                    SendMessageW(hYourdidit, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)yourdidit);
                } else if (winloss == 0) {
                    SendMessageW(hBetterluck, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)betterluck);
                }
            }

            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_KEYDOWN:
            switch (wParam) {
                case VK_LEFT:
                    p_movingLeft = true;
                    break;
                case VK_RIGHT:
                    p_movingRight = true;
                    break;
                case VK_ESCAPE:
                    if (GAME_START) {
                        GAME_START = false;
                        GAME_OVER = false;
                    }
                    break;
            }
            return 0;

        case WM_KEYUP:
            if (!GAME_START && wParam != VK_ESCAPE) {
                GAME_START = TRUE;
            }

            switch (wParam) {
                case VK_LEFT:
                    p_movingLeft = false;
                    break;
                case VK_RIGHT:
                    p_movingRight = false;
                    break;
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WinProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "PaddleGameClass";

    RegisterClassA(&wc);

    // Create the main window
    HWND hWnd = CreateWindowA(
        "PaddleGameClass",
        "Breakout",
        WS_CAPTION | WS_POPUP | WS_SYSMENU,
        50, 50, SCREEN_W, SCREEN_H,
        nullptr, nullptr,
        hInstance, nullptr
    );

    hYourdidit = CreateWindowW(
        L"Static",
        NULL,
        WS_VISIBLE | WS_CHILD | SS_BITMAP,
        SCREEN_W/2 - 235, SCREEN_H/2 - 160, 469, 321,
        hWnd, NULL,
        NULL, NULL
    );

    hBetterluck = CreateWindowW(
        L"Static",
        NULL,
        WS_VISIBLE | WS_CHILD | SS_BITMAP,
        SCREEN_W/2 - 95, SCREEN_H/2 - 87, 190, 164,
        hWnd, NULL,
        NULL, NULL
    );

    ShowWindow(hWnd, nCmdShow);

    // Game loop
    MSG msg;
    std::clock_t lastTime = std::clock();

    for (;;) {
        int bricksLeft = 0;
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        std::clock_t currentTime = std::clock();
        if ((currentTime - lastTime) * 1000 / CLOCKS_PER_SEC >= FRAME_DELAY) {
            if (GAME_START && !GAME_OVER) {
                updatePaddle();
                updateBall();

                for (Brick& brick : bricks) {
                    if (!brick.hit) {
                        bricksLeft++;
                    }
                    if (!brick.hit && brick.isTouchingBall()) {
                        brick.hit = true;

                        // Calculate centers
                        int ballCenterX = ballX + BALL_SIZE / 2;
                        int ballCenterY = ballY + BALL_SIZE / 2;
                        int brickCenterX = brick.x + brick.width / 2;
                        int brickCenterY = brick.y + brick.height / 2;

                        // Calculate the differences
                        int dx = ballCenterX - brickCenterX;
                        int dy = ballCenterY - brickCenterY;

                        // Normalize distances
                        float overlapX = (brick.width + BALL_SIZE) / 2.0f - abs(dx);
                        float overlapY = (brick.height + BALL_SIZE) / 2.0f - abs(dy);

                        // Determine collision axis
                        if (overlapX < overlapY) {
                            // Hit from left or right
                            b_movingLeft = dx < 0;
                            b_movingRight = dx >= 0;
                        } else {
                            // Hit from top or bottom
                            b_movingUp = dy < 0;
                            b_movingDown = dy >= 0;
                        }
                        break;
                    }
                }

                if (bricksLeft == 0) {
                    GAME_OVER = true;
                    winloss = 1;
                }
            }
            InvalidateRect(hWnd, nullptr, TRUE); // Trigger redraw
            lastTime = currentTime;
        }
    }

    return 0;
}
