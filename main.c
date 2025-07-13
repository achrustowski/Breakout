#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#define FPS                 60
#define FRAME_TARGET_TIME   (1000.0f/FPS)
#define SCREEN_WIDTH        960
#define SCREEN_HEIGHT       640
#define PLAYER_WIDTH        50
#define PLAYER_HEIGHT       10
#define PLAYER_SPEED        10
#define BRICK_GAP           5
#define BRICK_WIDTH         55
#define BRICK_HEIGHT        15
#define BALL_SIZE           10
#define ROWS                16
#define COLUMNS             6

typedef struct
{
    bool    left, right;
} Input;

typedef struct
{
    int     x, y;
} Position;

typedef struct
{
    int     x, y;
} Velocity;

typedef struct
{
    SDL_Rect    rect;
    bool        is_alive;
} Brick;

typedef struct
{
    SDL_Rect    rect;
    Position    position;
    Velocity    velocity;
} Ball;

typedef struct
{
    SDL_Rect    rect;
    Position    position;
    Velocity    velocity;
} Paddle;

typedef struct
{
    Brick       bricks[ROWS][COLUMNS];
    Ball        ball;
    Paddle      paddle;
    int         lives;
    int         score;
} Game;

int array_x[8] = {-200, 200, -300, 300, -400, 400, -500, 500};
int array_y[8] = {-200, 200, -300, 300, -400, 400, -500, 500};

int last_frame_time = 0;
bool game_is_running = false;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int init_paddle(Paddle* paddle);
int init_ball(Ball* ball);
int init_bricks(Game* game);
int init_game(Game* game);
int init_lives(Game* game);

void draw_paddle(SDL_Renderer* renderer, Paddle* paddle);
void draw_ball(SDL_Renderer* renderer, Ball* ball);
void draw_bricks(SDL_Renderer* renderer, Game* game);

void move_paddle(Paddle* paddle, SDL_KeyCode key);
void stop_paddle(Paddle* paddle);
void move_ball(Ball* ball, SDL_KeyCode key);

void update_paddle(Paddle* paddle);
void update_ball(Ball* ball, Paddle* paddle, float* delta_time);

void ball_bricks_collision(Ball* ball, Game* game, Brick* bricks);

void update(Paddle* paddle, Ball* ball, Game* game, Brick* bricks);
void do_input(Paddle* paddle, Ball* ball);
void render(SDL_Renderer*, Paddle* paddle, Ball* ball, Game* game);

int main()
{
    Game game;
    Game* game_ptr = &game;
    if (init_game(game_ptr) != 0)
    {
        printf("Error initializing the game\n");
        return 1;
    }

    Ball ball;
    Ball* ball_ptr = &ball;
    if (init_ball(ball_ptr) != 0)
    {
        printf("Error initializing the ball\n");
        return 1;
    }

    Paddle paddle;
    Paddle* paddle_ptr = &paddle;
    if (init_paddle(paddle_ptr) != 0)
    {
        printf("Error initializing the paddle\n");
        return 1;
    }

    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("Breakout", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    srand(time(NULL));

    while (!game_is_running)
    {
        do_input(paddle_ptr, ball_ptr);
        update(paddle_ptr, ball_ptr, game_ptr, *game_ptr->bricks);
        render(renderer, paddle_ptr, ball_ptr, game_ptr);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

int init_game(Game* game)
{
    game->score = 0;

    if (init_bricks(game) != 0)
    {
        printf("Error initializing bricks\n");
        return 1;
    }
    if (init_lives(game) != 0)
    {
        printf("Error initializing lives\n");
        return 1;
    }
    return 0;
}

int init_ball(Ball* ball)
{
    if (ball)
    {
        ball->rect.h = BALL_SIZE;
        ball->rect.w = BALL_SIZE;
        ball->position.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
        ball->position.y = SCREEN_HEIGHT / 2 + BALL_SIZE / 2;
        ball->velocity.x = 0;
        ball->velocity.y = 0;
        ball->rect.x = ball->position.x;
        ball->rect.y = ball->position.y;
        return 0;
    } else
    {
        printf("Error initializing ball\n");
        return 1;
    }
}

int init_paddle(Paddle* paddle)
{
    if (paddle)
    {
        paddle->rect.h = PLAYER_HEIGHT;
        paddle->rect.w = PLAYER_WIDTH;
        paddle->position.x = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
        paddle->position.y = SCREEN_HEIGHT - 2 * PLAYER_HEIGHT;
        paddle->velocity.x = 0;
        paddle->velocity.y = 0;
        paddle->rect.x = paddle->position.x;
        paddle->rect.y = paddle->position.y;
        return 0;
    } else
    {
        printf("Error initializing paddle\n");
        return 1;
    }
}

int init_bricks(Game* game)
{
    if (game)
    {
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLUMNS; j++)
        {
            game->bricks[i][j].rect.x = BRICK_GAP / 2 + (i * (BRICK_WIDTH + BRICK_GAP));
            game->bricks[i][j].rect.y = BRICK_GAP / 2 + (j * (BRICK_HEIGHT + BRICK_GAP));
            game->bricks[i][j].rect.w = BRICK_WIDTH;
            game->bricks[i][j].rect.h = BRICK_HEIGHT;
            game->bricks[i][j].is_alive = true;
        }
        }
        return 0;
    } else
    {
        printf("Game is null\n");
        return 1;
    }
}

int init_lives(Game* game)
{
    if (game)
    {
        game->lives = 3;
        return 0;
    } else
    {
        printf("Game is null\n");
        return 1;
    }
}

void draw_paddle(SDL_Renderer* renderer, Paddle* paddle)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &paddle->rect);
}

