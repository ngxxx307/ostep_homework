#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

// =================================================================
// Configuration
// =================================================================

const int TOTAL_INCREMENTS = 10000000; // Increased for a more meaningful test
const int THRESHOLD = 16;              // Update global count after this many local increments

// Dynamically determined number of CPUs
int NUMCPUS;

// =================================================================
// Sloppy Counter (Scalable Approximate Counter)
// =================================================================

typedef struct __counter_t
{
    int global;             // global count
    pthread_mutex_t glock;  // global lock
    int *local;             // per-CPU local count
    pthread_mutex_t *llock; // per-CPU local locks
    int threshold;          // update frequency
} counter_t;

// init: record threshold, init locks, init values
void init(counter_t *c, int threshold)
{
    c->threshold = threshold;
    c->global = 0;
    pthread_mutex_init(&c->glock, NULL);

    // Allocate memory for local counters and locks based on NUMCPUS
    c->local = malloc(sizeof(int) * NUMCPUS);
    c->llock = malloc(sizeof(pthread_mutex_t) * NUMCPUS);
    if (c->local == NULL || c->llock == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for local counters/locks\n");
        exit(1);
    }

    for (int i = 0; i < NUMCPUS; i++)
    {
        c->local[i] = 0;
        pthread_mutex_init(&c->llock[i], NULL);
    }
}

// update: update local count and transfer to global if threshold is reached
void update(counter_t *c, int threadID, int amt)
{
    int cpu = threadID % NUMCPUS;
    pthread_mutex_lock(&c->llock[cpu]);
    c->local[cpu] += amt;
    if (c->local[cpu] >= c->threshold)
    {
        // transfer to global
        pthread_mutex_lock(&c->glock);
        c->global += c->local[cpu];
        pthread_mutex_unlock(&c->glock);
        c->local[cpu] = 0;
    }
    pthread_mutex_unlock(&c->llock[cpu]);
}

// get_exact: return an exact count by locking everything. Slow. For verification only.
int get_exact(counter_t *c)
{
    int val = 0;
    pthread_mutex_lock(&c->glock);
    val = c->global;
    pthread_mutex_unlock(&c->glock);

    // Add any remaining values from local counters
    for (int i = 0; i < NUMCPUS; i++)
    {
        pthread_mutex_lock(&c->llock[i]);
        val += c->local[i];
        pthread_mutex_unlock(&c->llock[i]);
    }
    return val;
}

// destroy: clean up memory and mutexes
void destroy(counter_t *c)
{
    pthread_mutex_destroy(&c->glock);
    for (int i = 0; i < NUMCPUS; i++)
    {
        pthread_mutex_destroy(&c->llock[i]);
    }
    free(c->local);
    free(c->llock);
}

// =================================================================
// Performance Testing Framework
// =================================================================

counter_t global_counter;
int increments_per_thread;

// Thread function that increments the counter
void *thread_worker(void *arg)
{
    // The argument is the thread's unique ID
    long threadID = (long)arg;
    for (int i = 0; i < increments_per_thread; i++)
    {
        // We pass the threadID to the update function
        update(&global_counter, threadID, 1);
    }
    return NULL;
}

// Function to get current time in microseconds
long long get_time_usec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec;
}

int main()
{
    printf("Sloppy Concurrent Counter Performance Test\n");
    printf("==========================================\n\n");

    // Check number of CPUs
    NUMCPUS = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of CPUs available: %d\n", NUMCPUS);
    printf("Sloppy threshold set to: %d\n\n", THRESHOLD);

    // Test configuration
    const int max_threads = 16;

    printf("Total increments: %d\n", TOTAL_INCREMENTS);
    printf("Testing with thread counts: 1, 2, 4, 8, 16\n\n");

    printf("Threads\tTime(ms)\tOps/sec\t\tCounter Value\n");
    printf("-------\t--------\t---------\t-------------\n");

    // Test with different numbers of threads
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);

    for (int test = 0; test < num_tests; test++)
    {
        int num_threads = thread_counts[test];
        if (TOTAL_INCREMENTS % num_threads != 0)
        {
            // Ensure increments are distributed evenly for a fair test
            increments_per_thread = TOTAL_INCREMENTS / num_threads + 1;
        }
        else
        {
            increments_per_thread = TOTAL_INCREMENTS / num_threads;
        }

        long long effective_total_increments = (long long)increments_per_thread * num_threads;

        // Initialize counter
        init(&global_counter, THRESHOLD);

        // Create threads
        pthread_t threads[num_threads];

        // Start timing
        long long start_time = get_time_usec();

        // Launch threads, passing a unique ID to each
        for (long i = 0; i < num_threads; i++)
        {
            if (pthread_create(&threads[i], NULL, thread_worker, (void *)i) != 0)
            {
                printf("Error creating thread %ld\n", i);
                return 1;
            }
        }

        // Wait for all threads to complete
        for (int i = 0; i < num_threads; i++)
        {
            pthread_join(threads[i], NULL);
        }

        // End timing
        long long end_time = get_time_usec();
        long long duration_usec = end_time - start_time;
        double duration_ms = duration_usec / 1000.0;

        // Calculate operations per second
        double ops_per_sec = (double)effective_total_increments / (duration_usec / 1000000.0);

        // Get final counter value using the exact method for verification
        int final_value = get_exact(&global_counter);

        // Print results
        printf("%d\t%.2f\t\t%.0f\t\t%d\n",
               num_threads, duration_ms, ops_per_sec, final_value);

        // Clean up counter resources
        destroy(&global_counter);

        // Small delay between tests
        usleep(100000); // 100ms
    }
    return 0;
}