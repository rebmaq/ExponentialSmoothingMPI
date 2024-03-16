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

// Array of char* to store the file names
char *files[MAX_FILES];

int main(int argc, char* argv[]) {
	// interval of timesteps
	if (argc < 2) {
		printf("Missing Parameters.\n");
		return 0;
	}

	// Make sure intermediate directory is empty (it is a pain to program folder creation/deletion in this setting)
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

	// If third argument is not provided default time_step = 1
	int time_step = 1;
	if (argc == 3) time_step = atoi(argv[2]);
	int files_sz = get_sorted_file_names(argv[1], files);

	int num_atoms, timestamp;
	get_file_header(files[0], &timestamp, &num_atoms);

	// Create new subfolder in passed directory for the intermediate files
	// Set intermediate files creation flag
	int intermediate_atom_files_flags[num_atoms];
	for (int i = 0; i < num_atoms; ++i) {
		intermediate_atom_files_flags[i] = 0;
	}

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

	int current_atom = (world_rank + 1);
	double smoothing_factor = .5;
	FILE *intermediate_file;
	while (current_atom < num_atoms) {
		// Each process will work on an atom if its id % the number of processes = the current process rank
		if (world_size == 1 || current_atom % world_size == (world_rank + 1)) {

			local_path = "intermediate_atom_data/";
			char file_name[strlen("atom_intermediate_file.txt") + MAX_ATOM_DIGITS];
			sprintf(file_name, "atom%d_intermediate_file.txt", current_atom);
			char *full_path = malloc(strlen(argv[1]) + strlen(local_path) + strlen(file_name) + 1); // Can reduce mallocs by pulling outside of while loop and changing file naming
			sprintf(full_path, "%s%s%s", argv[1], local_path, file_name);
			// Read each file and skip based on the time_step
			for (int file_idx = 0; file_idx < files_sz - time_step; file_idx += time_step) {
					
				double atom_data_t[MAX_TOKENS], atom_data_t_1[MAX_TOKENS], result[MAX_TOKENS];
				get_atom_data(files[file_idx + time_step], current_atom, atom_data_t);
				// If the intermediate file exists take the next set of values to be smoothed from it
				if (file_exists(full_path)) {
					// Read the last line of the intermediate file
					intermediate_file = fopen(full_path, "r");
					char str[MAX_TOKENS * MAX_TOKEN_LENGTH];
					while (fgets(str, sizeof(str), intermediate_file) != NULL);
					fclose(intermediate_file);
					str_to_data(str, atom_data_t_1);
				}
				else {
					// Read the current atoms data from the original dataset, and write the first datapoint to the intermediate file
					get_atom_data(files[file_idx], current_atom, atom_data_t_1);
					write_line_to_file(full_path, files[file_idx], atom_data_t_1);
				}
				exponential_smoothing(smoothing_factor, atom_data_t_1, atom_data_t, result);
				write_line_to_file(full_path, files[file_idx + time_step], result);
			}
			free(full_path);
		}
		++current_atom;
	}
	MPI_Finalize();

	// write final dataset
}