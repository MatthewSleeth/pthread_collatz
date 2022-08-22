/*
Collatz code for CS 4380 / CS 5351

Copyright (c) 2022 Texas State University. All rights reserved.

Redistribution in source and binary forms, with or without modification, is not
permitted. Use in source or binary form, with or without modification, is only
permitted for academic use in CS 4380 and CS 5351 at Texas State University.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: Martin Burtscher
*/


#include <cstdlib>
#include <cstdio>
#include <pthread.h>
#include <sys/time.h>
#include <algorithm>

// mutex initializers
static pthread_mutex_t mutex;

// shared variables
static long threads;
static long top;
static int parallel_maxlen = 0;

static void* collatz(void* num)
{
    const long my_rank = (long)num;     // cast void* as long for use in for loop
    if (my_rank == threads - 1) {
        printf("There are %ld threads.\n", threads);
    }
    
    // determine work for each thread using cyclic assignment
    for (long x = my_rank; x < top; x += threads) {
        // compute sequence lengths
        for (long i = 1; i < top; i++) {
            long val = i;
            int len = 1;
            while (val != 1) {
                len++;
                if ((val % 2) == 0) {
                    val = val / 2;  // even
                }
                else {
                    val = 3 * val + 1;  // odd
                }
            }
            // lock thread and unlock thread for variable updates
            pthread_mutex_lock(&mutex);         
            parallel_maxlen = std::max(parallel_maxlen, len);
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}

/*
static int serial_collatz(const long num) {
    int serial_maxlen = 0;
    for (long i = 1; i < num; i++) {
        long serial_val = i;
        int serial_len = 1;
        while (serial_val != 1) {
            serial_len++;
            if ((serial_val % 2) == 0) {
                serial_val = serial_val / 2;  // even
            }
            else {
                serial_val = 3 * serial_val + 1;  // odd
            }
        }
        serial_maxlen = std::max(serial_maxlen, serial_len);
    }

    return serial_maxlen;
}
*/

int main(int argc, char* argv[])
{
    printf("Collatz pthread \n");
    pthread_mutex_init(&mutex, NULL);

    /* Get number of threads from command line */

    // check command line
    if (argc != 3) { fprintf(stderr, "USAGE: %s upper_bound\n", argv[0]); exit(-1); }
    top = atol(argv[1]);
    if (top < 1) { fprintf(stderr, "ERROR: elements must be at least 1\n"); exit(-1); }
    printf("upper_bound: %ld\n", top);
    threads = atol(argv[2]);
    if (threads < 1) { fprintf(stderr, "ERROR: threads must be at least 1\n"); exit(-1); }
    printf("threads: %ld\n", threads);

    // initialize pthread variables
    pthread_t* const handle = new pthread_t[threads - 1];

    // start time
    timeval beg, end;
    gettimeofday(&beg, NULL);

    //initialize maxlen to a meaningful value
    //parallel_maxlen = 0;

    // launch threads
    for (long thread = 0; thread < threads - 1; thread++) {
        pthread_create(&handle[thread], NULL, collatz, (void*)thread);
    }

    // work for master
    collatz((void*)(threads-1));

    // join threads
    for (long thread = 0; thread < threads - 1; thread++) {
        pthread_join(handle[thread], NULL);
    }

    // end time
    gettimeofday(&end, NULL);
    const double runtime = end.tv_sec - beg.tv_sec + (end.tv_usec - beg.tv_usec) / 1000000.0;
    printf("compute time: %.6f s\n", runtime);

    // calculate serial result
    //const int serial_maxlen = serial_collatz(top);
   
    // compare parallel to serial to verify results
    /*printf("serial_maxlen = %d\n", serial_maxlen);
    if (serial_maxlen != parallel_maxlen) {
        fprintf(stderr, "ERROR: incorrect result\n"); exit(-1);
    }
    printf("verification passed\n");*/

    

    // print result
    printf("maximum length: %ld\n", parallel_maxlen);

    // clean up
    pthread_mutex_destroy(&mutex);
    delete[] handle;
    return 0;
}