void draw_ball(SDL_Renderer* renderer, Ball* ball)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &ball->rect);
}

void draw_bricks(SDL_Renderer* renderer, Game* game)
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            if (game->bricks[i][j].is_alive)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &game->bricks[i][j].rect);
            }
        }
    }
}

void move_paddle(Paddle* paddle, SDL_KeyCode key)
{
    if (paddle)
    {
        if (key == SDLK_LEFT)
        {
            paddle->velocity.x = -1 * PLAYER_SPEED;
            paddle->velocity.y = 0;

            if (paddle->position.x <= 0)
            {
                paddle->velocity.x = 0;
                paddle->position.x = 0;
            }
        }
        if (key == SDLK_RIGHT)
        {
            paddle->velocity.x = 1 * PLAYER_SPEED;
            paddle->velocity.y = 0;

            if (paddle->position.x + PLAYER_WIDTH >= SCREEN_WIDTH)
            {
                paddle->velocity.x = 0;
                paddle->position.x = SCREEN_WIDTH - PLAYER_WIDTH;
            }
        }
    }
}

void stop_paddle(Paddle* paddle)
{
    if (paddle)
    {
        paddle->velocity.x = 0;
    }
}

void move_ball(Ball* ball, SDL_KeyCode key)
{
    int rand_x = rand() % 8;
    int rand_y = rand() % 8;
    if (ball)
    {
        if (key == SDLK_SPACE && ball->velocity.x == 0 && ball->velocity.y == 0)
        {
            ball->velocity.x = array_x[rand_x];
            ball->velocity.y = array_y[rand_y];

            if (ball->position.x <= 0 || ball->position.x + BALL_SIZE >= SCREEN_WIDTH)
            {

                ball->velocity.x *= -1;
            }
            if (ball->position.y <= 0 || ball->position.y + BALL_SIZE >= SCREEN_HEIGHT)
            {

                ball->velocity.y *= -1;
            }
        }
    }
}

void update_paddle(Paddle* paddle)
{
    if (paddle)
    {
        paddle->position.x += paddle->velocity.x;
        paddle->rect.x = paddle->position.x;
        paddle->rect.y = paddle->position.y;

        if (paddle->position.x <= 0)
        {
            paddle->position.x = 0;
            paddle->rect.x = 0;
        }

        if (paddle->position.x + PLAYER_WIDTH >= SCREEN_WIDTH)
        {
            paddle->position.x = SCREEN_WIDTH - PLAYER_WIDTH;
            paddle->rect.x = SCREEN_WIDTH - PLAYER_WIDTH;
        }
    }
}

void update_ball(Ball* ball, Paddle* paddle, float* delta_time)
{
    if (ball)
    {
        ball->position.x += ball->velocity.x * (*delta_time);
        ball->position.y += ball->velocity.y * (*delta_time);
        ball->rect.x = ball->position.x;
        ball->rect.y = ball->position.y;

        if (ball->position.x <= 0 || ball->position.x + BALL_SIZE >= SCREEN_WIDTH)
        {

            ball->velocity.x *= -1;
        }
        if (ball->position.y <= 0)
        {
            ball->velocity.y *= -1;
        }
        if (ball->position.y >= SCREEN_HEIGHT)
        {
            ball->position.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
            ball->position.y = SCREEN_HEIGHT / 2 + BALL_SIZE / 2;
            ball->velocity.x = 0;
            ball->velocity.y = 0;
        }
        if (SDL_HasIntersection(&ball->rect, &paddle->rect))
        {
            ball->velocity.y *= -1;
        }
    }
}

void ball_bricks_collision(Ball* ball, Game* game, Brick* bricks)
{
    int num_bricks = COLUMNS * ROWS;
    if (ball)
    {
        for (int i = 0; i < num_bricks; i++)
        {
            if (SDL_HasIntersection(&ball->rect, &bricks[i].rect) && bricks[i].is_alive)
            {
                ball->position.y += 2;
                ball->velocity.y *= -1;
                bricks[i].is_alive = false;
            }
        }
    }
}

void update(Paddle* paddle, Ball* ball, Game* game, Brick* bricks)
{
    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks();
    update_paddle(paddle);
    update_ball(ball, paddle,  &delta_time);
    ball_bricks_collision(ball, game, bricks);
}

void do_input(Paddle* paddle, Ball* ball)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                game_is_running = true;
                break;
            case SDL_KEYDOWN:
                move_paddle(paddle, event.key.keysym.sym);
                move_ball(ball, event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                stop_paddle(paddle);
            default:
                break;
        }
    }
}

void render(SDL_Renderer* renderer, Paddle* paddle, Ball* ball, Game* game)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    draw_paddle(renderer, paddle);
    draw_ball(renderer, ball);
    draw_bricks(renderer, game);

    SDL_RenderPresent(renderer);
}
