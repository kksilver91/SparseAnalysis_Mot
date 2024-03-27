#include "SPMM_analysis.h"
#include <iostream>
using namespace std;


#define DATA_TYPE int
#define BUFFER_SIZE 8



int main(int argc , char ** argv)
{
    sparse_debug SPARSE_DEBUG;
    sparse_debug::csr_ptr csr_malloc;
    sparse_debug::csr_ptr_vector csr_updated;
    sparse_debug::added_data added_data;
    sparse_debug::ioctl_info info;
    
    char* txt_file = NULL;
    int test_mode = 0;
    unsigned long long nonzero_cnt_mat = 0;  
    unsigned long long column_bitmap_cnt = 0;
    int dim;
    unsigned long long num;
    unsigned int width, height;
    DATA_TYPE* matrix;
    int* num_of_nozero_matrix;
    int* num_of_nozero_row;
    int nonzero_cnt_buf;
    int nonzero[17] = {0,};
    int nonzero_cnt_row;


    // for execution time debugging
    struct timespec begin, end;
    struct timespec begin1, end1;
    struct timespec begin2, end2;
    struct timespec begin3, end3;
    struct timespec begin4, end4;
    double a, b, c, d;



    if(argc>1)
    {
        txt_file = argv[1];
        // test_mode = atoi(argv[2]);
    }
    printf("txt file name : %s \n", txt_file);
    printf("test_mode : %d \n\n", test_mode);

/////////// read txt file code ////////////////////////////////////////////////
    ifstream file(txt_file, ios::in | ios::binary); 
    file.read((char*)&dim, sizeof(dim));
    file.read((char*)&width, sizeof(unsigned int));
    file.read((char*)&height, sizeof(unsigned int));
    info.width = width;
    info.height = height;
    printf("matrix width is %d, matrix height is %d \n\n", width, height);
    num = width * height;
    std::vector<DATA_TYPE> data;
    data.resize(num);
    file.read((char*)&data[0], sizeof(data[0]) * data.size());
////////////////////////////////////////////////////////////
    matrix = (DATA_TYPE*)calloc(num, sizeof(DATA_TYPE));

    num_of_nozero_row = (int*)calloc(height, sizeof(int));
    



    // compute simulation time 
    int expected_simulation_time;
    int core2_expected_simulation_time;
    int num_granurality;
    int original_num_granurality;
    int additional_data;
    // 16burst*2rank 32 for broadcasting
    // 8burst*2rank 16 for SpMVvrow_remain
    expected_simulation_time = (width/32+width/32+height/32+(nonzero_cnt_mat+row_remain[1]*(4-1)+row_remain[2]*(4-2)+row_remain[3]*(4-3))/2)*5;
    core2_expected_simulation_time = (width/32+width/32+height/32+(nonzero_cnt_mat)/4)*5;
    num_granurality = (nonzero_cnt_mat+row_remain[1]*(7)+row_remain[2]*(6)+row_remain[3]*(5)+row_remain[4]*(4)+row_remain[5]*(3)+row_remain[6]*(2)+row_remain[7]*(1))/8;
    original_num_granurality = nonzero_cnt_mat/8;
    additional_data = row_remain[1]*(7)+row_remain[2]*(6)+row_remain[3]*(5)+row_remain[4]*(4)+row_remain[5]*(3)+row_remain[6]*(2)+row_remain[7]*(1);
    

	DATA_TYPE* vector_in_malloc;
	DATA_TYPE* output_mat_malloc;
	DATA_TYPE* output_csr_malloc;
    DATA_TYPE* csr_data_merge;

	vector_in_malloc = (DATA_TYPE*)calloc(info.width, sizeof(DATA_TYPE));
	output_mat_malloc = (DATA_TYPE*)calloc(info.height, sizeof(DATA_TYPE));
	output_csr_malloc = (DATA_TYPE*)calloc(info.height, sizeof(DATA_TYPE));
	csr_malloc.row_ptr = (unsigned int*)calloc(info.height + 1, sizeof(unsigned int));
	csr_malloc.col_idx = (unsigned int*)calloc((info.width)*(info.height), sizeof(unsigned int));
	csr_malloc.nonzero = (DATA_TYPE*)calloc((info.width)*(info.height), sizeof(DATA_TYPE));
 
    
	
	

    a = ((double)(end.tv_sec - begin.tv_sec)*1000) + ((double)((end.tv_nsec - begin.tv_nsec) / 1.0e6));  
    b = ((double)(end1.tv_sec - begin1.tv_sec)*1000) + ((double)((end1.tv_nsec - begin1.tv_nsec) / 1.0e6));  
    c = ((double)(end2.tv_sec - begin2.tv_sec)*1000) + ((double)((end2.tv_nsec - begin2.tv_nsec) / 1.0e6));  
    d = ((double)(end4.tv_sec - begin4.tv_sec)*1000) + ((double)((end4.tv_nsec - begin4.tv_nsec) / 1.0e6));  
    
    // PRINT FUNCTION
    SPARSE_DEBUG.comp_out_int("\ngemv vs spmv compare", output_csr_malloc, output_mat_malloc, 1, info.height);
    printf("num of nonzero : %lld\n\n", nonzero_cnt_mat);
    printf("sparsity : %lf\n\n", ((float)nonzero_cnt_mat)/(width*height));
    printf("\nnum of rows which have corresponding nonzero\n");
    for(int i = 0; i < 17; i++)
    {
        printf("num of row for nonzero %d : %d\n", i, row_nonzero[i]);
    }
    printf("num of row for over 16 : %d\n", row_nonzero[17]);

    for(int i = 0; i < 8; i++)
    {
        printf("num of row for remain nonzero %d : %d\n", i, row_remain[i]);
    }

    printf("\nNormal mode performance\n");
    printf("    csr encodeing speed    %lfms\n", a);
    printf("    SPMV speed             %lfms\n", c);
    printf("    GEMM speed             %lfms\n", b);
    printf("    CSR update speed            %lfms\n", d);
    printf("expected simulation speed: %dns\n", expected_simulation_time); 
    printf("core2_expected_simulation_timen speed: %dns\n", core2_expected_simulation_time); 
    printf("num_granularity : %d\n", num_granurality); 
    printf("original_num_granularity : %d\n", original_num_granurality); 
    printf("additional_data : %d\n", additional_data); 
    // printf("    kwang speed             %lfms\n", d);
    free(matrix);
    free(num_of_nozero_row);
    free(vector_in_malloc); 
	free(output_mat_malloc);
	free(output_csr_malloc);
	free(csr_malloc.row_ptr); 
	free(csr_malloc.col_idx );
	free(csr_malloc.nonzero );
    csr_updated.row_ptr.clear();
    csr_updated.col_idx.clear();
    csr_updated.nonzero.clear();
    // free(csr_updated.row_ptr); 
	// free(csr_updated.col_idx );
	// free(csr_updated.nonzero );
    
//////////////////////////////////////////////////////////////////////////////////////////////

    return 0;
}













