#include "analysis.h"
#include <iostream>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <map>
#include<chrono>
#include <fstream>
#include <cassert>
#include<set>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <omp.h>
// #include <mkl.h>


using namespace std;

#define DATA_TYPE int

#define BUFFER_SIZE 8

std::mutex mtx; // Mutex for synchronizing access to match_counts
std::unordered_map<std::string, std::string> settings;


struct csr_ptr
{
	unsigned int* row_ptr;
	unsigned int* col_idx;
	int* nonzero;
};

// ini ������ �а� �������� �Ľ��ϴ� �Լ�
void parseIniFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open " << filename << std::endl;
        return;
    }

    std::string line;
    std::string currentSection;
    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty() || line[0] == ';')
            continue;

        if (line[0] == '[' && line[line.length() - 1] == ']') {
            currentSection = line.substr(1, line.length() - 2);
        } else {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);
                settings[currentSection + '.' + key] = value;
            }
        }
    }
}

vector<int> findTopEntries(const vector<int>& row, int n) {
    vector<pair<int, int>> indexed_values;

    for (int i = 0; i < row.size(); ++i) {
        indexed_values.push_back({row[i], i});
    }

    // ???? ?????? ??????????????? ??????
    sort(indexed_values.begin(), indexed_values.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        return a.first > b.first;
    });

    // ?????? n???? ?????????? 
    vector<int> top_indices;
    for (int i = 0; i < n && i < indexed_values.size(); ++i) {
        top_indices.push_back(indexed_values[i].second);
    }

    return top_indices;
}

vector<int> findBottomEntries(const vector<int>& row, int n) {
    vector<pair<int, int>> indexed_values;

    for (int i = 0; i < row.size(); ++i) {
        indexed_values.push_back({row[i], i});
    }

    // ???? ?????? ??????????????? ??????
    sort(indexed_values.begin(), indexed_values.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        return a.first > b.first;
    });

    // vector<int> bottom_indices;
    // for (int i = indexed_values.size() - 1; i >= indexed_values.size() - n && i >= 0; --i) {
    //     bottom_indices.push_back(indexed_values[i].second); 
    // }

    vector<int> bottom_indices;
    for (int i = n; i < indexed_values.size(); ++i) {
        bottom_indices.push_back(indexed_values[i].second);
    }

    return bottom_indices;
}
// ?????? ????????? ???? ??????????????? ????????? ???, ?????? divide_extent/







void printPairs(const std::vector<std::pair<int, int>>& pairs) {
    std::cout << "{ "; 
    for (const auto& pair : pairs) {
        std::cout << "(" << pair.first << "," << pair.second << "), ";
        // std::cout << pair.first+pair.second << " ";
    }
    std::cout << "}\n";
}


// ????? ???????? ??? ???????? ?????? ?????????.
void findPairs(std::vector<int>& nums, std::vector<std::pair<int, int>>& pairs, vector<vector<pair<int,int>>>& pairs_vec, vector<vector<int>>& pairs_sum, int startIndex = 0) {
    
    if (nums.size() - startIndex == 2) { // ?????? ???????? ??? ??????,
        pairs.push_back({nums[startIndex], nums[startIndex + 1]}); // ????????? ???????? ??????????.
        // printPairs(pairs); // ?????????????? ????????? ????????????.
        pairs_vec.push_back(pairs);
        
        vector<int> pairsum_values;
        for (size_t i=0; i<pairs.size(); i++) 
        {   
            // cout << pairs[i].first+pairs[i].second << " " ;
            pairsum_values.push_back(pairs[i].first+pairs[i].second);
        }
        pairs_sum.push_back(pairsum_values);
        // cout << endl;
        
        
        pairs.pop_back(); // ??????????? ???????? ?????? ?????????????.
    } else { // ????? ????????,
        for (int i = startIndex + 1; i < nums.size(); ++i) { // startIndex ????????? ??? ????????? ?????????,
            pairs.push_back({nums[startIndex], nums[i]}); // startIndex??? ?????? ???????? ?????? ??????????.
            std::swap(nums[startIndex + 1], nums[i]); // startIndex ????????? ????????? ?????? ???????? ????????????.
            findPairs(nums, pairs, pairs_vec, pairs_sum, startIndex + 2); // startIndex?? 2??? ?????? ??????? ???????????????.
            std::swap(nums[startIndex + 1], nums[i]); // ????????? ???????? ?????????????.
            pairs.pop_back(); // ??????????? ???????? ?????? ?????????????.
        }
    }
}

