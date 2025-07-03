#include <windows.h>
#include <windowsx.h>  // 添加这个头文件以解决 GET_X_LPARAM 问题
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>
#include <sstream>

// 游戏常量
const int GRID_SIZE = 20;       // 网格大小（像素）
const int GRID_WIDTH = 30;      // 网格宽度（格子数）
const int GRID_HEIGHT = 20;     // 网格高度（格子数）
const int SNAKE_SPEED = 150;    // 初始蛇速（毫秒）
const int SPEED_INCREASE = 5;   // 每吃一个食物增加的速度

// 颜色常量
const COLORREF BG_COLOR = RGB(10, 20, 30);
const COLORREF GRID_COLOR = RGB(40, 50, 70);
const COLORREF SNAKE_COLOR = RGB(50, 180, 100);
const COLORREF HEAD_COLOR = RGB(70, 220, 120);
const COLORREF FOOD_COLOR = RGB(220, 80, 60);
const COLORREF WALL_COLOR = RGB(70, 100, 150);
const COLORREF TEXT_COLOR = RGB(200, 220, 255);
const COLORREF BUTTON_COLOR = RGB(80, 120, 200);
const COLORREF BUTTON_HOVER_COLOR = RGB(100, 150, 230);

// 方向枚举
enum Direction { UP, DOWN, LEFT, RIGHT };

// 游戏状态
struct GameState {
    bool gameOver;
    bool gamePaused;
    int score;
    int highScore;
    int speed;
    Direction dir;
    Direction nextDir;

    // 蛇的位置
    struct Position {
        int x, y;
        Position(int x = 0, int y = 0) : x(x), y(y) {}
    };

    Position head;
    std::vector<Position> body;
    Position food;

    // 按钮状态
    bool restartHover;
    bool pauseHover;

    GameState() : gameOver(false), gamePaused(false), score(0), highScore(0),
        speed(SNAKE_SPEED), dir(RIGHT), nextDir(RIGHT),
        restartHover(false), pauseHover(false) {
        resetGame();
    }

    void resetGame() {
        gameOver = false;
        gamePaused = false;  // 添加这行确保重新开始时游戏不会暂停
        score = 0;
        speed = SNAKE_SPEED;
        dir = RIGHT;
        nextDir = RIGHT;

        // 初始化蛇的位置
        head = Position(GRID_WIDTH / 2, GRID_HEIGHT / 2);
        body.clear();
        body.push_back(Position(head.x - 1, head.y));
        body.push_back(Position(head.x - 2, head.y));

        // 放置食物
        placeFood();
    }

    void placeFood() {
        bool valid;
        do {
            valid = true;
            food.x = rand() % GRID_WIDTH;
            food.y = rand() % GRID_HEIGHT;

            // 检查食物是否在蛇头上
            if (food.x == head.x && food.y == head.y)
                valid = false;

            // 检查是否在蛇身上
            for (const auto& segment : body) {
                if (food.x == segment.x && food.y == segment.y) {
                    valid = false;
                    break;
                }
            }
        } while (!valid);
    }

    void move() {
        if (gameOver || gamePaused) return;

        // 更新方向
        dir = nextDir;

        // 移动身体
        if (!body.empty()) {
            for (int i = body.size() - 1; i > 0; i--) {
                body[i] = body[i - 1];
            }
            body[0] = head;
        }

        // 移动头部
        switch (dir) {
        case UP:    head.y--; break;
        case DOWN:  head.y++; break;
        case LEFT:  head.x--; break;
        case RIGHT: head.x++; break;
        }

        // 检查碰撞
        checkCollision();
    }

    void checkCollision() {
        // 检查撞墙
        if (head.x < 0 || head.x >= GRID_WIDTH || head.y < 0 || head.y >= GRID_HEIGHT) {
            gameOver = true;
            return;
        }

        // 检查撞到自己
        for (const auto& segment : body) {
            if (head.x == segment.x && head.y == segment.y) {
                gameOver = true;
                return;
            }
        }

        // 检查是否吃到食物
        if (head.x == food.x && head.y == food.y) {
            // 增加分数
            score += 10;
            if (score > highScore) {
                highScore = score;
            }

            // 增加蛇的长度
            if (body.empty()) {
                body.push_back(head);
            }
            else {
                body.push_back(body.back());
            }

            // 增加速度
            if (speed > 50) {
                speed -= SPEED_INCREASE;
                // 更新定时器速度
                KillTimer(GetActiveWindow(), 1);
                SetTimer(GetActiveWindow(), 1, speed, NULL);
            }

            // 放置新食物
            placeFood();
        }
    }
};

