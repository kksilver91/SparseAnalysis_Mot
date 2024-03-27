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
    std::vector<int> rowptr_1(10, 8);

    std::cout << rowptr_1[0] << " "; 
    
    return 0;
}