import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import matplotlib.cm as cm
import numpy as np

# Load CSV files
df_routers = pd.read_csv("selected_routers.csv")
df_users = pd.read_csv("users.csv")
df_assignments = pd.read_csv("user_assignments.csv")
df_mst = pd.read_csv("mst_edges.csv")

# Merge assignments with user positions
df_users_pos = df_users.set_index("id")
df_assignments = df_assignments.join(df_users_pos, on="user_id")

# Create a mapping for router positions
router_pos = df_routers.set_index("id")[["x", "y"]].to_dict("index")

# Generate color map for routers
num_routers = len(df_routers)
colors = cm.get_cmap('hsv', num_routers)

# Plot settings
plt.figure(figsize=(12, 10))
plt.title("Campus Wi-Fi: Routers, Users, and Connections", fontsize=16)

# Plot users
plt.scatter(df_users["x"], df_users["y"], s=10, c='gray', label='Users')

# Plot routers and coverage
for idx, r in df_routers.iterrows():
    router_color = colors(idx / num_routers)
    plt.scatter(r["x"], r["y"], s=150, color=router_color, marker='^', edgecolor='black', label=f'Router {r["id"]}' if idx < 10 else "")
    circle = plt.Circle((r["x"], r["y"]), r["coverage"], color=router_color, fill=True, alpha=0.1)
    plt.gca().add_patch(circle)

# Draw MST connections
for _, row in df_mst.iterrows():
    x1, y1 = router_pos[row["from"]]["x"], router_pos[row["from"]]["y"]
    x2, y2 = router_pos[row["to"]]["x"], router_pos[row["to"]]["y"]
    plt.plot([x1, x2], [y1, y2], 'k--', linewidth=1, alpha=0.6)

# Draw user-router assignments
for _, row in df_assignments.iterrows():
    router = router_pos[row["router_id"]]
    plt.plot([router["x"], row["x"]], [router["y"], row["y"]], 'g-', alpha=0.3, linewidth=0.5)

# Find unassigned users
assigned_users = set(df_assignments["user_id"])
all_users = set(df_users["id"])
unassigned_users = all_users - assigned_users

# Plot unassigned users
if unassigned_users:
    df_unassigned = df_users[df_users["id"].isin(unassigned_users)]
    plt.scatter(df_unassigned["x"], df_unassigned["y"], s=30, color='red', marker='x', label='Unassigned Users')


# Final plot tweaks
plt.xlabel("X-coordinate")
plt.ylabel("Y-coordinate")
plt.legend(loc='upper right', bbox_to_anchor=(1.2, 1))
plt.grid(True)
plt.tight_layout()
plt.savefig("wifi_network_visualization.png")
plt.show()

# Load and print summary
print("\n===== PROJECT SUMMARY =====")
with open("summary.txt") as f:
    print(f.read())
