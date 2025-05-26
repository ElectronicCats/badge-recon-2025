# REcon Badge 2025

This is the oficial repository of the REcon Badge 2025. Here you will find the source code for the badge, as well as the documentation and instructions to build it.

## Getting Started

Your badge comes with a pre-installed firmware, but in case you want to reprogram it, you can follow these steps:

1. Download the latest `firmware.uf2` file from the [releases](https://github.com/ElectronicCats/badge-recon-2025/releases/latest) page.

2. Connect the badge to your computer using a USB-C cable.

3. Press and hold the button on the badge while connecting it to the USB port. This will put the badge in bootloader mode.

4. Once the badge is connected, it will appear as a USB drive named `RPI-RP2`. Copy the `firmware.uf2` file to the root of the drive.

5. After copying the file, the badge will automatically reboot and start running the new firmware.

> **Note:** Sometimes you may need to press the reset button on the badge after copying the firmware file to ensure it starts correctly.

### Building the Firmware

To build the firmware from source, you will need to have [Docker](https://www.docker.com/) installed on your machine. Once you have Docker set up, you can follow these steps:

1. Clone the repository:

Using HTTPS:

```bash
git clone https://github.com/ElectronicCats/badge-recon-2025.git
```

Using SSH:

```bash
git clone git@github.com:ElectronicCats/badge-recon-2025.git
```

2. Navigate to the firmware directory:

```bash
cd badge-recon-2025/firmware
```

3. Build the firmware using Docker:

```bash
./build.sh
```

This script will create a `firmware.ino.uf2` file in the `firmware/build` directory.

#### Flashing the Firmware

To flash the firmware to the badge, follow the same steps as mentioned in the "Getting Started" section, but use the newly built `firmware.ino.uf2` file instead of the pre-built one.

## Maintainer

<a
href="https://github.com/sponsors/ElectronicCats">

<img  src="https://electroniccats.com/wp-content/uploads/2020/07/Badge_GHS.png"  height="104" />

</a>

Electronic Cats invests time and resources providing this open source design, please support Electronic Cats and open-source hardware by purchasing products from Electronic Cats!

[Agregando el link como referencia]: <https://github.com/ElectronicCats/Template-Project-KiCAD-CI>
