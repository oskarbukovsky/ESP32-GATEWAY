@echo off

del debug.esp.log
powershell -NoProfile -Command "esptool -p COM5 -b 460800 --before default_reset --after hard_reset write_flash 0x1000 firmware/bootloader/bootloader.bin 0x8000 firmware/partition_table/partition-table.bin 0x10000 firmware/ethernet_basic.bin 2>&1 | Tee-Object -FilePath 'flashing.log'"

python3 esp_serial_reader.py --port COM5 --log debug.esp.log
