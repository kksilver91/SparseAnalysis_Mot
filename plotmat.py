# -*- coding: utf-8 -*-

import scipy.io
import matplotlib.pyplot as plt
import scipy.sparse

def plot_sparse_matrix(mat):
    rows, cols = mat.nonzero()
    plt.figure(figsize=(8, 6))
    plt.plot(cols, rows, 's', color='blue', markersize=5)  # 's'�� square marker�� �ǹ��մϴ�.
    plt.xlabel('Columns')
    plt.ylabel('Rows')
    plt.title('Nonzero Entries of Sparse Matrix')
    plt.grid(True)
    plt.gca().invert_yaxis()  # y���� �������� �����Ͽ� ����� ����� 0���� ���̰� �մϴ�.
    plt.show()

# mat ������ �ε��մϴ�.
try:
    mat = scipy.io.loadmat("road_usa.mat")  # ���� �̸��� ���� �����մϴ�.
    if 'A' not in mat:
        print("Sparse matrix 'A' not found in the mat file.")
    else:
        # ��� ����� �׷����� �÷����մϴ�.
        plot_sparse_matrix(mat['A'])  # 'A'�� ��� ����� �̸��Դϴ�.
except FileNotFoundError:
    print("File not found.")
except Exception as e:
    print("An error occurred:", e)