#include "compute.h"

// #define DEBUG_MODE

#ifdef DEBUG_MODE
#define debug_printf(...) printf(__VA_ARGS__)
#define debug_print_m(...) print_m(__VA_ARGS__)
#else
#define debug_printf(...)
#define debug_print_m(...)
#endif

void print_m(int32_t row, int32_t col, int32_t *vec)
{
  for (int i = 0; i < row; i++)
  {
    for (int j = 0; j < col; j++)
    {
      printf("%4d ", vec[i * col + j]);
    }
    printf("\n");
  }
}

// Computes the dot product of vec1 and vec2, both of size n
int32_t dot(int32_t n, int32_t *vec1, int32_t *vec2)
{
  // TODO: implement dot product of vec1 and vec2, both of size n
  int32_t result = 0;

  for (int32_t i = 0; i < n; i++)
  {
    result += vec1[i] * vec2[i];
  }

  return result;
}

// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix)
{
  debug_printf("Matrix A:\n");
  debug_print_m(a_matrix->rows, a_matrix->cols, a_matrix->data);

  // convolve matrix a and matrix b, and store the resulting matrix in
  // Assign veriable
  int32_t rows_a = a_matrix->rows;
  int32_t cols_a = a_matrix->cols;
  int32_t rows_b = b_matrix->rows;
  int32_t cols_b = b_matrix->cols;
  int32_t rows_output = rows_a - rows_b + 1;
  int32_t cols_output = cols_a - cols_b + 1;

  // Allocate memory for the output matrix
  *output_matrix = malloc(sizeof(matrix_t));
  (*output_matrix)->rows = rows_output;
  (*output_matrix)->cols = cols_output;
  (*output_matrix)->data = malloc(sizeof(int32_t) * rows_output * cols_output);

  // Flip matrix b
  int32_t *flipped_b = malloc(sizeof(int32_t) * rows_b * cols_b);
  for (int32_t i = 0; i < rows_b; i++)
  {
    for (int32_t j = 0; j < cols_b; j++)
    {
      flipped_b[i * cols_b + j] = b_matrix->data[(rows_b - i - 1) * cols_b + (cols_b - j - 1)];
    }
  }

  debug_printf("Matrix B:\n");
  debug_print_m(rows_b, cols_b, b_matrix->data);
  debug_printf("Matrix Flipped:\n");
  debug_print_m(rows_b, cols_b, flipped_b);

  for (int32_t i = 0; i < rows_output; i++)
  {
    for (int32_t j = 0; j < cols_output; j++)
    {

      int sum = 0;

      for (int32_t k = 0; k < rows_b; k++)
      {
        debug_printf("Row %d Col %d :\n", k, j);
        debug_print_m(1, cols_b, arr_A);

        sum += dot(cols_b, &a_matrix->data[(i + k) * cols_a + j], &flipped_b[k * cols_b]);
      }

      (*output_matrix)->data[i * cols_output + j] = sum;
    }
  }
  free(flipped_b);

  return 0;
}

// Executes a task
int execute_task(task_t *task)
{
  matrix_t *a_matrix, *b_matrix, *output_matrix;

  if (read_matrix(get_a_matrix_path(task), &a_matrix))
    return -1;
  if (read_matrix(get_b_matrix_path(task), &b_matrix))
    return -1;

  if (convolve(a_matrix, b_matrix, &output_matrix))
    return -1;

  if (write_matrix(get_output_matrix_path(task), output_matrix))
    return -1;

  free(a_matrix->data);
  free(b_matrix->data);
  free(output_matrix->data);
  free(a_matrix);
  free(b_matrix);
  free(output_matrix);
  return 0;
}