void csrToCsc(const std::vector<int>& row_ptr, const std::vector<int>& col, const std::vector<int>& nonzero,
              std::vector<int>& col_ptr, std::vector<int>& row, std::vector<int>& val) {
    int n = row_ptr.size() - 1; // ����� �� ����
    int m = *std::max_element(col.begin(), col.end()) + 1; // ����� �� ����
    col_ptr.resize(m + 1, 0);
    
    // �� ������ ���
    for (size_t i = 0; i < col.size(); ++i) {
        ++col_ptr[col[i] + 1];
    }
    for (int i = 1; i <= m; ++i) {
        col_ptr[i] += col_ptr[i - 1];
    }
    
    row.resize(col.size());
    val.resize(nonzero.size());
    
    for (int i = 0; i < n; ++i) {
        for (int j = row_ptr[i]; j < row_ptr[i + 1]; ++j) {
            int c = col[j];
            int dest = col_ptr[c]++;
            row[dest] = i;
            val[dest] = nonzero[j];
        }
    }
    
    // col_ptr ����
    for (int i = m; i > 0; --i) {
        col_ptr[i] = col_ptr[i - 1];
    }
    col_ptr[0] = 0;
}

std::map<int, int> calculateFixedRangeDistribution(const std::vector<int>& col_ptr, int range_size) {
    std::map<int, int> distribution;
    int n = col_ptr.size() - 1; // ��ü ���� ����

    // �� ���� non-zero ���� ���
    for (int i = 0; i < n; ++i) {
        int count = col_ptr[i + 1] - col_ptr[i];
        int range_index = count / range_size; // ������ ���� �� ��ġ ����
        distribution[range_index]++;
    }

    return distribution;
}

int findMaxNonZero(const std::vector<int>& row_ptr) {
    int maxNonZero = 0;
    // �� ���� non-zero ���� ���
    for (size_t i = 0; i < row_ptr.size() - 1; ++i) {
        int nonZeroCount = row_ptr[i + 1] - row_ptr[i];
        if (nonZeroCount > maxNonZero) {
            maxNonZero = nonZeroCount;
        }
    }
    return maxNonZero;
}

std::map<int, int> countRowsByNonZero(const std::vector<int>& row_ptr) {
    std::map<int, int> non_zero_counts;
    
    // non-zero ������ 0���� row���� 20���� row���� �ʱ�ȭ
    for (int i = 0; i <= 20; ++i) {
        non_zero_counts[i] = 0;
    }

    // row_ptr�� ����Ͽ� �� row�� non-zero ���� ���
    for (size_t i = 0; i < row_ptr.size() - 1; ++i) {
        int non_zero_count = row_ptr[i + 1] - row_ptr[i];
        // 0������ 20�� ���̿� �ش��ϴ� ��츸 ī��Ʈ
        if (non_zero_count >= 0 && non_zero_count <= 20) {
            non_zero_counts[non_zero_count]++;
        }
    }

    return non_zero_counts;
}

void printNumericHistogram(const std::vector<int>& data, int minRange, int maxRange, int binSize) {
    std::map<int, int> histogram;
    // �����͸� ���� ������׷� ���� ����
    for (int value : data) {
        if (value >= minRange && value <= maxRange) {
            int bin = (value - minRange) / binSize;
            histogram[bin]++;
        }
    }

    // ������׷� ���
    for (const auto& bin : histogram ) {
        std::cout << "[" << (bin.first * binSize + minRange) << " - " << ((bin.first + 1) * binSize + minRange - 1) << "]: ";
        std::cout << bin.second << std::endl; // �� ��� ���ڷ� ���
    }
}

void calculateOverlap(const std::vector<std::map<int, int>>& partialSum) {
    int n = partialSum.size();
    // �� �� �ֿ� ���� ��ġ�� Ű�� ������ ����մϴ�.
    int overlapCount = 0;
    for (int i = 0; i < n - 1; ++i) {
        for (int j = i + 1; j < n; ++j) {
            // ù ��° ���� Ű�� ��ȸ�ϸ� �� ��° �ʿ��� �����ϴ��� Ȯ���մϴ�.
            for (const auto& keyValuePair : partialSum[i]) {
                if (partialSum[j].find(keyValuePair.first) != partialSum[j].end()) {
                    ++overlapCount;
                }
            }
        }
    }
    std::cout << "Overlap: "<< overlapCount << std::endl;
}

int calculateTotalOverlap(const std::vector<std::map<int, int>>& partialSum) {
    std::set<int> uniqueKeys;
    // ��� �ʿ��� Ű�� �����Ͽ� ���� Ű ��Ʈ�� �����մϴ�.
    for (const auto& map : partialSum) {
        for (const auto& keyValuePair : map) {
            uniqueKeys.insert(keyValuePair.first);
        }
    }

    int totalOverlap = 0;
    // ���� Ű ��Ʈ�� ��ȸ�ϸ� �� Ű�� �� ���� �ʿ� �����ϴ��� ����մϴ�.
    for (const int key : uniqueKeys) {
        int count = 0;
        for (const auto& map : partialSum) {
            if (map.find(key) != map.end()) {
                ++count;
            }
        }
        // Ű�� �� �� �̻��� �ʿ� �����ϸ� ��ģ�ٰ� �����մϴ�.
        if (count > 1) {
            ++totalOverlap;
        }
    }

    return totalOverlap;
}

