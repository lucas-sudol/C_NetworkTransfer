import subprocess
import time
import os
import sys


def get_file_size(file_path):
    """Get the size of the file in bytes."""
    try:
        return os.path.getsize(file_path)
    except OSError as e:
        print(f"Error: Unable to determine file size - {e}")
        sys.exit(1)


def ping_host(host, ping_file):
    """Ping the server and save results to a file."""
    try:
        with open(ping_file, "w") as f:
            subprocess.run(["ping", "-c", "20", host], stdout=f, stderr=subprocess.STDOUT)
    except Exception as e:
        print(f"Error: Unable to run ping - {e}")
        sys.exit(1)


def run_transfer(file_name, server_address, buffer_size, log_file, num_transfers):
    """Run the file transfers and log the results."""
    file_size = get_file_size(file_name)
    host = server_address.split(":")[0]
    total_time = 0
    min_time = float("inf")
    max_time = 0

    with open(log_file, "w") as log:
        log.write(f"File Size: {file_size} bytes\n")
        log.write(f"Server: {server_address}\n")
        log.write(f"Buffer Size: {buffer_size} bytes\n")

        for i in range(1, num_transfers + 1):
            log.write(f"Transfer {i}...\n")
            try:
                # Start timing
                start_time = time.time()
                subprocess.run(
                    ["./sendFile", file_name, server_address, str(buffer_size)],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                )
                # End timing
                elapsed_time = time.time() - start_time

                log.write(f"Transfer {i} Time: {elapsed_time:.6f} seconds\n")

                # Update metrics
                total_time += elapsed_time
                min_time = min(min_time, elapsed_time)
                max_time = max(max_time, elapsed_time)

            except Exception as e:
                log.write(f"Error during transfer {i}: {e}\n")
                continue

        # Calculate averages and rates
        if num_transfers > 0:
            avg_time = total_time / num_transfers
            avg_rate = file_size / avg_time
            min_rate = file_size / max_time
            max_rate = file_size / min_time
        else:
            avg_time = avg_rate = min_rate = max_rate = 0

        log.write("Summary:\n")
        log.write(f"Min Transfer Time: {min_time:.6f} seconds\n")
        log.write(f"Max Transfer Time: {max_time:.6f} seconds\n")
        log.write(f"Avg Transfer Time: {avg_time:.6f} seconds\n")
        log.write(f"Min Transfer Rate: {min_rate:.6f} bytes/second\n")
        log.write(f"Max Transfer Rate: {max_rate:.6f} bytes/second\n")
        log.write(f"Avg Transfer Rate: {avg_rate:.6f} bytes/second\n")


if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: python test_transfer.py <fileName> <server-hostname>:<port-number> <bufferSize> <numTransfers>")
        sys.exit(1)

    file_name = sys.argv[1]
    server_address = sys.argv[2]
    buffer_size = int(sys.argv[3])
    num_transfers = int(sys.argv[4])

    log_file = "transfer_results.log"
    ping_file = "ping_results.log"

    # Step 1: Ping the server
    host = server_address.split(":")[0]
    print(f"Pinging {host}...")
    ping_host(host, ping_file)

    # Step 2: Run the transfers and log results
    print("Starting file transfers...")
    run_transfer(file_name, server_address, buffer_size, log_file, num_transfers)
    print("File transfers completed.")
