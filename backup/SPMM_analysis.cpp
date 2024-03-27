#include "SPMM_analysis.h"

sparse_debug::sparse_debug()
{
    
}


void sparse_debug::print_mat(char* name, DATA_TYPE* output, int width, int height)
{
    printf("output mat %s\n", name);
	unsigned int i, j;
    for(j = 0; j < height; j ++)
    {
        for(i = 0; i < width ; i++)
        {
            printf(" %d", output[j*width + i]);
        }
        printf("\n");
    }
    printf("\n");
}

int  sparse_debug::comp_out_int(char* string , int* output1, int* output2, unsigned int width, unsigned int height )
{
	unsigned int num_of_fail = 0;
    unsigned int odd_idx = 0;
    unsigned int even_idx = 0;
    unsigned int i;
	unsigned int j;
    unsigned int k;
	char test = 0;
    unsigned int buf;

    for(j = 0; j < height; j ++)
    {
        for(i = 0; i < width ; i++)
        {	        
            buf = output1[j*width + i] - (output2[j*width + i]); 
            if(buf != 0)
            {
                num_of_fail++; 
                if(test < 10)
                {
                    // printf("%s ", string);
                    printf("	at (%d, %d) : %d != %d\n", i, j, output1[j*width + i], output2[j*width + i]);
                    // printf("test fail!\n");
                    test++;
                }
            
                // return -1;
            }          
        }       
        // usleep(1);
    }

	if(test==0)
	{
		printf("%s test passed!!!\n", string);
	
	}
    else
	{
		printf("%s test failed!!!\n", string);
        printf("num of failure  %d  \n\n", num_of_fail);
	}
   
    return 0;
}

void sparse_debug::create_matrix(DATA_TYPE* matrix, unsigned int sparsity, unsigned int width,unsigned int height)
{
    for(unsigned long long i = 0; i < width*height; i++)
    {
        if(i%sparsity==0) {
            matrix[i] = i;
        }
        else {
            matrix[i] = 0;
        }
    }
}

void sparse_debug::copy_matrix(DATA_TYPE* matrix1, DATA_TYPE* matrix2, unsigned int width,unsigned int height)
{
    unsigned int i, j, k;
    DATA_TYPE buf[16] ={0,};

    for( i = 0; i < height; i++)
    {
        for( j = 0; j < width/16; j++)
        {   
            memcpy( (void *)buf, (void *) &matrix1[i*width + j*16],   64);            
            // usleep(0.01);
            memcpy( (void *) &matrix2[i*width + j*16], (void *)buf,   64); 
        }
    }      
}
void sparse_debug::matmul(DATA_TYPE* matrix, DATA_TYPE* vector, DATA_TYPE* output,  unsigned int width,  unsigned int height)
{
    unsigned int i, j;
	DATA_TYPE buf;
    for( i = 0; i < height; i++)
    {
        buf = 0;
        for(  j = 0; j < width; j++)
        {
            buf += matrix[i*width + j] * vector[j];
        }
		output[i] = buf;
    }
}

void sparse_debug::matrix_multipy_vector(struct ioctl_info info, struct csr_ptr csr, DATA_TYPE* vector, DATA_TYPE* output_csr)
{
    unsigned int row, col;
    for(row = 0; row < info.height; row++)
	{
		output_csr[row] = 0;
		for(col = csr.row_ptr[row]; col < csr.row_ptr[row+1]; col++)
		{
			output_csr[row] += vector[csr.col_idx[col]] * csr.nonzero[col];
		}
	}
}

void sparse_debug::matrix_multipy_vector_merge(struct ioctl_info info, unsigned int* row_ptr, DATA_TYPE* col_non, DATA_TYPE* vector, DATA_TYPE* output_csr)
{
    unsigned int row, col;
    for(row = 0; row < info.height; row++)
	{
		output_csr[row] = 0;
		for(col = row_ptr[row]; col < row_ptr[row+1]; col++)
		{
			output_csr[row] += vector[col_non[2*col]] * col_non[2*col+1];
		}
	}
}

void sparse_debug::create_csr_sw(struct ioctl_info info, DATA_TYPE* matrix, struct csr_ptr csr)
{
	unsigned int l_nnzValues = 0;
    unsigned int num_zero;
	unsigned int row, col;
    csr.row_ptr[0] = 0;
	for ( row = 0; row < info.height ; row++)
	{
		for (col = 0; col < info.width; col++)
		{
			if (matrix[row*info.width + col] != (DATA_TYPE)0)
			{
				csr.col_idx[l_nnzValues] = col;
				csr.nonzero[l_nnzValues++] = matrix[row*info.width + col];
			}
		}
		csr.row_ptr[row + 1] = l_nnzValues;
	}
}

