
#ifndef ACCEL_DATA_QUEUE_H
#define ACCEL_DATA_QUEUE_H

#include "accel_interface.h"
#include <stdint.h>
#include <stdbool.h>

#define QUEUE_SIZE 100

typedef struct {
    AccelMeasurement data[QUEUE_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} AccelDataQueue;

// Initialize the queue
void queue_init(AccelDataQueue *queue);

// Add measurement to queue
// Returns 0 on success, -1 if queue is full
int queue_enqueue(AccelDataQueue *queue, AccelMeasurement meas);

// Remove measurement from queue
// Returns 0 on success, -1 if queue is empty
int queue_dequeue(AccelDataQueue *queue, AccelMeasurement *meas);

// Check if queue is empty
bool queue_is_empty(AccelDataQueue *queue);

// Check if queue is full
bool queue_is_full(AccelDataQueue *queue);

#endif
