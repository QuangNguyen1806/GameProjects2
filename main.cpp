/**
* Author: Quang Nguyen
* Assignment: Pong Clone
* Date due: 2025-10-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/cs3113.h"

// ---------- Enums ----------
enum Team { BLUETEAM, REDTEAM };
enum Mode { MULTIPLAYER, SINGLEPLAYER };
enum Direction { UP = 1, DOWN = -1, NEUTRAL = 0 };

// --------- Global Constants ---------
constexpr int SCREEN_WIDTH = 800,
              SCREEN_HEIGHT = 600,
              FPS = 60,
              MAX_BALLS = 3;

constexpr float PADDLE_WIDTH = 15.0f,
                PADDLE_HEIGHT = 120.0f,
                BALL_SIZE = 25.0f,
                PADDLE_SPEED = 400.0f,
                BALL_SPEED = 300.0f,
                PADDLE_OFFSET = 30.0f,
                VERTICAL_MULT = 5;

constexpr char BG_COLOUR[] = "#1a1a1a";
constexpr char LEFT_PLAYER[] = "assets/goalkeeper_blue.png";
constexpr char RIGHT_PLAYER[] = "assets/goalkeeper_red.png";
constexpr char BALL[] = "assets/soccer_ball.png";
constexpr char BG[] = "assets/field.png";
constexpr char BLUEWIN[] = "assets/bluewins.png";
constexpr char REDWIN[] = "assets/redwins.png";

// --------- Global Variables ---------
float gPreviousTicks = 0.0f;

// Left Paddle
Vector2 gLeftPaddlePos = { PADDLE_OFFSET, SCREEN_HEIGHT / 2.0f };
Vector2 gLeftPaddleMovement = { 0.0f, 0.0f };
Vector2 gLeftPaddleScale = { PADDLE_WIDTH, PADDLE_HEIGHT };

// Right Paddle
Vector2 gRightPaddlePos = { SCREEN_WIDTH - PADDLE_OFFSET, SCREEN_HEIGHT / 2.0f };
Vector2 gRightPaddleMovement = { 0.0f, 0.0f };
Vector2 gRightPaddleScale = { PADDLE_WIDTH, PADDLE_HEIGHT };

// Balls
Vector2 gBallPos[MAX_BALLS];
Vector2 gBallVel[MAX_BALLS];
Vector2 gBallScale = { BALL_SIZE, BALL_SIZE };
bool gBallActive[MAX_BALLS] = { true, false, false };

// Textures
Texture2D gLeftPaddleTexture;
Texture2D gRightPaddleTexture;
Texture2D gBallTexture;
Texture2D gBGTexture;
Texture2D gBlueWinScreenTexture;
Texture2D gRedWinScreenTexture;

// Game State
Team gWinner = BLUETEAM;
AppStatus gAppStatus = RUNNING;
Mode gMode = MULTIPLAYER;
Direction gBotDirection = UP;
int gActiveBallCount = 1;

// ---- Function Declarations ----
void initialise();
void processInput();
void update();
void render();
void shutdown();
bool isColliding(const Vector2* positionA, const Vector2* scaleA, 
                 const Vector2* positionB, const Vector2* scaleB);
void renderObject(const Texture2D* texture, const Vector2* position, const Vector2* scale);
void resetBall(int ballIndex);
void activateBalls(int count);

// --------------- Function Definitions ---------------

/**
 * @brief Checks for a square collision between 2 Rectangle objects.
 * 
 * @param positionA The position of the first object
 * @param scaleA The scale of the first object
 * @param positionB The position of the second object
 * @param scaleB The scale of the second object
 * @return true if a collision is detected,
 * @return false if a collision is not detected
 */