int calculateAdjacentOverlap(const std::vector<std::map<int, int>>& partialSum) {
    int totalOverlap = 0;
    // partialSum�� �� ��Ҹ� ��ȸ�ϸ� �ٷ� ���� ��ҿ� ������ ���� Ű-�� ���� ã���ϴ�.
    for (size_t i = 0; i < partialSum.size() - 1; ++i) {
        int overlapCount = 0;
        // ���� ���� Ű-�� ���� ��ȸ�ϸ� ���� �ʿ��� ������ ���� Ű-�� ���� �����ϴ��� Ȯ���մϴ�.
        for (const auto& keyValuePair : partialSum[i]) {
            auto it = partialSum[i + 1].find(keyValuePair.first);
            if (it != partialSum[i + 1].end() && it->second == keyValuePair.second) {
                ++overlapCount;
            }
        }
        totalOverlap += overlapCount;
    }

    return totalOverlap;
}

void countNonzerosPerColumnSection(const std::vector<int>& csc_col_ptr, const std::vector<int>& csc_row, const std::vector<int>& csc_val, int numberOfSections) {
    int totalColumns = csc_col_ptr.size() - 1; // ��ü ���� ����
    int columnsPerSection = totalColumns / numberOfSections; // ������ ���� ����
    if (totalColumns % numberOfSections != 0) ++columnsPerSection; // �������� �ִ� ��� ������ ���� ���� ����

    std::vector<int> nonzerosPerSection(numberOfSections, 0);

    for (int section = 0; section < numberOfSections; ++section) {
        int startCol = section * columnsPerSection;
        int endCol = std::min((section + 1) * columnsPerSection, totalColumns);

        std::cout << "Section " << section << " columns: [" << startCol << ", " << endCol - 1 << "]" << std::endl;

        for (int col = startCol; col < endCol; ++col) {
            nonzerosPerSection[section] += csc_col_ptr[col + 1] - csc_col_ptr[col];
        }
    }

    // ��� ���
    int total_nonzero_sum=0;
    for (int i = 0; i < numberOfSections; ++i) {
        std::cout << "Section " << i << " has " << nonzerosPerSection[i] << " nonzeros" << std::endl;
        total_nonzero_sum+=nonzerosPerSection[i];
    }
    cout << "total sum debug: " << total_nonzero_sum << endl;
}

std::vector<std::vector<int>> count_matches(
    const std::vector<int>& row_ptr, 
    const std::vector<int>& col, 
    const std::vector<int>& csc_col_ptr, 
    const std::vector<int>& csc_row) 
{
    std::vector<std::vector<int>> match_counts(row_ptr.size() - 1);

    // Define lambda function for thread execution
    auto thread_function = [&](size_t start_row, size_t end_row) {
        for (size_t i = start_row; i < end_row; ++i) { // For each row in CSR
            match_counts[i].reserve(csc_col_ptr.size() - 1); // Reserve space for the row
            
            for (size_t j = 0; j < csc_col_ptr.size() - 1; ++j) {
                int matchcountint=0;
                for (size_t k = row_ptr[i]; k < row_ptr[i + 1]; ++k) { // // A�� �� row�� �� element����
                    int colIndex = col[k]; // A�� �� row�� �� element
                    
                    for (size_t l = csc_col_ptr[j]; l < csc_col_ptr[j + 1]; ++l) { // B�� �� col�� �� element����
                        
                        int rowIndexCSC = csc_row[l]; // B�� �� col�� �� element
                        if (rowIndexCSC == colIndex) {
                            matchcountint++;
                            break;
                        }
                    }
                }
                if(matchcountint!=0)
                {
                    match_counts[i].push_back(matchcountint);
                }
            }
            
        }
    };

    // Determine number of threads to use
    // size_t num_threads = std::thread::hardware_concurrency();
    size_t num_threads = 19;
    size_t rows_per_thread = (row_ptr.size() - 1) / num_threads;

    // Create and start threads
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; ++i) {
        size_t start_row = i * rows_per_thread;
        size_t end_row = (i == num_threads - 1) ? row_ptr.size() - 1 : (i + 1) * rows_per_thread;
        threads.emplace_back(thread_function, start_row, end_row);
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    return match_counts;
}



void printMatchingIndices(const std::vector<std::vector<int>>& matching_indices) {
    std::cout << "Matching Indices:\n";
    for (size_t i = 0; i < matching_indices.size(); ++i) {
        std::cout << "Row " << i << ": ";
        for (size_t j = 0; j < matching_indices[i].size(); ++j) {
            std::cout << matching_indices[i][j] << " ";
        }
        std::cout << "\n";
    }
}

