#include "../include/file_handler.h"


int compare(const void *a, const void *b) {
	return strcmp(*(const char **)a, *(const char **)b);
}

int get_sorted_file_names(const char *dir_name, char *files[]){
	DIR *dir;
	struct dirent *ent;
	int file_count = 0;
	// Open the directory
	if ((dir = opendir(dir_name)) != NULL) {
		// Read each file in the directory
		while ((ent = readdir(dir)) != NULL) {
			// Skip ./ and ../ 
			if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
				// Store <dir_name><file_name> into files[] (dir_name is expected to include a / at the end)
				char *full_path = malloc(strlen(dir_name) + strlen(ent->d_name) + 1);
				sprintf(full_path, "%s%s", dir_name, ent->d_name);
				files[file_count] = strdup(full_path);
				free(full_path);
				++file_count;
			}
		}
		closedir(dir);
	}

	// Sort the files array based on the compare function
	qsort(files, file_count, sizeof(char *), compare);
	return file_count;
}

void get_file_header(const char* file_name, int *timestamp, int *num_atoms) {
	FILE *fp;
	// Open the file
	if ((fp = fopen(file_name, "r")) != NULL) {
		int line_no = 0;
		char *line = NULL;
		size_t len = 0;
		ssize_t read;
		// Read the file line by line
		while ((read = getline(&line, &len, fp) != -1)) {
			// If we have passed the header, break
			if (line_no > 4) {
				break;
			}
			// Save the timestamp
			else if (line_no == 1) {
				*timestamp = ((int) atoi(line));
			}
			// Save the number of atoms
			else if (line_no == 3) {
				*num_atoms = ((int) atoi(line));
			} 
			++line_no;
		}
		fclose(fp);
	}
}

void get_atom_data(const char *file_name, int id, double data[]) {
	FILE *fp;
	// Open the file, file_name
	if ((fp = fopen(file_name, "r")) != NULL) {		
		int line_no = 0;
		char *line = NULL;
		size_t len = 0;
		ssize_t read;

		char * token;
		int token_count = 0;
		// Read each line
		while ((read = getline(&line, &len, fp) != -1)) {
			// Skip the header lines
			if (line_no < 9) {
				++line_no;
				continue;
			}
			// Tokenize each line
			token = strtok(line, " ");
			// If the first element of the current line matches id, tokenize the entire line and save it
			if ((int)atoi(line) == id) {
				while (token != NULL && token_count < MAX_TOKENS) {
					// Convert strings to doubles
					data[token_count] = strtod(token, NULL);
					++token_count;
					token = strtok(NULL, " ");
				}
			}
			++line_no;
		}
		fclose(fp);
	}
}

void write_line_to_file(const char *file_name, double data[]) {
	char buf[MAX_TOKENS * MAX_TOKEN_LENGTH];
	FILE *fp;
	if((fp = fopen(file_name, "a")) != NULL) {
		fprintf(fp, "%d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
				(int)(data[ATOM_ID]), (int)(data[ATOM_TYPE]), data[ATOM_X], data[ATOM_Y], data[ATOM_Z], data[ATOM_XU], data[ATOM_YU], data[ATOM_ZU],
				data[ATOM_STRESS_1], data[ATOM_STRESS_2], data[ATOM_STRESS_3], data[ATOM_STRESS_4], data[ATOM_STRESS_5], data[ATOM_STRESS_6]);
	}
	fclose(fp);
}

void str_to_data(char str[], double data[]) {
	char *newline = strchr(str, '\n');
	if (newline) *newline = 0;

	int i = 0;
	char *temp;
	temp = strtok(str, " ");
	data[i] = strtod(temp, NULL);
	while(temp != NULL) {
		temp = strtok(NULL, " ");
		if (temp == 0) break;
		data[++i] = strtod(temp, NULL);
	}
}

int file_exists(const char *file_name) {
 	struct stat buffer;   
 	return (stat (file_name, &buffer) == 0);
}

int is_directory_empty(const char *path) {
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(path);
    if (dir == NULL) {
        perror("Error opening directory");
        return 0; // Unable to open directory
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            closedir(dir);
            return 0; // Directory is not empty
        }
    }

    // Close the directory
    closedir(dir);

    return 1; // Directory is empty
}