// 全局游戏状态
GameState gameState;

// 绘制圆形函数
void DrawCircle(HDC hdc, int x, int y, int radius, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, pen);

    Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

// 绘制圆角矩形
void DrawRoundRect(HDC hdc, int x, int y, int width, int height, int radius, COLORREF fillColor, COLORREF borderColor) {
    HBRUSH brush = CreateSolidBrush(fillColor);
    HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, pen);

    RoundRect(hdc, x, y, x + width, y + height, radius, radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

// 绘制按钮
void DrawButton(HDC hdc, int x, int y, int width, int height, const wchar_t* text, bool hover) {
    COLORREF bgColor = hover ? BUTTON_HOVER_COLOR : BUTTON_COLOR;
    DrawRoundRect(hdc, x, y, width, height, 10, bgColor, RGB(100, 140, 220));

    // 设置文本属性
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, TEXT_COLOR);

    // 居中文本
    RECT rect = { x, y, x + width, y + height };
    HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, L"Arial");
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

    DrawTextW(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}

// 绘制游戏
void DrawGame(HWND hwnd, HDC hdc) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // 创建双缓冲
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(memDC, memBitmap);

    // 绘制背景
    HBRUSH bgBrush = CreateSolidBrush(BG_COLOR);
    FillRect(memDC, &rect, bgBrush);
    DeleteObject(bgBrush);

    // 计算游戏区域位置（居中）
    int gameWidth = GRID_WIDTH * GRID_SIZE;
    int gameHeight = GRID_HEIGHT * GRID_SIZE;
    int gameX = (width - gameWidth) / 2;
    int gameY = (height - gameHeight) / 2 + 20;

    // 绘制游戏区域背景
    RECT gameRect = { gameX - 10, gameY - 10, gameX + gameWidth + 10, gameY + gameHeight + 10 };
    HBRUSH gameBgBrush = CreateSolidBrush(WALL_COLOR);
    FillRect(memDC, &gameRect, gameBgBrush);
    DeleteObject(gameBgBrush);

    // 绘制网格
    HPEN gridPen = CreatePen(PS_SOLID, 1, GRID_COLOR);
    SelectObject(memDC, gridPen);

    for (int x = 0; x <= GRID_WIDTH; x++) {
        MoveToEx(memDC, gameX + x * GRID_SIZE, gameY, NULL);
        LineTo(memDC, gameX + x * GRID_SIZE, gameY + gameHeight);
    }

    for (int y = 0; y <= GRID_HEIGHT; y++) {
        MoveToEx(memDC, gameX, gameY + y * GRID_SIZE, NULL);
        LineTo(memDC, gameX + gameWidth, gameY + y * GRID_SIZE);
    }

    DeleteObject(gridPen);

    // 绘制食物
    int foodX = gameX + gameState.food.x * GRID_SIZE + GRID_SIZE / 2;
    int foodY = gameY + gameState.food.y * GRID_SIZE + GRID_SIZE / 2;
    DrawCircle(memDC, foodX, foodY, GRID_SIZE / 2 - 2, FOOD_COLOR);

    // 绘制蛇头
    int headX = gameX + gameState.head.x * GRID_SIZE + GRID_SIZE / 2;
    int headY = gameY + gameState.head.y * GRID_SIZE + GRID_SIZE / 2;
    DrawCircle(memDC, headX, headY, GRID_SIZE / 2 - 2, HEAD_COLOR);

    // 绘制蛇眼睛
    int eyeOffset = GRID_SIZE / 4;
    int eyeX = headX;
    int eyeY = headY;

    switch (gameState.dir) {
    case UP:    eyeX -= eyeOffset; eyeY -= eyeOffset; break;
    case DOWN:  eyeX -= eyeOffset; eyeY += eyeOffset; break;
    case LEFT:  eyeX -= eyeOffset; break;
    case RIGHT: eyeX += eyeOffset; break;
    }

    DrawCircle(memDC, eyeX, eyeY, GRID_SIZE / 8, RGB(25
        5, 255, 255));

    // 绘制蛇身
    for (const auto& segment : gameState.body) {
        int bodyX = gameX + segment.x * GRID_SIZE + GRID_SIZE / 2;
        int bodyY = gameY + segment.y * GRID_SIZE + GRID_SIZE / 2;
        DrawCircle(memDC, bodyX, bodyY, GRID_SIZE / 2 - 2, SNAKE_COLOR);
    }

    // 绘制分数
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, TEXT_COLOR);

    HFONT hFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, L"Arial");
    HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

    std::wstringstream ssScore;
    ssScore << L"得分: " << gameState.score;
    std::wstring scoreText = ssScore.str();
    TextOutW(memDC, 20, 20, scoreText.c_str(), static_cast<int>(scoreText.length()));

    std::wstringstream ssHighScore;
    ssHighScore << L"最高分: " << gameState.highScore;
    std::wstring highScoreText = ssHighScore.str();
    TextOutW(memDC, 20, 60, highScoreText.c_str(), static_cast<int>(highScoreText.length()));

    SelectObject(memDC, oldFont);
    DeleteObject(hFont);

    // 绘制按钮
    int buttonWidth = 120;
    int buttonHeight = 40;
    int buttonX = width - buttonWidth - 20;
    int buttonY = 20;

    DrawButton(memDC, buttonX, buttonY, buttonWidth, buttonHeight,
        gameState.gamePaused ? L"继续游戏" : L"暂停游戏", gameState.pauseHover);

    buttonY += buttonHeight + 10;
    DrawButton(memDC, buttonX, buttonY, buttonWidth, buttonHeight, L"重新开始", gameState.restartHover);

    // 绘制游戏状态
    if (gameState.gameOver) {
        hFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            VARIABLE_PITCH, L"Arial");
        oldFont = (HFONT)SelectObject(memDC, hFont);

        SetTextColor(memDC, RGB(220, 80, 80));
        std::wstring gameOverText = L"游戏结束!";
        int textWidth = static_cast<int>(gameOverText.length()) * 24;
        TextOutW(memDC, (width - textWidth) / 2, height / 2 - 60, gameOverText.c_str(), static_cast<int>(gameOverText.length()));

        SelectObject(memDC, oldFont);
        DeleteObject(hFont);

        hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            VARIABLE_PITCH, L"Arial");
        oldFont = (HFONT)SelectObject(memDC, hFont);

        SetTextColor(memDC, TEXT_COLOR);
        std::wstring restartText = L"按空格键重新开始";
        textWidth = static_cast<int>(restartText.length()) * 12;
        TextOutW(memDC, (width - textWidth) / 2, height / 2, restartText.c_str(), static_cast<int>(restartText.length()));

        SelectObject(memDC, oldFont);
        DeleteObject(hFont);
    }
    else if (gameState.gamePaused) {
        hFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            VARIABLE_PITCH, L"Arial");
        oldFont = (HFONT)SelectObject(memDC, hFont);

        SetTextColor(memDC, RGB(80, 180, 220));
        std::wstring pausedText = L"游戏暂停";
        int textWidth = static_cast<int>(pausedText.length()) * 24;
        TextOutW(memDC, (width - textWidth) / 2, height / 2 - 30, pausedText.c_str(), static_cast<int>(pausedText.length()));

        SelectObject(memDC, oldFont);
        DeleteObject(hFont);
    }

    // 绘制标题
    hFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, L"Arial");
    oldFont = (HFONT)SelectObject(memDC, hFont);

    SetTextColor(memDC, RGB(100, 200, 255));
    std::wstring title = L"贪吃蛇游戏";
    int titleWidth = static_cast<int>(title.length()) * 18;
    TextOutW(memDC, (width - titleWidth) / 2, 10, title.c_str(), static_cast<int>(title.length()));

    SelectObject(memDC, oldFont);
    DeleteObject(hFont);

    // 绘制操作说明
    hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, L"Arial");
    oldFont = (HFONT)SelectObject(memDC, hFont);

    SetTextColor(memDC, RGB(150, 180, 220));
    std::wstring controls = L"方向键: 控制移动  空格键: 暂停/继续   R: 重新开始";
    int controlsWidth = static_cast<int>(controls.length()) * 8;
    TextOutW(memDC, (width - controlsWidth) / 2, height - 30, controls.c_str(), static_cast<int>(controls.length()));

    SelectObject(memDC, oldFont);
    DeleteObject(hFont);

    // 复制到屏幕
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // 清理
    DeleteDC(memDC);
    DeleteObject(memBitmap);
}