void sparse_debug::create_csr_vector(struct ioctl_info info, DATA_TYPE* matrix, struct csr_ptr_vector &csr)
{
	unsigned int l_nnzValues = 0;
    unsigned int num_zero;
    // csr.row_ptr[0] = 0;
	for ( unsigned int row = 0; row < info.height ; row++)
	{
		for ( unsigned int col = 0; col < info.width; col++)
		{
			if (matrix[row*info.width + col] != 0)
			{
				csr.col_idx[l_nnzValues] = col;
				csr.nonzero[l_nnzValues] = matrix[row*info.width + col];
                l_nnzValues +=1;
			}
            else
            {
                l_nnzValues = l_nnzValues;
            }
		}
		csr.row_ptr[row] = l_nnzValues;
	}
}

void sparse_debug::row_information_cnt(int* num_of_nozero_row, int* row_nonzero, int* row_remain, unsigned int height)
{
    for(int i= 0; i<height; i++)
    {
        if(num_of_nozero_row[i]%8 == 0) {
            row_remain[0]++;
        }
        else if(num_of_nozero_row[i]%8 == 1){
            row_remain[1]++;
        }
        else if(num_of_nozero_row[i]%8 == 2){
            row_remain[2]++;
        }
        else if(num_of_nozero_row[i]%8 == 3){
            row_remain[3]++;
        }
        else if(num_of_nozero_row[i]%8 == 4){
            row_remain[4]++;
        }
        else if(num_of_nozero_row[i]%8 == 5){
            row_remain[5]++;
        }
         else if(num_of_nozero_row[i]%8 == 6){
            row_remain[6]++;
        }
         else if(num_of_nozero_row[i]%8 == 7){
            row_remain[7]++;
        }
    }
    


    for(int i = 0; i < height; i++)
    {
        if(num_of_nozero_row[i] == 0)
        {
            row_nonzero[0]++;
        }
        else if(num_of_nozero_row[i] == 1)
        {
            row_nonzero[1]++;
        }
        else if(num_of_nozero_row[i] == 2)
        {
            row_nonzero[2]++;
        }
        else if(num_of_nozero_row[i] == 3)
        {
            row_nonzero[3]++;
        }
        else if(num_of_nozero_row[i] == 4)
        {
            row_nonzero[4]++;
        }
        else if(num_of_nozero_row[i] == 5)
        {
            row_nonzero[5]++;
        }
        else if(num_of_nozero_row[i] == 6)
        {
            row_nonzero[6]++;
        }
        else if(num_of_nozero_row[i] == 7)
        {
            row_nonzero[7]++;
        }
         else if(num_of_nozero_row[i] == 8)
        {
            row_nonzero[8]++;
        }
         else if(num_of_nozero_row[i] == 9)
        {
            row_nonzero[9]++;
        }
         else if(num_of_nozero_row[i] == 10)
        {
            row_nonzero[10]++;
        }
         else if(num_of_nozero_row[i] == 11)
        {
            row_nonzero[11]++;
        }
         else if(num_of_nozero_row[i] == 12)
        {
            row_nonzero[12]++;
        }
         else if(num_of_nozero_row[i] == 13)
        {
            row_nonzero[13]++;
        }
         else if(num_of_nozero_row[i] == 14)
        {
            row_nonzero[14]++;
        }
         else if(num_of_nozero_row[i] == 15)
        {
            row_nonzero[15]++;
        }
         else if(num_of_nozero_row[i] == 16)
        {
            row_nonzero[16]++;
        }
        else
        {
            row_nonzero[17]++;
        }
    }
}


 void sparse_debug::GEMM(DATA_TYPE* matrix_1, DATA_TYPE* matrix_2, DATA_TYPE* matrix_output,  unsigned int width,  unsigned int height)
 {
    unsigned int i, j, k;
	DATA_TYPE buf;
    for( i = 0; i < height; i++)
    {
        buf = 0;
        for(  j = 0; j < width; j++)
        {
            for(k=0; k< width; k++) {
                buf += matrix_1[i*width + k] * matrix_2[k*width + j];
            }
            matrix_output[i*width + j] = buf;
            buf = 0;
        }
    }
 }

 void sparse_debug::print_csrformat(struct csr_ptr_vector &csr)
 {
    printf("rowptr\n");
    for(unsigned int i = 0; i<100; i+=1){
        printf("%d  ", csr.row_ptr[i]);
    }
    printf("\n");

    printf("col idx\n");
    for(unsigned int i = 0; i<100; i+=1){
        printf("%d  ", csr.col_idx[i]);
    }
    printf("\n");
    printf("nonzero\n");
    for(unsigned int i = 0; i<100; i+=1){
        printf("%d  ", csr.nonzero[i]);
    }
    printf("\n");
 }
