#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define KB 1024
#define MB (KB * KB)
#define DEFAULT_START (1 * MB)
#define DEFAULT_END (10 * MB)
#define DEFAULT_STEP (1 * MB)
#define DEFAULT_ITER 1000
#define MAX_FILENAME 512

int main(int argc, char *argv[]) {
    int proc_rank, proc_count;
    double elapsed_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_count);
    
    if (proc_count != 2) {
        if (proc_rank == 0) {
            fprintf(stderr, "Error: This program requires exactly 2 processes\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    char output_file[MAX_FILENAME] = "measurements.txt";
    
    if (argc > 1) {
        if (strlen(argv[1]) >= MAX_FILENAME) {
            if (proc_rank == 0) {
                fprintf(stderr, "Error: Filename is too long\n");
            }
            MPI_Finalize();
            return 1;
        }
        strncpy(output_file, argv[1], MAX_FILENAME - 1);
        output_file[MAX_FILENAME - 1] = '\0';
    }
    
    int min_size = DEFAULT_START;
    int max_size = DEFAULT_END;
    int size_step = DEFAULT_STEP;
    int iterations = DEFAULT_ITER;
    
    if (proc_rank == 0) {
        printf("Benchmark configuration:\n");
        printf("  Min size: %d bytes (%.2f MB)\n", min_size, (float)min_size / MB);
        printf("  Max size: %d bytes (%.2f MB)\n", max_size, (float)max_size / MB);
        printf("  Step: %d bytes (%.2f MB)\n", size_step, (float)size_step / MB);
        printf("  Iterations per size: %d\n", iterations);
        printf("  Output file: %s\n", output_file);
        printf("----------------------------------------\n");
    }
    
    unsigned char *send_buffer = (unsigned char*)malloc(max_size);
    unsigned char *recv_buffer = (unsigned char*)malloc(max_size);
    
    if (!send_buffer || !recv_buffer) {
        if (proc_rank == 0) {
            fprintf(stderr, "Error: Failed to allocate memory\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    for (int i = 0; i < max_size; i++) {
        send_buffer[i] = (unsigned char)(i % 256);
    }
    
    MPI_Request reqs[2];
    char result_buffer[1000000] = {0};
    char line_buffer[256];
    int result_offset = 0;
    
    for (int current_size = min_size; current_size <= max_size; current_size += size_step) {
        MPI_Barrier(MPI_COMM_WORLD);
        
        double start_time = MPI_Wtime();
        
        if (proc_rank == 0) {
            for (int iter = 0; iter < iterations; iter++) {
                MPI_Isend(send_buffer, current_size, MPI_UNSIGNED_CHAR, 1, 0, MPI_COMM_WORLD, &reqs[0]);
                MPI_Irecv(recv_buffer, current_size, MPI_UNSIGNED_CHAR, 1, 0, MPI_COMM_WORLD, &reqs[1]);
                MPI_Waitall(2, reqs, MPI_STATUS_IGNORE);
            }
        } else {
            for (int iter = 0; iter < iterations; iter++) {
                MPI_Isend(send_buffer, current_size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, &reqs[0]);
                MPI_Irecv(recv_buffer, current_size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, &reqs[1]);
                MPI_Waitall(2, reqs, MPI_STATUS_IGNORE);
            }
        }
        
        double end_time = MPI_Wtime();
        double avg_time = (end_time - start_time) / iterations;
        
        if (proc_rank == 0) {
            double bandwidth = (2.0 * current_size) / (avg_time * 1024 * 1024);
            printf("Size: %8d bytes | Time: %8.2f us | Bandwidth: %6.2f MB/s\n", 
                   current_size, avg_time * 1e6, bandwidth);
            
            int written = snprintf(line_buffer, sizeof(line_buffer), 
                                  "%d\t%.10f\n", current_size, avg_time);
            
            if (result_offset + written < (int)sizeof(result_buffer)) {
                strcpy(result_buffer + result_offset, line_buffer);
                result_offset += written;
            }
        }
    }
    
    if (proc_rank == 0) {
        FILE *f = fopen(output_file, "w");
        if (f) {
            fputs(result_buffer, f);
            fclose(f);
            printf("\nResults saved to: %s\n", output_file);
        } else {
            fprintf(stderr, "Error: Cannot open file %s for writing\n", output_file);
        }
    }
    
    free(send_buffer);
    free(recv_buffer);
    MPI_Finalize();
    return 0;
}
