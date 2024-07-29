# Data file path and new directory name
DATA_DIR=/path/to/directory/
dirname=intermediate_atom_data

new_dir=$DATA_DIR$dirname

# Make new directory
mkdir -p -- "$new_dir"

# Check if directory creation was successful
if [ -d "$new_dir" ]; then
    echo "Directory '$new_dir' created successfully."
else
    echo "Failed to create directory '$new_dir'."
    exit 1
fi

make 
mpirun -np 2 --oversubscribe bin/main dataset/
