/*
 * Copyright (c) 2018      Intel, Inc. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 */
#include <pmix_common.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <event.h>

typedef struct event_base debug_event_base_t;
typedef struct event debug_event_t;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    volatile bool active;
    pmix_status_t status;
} mylock_t;

#define DEBUG_CONSTRUCT_LOCK(l)                     \
    do {                                            \
        pthread_mutex_init(&(l)->mutex, NULL);      \
        pthread_cond_init(&(l)->cond, NULL);        \
        (l)->active = true;                         \
        (l)->status = PMIX_SUCCESS;                 \
    } while(0)

#define DEBUG_DESTRUCT_LOCK(l)              \
    do {                                    \
        pthread_mutex_destroy(&(l)->mutex); \
        pthread_cond_destroy(&(l)->cond);   \
    } while(0)

#define DEBUG_WAIT_THREAD(lck)                                      \
    do {                                                            \
        pthread_mutex_lock(&(lck)->mutex);                          \
        while ((lck)->active) {                                     \
            pthread_cond_wait(&(lck)->cond, &(lck)->mutex);         \
        }                                                           \
        pthread_mutex_unlock(&(lck)->mutex);                        \
    } while(0)

#define DEBUG_WAKEUP_THREAD(lck)                        \
    do {                                                \
        pthread_mutex_lock(&(lck)->mutex);              \
        (lck)->active = false;                          \
        pthread_cond_broadcast(&(lck)->cond);           \
        pthread_mutex_unlock(&(lck)->mutex);            \
    } while(0)

#define DEBUG_THREADSHIFT(r, c)                     \
 do {                                               \
    event_assign(&((r)->ev), debug_evbase,          \
                      -1, EV_WRITE, (c), (r));      \
    event_active(&((r)->ev), EV_WRITE, 1);          \
} while (0)

#define PMIX_TEST_PRINT(a)          \
    if (verbose) {                  \
        fprintf(stderr, a);         \
    }

#define PMIX_TEST_THREAD_CANCELLED   ((void*)1);

int pmix_test_thread_start(debug_event_base_t **evbase);
void pmix_test_thread_stop(void);
