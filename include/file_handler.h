#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_FILES 				100 // The maximum number of LAMMPS data files to read 
#define MAX_CHAR_IN_HEADER_LINE 30  // The maximum number of characters in the lines representing the header in the LAMMPS data files
#define MAX_TOKENS 				14  // The maximum number of tokens representing an atom in the LAMMPS data files
#define MAX_TOKEN_LENGTH 		30  // The expected maximum length of the fields in the LAMMPS data files
#define MAX_ATOM_DIGITS			4   // The number of digits in the number of atoms, e.g. 8036 atoms has 4 digits

// Macros used for indexing the LAMMPS formatted data[] arrays
#define ATOM_ID	  		  0
#define ATOM_TYPE 		  1
#define ATOM_X	  		  2
#define ATOM_Y	  		  3
#define ATOM_Z	 		  4
#define ATOM_XU			  5
#define ATOM_YU	  		  6
#define ATOM_ZU	 		  7
#define ATOM_STRESS_1	  8
#define ATOM_STRESS_2	  9
#define ATOM_STRESS_3	  10
#define ATOM_STRESS_4	  11
#define ATOM_STRESS_5	  12
#define ATOM_STRESS_6	  13


// Modifies files[] and populates it with the names of the files, in alphanumeric sorted order, in the directory dir_name
// Returns the number of populated fields in files[]
int get_sorted_file_names(const char *dir_name, char *files[]);

// Modifies the timestamp and num_atoms with their respective values from file_name
void get_file_header(const char *file_name, int *timestamp, int *num_atoms);

// Stores the data from the line in file_name with a first element that matches id into data[]
void get_atom_data(const char *file_name, int id, double data[]);

// Writes data[] to the last line of the file with the name provided by file_name, in LAMMPS data format
void write_line_to_file(const char *file_name, double data[]);

// Writes the string-represented doubles stored in str[] to data[]
void str_to_data(char str[], double data[]);

// Checks if a file exists
// Returns 1 if a file with the name file_name exists, 0 otherwise
int file_exists(const char *file_name);

// Checks if a directory is empty
// Returns 1 if the directory with the name path is empty, 0 otherwise
int is_directory_empty(const char *path);

#endif