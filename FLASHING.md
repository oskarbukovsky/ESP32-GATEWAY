# Flashing Instructions for ESP32-GATEWAY

## Hardware Requirements

- **ESP32 Development Board** (with Ethernet module for OLIMEX ESP32-GATEWAY)
- **USB-UART Adapter** or built-in USB-Serial (e.g., CH340, CP2102)
- **Micro-USB Cable** for power and serial communication
- **Ethernet Cable** (optional, for network testing)

## Serial Port Connection

| ESP32 Pin | Adapter Pin |
|-----------|------------|
| TX (GPIO1) | RX |
| RX (GPIO3) | TX |
| GND | GND |

## Software Requirements

Install esptool.py:
```bash
pip install esptool
```

## Finding Your Serial Port

- **Linux/macOS**: `/dev/ttyUSB0`, `/dev/ttyUSB1`, etc. or `/dev/tty.usbserial-*`
- **Windows**: `COM1`, `COM2`, `COM3`, etc.

List available ports:
```bash
esptool.py list-ports
```

## Firmware Files

After building, you'll have:
- `bootloader.bin` - ESP32 bootloader
- `partition-table.bin` - Flash partition configuration
- `ethernet_basic.bin` - Application firmware

## Flashing Methods

### Method 1: One-Line Flash (Recommended)

Replace `/dev/ttyUSB0` with your serial port:

```bash
esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset write_flash \
  0x1000 firmware/bootloader/bootloader.bin \
  0x8000 firmware/partition_table/partition-table.bin \
  0x10000 firmware/ethernet_basic.bin
```

### Method 2: Flash Individually

```bash
# Flash bootloader
esptool.py -p /dev/ttyUSB0 write_flash 0x1000 firmware/bootloader/bootloader.bin

# Flash partition table
esptool.py -p /dev/ttyUSB0 write_flash 0x8000 firmware/partition_table/partition-table.bin

# Flash application
esptool.py -p /dev/ttyUSB0 write_flash 0x10000 firmware/ethernet_basic.bin
```

### Method 3: Erase Before Flashing

If you experience issues or have stale data:

```bash
# Erase entire flash
esptool.py -p /dev/ttyUSB0 erase_flash

# Flash everything fresh
esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset write_flash \
  0x1000 firmware/bootloader/bootloader.bin \
  0x8000 firmware/partition_table/partition-table.bin \
  0x10000 firmware/ethernet_basic.bin
```

## Build Your Own Firmware

```bash
idf.py build
```

Firmware files will be created in `firmware/` directory.

## Verify Flash

Check MAC address:
```bash
esptool.py -p /dev/ttyUSB0 read_mac
```

View flash content:
```bash
esptool.py -p /dev/ttyUSB0 read_flash 0x1000 256 dump.bin
```

## Monitor Serial Output

After flashing, monitor the device:

```bash
# Using ESP-IDF
idf.py monitor -p /dev/ttyUSB0

# Or using pyserial
python -m serial.tools.miniterm /dev/ttyUSB0 115200
```

## Boot Modes

ESP32 has boot modes controlled by GPIO0 and GPIO2 during power-up:

- **Normal Mode** (GPIO0 HIGH): Run application
- **Download Mode** (GPIO0 LOW): Wait for firmware upload

Most boards automatically handle this via DTR/RTS signals.

## Troubleshooting

### "Device not found"
- Check USB cable
- Install USB driver (CH340 or CP2102)
- Check serial port permissions (Linux: `sudo usermod -a -G dialout $USER`)

### "Connecting... Failed to connect to Espressif device"
- Hold BOOT button while starting flash
- Check serial connection
- Try lower baud rate: `-b 115200` instead of `460800`

### "Flash read error"
- Try erasing flash first: `esptool.py -p /dev/ttyUSB0 erase_flash`
- Check power supply (needs stable 3.3V)

### "Checksum error"
- Bad download, retry flash
- Use different USB cable
- Try lower baud rate

## Release Artifacts

Pre-built firmware is available in GitHub releases:
- Download release artifacts directly
- Follow flashing instructions above
- No build tools required

## GPIO Pin Configuration

- **GPIO0**: Boot mode (pulled high for normal boot)
- **GPIO2**: Boot mode (must be low or floating for normal boot)
- **GPIO3**: RX serial (also used for boot mode detection)
- **GPIO1**: TX serial

## Performance Tips

- Use **460800 baud** for faster flashing
- **Erase flash** periodically to maintain performance
- **Monitor RAM** if experiencing random crashes
