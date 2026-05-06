@echo off
REM Flash firmware and tee the esptool output into flashing.log while still showing it
REM Uses PowerShell's Tee-Object to capture both stdout and stderr

powershell -NoProfile -Command "esptool -p COM5 -b 460800 --before default_reset --after hard_reset write_flash 0x1000 firmware/bootloader/bootloader.bin 0x8000 firmware/partition_table/partition-table.bin 0x10000 firmware/ethernet_basic.bin 2>&1 | Tee-Object -FilePath 'flashing.log'"

REM After flashing, start the serial reader and write serial logs to debug.esp.log
python3 esp_serial_reader.py --port COM5 --log debug.esp.log

REM Exit code from esptool is returned by PowerShell; batch will continue to run serial reader regardless