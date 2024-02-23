
#--------TEST SETTINGS---------
com_port = "COM16"
num_tests = 1
#------------------------------

import serial
import matplotlib.pyplot as plt
import numpy as np
import time


def parse_parameters(input_string):
    parameters = {}
    lines = input_string.split('\n')
    for line in lines:
        if ':' in line:
            key_value_pair = line.split(': ')
            if len(key_value_pair) == 2:
                key, value = key_value_pair
                parameters[key.strip()] = value.strip()
    return parameters

# Function to parse data from the input string
def parse_data(input_string):
    data = []
    lines = input_string.split('\n')
    found_data = False
    for line in lines:
        if found_data and line.strip() != '':
            values = line.split(',')
            timestamp = int(values[0])
            wheel_position = float(values[1])
            swivel_position = float(values[2])
            data.append((timestamp, wheel_position, swivel_position))
        elif line.strip() == 'Timestamp (ms),Wheel Position (m),Swivel Position (m)':
            found_data = True
    return data

# Function to run a test and collect data
def run_test():
    # Open serial port
    ser = serial.Serial(com_port, 115200, timeout=1)

    # Send start signal
    ser.write(b't')

    # Read data until end signal is received
    input_string = ''
    start_received = False
    while True:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line == '---------------start---------------':
            start_received = True
            continue
        elif line == '---------------end---------------':
            break
        if start_received:
            input_string += line + '\n'

    # Close serial port
    ser.close()

    # Parse parameters and data
    parameters = parse_parameters(input_string)
    data = parse_data(input_string)

    # Extracting timestamps and positions
    timestamps = np.array([entry[0] for entry in data])
    wheel_positions = np.array([entry[1] for entry in data])
    swivel_positions = np.array([entry[2] for entry in data])

    # Calculating speed (1st derivative)
    dt = np.diff(timestamps)
    dwheel_dt = np.diff(wheel_positions) / dt
    dswivel_dt = np.diff(swivel_positions) / dt
    timestamps_speed = timestamps[:-1]

    # Calculating acceleration (2nd derivative)
    d2wheel_dt2 = np.diff(dwheel_dt) / dt[:-1]
    d2swivel_dt2 = np.diff(dswivel_dt) / dt[:-1]
    timestamps_acceleration = timestamps_speed[:-1]

    return timestamps, wheel_positions, swivel_positions, dwheel_dt, dswivel_dt, d2wheel_dt2, d2swivel_dt2, parameters

# Run multiple tests

all_data = []
for i in range(num_tests):
    print(f"Running test {i+1}/{num_tests}")
    test_data = run_test()
    all_data.append(test_data)

    # Sleep for a brief period between tests if necessary
    # Modify this if you find the need for a longer delay
    time.sleep(1)

# Calculate average data
avg_timestamps = np.mean([data[0] for data in all_data], axis=0)
avg_wheel_positions = np.mean([data[1] for data in all_data], axis=0)
avg_swivel_positions = np.mean([data[2] for data in all_data], axis=0)
avg_dwheel_dt = np.mean([data[3] for data in all_data], axis=0)
avg_dswivel_dt = np.mean([data[4] for data in all_data], axis=0)
avg_d2wheel_dt2 = np.mean([data[5] for data in all_data], axis=0)
avg_d2swivel_dt2 = np.mean([data[6] for data in all_data], axis=0)

# Multiply speed by 1000
avg_dwheel_dt *= 1000
avg_dswivel_dt *= 1000

# Multiply acceleration by 1000000
avg_d2wheel_dt2 *= 1000000
avg_d2swivel_dt2 *= 1000000

# Define the offsets for each dataset (in milliseconds)
offset_pos = 0  # Offset for position graph
offset_speed = 50  # Offset for speed graph
offset_accel = 100  # Offset for acceleration graph

# Create subplots with more space allocated for parameters
fig, axs = plt.subplots(3, figsize=(10, 12), gridspec_kw={'left': 0.15})

# Adjust layout to allocate more space to the left and increase horizontal space between subplots
plt.subplots_adjust(left=0.15, right=0.95, top=0.95, bottom=0.05, wspace=0.3)

# Plot position with shifted timestamps
shifted_avg_timestamps_pos = avg_timestamps + offset_pos
axs[0].plot(shifted_avg_timestamps_pos, avg_wheel_positions, label='Wheel Pos.')
axs[0].plot(shifted_avg_timestamps_pos, avg_swivel_positions, label='Swivel Pos.')
axs[0].set_ylabel('Position (m)')
axs[0].set_title('Average Position over Time')
axs[0].legend()
axs[0].grid(True, which='both', linestyle='--')  # Enable both major and minor grid lines
axs[0].grid(True, which='minor', linestyle=':', linewidth='0.5', color='gray')  # Minor grid lines

# Plot speed with shifted timestamps
shifted_avg_timestamps_speed = avg_timestamps[:-1] + offset_speed
axs[1].plot(shifted_avg_timestamps_speed, avg_dwheel_dt, label='Wheel Spd.')
axs[1].plot(shifted_avg_timestamps_speed, avg_dswivel_dt, label='Swivel Spd.')
axs[1].set_ylabel('Speed (m/s)')
axs[1].set_title('Average Speed over Time')
axs[1].legend()
axs[1].grid(True, which='both', linestyle='--')
axs[1].grid(True, which='minor', linestyle=':', linewidth='0.5', color='gray')

# Plot acceleration with shifted timestamps
shifted_avg_timestamps_accel = avg_timestamps[:-2] + offset_accel
axs[2].plot(shifted_avg_timestamps_accel, avg_d2wheel_dt2, label='Wheel Accel.')
axs[2].plot(shifted_avg_timestamps_accel, avg_d2swivel_dt2, label='Swivel Accel.')
axs[2].set_ylabel('Acceleration (m/s^2)')
axs[2].set_title('Average Acceleration over Time')
axs[2].legend()
axs[2].grid(True, which='both', linestyle='--')
axs[2].grid(True, which='minor', linestyle=':', linewidth='0.5', color='gray')

# Annotate the plot with the number of tests conducted
plt.figtext(0.02, 0.02, f"Number of tests conducted: {num_tests}", fontsize=10, ha='left')

for ax in axs:
    ax.tick_params(axis='x', rotation=45, labelsize=8)  # Rotate x-axis tick labels and set font size
    ax.grid(True, which='both', linestyle='--')  # Enable grid lines for both major and minor ticks

# Display parameters on the left side
parameters_text = '\n'.join([f"{key}: {value}" for key, value in all_data[0][7].items()])
plt.figtext(0.02, 0.5, parameters_text, fontsize=10, va='center', rotation='vertical')  # Rotate text by 90 degrees

plt.show()