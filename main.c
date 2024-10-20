#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glut.h>  // freeglut.h might be a better alternative, if available.
#include "queue.h"


#define MS_PER_FRAME 16  // 16 milliseconds per frame for ~60 FPS

#define BOARD_START 100
#define BOARD_WIDTH 800
#define CLOCK_MONOTONIC 0

void levelUp();

typedef struct
{

} Board;

typedef struct
{
    int x, y;
    float dx, dy;
    float step_size;
    int level;
    Queue* body;
} Snake;

typedef struct
{
    int x, y;
} SegmentPosition;

typedef struct
{
    int x, y;
} Food;

typedef struct
{
    Snake snake;
    Food food;
    bool paused;
    bool gameover;
    bool started;
    int tick_rate;
    int score;
} GameState;

GameState state;

int clock_gettime(int clock_id, struct timespec *spec) {
    static LARGE_INTEGER frequency;
    static BOOL initialized = FALSE;
    LARGE_INTEGER count;

    if (!initialized) {
        QueryPerformanceFrequency(&frequency);  // Get ticks per second
        initialized = TRUE;
    }

    QueryPerformanceCounter(&count);  // Get current tick count

    // Convert ticks to seconds and nanoseconds
    spec->tv_sec = count.QuadPart / frequency.QuadPart;
    spec->tv_nsec = (long)((count.QuadPart % frequency.QuadPart) * 1e9 / frequency.QuadPart);

    return 0;
}

double getCurrentTime() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}

void renderBitmapString(float x, float y, void *font, const char *string) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);  // Set the position for the text

    while (*string) {
        glutBitmapCharacter(font, *string);  // Render each character
        string++;
    }
}

bool check_overlap(double x1, double y1, double s1, double x2, double y2, double s2) {
    // Calculate the right and top edges of both squares
    double right1 = x1 + s1;  // Right edge of square 1
    double top1 = y1 + s1;    // Top edge of square 1
    double right2 = x2 + s2;  // Right edge of square 2
    double top2 = y2 + s2;    // Top edge of square 2

    // Check for no overlap condition
    if (x1 >= right2 || x2 >= right1 || y1 >= top2 || y2 >= top1) {
        return false;  // No overlap
    }

    return true;  // Overlap
}

bool checkWallCollision()
{
    Node* node = state.snake.body->tail;

    SegmentPosition* segmentPosition = node->data;
    if(segmentPosition->x >= BOARD_START + BOARD_WIDTH - 10 || segmentPosition->x <= BOARD_START || segmentPosition->y >= BOARD_START + BOARD_WIDTH - 12 || segmentPosition->y <= BOARD_START)
    {
        return true;
    }

    return false;
}

bool checkSnakeCollision()
{
    Node* node = state.snake.body->tail;

    SegmentPosition* tail = node->data;

    Node* snakeNode = state.snake.body->head;

    while (snakeNode != NULL)
    {
        if(snakeNode->next == NULL)
        {
            return false;
        }

        SegmentPosition* segmentPosition = snakeNode->data;

        if(check_overlap(segmentPosition->x, segmentPosition->y, 10, tail->x, tail->y, 10))
        {
            return true;
        }

        // if(tail->x == segmentPosition->x && tail->y == segmentPosition->y)
        // {
        //     return true;
        // }

        snakeNode = snakeNode->next;
    }

    return false;
}

bool checkFoodCollision()
{
    Node* node = state.snake.body->tail;
    SegmentPosition* segment = node->data;

    return check_overlap(segment->x, segment->y, 10, state.food.x, state.food.y, 10);
}

int random(int min, int max)
{
    return rand() % (max + 1 - min) + min;
}

void spawnFood()
{
    state.food.x = random(BOARD_START + 10, BOARD_WIDTH + BOARD_START - 10);
    state.food.y = random(BOARD_START + 10, BOARD_WIDTH + BOARD_START - 10);
}

bool checkCollision()
{
    return checkWallCollision() || checkSnakeCollision();
}

void updateGame() {
    if(!state.started)
    {
        return;
    }

    if(state.paused)
    {
        return;
    }

    if(checkCollision())
    {
        state.gameover = true;
        // glutPostRedisplay();
        return;
    }

    if(checkFoodCollision())
    {
        levelUp();
        spawnFood();
    }

    state.snake.x = state.snake.x + state.snake.dx;
    state.snake.y = state.snake.y + state.snake.dy;

    SegmentPosition* segment = (SegmentPosition*)malloc(sizeof(SegmentPosition));
    segment->x = state.snake.x;
    segment->y = state.snake.y;

    pushQueue(state.snake.body, segment);
    popQueue(state.snake.body);
}

void renderFood()
{
    glBegin(GL_QUADS);
    glColor3f(0.0, 1.0, 0.0);
    glVertex2f(state.food.x, state.food.y);
    glVertex2f(state.food.x + 10, state.food.y);
    glVertex2f(state.food.x + 10, state.food.y + 10);
    glVertex2f(state.food.x, state.food.y+ 10);
    glEnd();
}

