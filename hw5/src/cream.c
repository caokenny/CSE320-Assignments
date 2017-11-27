#include "cream.h"
#include "utils.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    queue_t *newQ;
    newQ = create_queue();
    int array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int x = 10;
    for (int i = 0; i < x; i++) {
        enqueue(newQ, &array[i]);
    }
    for (int i = 0; i < x; i++) {
        printf("%d\n", *(int*)newQ->front->item);
        dequeue(newQ);
    }
    x = 100;
    enqueue(newQ, &x);
    printf("%d\n", *(int*)newQ->front->item);
    dequeue(newQ);
    exit(0);
}
