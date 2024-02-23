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
    wheel_data = {'time': [], 'position': []}
    swivel_data = {'time': [], 'position': []}
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
                    if len(parts) == 3:
                        timestamp, wheel_position, swivel_position = map(float, parts)
                        wheel_data['time'].append(timestamp)
                        wheel_data['position'].append(wheel_position)
                        swivel_data['time'].append(timestamp)
                        swivel_data['position'].append(swivel_position)
                except ValueError as e:
                    print(f"Error processing line: '{line}' | Error: {e}")

    return wheel_data, swivel_data

def plot_data(wheel_data, swivel_data):
    print("Plotting data...")
    fig, axs = plt.subplots(2, figsize=(10, 10), dpi=100)
    metrics = ['position']
    titles = ['Wheel Position (m)', 'Swivel Position (m)']

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
        axs[i].set_title(titles[i])
        axs[i].set_xlabel("Time (s)")
        axs[i].set_ylabel(titles[i])
        axs[i].legend()

    plt.tight_layout()
    plt.show()


def main():
    com_port = input("Enter COM port (e.g., COM16): ")
    ser = serial.Serial(com_port, 115200, timeout=1)
    print(f"Serial connection established on {com_port}.")

    while True:
        user_command = input("Please enter the command to send to the ESP32 (type 'exit' to quit): ").strip()
        if user_command.lower() == 'exit':
            break  # Exit the loop and end the program
        else:
            send_command(ser, user_command)

        if user_command == "run test":
            if wait_for_start_signal(ser):
                wheel_data, swivel_data = read_data(ser)
                plot_data(wheel_data, swivel_data)

if __name__ == "__main__":
    main()