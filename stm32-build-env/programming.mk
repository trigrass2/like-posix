
dfu:
	python $(BUILD_ENV_DIR)/tools/dfu.py -b $(FLASH_BASE_ADDRESS):$(OUTPUT_PREFIX).bin $(OUTPUT_PREFIX).dfu

dfuutil:
	dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -R -D $(OUTPUT_PREFIX).dfu
	
stm32loader:
	stm32loader.py -p /dev/ttyUSB0 -b 230400 -e -w -v -i -f $(OUTPUT_PREFIX).bin

qstlink2:
	qstlink2 -cwVR $(OUTPUT_PREFIX).bin
	
#jtag: all
#	echo "reset halt" | nc localhost 4444
#	sleep 1
#	echo "stm32f1x mass_erase 0" | nc localhost 4444
#	sleep 1
#	echo "flash write_bank 0 Debug/stm32_freertos_example.bin 0" | nc localhost 4444
#	sleep 2
#	echo "reset halt" | nc localhost 4444
#
#oldjtag: all
#	echo "reset halt" | nc localhost 4444
#	echo "stm32f1x mass_erase 0" | nc localhost 4444
#	sleep 1
#	echo "flash write_bank 0 $(OUTPUT_PREFIX).bin 0" | nc localhost 4444
#	sleep 2
#	echo "reset halt" | nc localhost 4444
#
#run: jtag
#	echo "reset run" | nc localhost 4444