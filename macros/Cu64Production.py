import numpy as np
import matplotlib.pyplot as plt

# Thông số bài toán
R = 1000 * 3600  # 1.44 x 10^8 hạt/giờ
T_half = 12.7     # Chu kỳ bán rã Cu-64 (giờ)
lamda = np.log(2) / T_half

# Khởi tạo trục thời gian từ 0 đến 10 giờ
t = np.linspace(0, 100, 500)
N = (R / lamda) * (1 - np.exp(-lamda * t))

# Vẽ đồ thị
plt.figure(figsize=(7, 5))
plt.plot(t, N, 'b-', linewidth=2, label='${}^{64}$Cu Nuclei Accumulation')
plt.plot(10, N[-1], 'ro', label=f'End of Beam (EOB): {N[-1]:.2e} nuclei')

plt.title('Variation of ${}^{64}$Cu Nuclei during Proton Irradiation (Bateman Equation)')
plt.xlabel('Irradiation Time (hours)')
plt.ylabel('Number of ${}^{64}$Cu Particles')
plt.grid(True, linestyle='--')
plt.legend()
plt.show()