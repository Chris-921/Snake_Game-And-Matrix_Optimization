#include <stdio.h>
#include <string.h>

#include "snake_utils.h"
#include "state.h"

int main(int argc, char *argv[])
{
  ...

      // Parse arguments
      for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-i") == 0 && i < argc - 1)
    {
      if (io_stdin)
      {
        fprintf(stderr, "Usage: %s [-i filename | --stdin] [-o filename]\n", argv[0]);
        return 1;
      }
      in_filename = argv[i + 1];
      i++;
      continue;
    }
    else if (strcmp(argv[i], "--stdin") == 0)
    {
      ...
    }
    if (strcmp(argv[i], "-o") == 0 && i < argc - 1)
    {
      ...
    }
    ...
  }

  if (in_filename != NULL)
  {
    // Load the board from in_filename
    // If the file doesn't exist, return -1
    // Then call initialize_snakes on the state you made
    FILE *fp = fopen(in_filename, "r");
    if (fp == NULL)
    {
      return -1;
    }
    state = load_board(fp);
    initialize_snakes(state);
  }
  else if (io_stdin)
  {
    // Load the board from stdin
    // Then call initialize_snakes on the state you made
    state = load_board(stdin);
    initialize_snakes(state);
  }
  else
  {
    // Create default state
    state = create_default_state();
  }

  // Update state. Use the deterministic_food function
  // (already implemented in snake_utils.h) to add food.
  update_state(state, deterministic_food);
  // Write updated board to file or stdout
  if (out_filename != NULL)
  {
    // Save the board to out_filename
    save_board(state, out_filename);
  }
  else
  {
    // Print the board to stdout
    print_board(state, stdout);
  }

  // Free the state
  free_state(state);
  return 0;
}