// 窗口过程
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawGame(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN: {
        if (wParam == VK_LEFT && gameState.dir != RIGHT) {
            gameState.nextDir = LEFT;
        }
        else if (wParam == VK_RIGHT && gameState.dir != LEFT) {
            gameState.nextDir = RIGHT;
        }
        else if (wParam == VK_UP && gameState.dir != DOWN) {
            gameState.nextDir = UP;
        }
        else if (wParam == VK_DOWN && gameState.dir != UP) {
            gameState.nextDir = DOWN;
        }
        else if (wParam == VK_SPACE) {
            if (gameState.gameOver) {
                gameState.resetGame();
            }
            else {
                gameState.gamePaused = !gameState.gamePaused;
            }
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (wParam == 'R' || wParam == 'r') {
            gameState.resetGame();
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_MOUSEMOVE: {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        RECT rect;
        GetClientRect(hwnd, &rect);
        int width = rect.right - rect.left;

        // 按钮位置
        int buttonWidth = 120;
        int buttonHeight = 40;
        int buttonX = width - buttonWidth - 20;
        int pauseY = 20;
        int restartY = 70;

        // 检查鼠标是否在按钮上
        bool prevPauseHover = gameState.pauseHover;
        bool prevRestartHover = gameState.restartHover;

        gameState.pauseHover = (xPos >= buttonX && xPos <= buttonX + buttonWidth &&
            yPos >= pauseY && yPos <= pauseY + buttonHeight);

        gameState.restartHover = (xPos >= buttonX && xPos <= buttonX + buttonWidth &&
            yPos >= restartY && yPos <= restartY + buttonHeight);

        // 如果悬停状态改变，重绘
        if (prevPauseHover != gameState.pauseHover || prevRestartHover != gameState.restartHover) {
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        RECT rect;
        GetClientRect(hwnd, &rect);
        int width = rect.right - rect.left;

        // 按钮位置
        int buttonWidth = 120;
        int buttonHeight = 40;
        int buttonX = width - buttonWidth - 20;
        int pauseY = 20;
        int restartY = 70;

        // 检查是否点击了暂停按钮
        if (xPos >= buttonX && xPos <= buttonX + buttonWidth &&
            yPos >= pauseY && yPos <= pauseY + buttonHeight) {
            gameState.gamePaused = !gameState.gamePaused;
            InvalidateRect(hwnd, NULL, TRUE);
        }
        // 检查是否点击了重新开始按钮
        else if (xPos >= buttonX && xPos <= buttonX + buttonWidth &&
            yPos >= restartY && yPos <= restartY + buttonHeight) {
            gameState.resetGame();
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_TIMER:
        if (!gameState.gamePaused) {
            gameState.move();
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;

    case WM_CLOSE:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// 主函数
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // 注册窗口类
    const wchar_t CLASS_NAME[] = L"SnakeGameWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // 计算窗口大小
    int windowWidth = GRID_WIDTH * GRID_SIZE + 200;
    int windowHeight = GRID_HEIGHT * GRID_SIZE + 200;

    // 创建窗口
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"贪吃蛇游戏",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowWidth, windowHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    // 初始化随机数生成器
    srand(static_cast<unsigned int>(time(NULL)));

    // 设置初始游戏状态
    gameState.resetGame();

    // 创建定时器
    SetTimer(hwnd, 1, gameState.speed, NULL);

    // 显示窗口
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}