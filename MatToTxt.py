import scipy.io
import numpy as np

def convert_mat_to_txt(mat_file, txt_file):
    # Load the .mat file
    mat = scipy.io.loadmat(mat_file)

    # Extract the matrix data
    matrix = mat['Problem']['A'][0][0]

    # Extract row pointers, column indices, and values
    row_ptr = matrix.indptr
    col_idx = matrix.indices
    values = matrix.data

    # Write to the text file
    with open(txt_file, 'w') as f:
        # Write dimensions
        dim = matrix.shape
        f.write(f"{dim[0]} {dim[1]}\n")

        # Write row pointer size and column index size
        f.write(f"{len(row_ptr)} {len(col_idx)}\n")

        # Write row pointers
        for ptr in row_ptr:
            f.write(f"{ptr}\n")

        # Write column indices
        for idx in col_idx:
            f.write(f"{idx}\n")

        # Write values
        for val in values:
            f.write(f"{val}\n")

    print(f"Converted {mat_file} to {txt_file}")

# Example usage
convert_mat_to_txt('1138_bus.mat', '1138_bus.txt')