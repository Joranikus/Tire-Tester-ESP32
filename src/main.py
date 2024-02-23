import time
import serial
import matplotlib.pyplot as plt
import numpy as np

def numerical_derivaton(data: list, timeStamps: list):
    output = []
    for ix, element in enumerate(data):
        if ix == 0:
            try:
                output.append((data[ix + 1] - data[ix]) / (timeStamps[ix + 1] - timeStamps[ix]))
            except ZeroDivisionError:
                output.append(0)

        elif ix == len(data) - 1:
            try:
                output.append((data[ix] - data[ix - 1]) / (timeStamps[ix] - timeStamps[ix - 1]))
            except ZeroDivisionError:
                output.append(0)

        else:
            try:
                output.append((data[ix + 1] - data[ix - 1]) / (timeStamps[ix + 1] - timeStamps[ix - 1]))
            except ZeroDivisionError:
                output.append(0)

    return output

def moving_average_filter(data, window_size=5):
    """Applies a simple moving average filter to the data."""
    return np.convolve(data, np.ones(window_size)/window_size, mode='valid')

def send_command(ser, command):
    ser.write(f"{command}\n".encode())
    print(f"Command sent to ESP32: '{command}'")

def wait_for_start_signal(ser):
    print("Waiting for start signal from ESP32...")
    while True:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', 'ignore').strip()
            if line == "READY_FOR_DATA_COLLECTION":
                print("Start signal received from ESP32. Beginning data collection...")
                return True

def read_data(ser):
    print("Collecting data...")
    wheel_data = {'time': [], 'speed': [], 'distance': [], 'acceleration': []}
    swivel_data = {'time': [], 'speed': [], 'distance': [], 'acceleration': []}
    collecting_data = True

    while collecting_data:
        if ser.in_waiting:
            line = ser.readline().decode().strip()
            print(f"Received line: {line}")
            if "END_TEST" in line:
                collecting_data = False
                print("End of test signal received. Ending data collection.")
                break
            if line.startswith('Data: '):
                line = line.replace('Data: ', '')
                try:
                    parts = line.split(',')
                    if len(parts) == 7:
                        timestamp, w_speed, w_distance, w_acceleration, s_speed, s_distance, s_acceleration = map(float, parts)
                        wheel_data['time'].append(timestamp)
                        wheel_data['speed'].append(w_speed)
                        wheel_data['distance'].append(w_distance)
                        wheel_data['acceleration'].append(w_acceleration)
                        swivel_data['time'].append(timestamp)
                        swivel_data['speed'].append(s_speed)
                        swivel_data['distance'].append(s_distance)
                        swivel_data['acceleration'].append(s_acceleration)
                except ValueError as e:
                    print(f"Error processing line: '{line}' | Error: {e}")

    return wheel_data, swivel_data

def plot_data(wheel_data, swivel_data):
    print("Plotting data...")
    fig, axs = plt.subplots(3, figsize=(10, 15), dpi=100)
    metrics = ['speed', 'distance', 'acceleration']
    titles = ['Speed (m/s)', 'Distance (m)', 'Acceleration (m/s^2)']

    window_size = 5

    for i, metric in enumerate(metrics):
        wheel_metric = wheel_data[metric]
        swivel_metric = swivel_data[metric]

        wheel_metric_filtered = moving_average_filter(wheel_metric, window_size=window_size)
        swivel_metric_filtered = moving_average_filter(swivel_metric, window_size=window_size)

        times = wheel_data['time']
        adjusted_times = times[window_size - 1:]

        axs[i].plot(adjusted_times, wheel_metric_filtered, label='Wheel')
        axs[i].plot(adjusted_times, swivel_metric_filtered, label='Swivel')
        axs[i].set_title(f"{titles[i]} over Time")
        axs[i].set_xlabel("Time (s)")
        axs[i].set_ylabel(titles[i])
        axs[i].legend()

    plt.tight_layout()
    plt.show()


def main():
    com_port = input("Enter COM port (e.g., COM16): ")
    ser = serial.Serial(com_port, 115200, timeout=1)
    print(f"Serial connection established on {com_port}.")

    # Configuration settings, ask only once at the beginning
    wheel_diameter = input("Enter wheel diameter (mm): ")
    distance_center_to_wheel = input("Enter distance from center to wheel (mm): ")
    motor_voltage = input("Enter motor voltage (V): ")

    # Send configuration commands
    send_command(ser, f"set_wheel_diameter {wheel_diameter}")
    send_command(ser, f"set_distance_center_to_wheel {distance_center_to_wheel}")
    send_command(ser, f"set_motor_voltage {motor_voltage}")
    time.sleep(2)  # Give some time for commands to be processed

    while True:
        user_command = input("Please enter the command to send to the ESP32 (type 'exit' to quit): ").strip()
        if user_command.lower() == 'exit':
            break  # Exit the loop and end the program
        elif user_command.lower() == "run test":
            # Prompt for additional test parameters
            speed_percent = input("Enter motor speed percentage for the test: ")
            acceleration_time = input("Enter motor acceleration time (s) for the test: ")
            # Send the detailed run_test command with parameters
            send_command(ser, f"{user_command} {speed_percent} {acceleration_time}")
        else:
            send_command(ser, user_command)

        if user_command == "run test":
            if wait_for_start_signal(ser):
                wheel_data, swivel_data = read_data(ser)
                plot_data(wheel_data, swivel_data)

        # After plotting, ask the user if they want to run another test
        run_another = input("Run another test? (yes/no): ")
        if run_another.lower() != 'yes':
            break  # Exit the loop if the user does not want to run another test

if __name__ == "__main__":
    main()