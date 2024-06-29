#include "dining.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct dining {
  int capacity;
  int num_students_inside;
  int cleaning_in_progress;
  pthread_mutex_t mutex;
  pthread_cond_t student_enter_cond;
  pthread_cond_t cleaning_enter_cond;
} dining_t;

dining_t *dining_init(int capacity) {
  dining_t *dining = malloc(sizeof(dining_t));
  if (!dining) {
    perror("Error allocating memory for dining hall");
    exit(EXIT_FAILURE);
  }
  dining->capacity = capacity;
  dining->num_students_inside = 0;
  dining->cleaning_in_progress = 0;

  if (pthread_mutex_init(&dining->mutex, NULL) != 0) {
    perror("Error initializing mutex");
    exit(EXIT_FAILURE);
  }
  if (pthread_cond_init(&dining->student_enter_cond, NULL) != 0) {
    perror("Error initializing condition variable");
    exit(EXIT_FAILURE);
  }
  if (pthread_cond_init(&dining->cleaning_enter_cond, NULL) != 0) {
    perror("Error initializing condition variable");
    exit(EXIT_FAILURE);
  }

  return dining;
}
void dining_destroy(dining_t **dining) {
  dining_t *dining_ptr = *dining;
  pthread_mutex_destroy(&dining_ptr->mutex);
  pthread_cond_destroy(&dining_ptr->student_enter_cond);
  pthread_cond_destroy(&dining_ptr->cleaning_enter_cond);
  free(dining_ptr);
  *dining = NULL;
}

void dining_student_enter(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex);
  while (dining->num_students_inside >= dining->capacity ||
         dining->cleaning_in_progress) {
    pthread_cond_wait(&dining->student_enter_cond, &dining->mutex);
  }
  dining->num_students_inside++;
  pthread_mutex_unlock(&dining->mutex);
}

void dining_student_leave(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex);
  dining->num_students_inside--;
  pthread_cond_signal(&dining->student_enter_cond);
  pthread_cond_signal(&dining->cleaning_enter_cond);
  pthread_mutex_unlock(&dining->mutex);
}

void dining_cleaning_enter(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex);
  while (dining->num_students_inside > 0 || dining->cleaning_in_progress) {
    pthread_cond_wait(&dining->cleaning_enter_cond, &dining->mutex);
  }
  dining->cleaning_in_progress = 1;
  pthread_mutex_unlock(&dining->mutex);
}

void dining_cleaning_leave(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex);
  dining->cleaning_in_progress = 0;
  pthread_cond_broadcast(&dining->student_enter_cond);
  pthread_cond_signal(&dining->cleaning_enter_cond);
  pthread_mutex_unlock(&dining->mutex);
}
