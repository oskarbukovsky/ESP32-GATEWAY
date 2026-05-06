#!/usr/bin/env python3
"""Simple ESP32 serial log reader.

Usage examples:
  python esp_serial_reader.py --port COM5
  python esp_serial_reader.py --port COM5 --baud 115200 --log esp.log
  python esp_serial_reader.py --list
"""

from __future__ import annotations

import argparse
import re
import sys
import time
from datetime import datetime

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("Missing dependency: pyserial")
    print("Install with: pip install pyserial")
    raise SystemExit(1)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Read ESP logs from serial port")
    parser.add_argument("--port", help="Serial port, e.g. COM5 or /dev/ttyUSB0")
    parser.add_argument(
        "--baud", type=int, default=115200, help="Baud rate (default: 115200)"
    )
    parser.add_argument(
        "--timeout", type=float, default=0.2, help="Read timeout in seconds"
    )
    parser.add_argument("--log", help="Optional log file path")
    parser.add_argument(
        "--list", action="store_true", help="List serial ports and exit"
    )
    parser.add_argument(
        "--reconnect-delay",
        type=float,
        default=1.5,
        help="Delay before reconnect attempt in seconds",
    )
    parser.add_argument(
        "--no-colors", action="store_true", help="Strip ANSI color codes (useful for file output)"
    )
    return parser.parse_args()


def print_ports() -> None:
    ports = list(list_ports.comports())
    if not ports:
        print("No serial ports found")
        return

    print("Available serial ports:")
    for p in ports:
        desc = p.description or ""
        hwid = p.hwid or ""
        print(f"  {p.device:12} {desc} {hwid}".rstrip())


def choose_port(cli_port: str | None) -> str:
    if cli_port:
        return cli_port

    ports = list(list_ports.comports())
    if not ports:
        print("No serial ports found. Use --port COMx")
        raise SystemExit(2)

    # Pick the first available port if user did not specify one.
    return ports[0].device


def strip_ansi_codes(text: str) -> str:
    """Remove ANSI escape sequences (color codes, etc.) from text."""
    ansi_pattern = re.compile(r"\x1b\[[0-9;]*m")
    return ansi_pattern.sub("", text)


def run_reader(
    port: str, baud: int, timeout: float, log_path: str | None, reconnect_delay: float, no_colors: bool = False
) -> None:
    log_file = open(log_path, "a", encoding="utf-8") if log_path else None

    try:
        while True:
            ser = None
            try:
                ser = serial.Serial(port=port, baudrate=baud, timeout=timeout)
                print(f"Connected to {port} @ {baud} baud. Press Ctrl+C to stop.")

                while True:
                    raw = ser.readline()
                    if not raw:
                        continue

                    line = raw.decode("utf-8", errors="replace").rstrip("\r\n")
                    
                    # Strip colors for terminal output if requested or for file output
                    display_line = strip_ansi_codes(line) if (no_colors or log_file) else line
                    print(display_line, flush=True)

                    if log_file:
                        log_file.write(strip_ansi_codes(line) + "\n")
                        log_file.flush()

            except serial.SerialException as exc:
                print(f"Serial error: {exc}")
                print(f"Reconnecting in {reconnect_delay:.1f}s...")
                time.sleep(reconnect_delay)
            finally:
                if ser is not None and ser.is_open:
                    ser.close()

    except KeyboardInterrupt:
        print("\nStopped by user")
    finally:
        if log_file:
            log_file.close()


def main() -> None:
    args = parse_args()

    if args.list:
        print_ports()
        return

    port = choose_port(args.port)
    run_reader(
        port=port,
        baud=args.baud,
        timeout=args.timeout,
        log_path=args.log,
        reconnect_delay=args.reconnect_delay,
        no_colors=args.no_colors,
    )


if __name__ == "__main__":
    main()
