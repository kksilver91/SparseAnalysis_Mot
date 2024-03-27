#include "analysis.h"

using namespace std;

Sparse_debug::Sparse_debug(const std::vector<std::string>& filenames, const std::vector<std::string>& filenames_2, const std::vector<std::string>& filenames_3, const std::vector<std::string>& filenames_4)
{

    for (const auto& filename : filenames) {
      std::ofstream outfile;
      outfile.open(filename);
      if (outfile.is_open()) {
        outfileList.push_back(std::move(outfile));
      } else {
        // 파일 열기에 실패한 경우 처리
      }
    }
    
    for (const auto& filename : filenames_2) {
      std::ofstream outfile;
      outfile.open(filename);
      if (outfile.is_open()) {
        RowptroutfileList.push_back(std::move(outfile));
      } else {
        // 파일 열기에 실패한 경우 처리
      }
    }

    for (const auto& filename : filenames_3) {
      std::ofstream outfile;
      outfile.open(filename);
      if (outfile.is_open()) {
        SequentialoutfileList.push_back(std::move(outfile));
      } else {
        // 파일 열기에 실패한 경우 처리
      }
    }
    for (const auto& filename : filenames_4) {
      std::ofstream outfile;
      outfile.open(filename);
      if (outfile.is_open()) {
        NomergedoutfileList.push_back(std::move(outfile));
      } else {
        // 파일 열기에 실패한 경우 처리
      }
    }
}

