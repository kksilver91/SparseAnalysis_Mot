#ifndef ANALYSIS_V2_H
#define ANALYSIS_V2_H

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
#include <fstream>
#include <algorithm>

using namespace std;
using std::vector;


#define DATA_TYPE int
#define BUFFER_SIZE 8

class Sparse_debug
{
    private:
        
        
    public:
        struct csr_ptr
        {
            unsigned int* row_ptr;
            unsigned int* col_idx;
            DATA_TYPE* nonzero;
        };

 

        struct ioctl_info{
            unsigned int width;
            unsigned int height;
            char* ptr;
            char data;
        };

        
       Sparse_debug(const std::vector<std::string>& filenames, const std::vector<std::string>& filenames_2, const std::vector<std::string>& filenames_3, const std::vector<std::string>& filenames_4);
        
        ~Sparse_debug()
        {
            for (auto & outfile : outfileList)
            if(outfile.is_open())
            {
                outfile.close();
            }
            for (auto & outfile : RowptroutfileList)
            if(outfile.is_open())
            {
                outfile.close();
            }
            for (auto & outfile : SequentialoutfileList)
            if(outfile.is_open())
            {
                outfile.close();
            }
            for (auto & outfile : NomergedoutfileList)
            if(outfile.is_open())
            {
                outfile.close();
            }
            
        }
        vector<ofstream> outfileList;
        vector<ofstream> NomergedoutfileList;
        vector<ofstream> RowptroutfileList;
        vector<ofstream> SequentialoutfileList;

        

};      


#endif