std::vector<std::vector<bool>> segment_presence(const std::vector<int>& csc_col_ptr, const std::vector<int>& csc_row, const int num_segments) {
    std::vector<std::vector<bool>> segment_status(csc_col_ptr.size(), std::vector<bool>(num_segments, false)); // Initialize segment status vector

    int segment_size =csc_col_ptr.size() / num_segments; // Calculate segment size

    cout << "csc_row.size(): " << csc_row.size() << endl;
    cout << "segment_size: " << segment_size << endl;

    for (size_t col_index = 0; col_index < csc_col_ptr.size() - 1; ++col_index) { // Iterate through each CSC column
        int start_rowidx = csc_col_ptr[col_index]; // Get start row index for current column
        int end_rowidx = csc_col_ptr[col_index + 1]; // Get end row index for current column
        
        for (int row_index = start_rowidx; row_index < end_rowidx; ++row_index) { // Iterate through each element (csc_row) in the current column
            int segment_index = csc_row[row_index] / segment_size; // Calculate segment index for current element
            
            if (segment_index >= num_segments) // Ensure segment index is within range
                segment_index = num_segments - 1;

            segment_status[col_index][segment_index] = true; // Set segment status to true for corresponding column index and segment index
        }
    }

    return segment_status;
}

void splitRowsToCSR(const std::vector<int>& row_ptr, const std::vector<int>& col, const std::vector<int>& nonzero, int N, std::vector<std::vector<int>>& smallcsr, std::vector<std::vector<int>>& smallrowptr) {
    int total_rows = row_ptr.size() - 1; // ��ü ���� ����

    // ���� CSR�� ������ ���͸� �ʱ�ȭ�մϴ�.

    int remainderIndex=0; // remainder�� ���� �й��ϱ� ���� index

    // �� ���� ���� CSR�� ������ ���� �� ���� ��ȸ�ϸ鼭 ó���մϴ�.
    for (int i = 0; i < total_rows; ++i) {
        int start_idx = row_ptr[i];     // ���� ���� ���� �ε���
        int end_idx = row_ptr[i + 1];   // ���� ���� �� �ε���

        // �࿡ ���� ������ ������ ����մϴ�.
        int num_elements = end_idx - start_idx;

        // ���� ���� ���� CSR�� ������ ���� N���� ���� ��� �������� ����մϴ�.
        int quotient = num_elements / N;
        int remainder = num_elements % N;

        // �� Smallcsr�� �Ҵ�Ǿ���� CSR�� ���� ���Ѵ�.
        vector<int> NumColOfSmallCsr(N, quotient);

        // for (size_t j=0; j<N; j++)
        // {
        //     if(j<remainder)
        //     {
        //         NumColOfSmallCsr[j]++;
        //     }
        // }

        // ** �����ϰ� remaider�� �� smallcsr�� �й��ϱ� ���� �۾� 
        int NumRemainder=remainder;

        while(NumRemainder>0)
        {
            //�ش� index�� remainder ������Ʈ
            NumColOfSmallCsr[remainderIndex]++; 

            //index �� NumRemainder ������Ʈ
            if(remainderIndex==(N-1))
            {
                remainderIndex=0;
            }
            else {
                remainderIndex++;
            }

            NumRemainder--;
        }

        // smallrowptr ������Ʈ
        for (size_t j=0; j<N; j++)
        {
            if(smallrowptr[j].size()>0)
            {
                smallrowptr[j].push_back(smallrowptr[j].back()+NumColOfSmallCsr[j]);
            }
            else 
            {
                smallrowptr[j].push_back(NumColOfSmallCsr[j]);
            }
        }
        
        
        // CSR�� �� Smallcsr�� �ش� ������ŭ �Ҵ�
        int col_idx = start_idx;

        // �� smallCSR[j] ���� �Ҵ�
        for(size_t j=0; j<N; j++)
        {
            // CSR�� �� NumColOfSmallCsr[j] ������ŭ smallCSR[j]�� �Ҵ�
            for(size_t k=0; k<NumColOfSmallCsr[j]; k++)
            {
                smallcsr[j].push_back(col[col_idx+k]);
            }
            col_idx += NumColOfSmallCsr[j];
        }
    }
}

