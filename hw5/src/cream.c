#include "cream.h"
#include "utils.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#define NUM_THREADS 100
queue_t *global_queue;

void queue_free_function(void *item) {
    free(item);
}

void *thread_enqueue(void *arg) {
    enqueue(global_queue, arg);
    return NULL;
}
int main(int argc, char *argv[]) {
    global_queue = create_queue();
    pthread_t thread_ids[NUM_THREADS];

    // spawn NUM_THREADS threads to enqueue elements
    for(int index = 0; index < NUM_THREADS; index++) {
        int *ptr = malloc(sizeof(int));
        *ptr = index;

        if(pthread_create(&thread_ids[index], NULL, thread_enqueue, ptr) != 0)
            exit(EXIT_FAILURE);
    }

    // wait for threads to die before checking queue
    for(int index = 0; index < NUM_THREADS; index++) {
        pthread_join(thread_ids[index], NULL);
    }

    // get number of items in queue
    int num_items;
    if(sem_getvalue(&global_queue->items, &num_items) != 0)
        exit(EXIT_FAILURE);
    if (num_items != NUM_THREADS) {
        printf("Had %d items. Expected: %d\n", num_items, NUM_THREADS);
    } else printf("CORRECT\n");

    invalidate_queue(global_queue, queue_free_function);
    // queue_t *newQ;
    // newQ = create_queue();
    // int array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    // int x = 10;
    // for (int i = 0; i < x; i++) {
    //     enqueue(newQ, &array[i]);
    // }
    // for (int i = 0; i < x; i++) {
    //     printf("%d\n", *(int*)newQ->front->item);
    //     dequeue(newQ);
    // }
    // x = 100;
    // enqueue(newQ, &x);
    // printf("%d\n", *(int*)newQ->front->item);
    // dequeue(newQ);
    exit(0);
}
