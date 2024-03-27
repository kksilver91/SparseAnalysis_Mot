#ifndef SPARSE_LIBRARY_H
#define SPARSE_LIBRARY_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <sys/time.h>
#include <errno.h>
#include <ctype.h>
#include <fstream>
#include <vector>
#include <omp.h>
#include <time.h>
#include <sys/mman.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <x86intrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>

#define DATA_TYPE int
#define BUFFER_SIZE 8

class sparse_debug
{
    private:
        
        
    public:
        struct csr_ptr
        {
            unsigned int* row_ptr;
            unsigned int* col_idx;
            DATA_TYPE* nonzero;
        };

        struct added_data
        {
            DATA_TYPE* added_row_index;
            DATA_TYPE* added_column_index;
            DATA_TYPE* added_nonzero;
        };

        struct csr_ptr_vector
        {
            std::vector<unsigned int> row_ptr;
            std::vector<unsigned int> col_idx;
            std::vector<unsigned int> nonzero;
        };

        

        struct ioctl_info{
            unsigned int width;
            unsigned int height;
            char* ptr;
            char data;
        };
        sparse_debug();
        int  comp_out_int(char* string , int* output1, int* output2, unsigned int width, unsigned int height );
        void create_matrix(DATA_TYPE* matrix, unsigned int sparsity, unsigned int width,unsigned int height);
        // void create_matrix(DATA_TYPE* matrix, float sparsity, unsigned int width,unsigned int height);
        void create_csr_sw(struct ioctl_info info, DATA_TYPE* matrix, struct csr_ptr csr);
        void create_csr_vector(struct ioctl_info info, DATA_TYPE* matrix, struct csr_ptr_vector &csr);
        void copy_matrix(DATA_TYPE* matrix1, DATA_TYPE* matrix2, unsigned int width,unsigned int height);
        // gemv
        void matmul(DATA_TYPE* matrix, DATA_TYPE* vector, DATA_TYPE* output,  unsigned int width,  unsigned int height);
        // SpMV
        void matrix_multipy_vector(struct ioctl_info info, struct csr_ptr csr, DATA_TYPE* vector, DATA_TYPE* output_csr);
        void matrix_multipy_vector_merge(struct ioctl_info info, unsigned int* row_ptr, DATA_TYPE* col_non, DATA_TYPE* vector, DATA_TYPE* output_csr);
        //PRINT FUNCTION
        void print_mat(char* name, DATA_TYPE* output, int width, int height);
        void row_information_cnt(int* num_of_nozero_row, int* row_nonzero, int* row_remain, unsigned int height);
        void GEMM(DATA_TYPE* matrix_1, DATA_TYPE* matrix_2, DATA_TYPE* matrix_output,  unsigned int width,  unsigned int height);
        void print_csrformat(struct csr_ptr_vector &csr);
        // void print_matrix(DATA_TYPE* matrix);
        
        
        
};      


#endif