
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# ----------------------------
# 1. Load data
# ----------------------------
df = pd.read_csv("html_tiedot.csv")
feature_cols = ["sensorvalue_x", "sensorvalue_y", "sensorvalue_z"]
data = df[feature_cols].values  # shape: (n_samples, 3)
print("Parsed CSV data shape:", data.shape)

# ----------------------------
# 2. Initialize clusters
# ----------------------------

def initialize_clusters(data, k):
    # Valitse k satunnaista pistettä datasta aloituskeskuksiksi
    means = data[np.random.choice(data.shape[0], k, replace=False)]
    clusters = {i: {'center': means[i], 'points': []} for i in range(k)}
    return means, clusters


# ----------------------------
# 3. Distance function
# ----------------------------
def distance(p1, p2):
    return np.sqrt(np.sum((p1 - p2) ** 2))

# ----------------------------
# 4. Assign points to clusters
# ----------------------------
def assign_clusters(data, means, clusters):
    distances = np.zeros(means.shape[0])

    for idx in range(data.shape[0]):
        data_point = data[idx]

        for k in range(means.shape[0]):
            distances[k] = distance(means[k], data_point)
        closest_idx = np.argmin(distances)
        clusters[closest_idx]['points'].append(data_point)

    return clusters

# ----------------------------
# 5. Update clusters (handle empty clusters)
# ----------------------------
def update_clusters(data, clusters):

    for i in range(len(clusters)):
        points = np.array(clusters[i]['points'])
        #print("lenght", len(clusters))
        #print("points", points)0

        if points.shape[0] > 0:
            clusters[i]['center'] = points.mean(axis=0)
        else:
            # Reinitialize empty cluster center
            clusters[i]['center'] = data[np.random.randint(0, data.shape[0] - 1)]
            print(f"Cluster {i} was empty. Reinitialized center.")
        clusters[i]['points'] = []  # Clear for next iteration
    return clusters

# ----------------------------
# 6. Full K-Means loop
# ----------------------------
def kmeans(data, k, max_iters):
    means, clusters = initialize_clusters(data,k)

    for iteration in range(max_iters):
        print(f"Iteration {iteration + 1}")
        clusters = assign_clusters(data, means, clusters)

        for cluster_id, cluster_data in clusters.items():
            print(f"Cluster {cluster_id} has {len(cluster_data['points'])} points.")    
        
        clusters = update_clusters(data, clusters)
        means = np.array([clusters[i]['center'] for i in range(k)])
        print("means", means)
    return clusters, means

# ----------------------------
# Run K-Means
# ----------------------------
final_clusters, final_means = kmeans(data, k=6, max_iters=10)
print("Final cluster centers:\n", final_means)
