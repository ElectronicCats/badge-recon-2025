BOARD_TAG = arduino:mbed_rp2040:pico
MONITOR_PORT = $(PORT)

compile:
	arduino-cli compile --fqbn $(BOARD_TAG)

upload:
	arduino-cli upload -p $(MONITOR_PORT) --fqbn $(BOARD_TAG) --verbose

monitor:
	arduino-cli monitor -p $(MONITOR_PORT)

clean:
	arduino-cli cache clean

wait:
	sleep 2

all: compile upload wait monitor