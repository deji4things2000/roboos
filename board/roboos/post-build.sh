#!/bin/sh
#
# RoboOS post-build script
# Enables I2C for the PiCar-X Robot HAT
#

CONFIG_TXT="${BINARIES_DIR}/rpi-firmware/config.txt"

if [ -f "$CONFIG_TXT" ]; then
    grep -q "^dtparam=i2c_arm=on" "$CONFIG_TXT" || echo "dtparam=i2c_arm=on" >> "$CONFIG_TXT"
    grep -q "^dtparam=i2c1=on" "$CONFIG_TXT" || echo "dtparam=i2c1=on" >> "$CONFIG_TXT"
    echo "RoboOS: I2C parameters added to $CONFIG_TXT"
else
    echo "RoboOS: WARNING - config.txt not found at $CONFIG_TXT"
fi
