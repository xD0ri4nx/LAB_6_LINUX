#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstring>
#include <cstdlib>

#define NUM_PROCESSES 10
#define RANGE 1000
#define MAX_NUMBER 10000

// Function to check if a number is prime
bool is_prime(int number) {
    if (number <= 1) return false;
    if (number == 2) return true;
    if (number % 2 == 0) return false;
    for (int i = 3; i <= std::sqrt(number); i += 2) {
        if (number % i == 0) return false;
    }
    return true;
}

int main() {
    int pipes[NUM_PROCESSES][2]; // Array of pipes
    pid_t pids[NUM_PROCESSES];   // Array to store child PIDs

    // Create pipes
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe failure");
            exit(EXIT_FAILURE);
        }
    }

    // Create child processes
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork failure");
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0) { // Child process
            close(pipes[i][0]); // Close read end of the pipe

            int start = i * RANGE + 1;
            int end = (i + 1) * RANGE;
            std::vector<int> primes;

            for (int number = start; number <= end; ++number) {
                if (is_prime(number)) {
                    primes.push_back(number);
                }
            }

            // Write primes to the pipe
            for (int prime : primes) {
                write(pipes[i][1], &prime, sizeof(prime));
            }

            int done = -1; // Signal end of data
            write(pipes[i][1], &done, sizeof(done));

            close(pipes[i][1]); // Close write end of the pipe
            exit(EXIT_SUCCESS); // Exit the child process
        } else {
            close(pipes[i][1]); // Close write end of the pipe in the parent
        }
    }

    // Parent process
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        int prime;
        std::cout << "Primes from process " << i + 1 << ": ";

        while (read(pipes[i][0], &prime, sizeof(prime)) > 0) {
            if (prime == -1) break; // End of data from the child
            std::cout << prime << " ";
        }

        std::cout << std::endl;
        close(pipes[i][0]); // Close read end of the pipe
    }

    // Wait for all child processes to finish
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        waitpid(pids[i], nullptr, 0);
    }

    return 0;
}
