#include <Windows.h>
#include <ctime>
#include <vector>
#include <sstream>
#include <string>

// prototype function
void drawRect(HDC hdc, RECT rect, int color);
void loadImages();

// Constants for screen and paddle settings
// screen size is fixed to 720p
constexpr int SCREEN_H = 720;       // fixed screen height of 720 pixels
constexpr int SCREEN_W = 1280;      // fixed screen width of 1280 pixels
constexpr int PADDLE_WIDTH = 125;   // fixed paddle width of 125 pixels
constexpr int PADDLE_HEIGHT = 25;   // fixed paddle height of 25 pixels
constexpr int BALL_SIZE = 15;       // fixed ball size of 15 pixels, ball is square
constexpr int STEP = 8;             // paddle movement step size
constexpr int B_STEP = 4;           // ball movement step size
constexpr int FRAME_DELAY = 16;     // frame delay in milliseconds (60 FPS)
bool GAME_START = false;            // game stating state (used for pausing)
bool GAME_OVER = false;             // game over state (used for game end situations)
int lives = 3;                      // player lives
int winloss = -1;                   // -1 : game still going, 0 : game lost, 1 : game won

// Paddle position and movement states
int paddleX = (SCREEN_W / 2) - (PADDLE_WIDTH / 2); // draw the paddle in the center of the screen
int paddleY = SCREEN_H - 100;                      // ... 100 pixels from the bottom of the screen
bool p_movingLeft = false;                         // paddle movement states are initialized to false (paddle is still)
bool p_movingRight = false;

// Ball position and movement states
int ballX = (SCREEN_W / 2) - (BALL_SIZE / 2);      // draw the ball in the center of the screen
int ballY = SCREEN_H - 300;                        // ... 300 pixels from the bottom of the screen
bool b_movingLeft = false;                         // ball movement states are initialized to false (mostly)
bool b_movingRight = false;
bool b_movingUp = true;                            // ball starts moving upwards
bool b_movingDown = false;

// Brick params
// constants for creating the bricks at the top of the screen
// each brick is 125x30, there are 8 bricks in each row and 5 rows of bricks
constexpr int BRICK_WIDTH = 125;
constexpr int BRICK_HEIGHT = 30;
constexpr int BRICK_COLUMNS = 8;
constexpr int BRICK_ROWS = 5;
constexpr int BRICK_SPACING = 31;

// Brick class
class Brick {
    public:
    int x, y;           // x and y position
    int width, height;  // width and height
    bool hit;           // hit state (true if the brick has been hit by the ball)
    int color;          // color (see color mapping below)
    RECT rect;          // RECT object

    // constructor
    Brick(int xPos, int yPos, int w, int h, int c) {
        x = xPos; y = yPos;
        width = w; height = h;
        hit = false; // brick is not hit at the start
        rect = { x, y, x + width, y + height };
        color = c;
    }

    // draw function, draw the brick on the screen if it hasn't been hit
    void draw(HDC hdc) const {
        if (!hit) {
            drawRect(hdc, rect, color);
        }
    }

    // ball collision function
    bool isTouchingBall() const {
        return !(ballX + BALL_SIZE < x ||
            ballX > x + width ||
            ballY + BALL_SIZE < y ||
            ballY > y + height);
    }
};

// Update paddle position based on user input
void updatePaddle() {
    // if the paddle is moving left and the paddle is not at the left edge of the screen, move it left
    if (p_movingLeft && paddleX > 0) {
        paddleX -= STEP;
    }
    // if the paddle is moving right and the paddle is not at the right edge of the screen, move it right
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

    // Ball movement logic, bounces off walls and the paddle
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
    // create brush object
    HBRUSH brush = nullptr;

    // color mapping
    switch (color) {
        case 0: // 0 : Black
            brush = CreateSolidBrush(RGB(0, 0, 0));
            break;
        case 1: // 1 : White
            brush = CreateSolidBrush(RGB(255, 255, 255));
            break;
        case 2: // 2 : Red
            brush = CreateSolidBrush(RGB(255, 0, 0));
            break;
        case 3: // 3 : Orange
            brush = CreateSolidBrush(RGB(255, 165, 0));
            break;
        case 4: // 4 : Yellow
            brush = CreateSolidBrush(RGB(255, 255, 0));
            break;
        case 5: // 5 : Green
            brush = CreateSolidBrush(RGB(0, 128, 0));
            break;
        case 6: // 6 : Blue
            brush = CreateSolidBrush(RGB(0, 0, 255));
            break;
    }

    // Draw the rectangle with the specified color
    if (brush) {
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }
}

// win/loss image handles
HBITMAP yourdidit = nullptr;
HBITMAP betterluck = nullptr;

