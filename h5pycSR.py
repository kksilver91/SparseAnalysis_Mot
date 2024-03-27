import h5py
import scipy.sparse as sps
import numpy as np

# HDF5 파일 경로
hdf5_path = '/home/esoc/KWANG/sparse_analysis/mat/GAP-twitter.mat'

# HDF5 파일 열기
with h5py.File(hdf5_path, 'r') as hdf5_file:
    # SuiteSparse 행렬 데이터 읽기
    mat_data = hdf5_file['GAP-twitter.mat'][()]
    
    # SuiteSparse 행렬을 CSR 포맷으로 변환
    csr_mat = sps.csr_matrix(mat_data)
    
    # CSR 포맷으로 변환된 행렬을 txt 파일로 저장
    np.savetxt('GAP-twitter.txt', csr_mat.toarray(), fmt='%d')