#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glut.h>  // freeglut.h might be a better alternative, if available.
#include "queue.h"


#define MS_PER_FRAME 16  // 16 milliseconds per frame for ~60 FPS

#define BOARD_START 100
#define BOARD_WIDTH 800

typedef struct
{

} Board;

typedef struct
{
    int x, y;
    float dx, dy;
    float speed;
    int level;
    Queue* body;
} Snake;

typedef struct
{
    int x, y;
} SegmentPosition;

typedef struct
{
    Snake snake;
    bool paused;
    bool gameover;
    bool started;
} GameState;

GameState state;

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

        if(tail->x == segmentPosition->x && tail->y == segmentPosition->y)
        {
            return true;
        }

        snakeNode = snakeNode->next;
    }

    return false;
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

    state.snake.x = state.snake.x + state.snake.dx;
    state.snake.y = state.snake.y + state.snake.dy;

    SegmentPosition* segment = (SegmentPosition*)malloc(sizeof(SegmentPosition));
    segment->x = state.snake.x +50;
    segment->y = state.snake.y;

    pushQueue(state.snake.body, segment);
    popQueue(state.snake.body);
}

void renderSnake()
{
    Node* node = state.snake.body->head;

    while (node != NULL)
    {
        SegmentPosition *segment = (SegmentPosition*)node->data;
        glBegin(GL_QUADS);
        glColor3f(1.0, 0.0, 0.0);
        glVertex2f(segment->x, segment->y);
        glVertex2f(segment->x + 10, segment->y);
        glVertex2f(segment->x + 10, segment->y + 10);
        glVertex2f(segment->x, segment->y + 10);
        glEnd();

        node = node->next;
    }
}

void renderWalls()
{
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glColor3f(1.0, 1.0, 1.0);

    glVertex2d(100, 100);  // Adjusted to NDC
    glVertex2d(900, 100);   // Adjusted to NDC

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
    glClearColor(0, 0, 0, 1);  // Set background color to black
    glClear(GL_COLOR_BUFFER_BIT);  // Clear the color buffer

    renderWalls();
    renderSnake();
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

    double current = getCurrentTime();
    double elapsed = current - previous;
    previous = current;
    lag += elapsed;

    while (lag >= MS_PER_FRAME) {
        updateGame();
        lag -= MS_PER_FRAME;
    }

    display();  // Call the display function to render the updated frame

    // Call the timer function again after the defined interval
    glutTimerFunc(16, timer, 0);  // 16 ms for ~60 FPS
}

void initGameState(GameState* state)
{
    if(state == NULL)
    {
        return;
    }

    state->paused = false;
    state->gameover = false;
    state->snake.x = 500;
    state->snake.y = 500;
    state->snake.speed = 3;
    state->snake.level = 5;
    state->snake.body = initQueue();

}

void restart()
{
    initGameState(&state);
}

void levelUp()
{
    SegmentPosition* segment = malloc(sizeof(SegmentPosition));

    segment->x = state.snake.x + 1;
    segment->y = state.snake.y;
    pushQueue(state.snake.body, segment);
}

void pause()
{
    state.paused = !state.paused;
}

void start()
{
    SegmentPosition* segment = malloc(sizeof(SegmentPosition));
    SegmentPosition* segment2 = malloc(sizeof(SegmentPosition));
    SegmentPosition* segment3 = malloc(sizeof(SegmentPosition));
    SegmentPosition* segment4 = malloc(sizeof(SegmentPosition));

    state.snake.dx = state.snake.speed;
    segment->x = state.snake.x;
    segment->y = state.snake.y;

    segment2->x = state.snake.x + 10;
    segment2->y = state.snake.y;

    segment3->x = state.snake.x + 20;
    segment3->y = state.snake.y;

    segment4->x = state.snake.x + 30;
    segment4->y = state.snake.y;



    // segment2.x = state->snake.x + 10;
    // segment2.y = state->snake.y;
    pushQueue(state.snake.body, segment);
    pushQueue(state.snake.body, segment2);
    pushQueue(state.snake.body, segment3);
    pushQueue(state.snake.body, segment4);
    // pushQueue(state->snake.body, &segment2);

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
        state.snake.dy = -state.snake.speed;
        break;
    case 's': // Move down
        if(state.snake.dy < 0)
        {
            return;
        }
        state.snake.dx = 0;
        state.snake.dy = state.snake.speed;
        break;
    case 'a': // Move left
        if(state.snake.dx > 0)
        {
            return;
        }
        state.snake.dx = -state.snake.speed;
        state.snake.dy = 0;
        break;
    case 'd': // Move right
        if(state.snake.dx < 0)
        {
            return;
        }
        state.snake.dx = state.snake.speed;
        state.snake.dy = 0;
        break;
    case 'x':
        levelUp();
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
