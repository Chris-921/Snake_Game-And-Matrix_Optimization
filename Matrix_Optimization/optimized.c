#include <omp.h>
#include <x86intrin.h>

#include "compute.h"

// #define DEBUG_MODE

#ifdef DEBUG_MODE
#define debug true
#define debug_printf(...) printf(__VA_ARGS__)
#define debug_print_m(...) print_m(__VA_ARGS__)
#else
#define debug false
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
int32_t dot(uint32_t n, int32_t *vec1, int32_t *vec2)
{
  // implement dot product of vec1 and vec2, both of size n
  int32_t result = 0;
  int32_t i;
  int32_t end = n - 32;
  __m256i tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
  __m256i mul, mul2, mul3, mul4;

  int32_t *int_mul = (int32_t *)&mul;
  int32_t *int_mul2 = (int32_t *)&mul2;
  int32_t *int_mul3 = (int32_t *)&mul3;
  int32_t *int_mul4 = (int32_t *)&mul4;

  for (i = 0; i < end; i += 32)
  {
    tmp1 = _mm256_loadu_si256((__m256i *)(vec1 + i));
    tmp2 = _mm256_loadu_si256((__m256i *)(vec2 + i));
    tmp3 = _mm256_loadu_si256((__m256i *)(vec1 + i + 8));
    tmp4 = _mm256_loadu_si256((__m256i *)(vec2 + i + 8));
    tmp5 = _mm256_loadu_si256((__m256i *)(vec1 + i + 16));
    tmp6 = _mm256_loadu_si256((__m256i *)(vec2 + i + 16));
    tmp7 = _mm256_loadu_si256((__m256i *)(vec1 + i + 24));
    tmp8 = _mm256_loadu_si256((__m256i *)(vec2 + i + 24));

    debug_printf("vec1:\n");
    debug_print_m(1, 8, vec1 + i);
    debug_printf("vec2:\n");
    debug_print_m(1, 8, vec2 + i);

    mul = _mm256_mullo_epi32(tmp1, tmp2);
    mul2 = _mm256_mullo_epi32(tmp3, tmp4);
    mul3 = _mm256_mullo_epi32(tmp5, tmp6);
    mul4 = _mm256_mullo_epi32(tmp7, tmp8);

    result += int_mul[0] + int_mul[1] + int_mul[2] + int_mul[3] + int_mul[4] + int_mul[5] + int_mul[6] + int_mul[7] +
              int_mul2[0] + int_mul2[1] + int_mul2[2] + int_mul2[3] + int_mul2[4] + int_mul2[5] + int_mul2[6] + int_mul2[7] +
              int_mul3[0] + int_mul3[1] + int_mul3[2] + int_mul3[3] + int_mul3[4] + int_mul3[5] + int_mul3[6] + int_mul3[7] +
              int_mul4[0] + int_mul4[1] + int_mul4[2] + int_mul4[3] + int_mul4[4] + int_mul4[5] + int_mul4[6] + int_mul4[7];

    debug_printf("Calculate Element:\n%9d %9d %9d %9d %9d %9d %9d %9d \n",
                 tmp_arr[0], tmp_arr[1], tmp_arr[2], tmp_arr[3], tmp_arr[4], tmp_arr[5], tmp_arr[6], tmp_arr[7]);
    debug_printf("result:%d\n", result);
  }

  for (; i < n; i++)
  {
    result += vec1[i] * vec2[i];

    debug_printf("vec1:\n");
    debug_print_m(1, n - i, vec1 + i);
    debug_printf("vec2:\n");
    debug_print_m(1, n - i, vec2 + i);
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
  int32_t *origin_data = b_matrix->data;
  for (int32_t i = 0; i < rows_b; i++)
  {
    for (int32_t j = 0; j < cols_b; j++)
    {
      flipped_b[i * cols_b + j] = origin_data[(rows_b - i - 1) * cols_b + (cols_b - j - 1)];
    }
  }

  debug_printf("Matrix B:\n");
  debug_print_m(rows_b, cols_b, b_matrix->data);
  debug_printf("Matrix Flipped:\n");
  debug_print_m(rows_b, cols_b, flipped_b);

#pragma omp parallel for
  for (int32_t i = 0; i < rows_output; i++)
  {
    int32_t tail_point = cols_output - cols_output % 8;
    __m256i temp;
    int32_t *sum = (int32_t *)&temp;

    for (int32_t j = 0; j < tail_point; j += 8)
    {
      temp = _mm256_set1_epi32(0);

      int32_t *a_index = &a_matrix->data[i * cols_a + j];
      int32_t *flipped_b_index = flipped_b;

      for (int32_t k = 0; k < rows_b;
           k++, a_index += cols_a, flipped_b_index += cols_b)
      {
        debug_printf("Row %d Col %d :\n", k, j);
        debug_print_m(1, cols_b, arr_A);

        sum[0] += dot(cols_b, a_index, flipped_b_index);
        sum[1] += dot(cols_b, a_index + 1, flipped_b_index);
        sum[2] += dot(cols_b, a_index + 2, flipped_b_index);
        sum[3] += dot(cols_b, a_index + 3, flipped_b_index);
        sum[4] += dot(cols_b, a_index + 4, flipped_b_index);
        sum[5] += dot(cols_b, a_index + 5, flipped_b_index);
        sum[6] += dot(cols_b, a_index + 6, flipped_b_index);
        sum[7] += dot(cols_b, a_index + 7, flipped_b_index);
      }

      int32_t *output_index = &((*output_matrix)->data[i * cols_output + j]);
      _mm256_storeu_si256((__m256i *)output_index, temp);
    }

    for (int j = tail_point; j < cols_output; j += 1)
    {
      int sum = 0;
      int32_t *a_index = &a_matrix->data[i * cols_a + j];
      int32_t *flipped_b_index = flipped_b;

      for (int32_t k = 0; k < rows_b; k++, a_index += cols_a, flipped_b_index += cols_b)
      {
        debug_printf("Row %d Col %d :\n", k, j);
        debug_print_m(1, cols_b, arr_A);

        sum += dot(cols_b, a_index, flipped_b_index);
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
