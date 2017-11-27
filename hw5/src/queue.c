#include "queue.h"
#include <errno.h>
#include <stdio.h>

queue_t *create_queue(void) {
    queue_t *newQueue;
    newQueue = calloc(1, sizeof(queue_t));
    if (newQueue == NULL) return NULL;
    if (pthread_mutex_init(&(newQueue->lock), NULL) != 0) return NULL;
    sem_init(&(newQueue->items), 0, 0);
    newQueue->invalid = false;
    return newQueue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    self->invalid = true;
    return false;
}

bool enqueue(queue_t *self, void *item) {
    pthread_mutex_lock(&(self->lock));
    if (self == NULL || self->invalid == true) {
        errno = EINVAL;
        return false;
    }
    queue_node_t *newNode = calloc(1, sizeof(queue_node_t));
    if (newNode == NULL) return false;
    newNode->item = item;
    int x = 0;
    sem_getvalue(&(self->items), &x);
    if (x == 0) {
        self->front = newNode;
        self->rear = newNode;
    } else {
        self->rear->next = newNode;
        self->rear = newNode;
    }
    pthread_mutex_unlock(&(self->lock));
    sem_post(&(self->items));
    return true;
}

void *dequeue(queue_t *self) {
    pthread_mutex_lock(&(self->lock));
    if (self == NULL) {
        errno = EINVAL;
        return NULL;
    }
    sem_wait(&(self->items));
    queue_node_t *remove = self->front;
    self->front = self->front->next;
    free(remove);
    pthread_mutex_unlock(&(self->lock));
    return NULL;
}