std::vector<std::vector<int>> calculateNumOfBRead(const std::vector<int>& row_ptr, const std::vector<int>& col, const std::vector<std::vector<int>>& smallRowptr, int N) {
    std::vector<std::vector<int>> NumOfBRead(N, std::vector<int>(row_ptr.size() - 1, 0)); // ����� ������ ���� �ʱ�ȭ

    // k ���� �м�
    for (size_t k = 0; k < N; k++) {
         // �� �ึ�� ���� CSR ���� �󸶳� �о���ϴ��� ���
        for (size_t i = 0; i < (row_ptr.size() - 1); i++) {
            int A_first_idx = row_ptr[i]; // col �� ���� ù�� ° idx
            int A_end_idx = row_ptr[i + 1]; // col �� ���� ������ idx

            for (size_t j = A_first_idx; j < A_end_idx; j++) {
                    // �� smallcol ���� ���
                    int B_first_idx = smallRowptr[k][col[j]];
                    int B_end_idx = smallRowptr[k][col[j] + 1];
                    int NumelemetRead_in_a_Row = B_end_idx - B_first_idx;

                    NumOfBRead[k][i] += NumelemetRead_in_a_Row;
            }
        }
    }

    return NumOfBRead;
}

int countOverlap(const vector<vector<int>>& vectors) {
    unordered_map<int, int> frequencyMap;

    // ���� �� ��Ҹ� �ؽ� �ʿ� ���
    for (const auto& vec : vectors) {
        unordered_map<int, bool> seen;
        for (int num : vec) {
            if (!seen[num]) { // �� ���� ������ �ߺ��� ��Ұ� ���ٰ� ����
                frequencyMap[num]++;
                seen[num] = true;
            }
        }
    }

    // ��ġ�� ���� ������ ����
    int overlapCount = 0;
    for (const auto& pair : frequencyMap) {
        if (pair.second > 1) { // ��ġ�� ���� �ִ� ���
            overlapCount += pair.second - 1;
        }
    }

    return overlapCount;
}

int countEqualValues(const vector<vector<int>>& vectors) {
    int count = 0;
    
    map<int, int> mapset;

    // Iterate through each 1D vector
    for (const auto& vec : vectors) {
        // Check for duplicate values in the current 1D vector
        for (int value : vec) {
            if (mapset.find(value) != mapset.end()) { // value�� �ִ����� ã�� ������ �ش� �� ����
                mapset[value]++;
                count++;
            } 
            else { // 
                mapset.insert({value, 0});
            }
        }
    }

    return count;
}



std::vector<int> calculateReductionRate(const std::vector<int>& row_ptr, const std::vector<int>& col, const std::vector<std::vector<int>>& smallRowptr, const vector<vector<int>>& smallcol, int N) {
    std::vector<std::vector<int>> NumOfBRead(N, std::vector<int>(row_ptr.size() - 1, 0)); // ����� ������ ���� �ʱ�ȭ

    // k ���� �м�
    vector<int> overlap_k(N, 0);
    int read_flag=2;
    for (size_t k = 0; k < N; k++) {
         // �� �ึ�� ���� CSR ���� �󸶳� �о���ϴ��� ���
        for (size_t i = 0; i < (row_ptr.size() - 1); i++) {
            int A_first_idx = row_ptr[i]; // col �� ���� ù�� ° idx
            int A_end_idx = row_ptr[i + 1]; // col �� ���� ������ idx

          
            vector<vector<int>> WillBeReadBVectors; // A_end_idx-A_first_idx ���� B row�� �д´�.
            size_t currentvectoridx=0;
            for (size_t j = A_first_idx; j < A_end_idx; j++) {
                    // �� smallcol ���� ���
                    
                    int B_first_idx = smallRowptr[k][col[j]];
                    int B_end_idx = smallRowptr[k][col[j] + 1];
                    // if( k==0 && read_flag>0)
                    // {
                    //     cout << "j: " << j  << endl;
                    //     cout << "col[j]: " << col[j]<< endl;            
                    //     cout << "col[j] + 1: " << col[j] + 1<< endl;            
                    //     cout << "B_first_idx: " << B_first_idx<< endl;            
                    //     cout << "B_end_idx: " << B_end_idx<< endl;     
                    //     cout << endl; 
                    // }      
                    
                    WillBeReadBVectors.emplace_back();

                   
                    for(size_t l=B_first_idx; l<B_end_idx; l++)
                    {
                        
                        WillBeReadBVectors[currentvectoridx].push_back(smallcol[k][l]);
                    }
                   
                    currentvectoridx++;
            }
            // DEBUG
            int overlap=countEqualValues(WillBeReadBVectors);

            overlap_k[k]+=overlap;

            // if(k==0 && read_flag>0)
            // {
            //     cout << "WillBeReadBVectors"<< endl;
            //     cout << "k: " << k << endl;
            
            
            //     for(size_t i=0; i<WillBeReadBVectors.size(); i++)
            //     {
            //         for(size_t j=0; j<WillBeReadBVectors[i].size(); j++)
            //         {
            //             cout << WillBeReadBVectors[i][j] << " ";
            //         }
            //         cout << endl;
            //     }   
            //     cout << endl;
            //     cout << "overlap: " << overlap <<endl;
            // }
            
            // if(k==0)
            // {
            //     read_flag--;
            // }
            
            
        }
    }

    return overlap_k;
}

