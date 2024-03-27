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

    // struct ioctl_info info;
    struct timespec begin, end;
    struct timespec begin1, end1;
    struct timespec begin2, end2;
    struct timespec begin3, end3;
    struct timespec begin4, end4;
    double a, b, c, d;

    // csr_updated.row_ptr(1024, 0);

//     if(argc>1)
//     {
//         txt_file = argv[1];
//         // test_mode = atoi(argv[2]);
//     }
//     printf("txt file name : %s \n", txt_file);
//     printf("test_mode : %d \n\n", test_mode);

// /////////// read txt file code ////////////////////////////////////////////////
//     ifstream file(txt_file, ios::in | ios::binary); 
//     file.read((char*)&dim, sizeof(dim));
//     file.read((char*)&width, sizeof(unsigned int));
//     file.read((char*)&height, sizeof(unsigned int));
//     info.width = width;
//     info.height = height;
//     printf("matrix width is %d, matrix height is %d \n\n", width, height);
//     num = width * height;
//     std::vector<DATA_TYPE> data;
//     data.resize(num);
//     file.read((char*)&data[0], sizeof(data[0]) * data.size());
//////////////////////////////////////////////////////////////
    // matrix = (DATA_TYPE*)calloc(num, sizeof(DATA_TYPE));

    num_of_nozero_row = (int*)calloc(height, sizeof(int));
    

    // // data -> matrix, analyse nonzero count
    // for(unsigned int i=0; i < height; i++)
    // {
    //     nonzero_cnt_row = 0;
    //     for(unsigned int j=0; j<width; j++)
    //     {
    //         if(data[i*width+j] != 0)
    //         {
    //             nonzero_cnt_mat++;
    //             nonzero_cnt_row++;
    //             // if(i>=0 && i<=width/2) {
    //                 // column_bitmap[j] = 1;
    //             // }
    //             // printf("nonzero data is %d", data[i*width+j]);
    //         }
    //         // matrix[i*width+j] = data[i*width+j];
    //     }
    //     num_of_nozero_row[i] = nonzero_cnt_row;
    // }
    
    //// DEBUG: column bitmap 
    // for(unsigned int i=0; i<width; i++) {
    //     if(column_bitmap[i] == 1) {
    //         column_bitmap_cnt += 1;
    //     }
    // }
    // printf("print bitmap\n\n");
    // for(unsigned int i=0; i<width; i++) {
    //     printf("%d ", column_bitmap[i]);
    // }
    // printf("bitmap sparsity : %lf\n\n", ((float)column_bitmap_cnt)/(width));
    // printf("\n\n");
    ////
    // std::vector<DATA_TYPE>().swap(data);




    int row_nonzero[18] = {0, };
    int row_remain[8] = {0, };
    
    // nonzero cnt for each row
    SPARSE_DEBUG.row_information_cnt(num_of_nozero_row, row_nonzero, row_remain, height);
    
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
    // csr_updated.row_ptr = (unsigned int*)calloc(info.height + 1, sizeof(unsigned int));
	// csr_updated.col_idx = (unsigned int*)calloc((info.width)*(info.height), sizeof(unsigned int));
	// csr_updated.nonzero = (DATA_TYPE*)calloc((info.width)*(info.height), sizeof(DATA_TYPE));
    // // ***** for DEBUG GEMM *****    
    // DATA_TYPE* matrix_gemm_debug;
    // DATA_TYPE* matrix_gemm_output_debug;
    // matrix_gemm_debug = (DATA_TYPE*)calloc(9, sizeof(DATA_TYPE));
    // matrix_gemm_output_debug = (DATA_TYPE*)calloc(9, sizeof(DATA_TYPE));
    // SPARSE_DEBUG.create_matrix(matrix_gemm_debug, 0.01, 3, 3);

    // printf("matrix_gemm_debug data\n\n");
    // for(int i=0; i<9; i++) {
    //     printf("%d \n", matrix_gemm_debug[i]);
    // }
    // // Esitmate GEMM 
	//     SPARSE_DEBUG.GEMM(matrix_gemm_debug, matrix_gemm_debug, matrix_gemm_output_debug ,3,3);
    // printf("matrix_gemm_debug data\n\n");
    // for(int i=0; i<9; i++) {
    //     printf("%d \n", matrix_gemm_output_debug[i]);
    // }
    // // ***** for DEBUG GEMM finished *****

    
    // ***** randomly making CSR matrix ****
    info.height = 1000;
    info.width = 1000;
    csr_updated.row_ptr = vector<unsigned int> (info.height);
    csr_updated.col_idx = vector<unsigned int> (info.height*info.width);
    csr_updated.nonzero = vector<unsigned int> (info.height*info.width);
    
    matrix = (DATA_TYPE*)calloc(info.height*info.width, sizeof(DATA_TYPE));

    // create random matrix
    SPARSE_DEBUG.create_matrix(matrix, 10, info.height, info.width); // matrix, sparsity, width, height    
    // DEBUG: print matrix
    printf("matrix\n");
    for(unsigned int i = 0; i<100; i+=1){
        printf("%d  ", matrix[i]);
    }
    printf("\n");

    // encode matrix to csr vector and estimate the execution time
    clock_gettime(CLOCK_MONOTONIC, &begin);
	SPARSE_DEBUG.create_csr_vector(info, matrix, csr_updated);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // DEBUG: print csrformat
    SPARSE_DEBUG.print_csrformat(csr_updated);


    added_data.added_row_index = (DATA_TYPE*)calloc(1024, sizeof(DATA_TYPE));
    added_data.added_column_index = (DATA_TYPE*)calloc(1024, sizeof(DATA_TYPE));
    added_data.added_nonzero = (DATA_TYPE*)calloc(1024, sizeof(DATA_TYPE));

    clock_gettime(CLOCK_MONOTONIC, &begin4);
    for(unsigned int i = 0; i< 1 ; i+=1) {
        unsigned int rowptr_index;
        csr_updated.col_idx.insert(csr_updated.col_idx.begin()+added_data.added_row_index[i], added_data.added_column_index[i]);
        csr_updated.nonzero.insert(csr_updated.nonzero.begin()+added_data.added_row_index[i], added_data.added_nonzero[i]);
        rowptr_index = added_data.added_row_index[i];
        // printf("csr_updated.row_ptr[0]= %d \n", csr_updated.row_ptr[i]);
        for(unsigned int j = rowptr_index; j<1024; j+=1){
            csr_updated.row_ptr[rowptr_index] += 1;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end4);


    // Estimate GEMM
    // clock_gettime(CLOCK_MONOTONIC, &begin1);
    // SPARSE_DEBUG.matmul(matrix, vector_in_malloc, output_mat_malloc, info.width, info.height);
    // clock_gettime(CLOCK_MONOTONIC, &end1);
 
    // Estimate SPMV
    // clock_gettime(CLOCK_MONOTONIC, &begin2);
    // SPARSE_DEBUG.matrix_multipy_vector(info, csr_malloc, vector_in_malloc, output_csr_malloc);
    // clock_gettime(CLOCK_MONOTONIC, &end2);


	// comp_out_data_type("gemv vs spmv : addr comp", output_csr_malloc, output_mat_malloc, 1, info.height);
	
	

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