// defined the protoype function "loadImages"
void loadImages() {
    // load the images and casst them to an HBITMAP object for all images
    yourdidit = (HBITMAP)LoadImageW(NULL, L"bmps\\yourdidit.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    betterluck = (HBITMAP)LoadImageW(NULL, L"bmps\\betterluck.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}


// Window procedure for handling messages
LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // handles for double buffering the bitmaps
    static HDC yourdiditMemDC = nullptr;
    static HBITMAP yourdiditMemBitmap = nullptr;
    static HDC betterluckMemDC = nullptr;
    static HBITMAP betterluckMemBitmap = nullptr;

    // window message switch case
    switch (uMsg) {
        // when the window is created
        case WM_CREATE:
            loadImages();  // load images
            initBricks();  // initialize bricks

            // Create memory DCs and bitmaps for the images
            yourdiditMemDC = CreateCompatibleDC(GetDC(hWnd));
            yourdiditMemBitmap = CreateCompatibleBitmap(GetDC(hWnd), 469, 321);
            SelectObject(yourdiditMemDC, yourdiditMemBitmap);
            {
                HDC yourdiditTempDC = CreateCompatibleDC(GetDC(hWnd));
                SelectObject(yourdiditTempDC, yourdidit);
                BitBlt(yourdiditMemDC, 0, 0, 469, 321, yourdiditTempDC, 0, 0, SRCCOPY);
                DeleteDC(yourdiditTempDC);
            }
            betterluckMemDC = CreateCompatibleDC(GetDC(hWnd));
            betterluckMemBitmap = CreateCompatibleBitmap(GetDC(hWnd), 190, 164);
            SelectObject(betterluckMemDC, betterluckMemBitmap);
            {
                HDC betterluckTempDC = CreateCompatibleDC(GetDC(hWnd));
                SelectObject(betterluckTempDC, betterluck);
                BitBlt(betterluckMemDC, 0, 0, 190, 164, betterluckTempDC, 0, 0, SRCCOPY);
                DeleteDC(betterluckTempDC);
            }

            break;
        // when stuff is drawn on the window
        case WM_PAINT:
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

            // write the text "Lives: {lives}" to the screen
            // use the font created above
            if (hFont) {
                HFONT hOldFont = (HFONT)SelectObject(memDC, hFont);
                SetTextColor(memDC, RGB(255, 255, 255));
                SetBkMode(memDC, TRANSPARENT);
                TextOut(memDC, 30, 10, liveCounter.c_str(), liveCounter.length());
                SelectObject(memDC, hOldFont);
                DeleteObject(hFont);
            }

            // Draw game over screen if the game is over
            // different images used for different game states
            if (GAME_OVER) {
                if (winloss == 0) {
                    BitBlt(memDC, (SCREEN_W - 190) / 2, (SCREEN_H - 164) / 2, 190, 164, betterluckMemDC, 0, 0, SRCCOPY);
                } else if (winloss == 1) {
                    BitBlt(memDC, (SCREEN_W - 469) / 2, (SCREEN_H - 321) / 2, 469, 321, yourdiditMemDC, 0, 0, SRCCOPY);
                }
            }

            // Copy the back buffer to the front buffer
            BitBlt(hdc, 0, 0, SCREEN_W, SCREEN_H, memDC, 0, 0, SRCCOPY);

            // delete the memory DC and bitmap to protect against memory leaks
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hWnd, &ps);
            return 0;
        
        // when a key is pressed down
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_LEFT:   // left arrow key
                    p_movingLeft = true;
                    break;
                case VK_RIGHT:  // right arrow key
                    p_movingRight = true;
                    break;
                case VK_ESCAPE: // escape key (used for pausing the game)
                    if (GAME_START) {
                        GAME_START = false;
                        GAME_OVER = false;
                    }
                    break;
            }
            return 0;
        // when a key is released
        case WM_KEYUP:
            // this will start the game when a key is released for the first time
            // this makes sure the key isn't the escape key, otherwise it would never pause
            if (!GAME_START && wParam != VK_ESCAPE) {
                GAME_START = TRUE;
            }

            switch (wParam) {
                case VK_LEFT:  // left arrow key
                    p_movingLeft = false;
                    break;
                case VK_RIGHT: // right arrow key
                    p_movingRight = false;
                    break;
            }
            return 0;

        // when the window is closed
        case WM_DESTROY:
            // delete objects
            if (yourdiditMemDC) {
                DeleteDC(yourdiditMemDC);
            }
            if (yourdiditMemBitmap) {
                DeleteObject(yourdiditMemBitmap);
            }
            if (betterluckMemDC) {
                DeleteDC(betterluckMemDC);
            }
            if (betterluckMemBitmap) {
                DeleteObject(betterluckMemBitmap);
            }
            PostQuitMessage(0);
            return 0;
        // default case for unhandled windows messages
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

    // show the window
    ShowWindow(hWnd, nCmdShow);

    // Game loop
    MSG msg;
    // clock object used for timing
    std::clock_t lastTime = std::clock();

    // infinite loop for gameplay, user will close window when done
    for (;;) {
        int bricksLeft = 0; // bricksLeft var used for detecting if the user has beat the game
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        std::clock_t currentTime = std::clock(); // get current time
        // check if the time since the last frame is greater than the frame delay
        // if so, update the game state and redraw the screen
        // this is used to cap the frame and update rate to 60 FPS
        if ((currentTime - lastTime) * 1000 / CLOCKS_PER_SEC >= FRAME_DELAY) {
            if (GAME_START && !GAME_OVER) {
                // update the ball and paddle
                updatePaddle();
                updateBall();
                
                // check each bricks colision with the ball and if it exists or not
                for (Brick& brick : bricks) {
                    if (!brick.hit) {
                        bricksLeft++; // inc bricksLeft if this brick has not been hit
                    }
                    if (!brick.hit && brick.isTouchingBall()) {
                        // this code executes if the ball hits the brick
                        brick.hit = true;

                        // Calculate centers, used for determining how the ball should behave when bouncing off the brick
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
                            // Hit from left or right, bounce on the x-axis
                            b_movingLeft = dx < 0;
                            b_movingRight = dx >= 0;
                        } else {
                            // Hit from top or bottom, bounce on the y-axis
                            b_movingUp = dy < 0;
                            b_movingDown = dy >= 0;
                        }
                        break;
                    }
                }
                
                // if no bricks were left (all have been hit), end the game
                if (bricksLeft == 0) {
                    GAME_OVER = true;
                    winloss = 1;
                }
            }

            // Trigger redraw
            InvalidateRect(hWnd, nullptr, TRUE);
            lastTime = currentTime;
        }
    }
    return 0;
}