std::vector<int> calculateReductionRate_OpenMP(const std::vector<int>& row_ptr, const std::vector<int>& col, const std::vector<std::vector<int>>& smallRowptr, const std::vector<std::vector<int>>& smallcol, int N, int readNum) {
    std::vector<std::vector<int>> NumOfBRead(N, std::vector<int>(row_ptr.size() - 1, 0)); // ����� ������ ���� �ʱ�ȭ

    // k ���� �м�
    std::vector<int> overlap_k(N, 0);
    int read_flag=readNum;

    #pragma omp parallel for shared(overlap_k)
    for (size_t k = 0; k < N; k++) {
         // �� �ึ�� ���� CSR ���� �󸶳� �о���ϴ��� ���
        for (size_t i = 0; i < (row_ptr.size() - 1); i++) {
            int A_first_idx = row_ptr[i]; // col �� ���� ù�� ° idx
            int A_end_idx = row_ptr[i + 1]; // col �� ���� ������ idx

            std::vector<std::vector<int>> WillBeReadBVectors; // A_end_idx-A_first_idx ���� B row�� �д´�.
            size_t currentvectoridx=0;
            for (size_t j = A_first_idx; j < A_end_idx; j++) {
                // �� smallcol ���� ���
                int B_first_idx = smallRowptr[k][col[j]];
                int B_end_idx = smallRowptr[k][col[j] + 1];
                if( k==0 && read_flag>0)
                {
                    cout << "j: " << j  << endl;
                    cout << "col[j]: " << col[j]<< endl;            
                    cout << "col[j] + 1: " << col[j] + 1<< endl;            
                    cout << "B_first_idx: " << B_first_idx<< endl;            
                    cout << "B_end_idx: " << B_end_idx<< endl;     
                    cout << endl; 
                }    
                
                WillBeReadBVectors.emplace_back();
                
                for(size_t l=B_first_idx; l<B_end_idx; l++) {
                    WillBeReadBVectors[currentvectoridx].push_back(smallcol[k][l]);
                }
                
                currentvectoridx++;
            }

            int overlap = countEqualValues(WillBeReadBVectors);
            overlap_k[k] += overlap;

            if(k==0 && read_flag>0)
            {
                cout << "WillBeReadBVectors"<< endl;
                cout << "k: " << k << endl;
            
            
                for(size_t i=0; i<WillBeReadBVectors.size(); i++)
                {
                    for(size_t j=0; j<WillBeReadBVectors[i].size(); j++)
                    {
                        cout << WillBeReadBVectors[i][j] << " ";
                    }
                    cout << endl;
                }   
                cout << endl;
                cout << "overlap: " << overlap <<endl;
            }
            
            if(k==0)
            {
                read_flag--;
            }
        }
    }

    return overlap_k;
}

int calculateMAXBRead_OpenMP(const std::vector<int>& row_ptr, const std::vector<int>& col, const std::vector<std::vector<int>>& smallRowptr, const std::vector<std::vector<int>>& smallcol, int N, int readNum) {
    std::vector<std::vector<int>> NumOfBRead(N, std::vector<int>(row_ptr.size() - 1, 0)); // ����� ������ ���� �ʱ�ȭ

    // k ���� �м�

    int read_flag=readNum;
    int max_BRead=0;
    #pragma omp parallel for shared(overlap_k)
    for (size_t k = 0; k < N; k++) {
         // �� �ึ�� ���� CSR ���� �󸶳� �о���ϴ��� ���
         
        for (size_t i = 0; i < (row_ptr.size() - 1); i++) {
            int A_first_idx = row_ptr[i]; // col �� ���� ù�� ° idx
            int A_end_idx = row_ptr[i + 1]; // col �� ���� ������ idx

            int totalBRead = 0;
            for (size_t j = A_first_idx; j < A_end_idx; j++) {
                // �� smallcol ���� ���
                int B_first_idx = smallRowptr[k][col[j]];
                int B_end_idx = smallRowptr[k][col[j] + 1];
                totalBRead += (B_end_idx-B_first_idx);
            }
            if(max_BRead<(totalBRead))
            {
                max_BRead = (totalBRead);
            }
        }
    }

    return max_BRead;
}


