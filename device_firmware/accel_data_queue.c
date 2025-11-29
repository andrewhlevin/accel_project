#include "accel_data_queue.h"

// Initialize the queue
void queue_init(AccelDataQueue *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

// Add measurement to queue
// Returns 0 on success, -1 if queue is full
int queue_enqueue(AccelDataQueue *queue, AccelMeasurement meas)
{
    if (queue_is_full(queue))
    {
        return -1; // Queue is full
    }

    queue->data[queue->tail] = meas;
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->count++;
    return 0;
}

// Remove measurement from queue
// Returns 0 on success, -1 if queue is empty
int queue_dequeue(AccelDataQueue *queue, AccelMeasurement *meas)
{
    if (queue_is_empty(queue))
    {
        return -1; // Queue is empty
    }

    *meas = queue->data[queue->head];
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->count--;
    return 0;
}

// Check if queue is empty
bool queue_is_empty(AccelDataQueue *queue)
{
    return (queue->count == 0);
}

// Check if queue is full
bool queue_is_full(AccelDataQueue *queue)
{
    return (queue->count == QUEUE_SIZE);
}