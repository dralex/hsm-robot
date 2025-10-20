#!/usr/bin/python3

import matplotlib.pyplot as plt
import numpy
import csv

TELEMETRY='telemetry.log'
matrix = []
THRESHOLD = 0.1

def ray_filter(n):
    if n < 0.0:
        return 0.0
    elif n > 8.0:
        return 8.0
    else:
        return n

with open(TELEMETRY, 'r') as f:
    r = csv.reader(f, delimiter=',')
    for row in r:
        if row[0] is None:
            continue
        line = list(map(lambda n: ray_filter(float(n)), row))
        mean = numpy.mean(line)
        if mean < THRESHOLD:
            continue
        matrix.append(line)
#        nonzero_index = len(line)
#        while nonzero_index >= 0:
#            if line[nonzero_index - 1] == 0.0:
#                nonzero_index -= 1
#            else:
#                break
#        matrix.append(line[0:nonzero_index])

m = numpy.array(matrix)
print(m.shape)
print(m)

N = 10

figure, axis = plt.subplots(N, N)

def f(x):
    if 360 / 6 < x < 360 * 5 / 6:
        return 4
    else:
        return 0
fx = list(map(lambda x: f(x), range(0, 360)))
    
for i in range(N):
    for j in range(N):
        idx = (i * float(N) + j) / (N * N - 1)
        angle = str(i * float(N) + j)
        m_idx = int(idx * m.shape[0])
        if m_idx >= m.shape[0]:
            m_idx = m.shape[0] - 1
        axis[i, j].plot(m[m_idx])
        axis[i, j].plot(fx)
        axis[i, j].set_ylim(0, 8)
        axis[i, j].set_title(angle)
plt.show()