int main(int argc , char ** argv)
{
    int i;
    char* txt_file = NULL;
    char* divide_extent_char =NULL;
    char* rank_char =NULL;
    char* third_char =NULL;
    char* fourth_char =NULL;
    float sparsity;  
    unsigned long long nonzero_cnt_mat = 0;  
    int dim;
    unsigned long long num;
    unsigned int row_ptr_size, col_idx_size;
    unsigned int* row_ptr_malloc;
    int* col_nonzero_malloc;

    if(argc>1)
    {
        txt_file = argv[2];

    }
    
    
    cout << "-------------------------------------------------------------------------"<<endl; 
    cout << "-------------------------------------------------------------------------"<<endl;
    cout << "-------------------------------------------------------------------------"<<endl;
    cout << "-------------------------------------------------------------------------"<<endl;
    
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
    printf("Sparsity: %f\n\n", float(col_idx_size)/row_ptr_size/row_ptr_size);

    printf("Original Iterate Count: %d\n", row_ptr_size);
    printf("Fixed Iterate Count (20 size each rank 16): %f\n", float(col_idx_size)/20/16);
    printf("Fixed Iterate Count (40 size each rank 16): %f\n", float(col_idx_size)/40/16);

    // 
    std::vector<int> col_nonzero(2*col_idx_size,0);
    std::vector<int> row_ptr(row_ptr_size,0);
    std::vector<int> col(col_idx_size, 0);
    std::vector<int> nonzero(col_idx_size, 0);
    int col_width = row_ptr_size;
    


    file.read((char*)&row_ptr[0], sizeof(row_ptr[0]) * row_ptr.size());
    file.read((char*)&col_nonzero[0], sizeof(col_nonzero[0]) * col_nonzero.size());
    
    // COL ?? col_nonzero ??????
    for(int i = 0; i<col_idx_size; i++)
    {
        col[i] = col_nonzero[2*i];
    }

       
    int NumNonzero = row_ptr[row_ptr.size()-1];


    cout << "nonzero #: "<< NumNonzero << endl;
    cout << "col_width: " << col_width << endl;
    cout << "***** Rowptr # nonzero # ***** " << endl;



    // CSC �������� ��ȯ�� ���͵�

    // ** CSC format
    std::vector<int> csc_col_ptr, csc_row, csc_val;
    // ** CSR format
    // std::vector<int> row_ptr(row_ptr_size,0);
    // std::vector<int> col(col_idx_size, 0);
    // std::vector<int> nonzero(col_idx_size, 0);


    
   

    // csrToCsc(row_ptr, col, nonzero, csc_col_ptr, csc_row, csc_val);
    csrToCsc(row_ptr, col, nonzero, 
             csc_col_ptr, csc_row, csc_val);
    int maxNonzero_rowptr = findMaxNonZero(row_ptr);
    int maxNonzero_colptr = findMaxNonZero(csc_col_ptr);

    cout << "maxNonzero_rowptr: " << maxNonzero_rowptr << endl;
    cout << "maxNonzero_colptr: "<< maxNonzero_colptr << endl;
    cout << "max row sparsity: " << float(maxNonzero_rowptr)/col_width<< endl; 
    cout << "max col sparsity: " << float(maxNonzero_colptr)/col_width<< endl; 
    

    // ��ȯ�� CSC ���� ���
     std::cout << "col_ptr: ";
    for(size_t i=0; i<20; i++)
    {
        cout << csc_col_ptr[i] << " ";
    }
    cout << endl;

    std::cout << "row_ptr: ";
    for(size_t i=0; i<20; i++)
    {
        cout << row_ptr[i] << " ";
    }
    cout << endl;


    // ******* csr �� csc ���� �м�
    std::vector<int> col_ptr = {0, 3, 5, 8, 10, 14, 17, 20, 22, 25}; // �̴� 9���� ���� �ǹ�
    int range_size_rowptr = maxNonzero_rowptr/20+1; // ������ ���� ũ��
    int range_size_colptr = maxNonzero_colptr/20+1; // ������ ���� ũ��

    cout << "range_size_rowptr: " << range_size_rowptr << endl;
    cout << "range_size_colptr: "<< range_size_colptr << endl;

    auto csc_distribution = calculateFixedRangeDistribution(csc_col_ptr, range_size_colptr);
    auto csr_distribution = calculateFixedRangeDistribution(row_ptr, range_size_rowptr);

    std::map<int, int> result = countRowsByNonZero(row_ptr);
    std::map<int, int> result_2 = countRowsByNonZero(csc_col_ptr);
    




    // **** CSR --> small csr N���� ������
    int N=16;
    std::vector<std::vector<int>> smallCol(N);
    std::vector<std::vector<int>> smallRowptr(N);

    splitRowsToCSR(row_ptr, col, nonzero, N, smallCol, smallRowptr);
    // �տ� 0�߰�
    for (int i = 0; i < N; ++i) {
        smallRowptr[i].insert(smallRowptr[i].begin(), 0);
    }




    
    // small csr ���� �󸶳� �д��� ȹ�� (�󸶳� ���� �д��� ���)
    std::vector<std::vector<int>> NumOfBRead = calculateNumOfBRead(row_ptr, col, smallRowptr, N);

 




    // *******************************************************PRINT OPTION start **********
    std::string configFile = argv[1];
    parseIniFile(configFile);

    int ReadNum=0;
    if (settings.find("Settings.PrintWillBeReadBVectors") != settings.end()) {
        ReadNum = std::stoi(settings["Settings.PrintWillBeReadBVectors"]);
    }
    // *******************************************************PRINT OPTION end **********


    // Reduction �� ����
    // std::vector<int> NumOfBReduction = calculateReductionRate_OpenMP(row_ptr, col, smallRowptr, smallCol, N, ReadNum);

    int MaxcBRead = calculateMAXBRead_OpenMP(row_ptr, col, smallRowptr, smallCol, N, ReadNum);

    cout <<"MaxcBRead: " << MaxcBRead << endl;

    // cout << "*** overlap count"<< endl;
    // for(size_t i=0; i<NumOfBReduction.size(); i++)
    // {
    //      cout << NumOfBReduction[i] << endl;
    // }




    // *******************************************************PRINT OPTION start **********

    // �о�� ������ ����Ͽ� �۾� ����
    if (settings.find("Settings.PrintCsrDistribution") != settings.end() && settings["Settings.PrintCsrDistribution"] == "on") {
        cout << "*** csr row distribution result (below 20) *** " << endl;

        int total_20belowsizeRow=0;
        for (int i = 0; i <= 20; ++i) {
            std::cout << "Rows with " << i << " non-zero elements: " << result[i] << std::endl;
            total_20belowsizeRow+=result[i];
        }
        // ��� ���
        cout << "total_20belowsizeRow: "<< total_20belowsizeRow << endl;
        cout << endl;
        cout << endl;
         // ��� ���
        cout << "*** csr row distribution result ***" << endl;
        for (const auto& pair : csr_distribution) {
            std::cout << "Non-zero count range [" << pair.first * range_size_rowptr
                    << ", " << (pair.first + 1) * range_size_rowptr - 1
                    << "]: " << pair.second << " rows\n";
        }

        
    }

    if (settings.find("Settings.PrintCscDistribution") != settings.end() && settings["Settings.PrintCscDistribution"] == "on") {
         cout << "*** csc column distribution result (below 20) *** " << endl;
        int total_20belowsizeCol=0;
        for (int i = 0; i <= 20; ++i) {
            std::cout << "cols with " << i << " non-zero elements: " << result_2[i] << std::endl;
            total_20belowsizeCol+=result_2[i];
        }
        cout << "total_20belowsizeCol: "<< total_20belowsizeCol << endl;
        cout << endl;
        cout << endl;
        cout << "*** csc column distribution result *** " << endl;
        for (const auto& pair : csc_distribution) {
            std::cout << "Non-zero count range [" << pair.first * range_size_colptr
                    << ", " << (pair.first + 1) * range_size_colptr - 1
                    << "]: " << pair.second << " columns\n";
        }

        cout << endl;
        cout << endl;
    }

   
    if (settings.find("Settings.PrintNnzForSection") != settings.end() && settings["Settings.PrintNnzForSection"] == "on") {
        cout << "***  # of nonzero for each region (csc)***" << endl;  
        int numberOfSections=4;
        countNonzerosPerColumnSection(csc_col_ptr, csc_row, csc_val, numberOfSections);
        cout << "***  # of nonzero for each region (csr)***" << endl;  
        countNonzerosPerColumnSection(row_ptr, csc_row, csc_val, numberOfSections);
    }

    if (settings.find("Settings.PrintSmallCsr") != settings.end() && settings["Settings.PrintSmallCsr"] == "on") {
        

        cout << "*** print small row_ptr size" << endl;
        for(size_t i=0; i<smallRowptr.size(); i++)
        {
            cout << smallRowptr[i].size() << endl;
        }
        
        cout << "*** original row_ptr" << endl;
        for(size_t j=0; j<20; j++)
        {
            cout << row_ptr[j] << " ";
        }
        cout << endl;
        cout << endl;


        cout << "*** splitted row_ptr" << endl;
        for(size_t i=0; i<smallRowptr.size(); i++)
        {    
            cout << "smallrowptr " << i << endl;
            for(size_t j=0; j<20; j++)
            {
                cout << smallRowptr[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
        
        cout << "*** original col" << endl;
        for(size_t j=0; j<60; j++)
        {
            cout << col[j] << " ";
        }
        cout << endl;
        
        for(size_t i=0; i<smallCol.size(); i++)
        {   
            cout << "smallcol " << i << endl;
            for(size_t j=0; j<20; j++)
            {
                cout << smallCol[i][j] << " ";
            }
            cout << endl;
        }
    }

    
   

    
    
    

    
    
    

    



    


    return 0;
}