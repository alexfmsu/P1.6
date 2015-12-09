import numpy as np
import matplotlib.pyplot as plt

size     = np.loadtxt('size.dat')
s_size   = np.loadtxt('s_size.dat')
timing   = np.loadtxt('timing.dat')
s_timing = np.loadtxt('s_timing.dat')

compr = np.arange(1, 10, 1)

t_calc = np.zeros(len(size))
s_t_calc = np.zeros(len(size))

i     = 0
j     = 0
ctrl  = 10
n_mes = 10
tmp   = 0.
s_tmp = 0.

for i in range(len(timing)):
    tmp += timing[i]
    s_tmp += s_timing[i]
    if i + 1 == ctrl:
        t_calc[j] = tmp / n_mes
        s_t_calc[j] = s_tmp / n_mes
        j += 1
        ctrl += n_mes
        tmp = 0.
        s_tmp = 0.
        
plt.figure()
plt.plot(compr, t_calc, label = 'normal')
plt.plot(compr, s_t_calc, label = 'shuffle')
plt.xlabel('compression factor')
plt.ylabel('time access (s)')
plt.legend(bbox_to_anchor = (0.26, 1))
# plt.show()
plt.savefig("time_comparison.png")

plt.close('all')

plt.figure()
plt.plot(compr, size / 10**6, label = 'normal')
plt.plot(compr, s_size / 10**6, label = 'shuffle')
plt.xlabel('compression factor')
plt.ylabel('file size (MB)')
plt.legend()
# plt.show()
plt.savefig("size_comparison.png")