bool isColliding(const Vector2* positionA, const Vector2* scaleA, 
                 const Vector2* positionB, const Vector2* scaleB)
{
    float xDistance = fabs(positionA->x - positionB->x) - ((scaleA->x + scaleB->x) / 2.0f);
    float yDistance = fabs(positionA->y - positionB->y) - ((scaleA->y + scaleB->y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void renderObject(const Texture2D* texture, const Vector2* position, const Vector2* scale)
{
    // Whole texture (UV coordinates)
    Rectangle textureArea = {
        0.0f, 0.0f,
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };

    // Destination rectangle – centred on position
    Rectangle destinationArea = {
        position->x,
        position->y,
        static_cast<float>(scale->x),
        static_cast<float>(scale->y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(scale->x) / 2.0f,
        static_cast<float>(scale->y) / 2.0f
    };

    // Render the texture on screen
    DrawTexturePro(*texture, textureArea, destinationArea, originOffset, 0.0f, WHITE);
}

void resetBall(int ballIndex)
{
    gBallPos[ballIndex] = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    
    float directionX = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;
    float directionY = (GetRandomValue(0, 100) / 100.0f - 0.5f) * 2.0f;
    
    gBallVel[ballIndex] = { directionX * BALL_SPEED, directionY * BALL_SPEED };
}

void activateBalls(int count)
{
    if (count < 1) count = 1;
    if (count > MAX_BALLS) count = MAX_BALLS;
    gActiveBallCount = count;
    
    for (int i = 0; i < MAX_BALLS; i++)
    {
        if (i < count)
        {
            if (!gBallActive[i])
            {
                gBallActive[i] = true;
                resetBall(i);
            }
        }
        else
        {
            gBallActive[i] = false;
        }
    }
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong Clone");

    gLeftPaddleTexture = LoadTexture(LEFT_PLAYER);
    gRightPaddleTexture = LoadTexture(RIGHT_PLAYER);
    gBallTexture = LoadTexture(BALL);
    gBGTexture = LoadTexture(BG);
    gBlueWinScreenTexture = LoadTexture(BLUEWIN);
    gRedWinScreenTexture = LoadTexture(REDWIN);

    // Initialize balls
    for (int i = 0; i < MAX_BALLS; i++)
    {
        resetBall(i);
    }

    SetTargetFPS(FPS);
}

void processInput()
{
    // Reset movement without input
    gLeftPaddleMovement = { 0.0f, 0.0f };
    gRightPaddleMovement = { 0.0f, 0.0f };

    // Movement controls
    if (IsKeyDown(KEY_W)) gLeftPaddleMovement.y = -1;
    else if (IsKeyDown(KEY_S)) gLeftPaddleMovement.y = 1;
    
    if (IsKeyDown(KEY_UP)) gRightPaddleMovement.y = -1;
    else if (IsKeyDown(KEY_DOWN)) gRightPaddleMovement.y = 1;

    // Settings
    if (IsKeyPressed(KEY_ONE)) activateBalls(1);
    else if (IsKeyPressed(KEY_TWO)) activateBalls(2);
    else if (IsKeyPressed(KEY_THREE)) activateBalls(3);
    
    if (IsKeyPressed(KEY_T)) gMode = gMode == MULTIPLAYER ? SINGLEPLAYER : MULTIPLAYER;
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    // Delta time
    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    // Update left paddle
    float newLeftY = gLeftPaddlePos.y + PADDLE_SPEED * gLeftPaddleMovement.y * deltaTime;
    if (newLeftY - PADDLE_HEIGHT / 2.0f >= 0 && newLeftY + PADDLE_HEIGHT / 2.0f <= SCREEN_HEIGHT)
    {
        gLeftPaddlePos.y = newLeftY;
    }

    // Update right paddle
    if (gMode == MULTIPLAYER)
    {
        float newRightY = gRightPaddlePos.y + PADDLE_SPEED * gRightPaddleMovement.y * deltaTime;
        if (newRightY - PADDLE_HEIGHT / 2.0f >= 0 && newRightY + PADDLE_HEIGHT / 2.0f <= SCREEN_HEIGHT)
        {
            gRightPaddlePos.y = newRightY;
        }
    }
    else
    {
        // AI: Simple oscillation
        gRightPaddlePos.y += PADDLE_SPEED / 2.0f * gBotDirection * deltaTime;
        
        if (gRightPaddlePos.y - PADDLE_HEIGHT / 2.0f <= 0)
        {
            gRightPaddlePos.y = PADDLE_HEIGHT / 2.0f;
            gBotDirection = UP;
        }
        else if (gRightPaddlePos.y + PADDLE_HEIGHT / 2.0f >= SCREEN_HEIGHT)
        {
            gRightPaddlePos.y = SCREEN_HEIGHT - PADDLE_HEIGHT / 2.0f;
            gBotDirection = DOWN;
        }
    }

    // Update all active balls
    for (int i = 0; i < MAX_BALLS; i++)
    {
        if (!gBallActive[i]) continue;

        Vector2* ball = &gBallPos[i];
        Vector2* vel = &gBallVel[i];

        // Update ball position based on velocity
        ball->x += vel->x * deltaTime;
        ball->y += vel->y * deltaTime;

        // Bounce off top/bottom
        if (ball->y - BALL_SIZE / 2.0f <= 0 || ball->y + BALL_SIZE / 2.0f >= SCREEN_HEIGHT)
        {
            vel->y *= -1.0f;
            if (ball->y - BALL_SIZE / 2.0f < 0)
                ball->y = BALL_SIZE / 2.0f;
            if (ball->y + BALL_SIZE / 2.0f > SCREEN_HEIGHT)
                ball->y = SCREEN_HEIGHT - BALL_SIZE / 2.0f;
        }

        // Check for ball / paddle collisions
        if (isColliding(ball, &gBallScale, &gLeftPaddlePos, &gLeftPaddleScale))
        {
            vel->x = fabs(vel->x);
            vel->y = (ball->y - gLeftPaddlePos.y) * VERTICAL_MULT;
        }

        if (isColliding(ball, &gBallScale, &gRightPaddlePos, &gRightPaddleScale))
        {
            vel->x = -fabs(vel->x);
            vel->y = (ball->y - gRightPaddlePos.y) * VERTICAL_MULT;
        }

        // Ball off screen, change to end screen!
        if (ball->x - BALL_SIZE / 2.0f <= 0)
        {
            gWinner = REDTEAM;
            gAppStatus = TERMINATED;
        }
        else if (ball->x + BALL_SIZE / 2.0f >= SCREEN_WIDTH)
        {
            gWinner = BLUETEAM;
            gAppStatus = TERMINATED;
        }
    }
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    if (gAppStatus == RUNNING)
    {
        // Render background
        Vector2 bgPos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        Vector2 bgScale = { static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) };
        renderObject(&gBGTexture, &bgPos, &bgScale);

        // Render paddles
        renderObject(&gLeftPaddleTexture, &gLeftPaddlePos, &gLeftPaddleScale);
        renderObject(&gRightPaddleTexture, &gRightPaddlePos, &gRightPaddleScale);

        // Render all active balls
        for (int i = 0; i < MAX_BALLS; i++)
        {
            if (gBallActive[i])
            {
                renderObject(&gBallTexture, &gBallPos[i], &gBallScale);
            }
        }
    }
    else if (gAppStatus == TERMINATED)
    {
        // Render win screen
        Vector2 screenPos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        Vector2 screenScale = { static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) };
        
        if (gWinner == REDTEAM)
            renderObject(&gRedWinScreenTexture, &screenPos, &screenScale);
        else
            renderObject(&gBlueWinScreenTexture, &screenPos, &screenScale);
    }

    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gLeftPaddleTexture);
    UnloadTexture(gRightPaddleTexture);
    UnloadTexture(gBallTexture);
    UnloadTexture(gBGTexture);
    UnloadTexture(gBlueWinScreenTexture);
    UnloadTexture(gRedWinScreenTexture);

    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    // Show end screen
    while (!WindowShouldClose())
    {
        render();
    }

    shutdown();

    return 0;
}