void renderSnake()
{
    glBegin(GL_QUADS);
    Node* node = state.snake.body->head;

    while (node != NULL)
    {
        SegmentPosition *segment = (SegmentPosition*)node->data;
        glColor3f(1.0, 0.0, 0.0);
        glVertex2f(segment->x, segment->y);
        glVertex2f(segment->x + 10, segment->y);
        glVertex2f(segment->x + 10, segment->y + 10);
        glVertex2f(segment->x, segment->y + 10);
        node = node->next;
    }

    glEnd();
}

void renderWalls()
{
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glColor3f(1.0, 1.0, 1.0);

    glVertex2d(100, 100);
    glVertex2d(900, 100);

    glVertex2d(900, 100);
    glVertex2d(900, 900);

    glVertex2d(900, 900);
    glVertex2d(100, 900);

    glVertex2d(100, 900);
    glVertex2d(100, 100);
    glEnd();
}

void render() {
    // This function is called to render graphics
    glClear(GL_COLOR_BUFFER_BIT);

    char* score = (char*)malloc(sizeof(char) * 20);
    sprintf(score, "Score: %d", state.score);
    renderBitmapString(10, 50, GLUT_BITMAP_HELVETICA_12, "SPACE to start");
    renderBitmapString(10, 70, GLUT_BITMAP_HELVETICA_12, "R to restart");
    renderBitmapString(900, 50, GLUT_BITMAP_HELVETICA_18, score);

    renderWalls();
    renderSnake();
    renderFood();
    if(state.gameover)
    {
        renderBitmapString(500, 50, GLUT_BITMAP_HELVETICA_18, "GAME OVER");

    }
    glutSwapBuffers(); // Swap the front and back buffers
}

void display() {
    // Call render to draw the frame
    render();
}

void timer(int value) {
    static double previous = 0.0;
    static double lag = 0.0;
    static double tickLag = 0.0;

    double current = getCurrentTime();
    double elapsed = current - previous;
    previous = current;
    lag += elapsed;
    tickLag += elapsed;

    while (tickLag >= state.tick_rate) {
        updateGame();
        tickLag -= state.tick_rate;
    }

    display();  // Call the display function to render the updated frame

    // Call the timer function again after the defined interval
    glutTimerFunc(MS_PER_FRAME, timer, 0);  // 16 ms for ~60 FPS
}

void initGameState(GameState* state)
{
    if(state == NULL)
    {
        return;
    }

    state->score = 0;
    state->snake.dx = state->snake.step_size;
    state->snake.dy = 0;
    state->tick_rate = 80; // every 100ms
    state->paused = false;
    state->gameover = false;
    state->snake.x = 500;
    state->snake.y = 500;
    state->snake.step_size = 10;
    state->snake.level = 5;
    state->snake.body = initQueue();

    SegmentPosition* segment = malloc(sizeof(SegmentPosition));

    state->snake.dx = state->snake.step_size;
    segment->x = state->snake.x;
    segment->y = state->snake.y;

    pushQueue(state->snake.body, segment);

    spawnFood();
}

void restart()
{
    initGameState(&state);
}

void levelUp()
{
    SegmentPosition* segment = malloc(sizeof(SegmentPosition));

    SegmentPosition* tail = state.snake.body->tail->data;

    if(state.snake.body->head->next == NULL)
    {
        segment->x = tail->x + -state.snake.dx;
        segment->y = tail->y + -state.snake.dy;
    } else
    {
        segment->x = tail->x;
        segment->y = tail->y;
    }

    pushQueue(state.snake.body, segment);

    state.tick_rate -= 3;
    state.score += 10;
}

void pause()
{
    state.paused = !state.paused;
}

void start()
{
    state.started = true;
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': // Move up
        if(state.snake.dy > 0)
        {
            return;
        }
        state.snake.dx = 0;
        state.snake.dy = -state.snake.step_size;
        break;
    case 's': // Move down
        if(state.snake.dy < 0)
        {
            return;
        }
        state.snake.dx = 0;
        state.snake.dy = state.snake.step_size;
        break;
    case 'a': // Move left
        if(state.snake.dx > 0)
        {
            return;
        }
        state.snake.dx = -state.snake.step_size;
        state.snake.dy = 0;
        break;
    case 'd': // Move right
        if(state.snake.dx < 0)
        {
            return;
        }
        state.snake.dx = state.snake.step_size;
        state.snake.dy = 0;
        break;
    case 'x':
        // levelUp();
        break;
    case 'p':
        pause();
        break;
    case 'r':
        restart();
        break;
    case 32:
        if(!state.started)
        {
            start();
        }
        break;
    case 27: // ESC key to exit
        exit(0);
        break;
    }
    glutPostRedisplay(); // Request to redraw the scene
}


int main(int argc, char** argv) {

    int windowWidth = 1000;
    int windowHeight = 1000;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // Use double buffering
    glutInitWindowSize(windowWidth, windowHeight);  // Size of display area
    glutInitWindowPosition(100, 100); // Location of window
    glutCreateWindow("GL RGB Triangle"); // Window title
    initGameState(&state);
    glMatrixMode(GL_PROJECTION); // Set the projection matrix mode
    glLoadIdentity(); // Load the identity matrix
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);

    // Register the display callback
    glutDisplayFunc(display);

    // Start the timer
    glutTimerFunc(0, timer, 0);  // Start the timer immediately
    glutKeyboardFunc(keyboard);

    glutMainLoop();  // Run the event loop

    return 0;
}
