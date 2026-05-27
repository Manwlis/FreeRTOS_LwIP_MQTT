import socket
import threading
import struct
import time

HOST = '0.0.0.0'
PORT = 55151

MAGIC = b'\xAA\x55\xAA\x55'
HEADER_SIZE = 8  # magic (4) + seq (4)
PAYLOAD_SIZE = 1448
PACKET_SIZE = HEADER_SIZE + PAYLOAD_SIZE


def make_packet(seq):
    payload = bytes([seq % 256]) * PAYLOAD_SIZE
    return MAGIC + struct.pack("!I", seq) + payload


def sender(sock):
    seq = 0
    try:
        while True:
            sock.sendall(make_packet(seq))
            seq += 1
    except (BrokenPipeError, ConnectionResetError):
        pass


def receiver(sock):
    expected_seq = 0
    errors = 0
    lost_packets = 0
    retransmissions = 0

    total_bytes = 0
    start_time = time.time()

    buffer = b''

    try:
        while True:
            data = sock.recv(4096)
            if not data:
                break

            buffer += data

            while True:
                # Find magic header
                idx = buffer.find(MAGIC)
                if idx < 0:
                    # keep last few bytes in case magic is split
                    buffer = buffer[-3:]
                    break

                # Discard garbage before magic
                if idx > 0:
                    buffer = buffer[idx:]

                if len(buffer) < PACKET_SIZE:
                    break

                pkt = buffer[:PACKET_SIZE]
                buffer = buffer[PACKET_SIZE:]

                seq = struct.unpack("!I", pkt[4:8])[0]
                payload = pkt[8:]

                # --- Sequence analysis ---
                if seq > expected_seq:
                    lost = seq - expected_seq
                    lost_packets += lost
                    print(f"[LOSS] Lost {lost} packet(s), expected {expected_seq}, got {seq}")
                    expected_seq = seq

                elif seq < expected_seq:
                    retransmissions += 1
                    # Not an error!
                    continue

                # --- Payload verification ---
                if payload != bytes([seq % 256]) * PAYLOAD_SIZE:
                    print(f"[CORRUPTION] seq {seq}")
                    errors += 1

                expected_seq += 1
                total_bytes += len(pkt)

                # --- Stats every second ---
                now = time.time()
                if now - start_time >= 1:
                    mbps = (total_bytes * 8) / 1e6
                    print(
                        f"[STATS] {mbps:.2f} Mbps | "
                        f"errors={errors} | "
                        f"lost={lost_packets} | "
                        f"retrans={retransmissions}"
                    )
                    total_bytes = 0
                    start_time = now

    except ConnectionResetError:
        pass


def start_server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.bind((HOST, PORT))
        server.listen(5)
        print(f"[*] Listening on {HOST}:{PORT}")

        while True:
            client_socket, addr = server.accept()
            print(f"[+] Connection from {addr}")

            threading.Thread(target=sender, args=(client_socket,), daemon=True).start()
            threading.Thread(target=receiver, args=(client_socket,), daemon=True).start()


if __name__ == "__main__":
    start_server()