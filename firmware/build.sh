#!/bin/bash

# Create output directory if it doesn't exist
mkdir -p output

# Build Docker image and run container
docker-compose up --build

echo ""
echo "Build complete! The UF2 file is in the 'output' directory."
echo "To flash your RP2040 board:"
echo "1. Connect the board while holding the BOOTSEL button"
echo "2. Release the button after connecting"
echo "3. Copy the UF2 file to the mounted RPI-RP2 drive"