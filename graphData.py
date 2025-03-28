import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV with correct column names
df = pd.read_csv("bufferLog.csv", names=["Timestamp", "Radiator", "Room"], header=0)

# Convert Timestamp column to datetime format
df['Timestamp'] = pd.to_datetime(df['Timestamp'])

# Calculate elapsed time in hours from the first timestamp
df['Elapsed Hours'] = (df['Timestamp'] - df['Timestamp'].iloc[0]).dt.total_seconds() / 3600

# Plot the data
plt.figure(figsize=(10, 5))
plt.plot(df['Elapsed Hours'], df['Radiator'], label='Radiator Temperature', marker='o')
plt.plot(df['Elapsed Hours'], df['Room'], label='Room Temperature', marker='s')

plt.xlabel("Time (Hours)")
plt.ylabel("Temperature (°C)")
plt.title("Temperature Over Time")
plt.legend()
plt.grid()

plt.show()
