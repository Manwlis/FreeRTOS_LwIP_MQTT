import os
import struct
import paho.mqtt.client as mqtt

# MQTT broker settings
BROKER = "192.168.0.1"
PORT = 1883
TOPIC = "#"

DEBUG_DIR = "../code/debug/lwl/"
INFO_PATH = os.path.join(DEBUG_DIR, "info.txt")
DUMP_PATH = os.path.join(DEBUG_DIR, "dump.bin")

expected_size = 0
received_size = 0


def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Connected to broker")
        client.subscribe(TOPIC)
        print(f"Subscribed to: {TOPIC}")
        print("-" * 40)
    else:
        print(f"Connection failed with code {rc}")


def on_message(client, userdata, msg):
    global expected_size, received_size

    payload_hex = msg.payload.hex(' ')

    print(f"Topic:   {msg.topic}")
    print(f"Payload: {payload_hex}")

    if msg.topic == "mod/lwl/meta":
        if len(msg.payload) != 8:
            print("Error: mod/lwl/meta payload must be exactly 8 bytes")
            return

        next_entry_index, expected_size = struct.unpack("<II", msg.payload)
        received_size = 0

        os.makedirs(DEBUG_DIR, exist_ok=True)

        # Write info.txt
        with open(INFO_PATH, "w") as f:
            f.write(f"next_entry_index = {next_entry_index}\n")
            f.write(f"size = {expected_size}\n")

        # Reset dump.bin
        with open(DUMP_PATH, "wb"):
            pass

        print(f"Saved metadata to {INFO_PATH}")
        print(f"Initialized {DUMP_PATH}")

    elif msg.topic == "mod/lwl/data":
        if expected_size == 0:
            print("Warning: received data before meta")
            return

        os.makedirs(DEBUG_DIR, exist_ok=True)

        # Append chunk to dump.bin
        with open(DUMP_PATH, "ab") as f:
            f.write(msg.payload)

        received_size += len(msg.payload)

        print(
            f"Stored {len(msg.payload)} bytes "
            f"({received_size}/{expected_size})"
        )

        if received_size >= expected_size:
            print("Dump transfer complete")

    print("-" * 40)


client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, PORT, 60)
client.loop_forever()