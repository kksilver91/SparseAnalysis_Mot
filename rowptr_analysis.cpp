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
#include <assert.h>
#include <queue>

using namespace std;

int main()
{
    std::vector<signed int> BlockNum;
    std::vector<signed int> BlockPtr;

    vector<vector<unsigned int> > BlockMask(100, vector<unsigned int>(8,0));
    // rowptr_1.resize(100);
    BlockNum.resize(100);
    BlockPtr.resize(100);


    std::vector<int> rowptr_1{ 0, -2, 1, -2, 9, 10, 17, 20, 23, 24, 30, 31, 50, 52, 53, 54, 55, 56, 57, 58 };


  
    std::cout << "rowptr: "<<endl;
    for(size_t i=0; i<rowptr_1.size(); i++)
    {
        std::cout << "row " << i << ": " << rowptr_1[i] << " " << endl; 
    }
    cout << endl;

    for(size_t i=0; i<rowptr_1.size(); i++){
        BlockNum[i] = rowptr_1[i]/8;
        BlockPtr[i] = rowptr_1[i]%8;
    }  

    

    // for(size_t i=0; i<rowptr_1.size()-1; i++){
    //     if(rowptr_1[i+1]<0){
    //         Added -=rowptr_1[i+1];
    //     }   
    //     else if(BlockNum[i]==BlockNum[i+1]){
    //         for(size_t j=BlockPtr[i]; j<BlockPtr[i+1];j++){
    //             BlockMask[BlockNum[i]][j] = i;
    //         }
    //     }
    //     else{
    //         for(size_t j=BlockPtr[i]; j<8; j++)
    //         {
    //             BlockMask[BlockNum[i]][j] = i;
    //         }
    //         for(size_t j=(BlockNum[i]+1); j<(BlockNum[i+1]-1); j++)
    //         {
    //             for(size_t k=0; k<8; k++)
    //             {
    //                 BlockMask[j][k] = i;
    //             }
    //         }
    //         for(size_t j=0; j<BlockPtr[i+1]; j++)
    //         {
    //             BlockMask[BlockNum[i+1]][j] = i;
    //         }
    //     }
    // }  

    unsigned int ptr=0;
    unsigned int RowIdx=0;
    int Added=0;

    while(ptr<rowptr_1.size()-1){
        unsigned int NextPtr = ptr+1;

        if(rowptr_1[NextPtr]<0){ 
            RowIdx -= rowptr_1[NextPtr];
            NextPtr++;
        }   
        if(BlockNum[ptr]==BlockNum[NextPtr]){
            for(size_t j=BlockPtr[ptr]; j<BlockPtr[NextPtr];j++){
                BlockMask[BlockNum[ptr]][j] = RowIdx;
            }
        }
        else{
            for(size_t j=BlockPtr[ptr]; j<8; j++)
            {
                BlockMask[BlockNum[ptr]][j] = RowIdx;
            }
            for(size_t j=(BlockNum[ptr]+1); j<(BlockNum[NextPtr]); j++)
            {
                for(size_t k=0; k<8; k++)
                {
                    BlockMask[j][k] = RowIdx;
                }
            }
            for(size_t j=0; j<BlockPtr[NextPtr]; j++)
            {
                BlockMask[BlockNum[NextPtr]][j] = RowIdx;
            }
        }
        RowIdx++;
        ptr = NextPtr;
    }  

    for(size_t i=0; i<20;i++)
    {
        cout << endl;
        cout << "Block: "<< i << ": ";
        for(size_t j=0; j<8; j++)
        {
           
           cout << BlockMask[i][j] << " ";
        }
    }       

    return 0;
}   