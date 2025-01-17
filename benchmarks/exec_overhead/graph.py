import sys
import numpy as np
import matplotlib.pyplot as plt

# Input data
print(f"Load data from {sys.argv[1]}")
data = np.loadtxt(sys.argv[1])

# Calculate median and standard deviation for each column
medians = np.median(data, axis=0)
std_devs = np.std(data, axis=0)

print(medians)
print(std_devs)

# Create labels for the plot
labels = ['Normal', 'Traced-Normal', 'Traced-Inserted']

# Plot the results
x = np.arange(len(labels))
width = 0.35

fig, ax = plt.subplots()

# Plot bars for medians
rects1 = ax.bar(x, medians, width, label='Types', yerr=std_devs, capsize=5, color='skyblue', edgecolor='black')

# Add error bars for medians using standard deviation
ax.errorbar(x, medians, yerr=std_devs, fmt='none', color='black', capsize=5)

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Latency (ns)')
ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()

# Function to label the bars with their heights
def add_labels(rects):
    for rect in rects:
        height = rect.get_height()
        ax.annotate(f'{height:.2f}',
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),  # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom')

# Call the function to add labels
add_labels(rects1)

# Show the plot
plt.tight_layout()
plt.savefig("overhead.pdf", format="pdf")
plt.show()
