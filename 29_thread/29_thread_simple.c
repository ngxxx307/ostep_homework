#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct __counter_t
{
    int value;
    pthread_mutex_t lock;
} counter_t;

void init(counter_t *c)
{
    c->value = 0;
    pthread_mutex_init(&c->lock, NULL);
}

void increment(counter_t *c)
{
    pthread_mutex_lock(&c->lock);
    c->value++;
    pthread_mutex_unlock(&c->lock);
}

void decrement(counter_t *c)
{
    pthread_mutex_lock(&c->lock);
    c->value--;
    pthread_mutex_unlock(&c->lock);
}

int get(counter_t *c)
{
    pthread_mutex_lock(&c->lock);
    int value = c->value;
    pthread_mutex_unlock(&c->lock);
    return value;
}

// Global counter and configuration
counter_t global_counter;
int increments_per_thread;

// Thread function that increments the counter
void *thread_increment(void *arg)
{
    for (int i = 0; i < increments_per_thread; i++)
    {
        increment(&global_counter);
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
    printf("Concurrent Counter Performance Test\n");
    printf("===================================\n\n");

    // Check number of CPUs
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of CPUs available: %d\n\n", num_cpus);

    // Test configuration
    const int total_increments = 1000000; // Total increments to perform
    const int max_threads = 16;           // Maximum number of threads to test

    printf("Total increments: %d\n", total_increments);
    printf("Testing with thread counts: 1, 2, 4, 8, 16\n\n");

    printf("Threads\tTime(ms)\tOps/sec\t\tCounter Value\n");
    printf("-------\t--------\t-------\t\t-------------\n");

    // Test with different numbers of threads
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);

    for (int test = 0; test < num_tests; test++)
    {
        int num_threads = thread_counts[test];
        increments_per_thread = total_increments / num_threads;

        // Initialize counter
        init(&global_counter);

        // Create threads
        pthread_t threads[num_threads];

        // Start timing
        long long start_time = get_time_usec();

        // Launch threads
        for (int i = 0; i < num_threads; i++)
        {
            if (pthread_create(&threads[i], NULL, thread_increment, NULL) != 0)
            {
                printf("Error creating thread %d\n", i);
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
        double ops_per_sec = (double)total_increments / (duration_usec / 1000000.0);

        // Get final counter value
        int final_value = get(&global_counter);

        // Print results
        printf("%d\t%.2f\t\t%.0f\t\t%d\n",
               num_threads, duration_ms, ops_per_sec, final_value);

        // Clean up mutex
        pthread_mutex_destroy(&global_counter.lock);

        // Small delay between tests
        usleep(100000); // 100ms
    }
    return 0;
}