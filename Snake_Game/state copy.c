#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

// #define DEBUG_MODE // uncomment this line to enable all debug purpose function

#ifdef DEBUG_MODE
#define debug_print_game(...) print_game(__VA_ARGS__)
#define debug_printf(...) printf(__VA_ARGS__)
#else
#define debug_print_game(...)
#define debug_printf(...)
#endif

typedef unsigned int uint;

/* Helper function definitions */
static void set_board_at(game_state_t *state, uint row, uint col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static uint get_next_row(uint cur_row, char c);
static uint get_next_col(uint cur_col, char c);
static void find_head(game_state_t *state, uint snum);
static char next_square(game_state_t *state, uint snum);
static void update_tail(game_state_t *state, uint snum);
static void update_head(game_state_t *state, uint snum);

/* Test purpose function, uncomment debug line to enable these functions */
static void print_game(const char *msg, game_state_t *game_state);

/* Create Default State
 This function should create a default snake game in memory with the following
 starting state (which you can hardcode), and return a pointer to the newly created
 game_state_t struct.*/
game_state_t *create_default_state()
{
  const uint COLUMNS = 20;
  const uint ROWS = 18;
  const uint NUM_SNAKE = 1;
  const uint ZERO_CHAR = 1;
  const uint MAX_LENGTH = 21;
  const char edge_line[] = "####################";
  const char snake_line[] = "# d>D    *         #";
  const char empty_line[] = "#                  #";

  // because we need to neturn this, it should be created on the heap
  game_state_t *default_state = malloc(sizeof(game_state_t));

  // initialize struct and allocate memory
  default_state->num_rows = ROWS;
  default_state->board = malloc(ROWS * sizeof(char *));
  for (int counter = 0; counter < ROWS; counter++)
  {
    // the length of charater is columns plus a terminating character
    *(default_state->board + counter) = malloc(COLUMNS * sizeof(char) + ZERO_CHAR);
  }
  default_state->num_snakes = NUM_SNAKE;
  default_state->snakes = malloc(sizeof(snake_t));

  // set snake position
  default_state->snakes->head_col = 4;
  default_state->snakes->head_row = 2;
  default_state->snakes->tail_col = 2;
  default_state->snakes->tail_row = 2;
  default_state->snakes->live = true;

  // write default game map
  strncpy(*(default_state->board), edge_line, MAX_LENGTH);
  strncpy(*(default_state->board + 1), empty_line, MAX_LENGTH);
  strncpy(*(default_state->board + 2), snake_line, MAX_LENGTH);
  for (int counter = 3; counter < 17; counter++)
  {
    strncpy(*(default_state->board + counter), empty_line, MAX_LENGTH);
  }
  strncpy(*(default_state->board + 17), edge_line, MAX_LENGTH);

  debug_print_game("create default", default_state);

  return default_state;
}

/*Free State
This function should free all memory allocated for the given state,
including all snake structs and all map->board contents.
*/
void free_state(game_state_t *state)
{
  // free game board with each row first and board pointer next
  for (int row = 0; row < state->num_rows; row++)
  {
    free(*(state->board + row));
  }
  free(state->board);

  free(state->snakes);
  free(state);
  return;
}

/* Print Board
This function should print out the given game board to the given file pointer.
*/
void print_board(game_state_t *state, FILE *fp)
{
  for (int row = 0; row < state->num_rows; row++)
  {
    fprintf(fp, "%s\n", *(state->board + row));
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename)
{
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/*
  Helper function to get a character from the board
*/
char get_board_at(game_state_t *state, uint row, uint col)
{
  ...
}

/*
  Helper function to set a character on the board
*/
static void set_board_at(game_state_t *state, uint row, uint col, char ch)
{
  ...
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c)
{
  if (c == 'w' || c == 'a' || c == 's' || c == 'd')
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c)
{
  if (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x')
  {
    return true;
  }
  else
  {
    return false;
  }
  return true;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c)
{
  const int size = 13;
  const char snake[] = "wasd^<v>WASDx";
  for (int index = 0; index < size; index++)
  {
    if (c == snake[index])
    {
      return true;
    }
  }
  return false;
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c)
{
  const int size = 4;
  const char body[] = "^<v>";
  const char tail[] = "wasd";
  for (int index = 0; index < size; index++)
  {
    if (c == body[index])
    {
      return tail[index];
    }
  }
  return '?';
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c)
{
  const int size = 4;
  const char body[] = "^<v>";
  const char head[] = "WASD";
  for (int index = 0; index < size; index++)
  {
    if (c == head[index])
    {
      return body[index];
    }
  }
  return '?';
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static uint get_next_row(uint cur_row, char c)
{
  if (c == 'v' || c == 's' || c == 'S')
  {
    debug_printf("next_row receive %c return %d\n", c, cur_row + 1);
    return cur_row + 1;
  }
  else if (c == '^' || c == 'w' || c == 'W')
  {
    debug_printf("next_row receive %c return %d\n", c, cur_row - 1);
    return cur_row - 1;
  }
  else
  {
    debug_printf("next_row receive %c return %d\n", c, cur_row);
    return cur_row;
  }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static uint get_next_col(uint cur_col, char c)
{
  if (c == '>' || c == 'd' || c == 'D')
  {
    debug_printf("next_col receive %c return %d\n", c, cur_col + 1);
    return cur_col + 1;
  }
  else if (c == '<' || c == 'a' || c == 'A')
  {
    debug_printf("next_col receive %c return %d\n", c, cur_col - 1);
    return cur_col - 1;
  }
  else
  {
    debug_printf("next_col receive %c return %d\n", c, cur_col);
    return cur_col;
  }
}

/* Next Square
  This function returns the character in the cell the given snake is moving into.
  This function should not modify anything in the game stored in memory.
  */
static char next_square(game_state_t *state, uint snum)
{
  snake_t snake = state->snakes[snum];
  char head = state->board[snake.head_row][snake.head_col];
  uint next_col = get_next_col(snake.head_col, head);
  uint next_row = get_next_row(snake.head_row, head);

  debug_printf("Current square: %d, %d; Next square: %d, %d\n", snake.head_col, snake.head_row, next_col, next_row);

  debug_print_game("next square", state);

  return state->board[next_row][next_col];
}

/*Update Head
  This function will update the head of the snake
  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t *state, uint snum)
{
  debug_printf("update_head:\n");

  snake_t *snake = &(state->snakes[snum]);
  char *curr_head = &(state->board[snake->head_row][snake->head_col]);
  uint next_col = get_next_col(snake->head_col, *curr_head);
  uint next_row = get_next_row(snake->head_row, *curr_head);

  // put new head character on next location
  char *next_head = &(state->board[next_row][next_col]);
  if (*next_head != '#' && !is_snake(*next_head))
  {
    *next_head = *curr_head;

    // change old head to body
    *curr_head = head_to_body(*curr_head);

    // location of new head
    snake->head_row = next_row;
    snake->head_col = next_col;
  }
  else
  {
    *curr_head = 'x';
  }

  debug_print_game("update_head", state);
  return;
}

/* Update Tail
  This function will update the tail of the snake.
*/
static void update_tail(game_state_t *state, uint snum)
{
  debug_printf("update_tail:\n");
  snake_t *snake = &(state->snakes[snum]);
  char *curr_tail = &(state->board[snake->tail_row][snake->tail_col]);

  uint next_col = get_next_col(snake->tail_col, *curr_tail);
  uint next_row = get_next_row(snake->tail_row, *curr_tail);

  // put new head character on next location
  char *next_tail = &state->board[next_row][next_col];
  *next_tail = body_to_tail(*next_tail);

  // change old head empty space
  *curr_tail = ' ';

  // location of new head
  snake->tail_row = next_row;
  snake->tail_col = next_col;

  debug_print_game("update tail", state);
  return;
}

/* Update State */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state))
{
  debug_printf("update_state:\n");
  uint snake_count = state->num_snakes;
  for (uint snake_index = 0; snake_index < snake_count; snake_index++)
  {
    // fetch sanke information
    snake_t *snake = &(state->snakes[snake_index]);
    char next_head = next_square(state, snake_index);

    // check if it can eat a food
    bool eat_food = next_head == '*';

    update_head(state, snake_index);

    // update snake live information
    bool alive = state->board[snake->head_row][snake->head_col] != 'x';
    snake->live = alive;

    // if snake not eat food or die, update tail
    if (alive && !eat_food)
    {
      update_tail(state, snake_index);
    }
    // add food if it eats a food
    else if (eat_food)
    {
      add_food(state);
    }
  }

  debug_print_game("update state", state);
  return;
}

/* Load Board
This function will read a game board from a stream (FILE *) into memory.
 */
game_state_t *load_board(FILE *fp)
{
  game_state_t *state = malloc(sizeof(game_state_t));

  char read;
  char buffer[1000000];
  char line[200];
  uint index = 0;
  uint row_index = 0;
  uint row = 0;

  // read the entire file
  do
  {
    read = (char)fgetc(fp);
    buffer[index] = read;
    index++;

    // count how many rows to allocate later
    if (read == '\n')
    {
      row++;
    }

    debug_printf("read a %c from file\n", read);
  } while (read != EOF);

  state->board = malloc(sizeof(char *) * row);
  debug_printf("Read %d rows from file\n", row);

  index = 0;
  while (row_index < row)
  {
    uint line_index = 0;
    while (buffer[index] != '\n')
    {
      line[line_index] = buffer[index];
      line_index++;
      index++;
    }

    index++; // skip new line character

    debug_printf("Line read: %s with size of %d\n", line, line_index + 1);

    // copy each map string to state
    *(state->board + row_index) = malloc(sizeof(char) * (line_index + 1));
    strncpy(*(state->board + row_index), line, line_index + 1);
    state->board[row_index][line_index] = '\0';

    row_index++;
  }

  state->num_rows = row;
  state->snakes = NULL;
  state->num_snakes = 0;

  debug_print_game("load board", state);
  return state;
}

/* Initialize Snake
This function takes in a game board and creates the array of snake_t structs.
*/
static void find_head(game_state_t *state, uint snum)
{
  debug_printf("find_head start\n");
  snake_t *snake = &state->snakes[snum];
  debug_printf("find_head snakefind\n");
  char *ptr = &(state->board[snake->tail_row][snake->tail_col]);
  debug_printf("find_head ptr\n");
  uint new_head_row = snake->tail_row;
  uint new_head_col = snake->tail_col;
  while (!is_head(*ptr))
  {
    switch (*ptr)
    {
    case 'w':
      new_head_row--;
      break;
    case '^':
      new_head_row--;
      break;
    case 's':
      new_head_row++;
      break;
    case 'v':
      new_head_row++;
      break;
    case 'a':
      new_head_col--;
      break;
    case '<':
      new_head_col--;
      break;
    case 'd':
      new_head_col++;
      break;
    case '>':
      new_head_col++;
      break;
    }
    ptr = &(state->board[new_head_row][new_head_col]);
  }
  snake->head_row = new_head_row;
  snake->head_col = new_head_col;
  debug_printf("find_head\n");
  return;
}

game_state_t *initialize_snakes(game_state_t *state)
{
  // TODO: Implement this function.
  uint num_snake = 0;
  // Initinize num of snakes
  for (int i = 0; i < state->num_rows; i++)
  {
    for (int j = 0; state->board[i][j] != '\0'; j++)
    {
      if (is_tail(state->board[i][j]))
      {
        num_snake++;
      }
    }
  }
  state->snakes = malloc(sizeof(snake_t) * num_snake);

  // Assign all snake charactor
  uint snake_index = 0;
  for (uint i = 0; i < state->num_rows; i++)
  {
    for (uint j = 0; state->board[i][j] != '\0'; j++)
    {
      if (is_tail(state->board[i][j]))
      {
        state->snakes[snake_index].tail_row = i;
        state->snakes[snake_index].tail_col = j;
        state->snakes[snake_index].live = true;
        find_head(state, snake_index);
        snake_index++;
      }
    }
  }
  state->num_snakes = num_snake;
  debug_printf("initinaze_snake\n");
  return state;
}

// print the game information on the screen, including board and snake location
static void print_game(const char *msg, game_state_t *game_state)
{
  printf("debug print information for %s:\n", msg);
  for (int i = 0; i < game_state->num_rows; i++)
  {
    printf("%s\n", *(game_state->board + i));
  }

  if (game_state->snakes != NULL && game_state->num_snakes != 0)
  {
    uint snake_count = game_state->num_snakes;
    for (uint snake_index = 0; snake_index < snake_count; snake_index++)
    {
      snake_t *snake = &game_state->snakes[snake_index];
      printf("Snake%d tail: %d, %d\n", snake_index, snake->tail_col, snake->tail_row);
      printf("Snake%d head: %d, %d\n", snake_index, snake->head_col, snake->head_row);
    }
  }
  else
  {
    printf("Snake is NULL\n");
  }
}