# -*- coding: utf-8 -*-

import scipy.io
import matplotlib.pyplot as plt
import scipy.sparse

def plot_sparse_matrix(mat):
    rows, cols = mat.nonzero()
    plt.figure(figsize=(8, 6))
    plt.plot(cols, rows, 's', color='blue', markersize=5)  # 's'는 square marker를 의미합니다.
    plt.xlabel('Columns')
    plt.ylabel('Rows')
    plt.title('Nonzero Entries of Sparse Matrix')
    plt.grid(True)
    plt.gca().invert_yaxis()  # y축을 역순으로 설정하여 행렬의 상단을 0으로 보이게 합니다.
    plt.show()

# mat 파일을 로드합니다.
try:
    mat = scipy.io.loadmat("road_usa.mat")  # 파일 이름을 직접 지정합니다.
    if 'A' not in mat:
        print("Sparse matrix 'A' not found in the mat file.")
    else:
        # 희소 행렬을 그래프로 플로팅합니다.
        plot_sparse_matrix(mat['A'])  # 'A'는 희소 행렬의 이름입니다.
except FileNotFoundError:
    print("File not found.")
except Exception as e:
    print("An error occurred:", e)