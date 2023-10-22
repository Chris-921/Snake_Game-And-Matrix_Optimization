#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "coordinator.h"

#define READY 0
#define NEW_TASK 1
#define TERMINATE -1

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Error: not enough arguments\n");
    printf("Usage: %s [path_to_task_list]\n", argv[0]);
    return -1;
  }
  int num_tasks;
  task_t **tasks;
  if (read_tasks(argv[1], &num_tasks, &tasks))
    return -1;

  // Implement Open MPI coordinator
  // Use MPI_Init to initialize the program
  MPI_Init(&argc, &argv);

  int procID, totalProcs;
  // Get the total number of processes and store in `totalProcs`
  MPI_Comm_size(MPI_COMM_WORLD, &totalProcs);

  // Get the ID of the current program, and store in `procID`
  MPI_Comm_rank(MPI_COMM_WORLD, &procID);

  // check if the current process is the manager
  if (procID == 0)
  {
    // Manager node
    int nextTask = 0;
    MPI_Status status;
    int32_t message;

    // loop until we've completed `numTasks`
    while (nextTask < num_tasks)
    {
      // receive a message from any source (so we know that this node is done with its task)
      MPI_Recv(&message, 1, MPI_INT32_T, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

      // get the source process using the `status` struct
      int sourceProc = status.MPI_SOURCE;

      // send `nextTask` as the message to the process we just received a message from
      MPI_Send(&nextTask, 1, MPI_INT32_T, sourceProc, 0, MPI_COMM_WORLD);

      // increment `nextTask` by 1
      nextTask++;
    }

    // Wait for all processes to finish
    // loop through all processes
    // we have `totalProcs - 1` total, since there's a manager node
    for (int i = 0; i < totalProcs - 1; i++)
    {
      // receive a message from any source (so we know that this node is done with its task)
      MPI_Recv(&message, 1, MPI_INT32_T, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

      // get the source process using the `status` struct
      int sourceProc = status.MPI_SOURCE;

      // send `TERMINATE` as the message to the process we just received a message from
      message = TERMINATE;
      MPI_Send(&message, 1, MPI_INT32_T, sourceProc, 0, MPI_COMM_WORLD);
    }
  }
  else
  {
    // Worker node
    int32_t message;

    while (true)
    {
      // let the manager node know that this worker is ready
      // use MPI_Send to send a message
      message = READY;
      MPI_Send(&message, 1, MPI_INT32_T, 0, 0, MPI_COMM_WORLD);

      // receive 1 message from the manager and store it in `message`
      // use MPI_Recv to receive a message
      MPI_Recv(&message, 1, MPI_INT32_T, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      // if the `message` is TERMINATE, break out of the loop to terminate
      if (message == TERMINATE)
        break;

      // Add your computation function here and call it to execute the task
      // The computation function should take the task as an argument and perform the required computations.
      // Example: execute_task(task);
      if (execute_task(tasks[message]))
      {
        printf("Task %d failed\n", message);
        return -1;
      }
      free(tasks[message]->path);
    }
  }

  // Finalize MPI
  MPI_Finalize();
  return 0;
}