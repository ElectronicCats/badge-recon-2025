# BOARD_TAG = arduino:mbed_rp2040:pico
# BOARD_TAG = arduino:mbed:pico
BOARD_TAG = rp2040:rp2040:rpipico
MONITOR_PORT = $(PORT)

compile:
	arduino-cli compile --fqbn $(BOARD_TAG) --build-path build

upload:
	arduino-cli upload -p $(MONITOR_PORT) --fqbn $(BOARD_TAG) --verbose

monitor:
	arduino-cli monitor -p $(MONITOR_PORT)

clean:
	arduino-cli cache clean
	rm -rf build

wait:
	sleep 2

all: compile upload wait monitor