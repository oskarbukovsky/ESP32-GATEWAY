@echo off

REM Ensure logs directory exists
if not exist logs (
	mkdir logs
)

REM Remove previous runtime debug log if present
if exist logs\debug.esp.log (
	del logs\debug.esp.log
)

REM Flash firmware and tee the esptool output into logs\flashing.log while still showing it
REM Note: this captures stdout and stderr
powershell -NoProfile -Command "esptool -p COM5 -b 460800 --before default_reset --after hard_reset write_flash 0x1000 firmware/bootloader/bootloader.bin 0x8000 firmware/partition_table/partition-table.bin 0x10000 firmware/ethernet_basic.bin 2>&1 | Tee-Object -FilePath '%CD%\\logs\\flashing.log'"

REM After flashing, start the serial reader and write serial logs to logs\debug.esp.log
python3 esp_serial_reader.py --port COM5 --log logs\debug.esp.log
