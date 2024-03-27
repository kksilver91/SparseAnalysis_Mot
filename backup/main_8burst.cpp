// compile : g++ -O3 -o test ./sparsity_analysis_int_new.cpp
// execution: ./test ./rgg010.txt
// #include <fcntl.h>
// #include <string.h>
// #include <sys/ioctl.h>
// #include <linux/fs.h>
// #include <sys/time.h>
// #include <errno.h>
// #include <ctype.h>
// #include <fstream>
// #include <vector>
// #include <omp.h>
// #include <time.h>
// #include <sys/mman.h>
// #include <iostream>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <cstdio>
// #include <cstdlib>
// #include <string>
#include "analysis.h"
#include <iostream>
using namespace std;

#define DATA_TYPE int

#define BUFFER_SIZE 8

struct csr_ptr
{
	unsigned int* row_ptr;
	unsigned int* col_idx;
	int* nonzero;
};


int main(int argc , char ** argv)
{
    Sparse_debug DEBUG_MODULE;



    int i;
    char* txt_file = NULL;
    float sparsity;  
    unsigned long long nonzero_cnt_mat = 0;  
    int dim;
    unsigned long long num;
    unsigned int row_ptr_size, col_idx_size;
    unsigned int* row_ptr_malloc;
    int* col_nonzero_malloc;

    if(argc>1)
    {
        txt_file = argv[1];
        // test_mode = atoi(argv[2]);
    }
    printf("txt file name : %s \n", txt_file);

/////////// read txt file code ////////////////////////////////////////////////

    ifstream file(txt_file, ios::in | ios::binary); 

    file.read((char*)&dim, sizeof(unsigned int));
    file.read((char*)&row_ptr_size, sizeof(unsigned int));
    file.read((char*)&col_idx_size, sizeof(unsigned int));
    
    // file.read((int*)&dim, sizeof(int));
    // file.read((int*)&row_ptr_size, sizeof(unsigned int));
    // file.read((int*)&col_idx_size, sizeof(unsigned int));

    // printf("dim size is %u\n", dim);
    printf("matrix row_ptr_size is %d, # of nonzero is %d \n\n", row_ptr_size, col_idx_size);

    // row_ptr, col_nonzero, col, nonzero 선언 
    std::vector<unsigned int> row_ptr;
    std::vector<int> col_nonzero;
    std::vector<int> col;
    std::vector<int> nonzero;
    row_ptr.resize(row_ptr_size);
    col_nonzero.resize(2*col_idx_size);
    col.resize(col_idx_size);
    nonzero.resize(col_idx_size);


    // STAGE
    // row_ptr, col_nonzero 생성
    // *** col 생성 ***

    file.read((char*)&row_ptr[0], sizeof(row_ptr[0]) * row_ptr.size());
    file.read((char*)&col_nonzero[0], sizeof(col_nonzero[0]) * col_nonzero.size());

    for(int i = 0; i<col_idx_size; i++)
    {
        col[i] = col_nonzero[2*i];
    }
    

    // STAGE
    // *** rank0과 rank1에 들어갈 col size 결정: col_rank0_size, col_rank1_size
    // *** rank 별 col idx 매핑 ***
    std::vector<int> col_rank0;
    std::vector<int> col_rank1;
    unsigned int col_rank0_size;
    unsigned int col_rank1_size;
    col_rank0.resize(col_idx_size);
    col_rank1.resize(col_idx_size);

    // *** col_idx_size 기반으로 col_rank0_size와 col_rank1_size 결정  ***
    DEBUG_MODULE.Decide_ColRankSize(col_idx_size, &col_rank0_size, &col_rank1_size);

    // *** col_rank0과 col_rank1 생성 ***
    DEBUG_MODULE.Divide_Col_Rank(col_idx_size, col, col_rank0, col_rank1);


    
    // *****************************
    // STAGE
    // *** rowptr_rank0, rowptr_rank1 생성 ***
    std::vector<unsigned int> row_ptr_rank0;
    std::vector<unsigned int> row_ptr_rank1;
    std::vector<unsigned int> Num_RowData;
    std::vector<unsigned int> Num_RowData_rank0;
    std::vector<unsigned int> Num_RowData_rank1;
    row_ptr_rank0.resize(row_ptr_size);
    row_ptr_rank1.resize(row_ptr_size);
    Num_RowData.resize(row_ptr_size);
    Num_RowData_rank0.resize(row_ptr_size);
    Num_RowData_rank1.resize(row_ptr_size);
    
    


    for(size_t i=0; i<row_ptr_size; i++)
    {
        Num_RowData[i]=row_ptr[i+1]-row_ptr[i];  
    }


    for(int i=1; i<row_ptr_size; i++)
    {
        if((row_ptr[i]/8)%2==0) // rank1은 8의 배수
        {
            row_ptr_rank0[i] = (row_ptr[i]/8)/2*8+row_ptr[i]%8;
            row_ptr_rank1[i] = (row_ptr[i]/8)/2*8;
        }
        else
        {
            row_ptr_rank0[i] = ((row_ptr[i]/8)/2+1)*8;
            row_ptr_rank1[i] = (row_ptr[i]/8)/2*8+row_ptr[i]%8;
        }
    }
    

    for(size_t i=0; i<row_ptr_size; i++)
    {
        Num_RowData_rank0[i]=row_ptr_rank0[i+1]-row_ptr_rank0[i];  
    }

    for(size_t i=0; i<row_ptr_size; i++)
    {
        Num_RowData_rank1[i]=row_ptr_rank1[i+1]-row_ptr_rank1[i];  
    }

    
    

   
    // STAGE
    // *** divide_extent에 따라 col_idx의 배분 비율 비교***
    // col, col_rank0, col_rank1에 대해

    std::vector<int> col_idx_analysis;
    std::vector<int> col_idx_analysis_rank0;
    std::vector<int> col_idx_analysis_rank1;
    
    unsigned int cache_group_total;
    unsigned int cache_group_rank0;
    unsigned int cache_group_2_rank0;
    unsigned int cache_group_rank1;

    unsigned int divide_extent = 4;
    unsigned int col_width = row_ptr_size;
    col_idx_analysis.resize(divide_extent);
    col_idx_analysis_rank0.resize(divide_extent);
    col_idx_analysis_rank1.resize(divide_extent);
    

    DEBUG_MODULE.Analysis_ColDivision(col_idx_size, divide_extent, col_width, col, col_idx_analysis);
    DEBUG_MODULE.Analysis_ColDivision(col_rank0_size, divide_extent, col_width, col_rank0, col_idx_analysis_rank0);
    DEBUG_MODULE.Analysis_ColDivision(col_rank1_size, divide_extent, col_width, col_rank1, col_idx_analysis_rank1);


    unsigned int cache_max = 0;
    for(size_t i=0; i<divide_extent; i++)
    {
        if(cache_max < col_idx_analysis[i])
        {
           cache_max = col_idx_analysis[i];
           cache_group_total = i;
        }
    }  

    unsigned int cache_max_rank0 = 0;
    for(size_t i=0; i<divide_extent; i++)
    {
        if(cache_max_rank0 < col_idx_analysis_rank0[i])
        {
           cache_max_rank0 = col_idx_analysis_rank0[i];
           cache_group_rank0 = i;
        }
    }  

    unsigned int cache_second_max_rank0 = 0;
    for(size_t i=0; i<divide_extent; i++)
    {
        if(cache_second_max_rank0 < col_idx_analysis_rank0[i] && col_idx_analysis_rank0[i]!=cache_max_rank0)
        {
           cache_second_max_rank0 = col_idx_analysis_rank0[i];
           cache_group_2_rank0 = i;
        }
    }  

    
    unsigned int cache_max_rank1 = 0;
    for(size_t i=0; i<divide_extent; i++)
    {
        if(cache_max_rank1 < col_idx_analysis_rank1[i])
        {
           cache_max_rank1 = col_idx_analysis_rank1[i];
           cache_group_rank1= i;
        }
    }  

   
    // *** PRINT:  main cache group 결정 각각 ***
    std::cout << "cache_group_total: " << cache_group_total << endl; 
    std::cout << "cache_group_rank0: " << cache_group_rank0 << endl; 
    std::cout << "cache_group_2_rank0: " << cache_group_2_rank0 << endl; 
    std::cout << "cache_group_rank1: " << cache_group_rank1 << endl; 
    // ******************************************

    // STAGE
    // ******* PRINT FOR DEBUG ******* ///
    unsigned int sum=0;
    unsigned int sum_rank0=0;
    unsigned int sum_rank1=0;


    DEBUG_MODULE.Print_ColIdx_Analysis(2, divide_extent, col_idx_analysis, &sum);
    DEBUG_MODULE.Print_ColIdx_Analysis(0, divide_extent, col_idx_analysis_rank0, &sum_rank0);
    DEBUG_MODULE.Print_ColIdx_Analysis(1, divide_extent, col_idx_analysis_rank1, &sum_rank1);


    if(sum==(sum_rank0+sum_rank1))
    {   
       
        std::cout << "*** DEBUG: correct sum!" <<std::endl;
    }
    else
    {   
        assert(sum!=(sum_rank0+sum_rank1));
        std::cout << "*** ERROR: wrong sum!" <<std::endl;
    }
    
    // STAGE
    // proposed matrix

    unsigned int range_start_rank0;
    unsigned int range_end_rank0;
    unsigned int SecondRange_start_rank0;
    unsigned int SecondRange_end_rank0;
    unsigned int num_parallel=0;
    unsigned int num_total_block=0;
    unsigned int num_subblock=0;

    std::vector<int> col_remapped_rank0;
    std::vector<int> col_remapped_rank1;
    col_remapped_rank0.resize(col_idx_size);
    col_remapped_rank1.resize(col_idx_size);

    range_start_rank0 = cache_group_rank0 * (col_width/divide_extent+1);
    range_end_rank0 = (cache_group_rank0+1) * (col_width/divide_extent+1);
    
    SecondRange_start_rank0 = cache_group_2_rank0 * (col_width/divide_extent+1);
    SecondRange_end_rank0 = (cache_group_2_rank0+1) * (col_width/divide_extent+1);

    std::cout  << std::endl;
    std::cout << "range_start_rank0: " << range_start_rank0 << std::endl;
    std::cout << "range_end_rank0: " << range_end_rank0 << std::endl;
    std::cout  << std::endl;

    for(size_t i=0; i<row_ptr_size; i++)
    {     
        std::vector<int> Frequent_FIFO_rank0;
        std::vector<int> NonFrequent_FIFO_rank0;
        // queue<int> Frequent_FIFO_rank0;
        // queue<int> NonFrequent_FIFO_rank0;
        
        if(i!=(row_ptr_size-1))
        {
            num_total_block+=(row_ptr_rank0[i+1]-row_ptr_rank0[i])/8;
        }
        
        
        if((row_ptr_rank0[i+1]-row_ptr_rank0[i])%8 !=0 && (i != row_ptr_size))
        {
            num_subblock+=1;
        }
        // cout << "num_total_block: "<< num_total_block << endl;

        // insert frequent fifo and nonfrequent fifo
        for(size_t j=row_ptr_rank0[i]; j<row_ptr_rank0[i+1]; j++)
        {       
            if(range_start_rank0<=col_rank0[j] && range_end_rank0 >col_rank0[j])
            {
                // Frequent_FIFO_rank0.push(col_rank0[j]);
                Frequent_FIFO_rank0.push_back(col_rank0[j]);
            }
            else if(SecondRange_start_rank0<=col_rank0[j] && SecondRange_end_rank0 >col_rank0[j])
            {
                Frequent_FIFO_rank0.push_back(col_rank0[j]);
            }
            else
            {
                NonFrequent_FIFO_rank0.push_back(col_rank0[j]); 
            }
        }
        

        unsigned int Frequnet_size = Frequent_FIFO_rank0.size()/4;
        unsigned int NonFrequnet_size = NonFrequent_FIFO_rank0.size()/4;
        unsigned int Num_merged;

        if(Frequnet_size>=NonFrequnet_size)
        {
            Num_merged = NonFrequnet_size;
            if(i!=(row_ptr_size-1))
            {
                num_parallel += (row_ptr_rank0[i+1]-row_ptr_rank0[i])/8;
            }
        }
        else
        {
            Num_merged = Frequnet_size;
            if(i!=(row_ptr_size-1))
            {
                num_parallel += Frequnet_size;
            }
        }
        
        // cout << "num_parallel: "<< num_parallel << endl;

        bool merge_flag;
            // std::cout << std::endl;
            // std::cout << "Frequent_FIFO_rank0_front: "  <<Frequent_FIFO_rank0[0] << std::endl;
            // std::cout <<  "Frequent_FIFO_rank0 size: " << Frequent_FIFO_rank0.size() << std::endl;
            // Frequent_FIFO_rank0.erase(Frequent_FIFO_rank0.begin());
            // std::cout << "Frequent_FIFO_rank0_front after erase: "  <<Frequent_FIFO_rank0[0] << std::endl;
            // std::cout <<  "Frequent_FIFO_rank0 size after erase: " << Frequent_FIFO_rank0.size() << std::endl;

        if(Num_merged !=0)
        {
            // std::cout << "Num_merged: " << Num_merged << std::endl;
            // std::cout << "current row: " << i << std::endl;
            // std::cout << "current row size: " << row_ptr_rank0[i+1]-row_ptr_rank0[i] << std::endl;

            // unsigned int count_freq=0;
            // std::cout << "Freqeunt FIFO: " <<std::endl;
            // for(size_t j=0; j<Frequent_FIFO_rank0.size(); j++)
            // {
            //     std::cout << Frequent_FIFO_rank0[j] << " " ;
            //     if(count_freq%4==3)
            //     {
            //         std::cout << "|";
            //     }
            //     count_freq++;
            // }

            // std::cout << std::endl;
            // count_freq=0;
            // std::cout << "NonFreqeunt FIFO: " <<std::endl;
            // for(size_t j=0; j<NonFrequent_FIFO_rank0.size(); j++)
            // {
            //     std::cout << NonFrequent_FIFO_rank0[j] << " ";
            //     if(count_freq%4==3)
            //     {
            //         std::cout << "|";
            //     }
            //     count_freq++;
            // }
            // std::cout << std::endl;
            // std::cout <<  "Frequent_FIFO_rank0 size: " << Frequent_FIFO_rank0.size() << std::endl;
            // std::cout <<  "NONFrequent_FIFO_rank0 size: " << NonFrequent_FIFO_rank0.size() << std::endl;
            // std::cout << std::endl;
            // std::cout << std::endl;   
            merge_flag = 1; 
        }
        else
        {
            merge_flag = 0;
        }

        
        
        size_t insert_index = 0;
        
        

        if(!merge_flag) // 해당 row에 해당하는 col들은 merge 없이 그대로 매핑
        {
            for(size_t j=row_ptr_rank0[i]; j<row_ptr_rank0[i+1]; j++)
            {   
                col_remapped_rank0[j] = col_rank0[j];
            }
        }
        else
        {
            for(size_t j=row_ptr_rank0[i]; j<row_ptr_rank0[i+1]; j++)
            {   
                bool shift_flag=0;
                bool shift_finish_flag=0;
                size_t real_index = row_ptr_rank0[i]+insert_index;

                if(shift_finish_flag==0)
                {
                    if(insert_index%8<=3)
                    {
                        shift_flag = 0;
                    }
                    else
                    {
                        shift_flag = 1;
                    }

                    if(shift_flag) // insert Frequent elements
                    {
                        // col_remapped_rank0[insert_index] = Frequent_FIFO_rank0.front();
                        // Frequent_FIFO_rank0.pop();
                        col_remapped_rank0[real_index] = Frequent_FIFO_rank0[0];
                        Frequent_FIFO_rank0.erase(Frequent_FIFO_rank0.begin());
                        // std::cout << " shift flag=1"<<std::endl;
                        
                    }
                    else
                    {
                        // std::cout << " shift flag=0"<<std::endl;
                        // col_remapped_rank0[insert_index] = NonFrequent_FIFO_rank0.front();
                        // NonFrequent_FIFO_rank0.pop();
                        col_remapped_rank0[real_index] = NonFrequent_FIFO_rank0[0];
                        NonFrequent_FIFO_rank0.erase(NonFrequent_FIFO_rank0.begin());
                    }
                }
            
                insert_index++;
                
                if(Num_merged*8 <= insert_index)
                {   
                    shift_finish_flag = 1;
                    break;
                }
            }

            // merge하고 나머지 차례로 input
            for(size_t j = 0; j<Frequent_FIFO_rank0.size(); j++)
            {
                // col_remapped_rank0[insert_index] = Frequent_FIFO_rank0.front();
                // Frequent_FIFO_rank0.pop();
                size_t real_index = row_ptr_rank0[i]+insert_index;
                col_remapped_rank0[real_index] = Frequent_FIFO_rank0[j];
                insert_index++;
            }

            for(size_t j = 0; j<NonFrequent_FIFO_rank0.size(); j++)
            {
                // col_remapped_rank0[insert_index] = NonFrequent_FIFO_rank0.front();
                // NonFrequent_FIFO_rank0.pop();
                size_t real_index = row_ptr_rank0[i]+insert_index;
                col_remapped_rank0[real_index] = NonFrequent_FIFO_rank0[j];
                insert_index++;
            }

            unsigned int count=0;
            // std::cout << "col_remapped_rank0 " << std::endl;
            // for(size_t j=row_ptr_rank0[i]; j<row_ptr_rank0[i+1]; j++)
            // {
                 
            //      std::cout << col_remapped_rank0[j] << " ";
            //      if(count%4==3)
            //      {
            //         std::cout << "|";
            //      }
            //      count++;
            // }
            // std::cout << std::endl;
            // std::cout << std::endl;
        }
        //reset insert index
        insert_index = 0;
    }

    // std::cout << "col_remapped_rank0 " << std::endl;
    // for(size_t j=0; j<col_rank0_size; j++)
    // {
         
    //      std::cout << col_remapped_rank0[j] << " ";
         
    // }
    std::cout << "num_total_block:" << num_total_block << std::endl;
    std::cout << "num_sub_block:" << num_subblock << std::endl;
    std::cout << "num_parallel: " << num_parallel << std::endl;

    std::cout << "col_remapped_rank0 " << std::endl;
    for(size_t j=0; j<100; j++)
    {
         
         std::cout << col_remapped_rank0[j] << " ";
         if(j %8==7)
        {
            std::cout << " | ";
        }
    }

    // DEBUG PART

    // // *** PRINT: col, col_ranks
    // std::cout << "print col: " << std::endl;
    // for(size_t i=0; i<col_idx_size; i++)
    // {
    //     std::cout << col[i] << " ";
    //     if(i%8==7)
    //     {
    //         std::cout << " | ";
    //     }
    // }
    // std::cout << std::endl;
    // std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "print col_rank0: " << std::endl;
    for(size_t i=0; i<100; i++)
    {
        std::cout << col_rank0[i] << " ";
        if(i%8==7)
        {
            std::cout << " | ";
        }
    }
    std::cout << std::endl;
    // std::cout << std::endl;
    // std::cout << std::endl;

    // std::cout << "print col_rank1: " << std::endl;
    // for(size_t i=0; i<col_rank1_size; i++)
    // {
    //     std::cout << col_rank1[i] << " ";
    //     if(i%8==7)
    //     {
    //         std::cout << " | ";
    //     }
    // }
    // std::cout << std::endl;
    // std::cout << std::endl;
    // // ****************************************
    // // ***PRINT: Print rowptr***
    // std::cout << "print rowptr " << std::endl;
    // for(size_t i=0; i<row_ptr_size; i++)
    // {
    //     std::cout << row_ptr[i] << " ";
       
    // }
    // std::cout << std::endl;
    // std::cout << std::endl;
    // std::cout << std::endl;
    // // *************************
    // // ***PRINT: Print rowptr_rank0 and rank1***
    // std::cout << "print rowptr_rank0 " << std::endl;
    // for(size_t i=0; i<row_ptr_size; i++)
    // {
    //     std::cout << row_ptr_rank0[i] << " ";
       
    // }
    // std::cout << "" << std::endl;

    // std::cout << "print rowptr_rank1 " << std::endl;
    // for(size_t i=0; i<row_ptr_size; i++)
    // {
    //     std::cout << row_ptr_rank1[i] << " ";
       
    // }
    // std::cout << "" << std::endl;
    // std::cout << "" << std::endl;
    // // ****************************************** 

    // //  *** PRINT: print NUM_Rowdata, rank0, rank1
    // std::cout << "print Num_RowData " << std::endl;
    // for(size_t i=0; i<row_ptr_size-1; i++)
    // {
    //     std::cout << Num_RowData[i] << " ";
       
    // }
    // std::cout << "" << std::endl;

    // std::cout << "print Num_RowData_rank0 " << std::endl;
    // for(size_t i=0; i<row_ptr_size-1; i++)
    // {
    //     std::cout << Num_RowData_rank0[i] << " ";
       
    // }
    // std::cout << "" << std::endl;
    // std::cout << "print Num_RowData_rank1 " << std::endl;
    // for(size_t i=0; i<row_ptr_size-1; i++)
    // {
    //     std::cout << Num_RowData_rank1[i] << " ";
       
    // }
    // std::cout << std::endl;
    // std::cout << std::endl;
    // // ************************************************

    // STAGE
    /// 무슨 작업??
    row_ptr_malloc = (unsigned int*)calloc(row_ptr_size,sizeof(unsigned int));
    for(int i = 0; i < row_ptr_size; i++)
    {
        row_ptr_malloc[i] = row_ptr[i];
    }
    
    int col_non_malloc_size = 0;
    for(int i = 0; i < row_ptr_size-1; i++)
    {
        col_non_malloc_size += ((row_ptr_malloc[i+1]%BUFFER_SIZE)==0) ? (row_ptr_malloc[i+1]) : (((row_ptr_malloc[i+1]/BUFFER_SIZE)+1)*BUFFER_SIZE);
        // printf("row_ptr = %d \n", row_ptr_malloc[i+1]);
        // printf("row_ptr add = %d \n", ((row_ptr_malloc[i+1]%BUFFER_SIZE)==0) ? (row_ptr_malloc[i+1]) : (((row_ptr_malloc[i+1]/BUFFER_SIZE)+1)*BUFFER_SIZE));
        // printf("col_non_malloc_size = %d \n", col_non_malloc_size);
    }

    
    // printf("\n col_non_malloc_size = %d \n", col_non_malloc_size);
    col_nonzero_malloc = (int*)calloc(2*col_non_malloc_size,sizeof(int)); 

    std::vector<unsigned int>().swap(row_ptr);
    std::vector<int>().swap(col_nonzero);
    return 0;
}
