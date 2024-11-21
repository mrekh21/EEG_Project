import asyncio
from bleak import BleakClient, BleakScanner
import csv
from datetime import datetime

# Define the Service and Characteristic UUIDs
SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0"
CHARACTERISTIC_UUID = "abcdef01-1234-5678-1234-56789abcdef0"

# Define the BLE device name (as set in ESP32)
DEVICE_NAME = "ESP32_EEG"

# CSV File Path
#CSV_FILE = "eeg_data_emotion1.csv"
CSV_FILE = "eeg_data_emotion2.csv"

#emotion = "Negative"
emotion = "Positive"

# Initialize CSV file with headers if it doesn't exist
try:
    with open(CSV_FILE, 'x', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Differential1", "Differential2", "Differential3", "Emotion", "Local_Timestamp"])
except FileExistsError:
    pass  # File already exists


# Callback function to handle notifications
def notification_handler(sender, data):
    """
    Callback function that is called when a notification is received.
    Parses the comma-separated EEG differential values.
    """
    try:
        # Decode bytes to string
        data_str = data.decode('utf-8').strip()
        # Split the string into individual values
        parts = data_str.split(',')
        if len(parts) != 3:
            print(f"Unexpected data format: {data_str}")
            return
        differential1, differential2, differential3 = parts
        # Get local timestamp
        local_timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
        # Print the values
        print(
            f"Differential1: {differential1}, Differential2: {differential2}, Differential3: {differential3}, Emotion: {emotion} Local_Timestamp: {local_timestamp}")
        # Log to CSV
        with open(CSV_FILE, 'a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([differential1, differential2, differential3, emotion, local_timestamp])
    except Exception as e:
        print(f"Error parsing data: {e}")


async def main():
    # Scan for devices
    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover(timeout=5.0)

    # Find the ESP32 device by name
    target_device = None
    for device in devices:
        if device.name == DEVICE_NAME:
            target_device = device
            break

    if not target_device:
        print(f"Device with name '{DEVICE_NAME}' not found.")
        return

    print(f"Found device: {target_device.name} [{target_device.address}]")

    # Connect to the ESP32 device
    async with BleakClient(target_device.address) as client:
        # Check if connected
        if not client.is_connected:
            print("Failed to connect to the device.")
            return
        print("Connected to the ESP32 BLE device.")

        # Start receiving notifications
        print("Subscribing to notifications...")
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)

        print("Listening for data... Press Ctrl+C to exit.")

        try:
            # Keep the program running to receive notifications
            while True:
                await asyncio.sleep(1)
        except KeyboardInterrupt:
            print("Disconnecting...")
        finally:
            # Stop notifications
            await client.stop_notify(CHARACTERISTIC_UUID)
            print("Disconnected.")


if __name__ == "__main__":
    asyncio.run(main())
