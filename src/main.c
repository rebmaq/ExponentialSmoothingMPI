#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include "../include/file_handler.h"

void exponential_smoothing(double smoothing_factor, double data_0[], double data_1[], double result[]) {
	for (int i = 0; i < ATOM_STRESS_1; ++i) {
		result[i] = data_1[i];
	}
	for (int i = ATOM_STRESS_1; i < MAX_TOKENS; ++i) {
		result[i] = smoothing_factor * data_1[i] + ( 1 - smoothing_factor) * data_0[i];
	}
}

// Window averaging function
void window_average(double **atom_data, int window_size, double result[]) {
	// Data that is not averaged
	for (int i = 0; i < ATOM_STRESS_1; ++i) {
		for (int j = 0; j < window_size; ++j) {
			result[i] = atom_data[j][i];
		}
	}
	// Window average stress values
	double *average = malloc(MAX_TOKENS * sizeof(double));
	for (int i = ATOM_STRESS_1; i < MAX_TOKENS; ++i) {
		average[i] = 0.0;
		for (int j = 0; j < window_size; ++j) {
			average[i] += atom_data[j][i];
		}
		result[i] = average[i] / window_size;
	}
}

// Array of char* to store the file names
char *files[MAX_FILES];

int main(int argc, char* argv[]) {
	// interval of timesteps
	if (argc < 2) {
		printf("Missing Parameters.\n");
		return 0;
	}

	// Make sure intermediate directory is empty
	char *local_path = "intermediate_atom_data/";
	char *full_path = malloc(strlen(argv[1]) + strlen(local_path) + 1);
	sprintf(full_path, "%s%s", argv[1], local_path);
	if (file_exists(full_path) && !is_directory_empty(full_path)) {
		printf("Please delete the files in the intermediate folder %s\n", full_path);
		free(full_path);
		return 0;
	}
	else if (!file_exists(full_path)) {
		printf("Please create an empty folder in the %s directory named %s\n", argv[1], local_path);
	}
	free(full_path);

	// If third argument is not provided, the default time_step = 1
	int time_step = 1;
	if (argc == 3) time_step = atoi(argv[2]);
	int files_sz = get_sorted_file_names(argv[1], files);

	// If fourth argument is not provided, the default smoothing_factor is 0.5
	double smoothing_factor = 0.5;
	if (argc == 4) {
		char *eptr;
		smoothing_factor = strtod(argv[3], &eptr);
	}
	if (!(smoothing_factor > 0 && smoothing_factor < 1)) {
		printf("Please provide a smoothing factor between 0 and 1 non-inclusive");
		return 0;
	}

	int num_atoms, timestamp;
	get_file_header(files[0], &timestamp, &num_atoms);

	// Initialize MPI processes 
	MPI_Init(&argc, &argv);

	int world_rank_buf;
	// Get the rank of the current process
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank_buf);
	uint32_t world_rank = world_rank_buf;
	int world_size_buf;
	// Get the number of processes in the world
	MPI_Comm_size(MPI_COMM_WORLD, &world_size_buf);
	uint32_t world_size = world_size_buf;

	// world_rank ranges from [0, number of processes], an atoms identifier ranges from [1, the maximum number of atoms]
	int current_atom = (world_rank + 1);
	
	FILE *intermediate_file;
	char file_name[strlen("atom_intermediate_file.txt") + MAX_ATOM_DIGITS];
	local_path = "intermediate_atom_data/";
	size_t max_full_path_len = strlen(argv[1])                      // Base directory
	                           + strlen(local_path)                 // Intermediate folder
	                           + strlen("atom")                    // Fixed part of file_name
	                           + MAX_ATOM_DIGITS                   // Max digits for atom number
	                           + strlen("_intermediate_file.txt")  // Fixed suffix
	                           + 1;                                // Null terminator
	full_path = malloc(max_full_path_len);
	while (current_atom <= num_atoms) {
		// If there is only one process, it will operate on every atom
		// Each process will work on an atom if the current atom number % the number of processes = (the current process rank + 1) % the number of processes
		// i.e. each process will operate on an atom if they are in the same congruence class mod the total number of processes
		if (world_size == 1 || current_atom % world_size == (world_rank + 1) % world_size) { // Smoothing logic
			
			sprintf(file_name, "atom%0*d_intermediate_file.txt", MAX_ATOM_DIGITS, current_atom);
			sprintf(full_path, "%s%s%s", argv[1], local_path, file_name);

			// Read each file and skip based on the time_step
			// Each loop reads the current timestamp and the next timestamp
			for (int file_idx = 0; file_idx < (files_sz - 1) - time_step; file_idx += time_step) {
					
				double atom_data_t[MAX_TOKENS], atom_data_t_1[MAX_TOKENS], result[MAX_TOKENS]; //atom_data_t is the current time stamp, atom_data_t_1 is the next time_stamp 
				get_atom_data(files[file_idx + time_step], current_atom, atom_data_t_1);
				// If the intermediate file exists take the next set of values to be smoothed from it
				if (file_exists(full_path)) {
					// Read the last line of the intermediate file
					intermediate_file = fopen(full_path, "r");
					char str[MAX_TOKENS * MAX_TOKEN_LENGTH];
					while (fgets(str, sizeof(str), intermediate_file) != NULL);
					fclose(intermediate_file);
					str_to_data(str, atom_data_t);
				}
				else {
					// Read the current atoms data from the original dataset, and write the first datapoint to the intermediate file
					get_atom_data(files[file_idx], current_atom, atom_data_t);
					// write_line_to_file(full_path, files[file_idx], atom_data_t);
					write_line_to_file(full_path, atom_data_t);
				}
				exponential_smoothing(smoothing_factor, atom_data_t, atom_data_t_1, result);
				write_line_to_file(full_path, result);
			}
		}
		++current_atom;
	}
	free(full_path);
	MPI_Finalize();
}
