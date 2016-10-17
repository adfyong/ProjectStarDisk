#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "algo.h"
#include "game.h"
#include "hashset.h"
#include "heap.h"


static void print_array(int8_t *arr)
{
    for (int8_t i = 0; i < game_size; i++)
        printf("%d", arr[i]);
    printf("\n");
}

/*
struct p_queue_node {
    struct node *node;
    struct p_queue_node *next;
    struct p_queue_node *prev;
};

struct p_queue {
    struct p_queue_node *head;
    struct p_queue_node *tail;
};


static void print_queue(struct p_queue *queue)
{
    struct p_queue_node *node = queue->head;
    while (node) {
        printf("\t Cost %d: ", node->node->f);
        print_array(node->node->state);
        node = node->next;
    }
}


static struct p_queue *make_p_queue()
{
    struct p_queue *queue = malloc(sizeof(struct p_queue));
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

static void add_to_queue(struct p_queue *queue, struct node *node)
{
    // printf("Adding child of cost %d: ", node->f);
    // print_array(node->state);
    struct p_queue_node *qnode = malloc(sizeof(struct p_queue_node));
    qnode->node = node;

    if (!queue->head) {
        queue->head = qnode;
        queue->tail = qnode;
        qnode->next = NULL;
        qnode->prev = NULL;
        return;
    }

    struct p_queue_node *search = queue->tail;
    while (search) {
        if (search->node->f <= node->f) {
            if (search->next)
                search->next->prev = qnode;
            qnode->next = search->next;
            search->next = qnode;
            qnode->prev = search;
            if (queue->tail == search)
                queue->tail = qnode;
            return;
        }

        search = search->prev;
    }

    // adding to beginning of queue
    qnode->next = queue->head;
    queue->head->prev = qnode;
    qnode->prev = NULL;
    queue->head = qnode;
}


static struct node *queue_front(struct p_queue *queue)
{
    if (!queue->head)
        return NULL;

    struct node *node = queue->head->node;
    struct p_queue_node *old_node = queue->head;
    queue->head = queue->head->next;
    if (queue->head)
        queue->head->prev = NULL;
    free(old_node);
    return node;
}
*/

static int8_t indexof(int8_t *state, int8_t needle)
{
    for (int8_t i = 0; i < game_size; i++) {
        if (state[i] == needle)
            return i;
    }

    return -1;
}

static int8_t *swap(int8_t *state, int8_t index1, int8_t index2)
{
    int8_t *ret = malloc(game_size * sizeof(int8_t));
    if (!ret)
        exit(EXIT_FAILURE);

    memcpy(ret, state, game_size * sizeof(int8_t));

    ret[index1] = state[index2];
    ret[index2] = state[index1];

    return ret;
}

struct node *make_node(int8_t *state, struct node *parent, int8_t (*h)(int8_t *, int8_t *))
{
    struct node *node = malloc(sizeof(struct node));
    if (!node)
        exit(EXIT_FAILURE);

    node->heuristic = h(NULL, state);
    node->parent = parent;
    node->state = state;
    node->index0 = indexof(state, 0);

    if (!parent)
        node->cost = 0;
    else
        node->cost = parent->cost + 1;

    node->f = node->cost + node->heuristic;

    //if(no_states % 1000 == 0){
    //    printf("%d ", no_states);
    //    printf("%d\n", node->cost);
    //    print_array(state);
    //}

    return node;
}

void expand(struct node *node, struct node **children, int8_t big_disks[],
        int8_t (*h)(int8_t *, int8_t *))
{
    int8_t index0 = node->index0;

    children[0] = make_node(
            swap(node->state, index0, index0 == 0 ? game_size - 1 : index0 - 1),
            node, h);

    children[1] = make_node(
            swap(node->state, index0, (index0 + 1) % game_size), node, h);

    int8_t jump = big_disks[index0];
    if (jump == 1)
        return;

    children[2] = make_node(
            swap(node->state, index0, index0 < jump ?
                game_size - jump + index0 : index0 - jump),
            node, h);

    children[3] = make_node(
            swap(node->state, index0, (index0 + jump) % game_size), node, h);

}

int no_states;

struct node *a_star(int8_t *state, int8_t big_disks[], int8_t (*h)(int8_t *, int8_t *))
{
    struct hashset *set = create_hashset(3);
    //struct p_queue *fringe = make_p_queue();
    Heap *fringe = create_heap();
    //add_to_queue(fringe, make_node(state, NULL, h));
    add(fringe, make_node(state, NULL, h));

    struct node *cur_node;
    struct node *children[4] = {};
    int8_t num_children;
    //while ((cur_node = queue_front(fringe))) {
    while ((cur_node = pop(fringe))) {
        // printf("Checking cost %d: ", cur_node->f);
        // print_array(cur_node->state);

        if (is_goal(cur_node->state)) {
            // printf("Solution!\n");
            free(set);
            return cur_node;
        }

        if (!lookup(set, cur_node->state)) {
            insert(&set, cur_node->state);

            num_children = (big_disks[cur_node->index0] == 1) ? 2 : 4;

            expand(cur_node, children, big_disks, h);

            for (int i = 0; i < num_children; i++) {
                //add_to_queue(fringe, children[i]);
                add(fringe, children[i]);
            }

            // printf("Queue:\n");
            // print_queue(fringe);
        }
        //printf("cost: %d\n", cur_node->f);
    }

    // printf("No solution\n");
    return NULL;
}

struct node *RBFS_wrapper(int8_t *state, int8_t big_disks[], int8_t (*h)(int8_t *, int8_t *))
{
    printf("RBFS_wrapper\n");
    int depth = 0;
    struct hashset *set = create_hashset(3);
    struct node *cur_node = make_node(state, NULL, h);

    return RBFS(cur_node, set, big_disks, h, depth);
}

struct node *RBFS(struct node *cur_node, struct hashset *set, int8_t big_disks[], int8_t (*h)(int8_t *, int8_t *), int depth)
{
    struct node *children[4] = {};
    int8_t num_children;
    int8_t best_node, low_h = 127;

    insert(&set, cur_node->state);

    if (is_goal(cur_node->state)) {
            printf("Solution!\n");
            free(set);
            return cur_node;
    }

    if(depth > 10){
        return RBFS(cur_node->parent, set, big_disks, h, depth - 1);
    }

    num_children = (big_disks[cur_node->index0] == 1) ? 2 : 4;

    expand(cur_node, children, big_disks, h);

    for (int8_t i = 0; i < num_children; ++i)
    {
        printf("low_h: %d children[i]->heuristic: %d\n", low_h, children[i]->heuristic);
        if(low_h > children[i]->heuristic && !lookup(set, children[i]->state))
        {
            low_h = children[i]->heuristic;
            best_node = i;
        }
    }
    printf("best: %d\n", children[best_node]->heuristic);

    if(low_h == 127){
        return RBFS (cur_node->parent, set, big_disks, h, depth - 1);
    }
    return RBFS(children[best_node], set, big_disks, h, depth + 1);
}
