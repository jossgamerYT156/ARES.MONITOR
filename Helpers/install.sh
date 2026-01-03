#!/usr/bin/env bash
BNAME="ares-mon"

LINUX_BINARY="bin/ARES.MON.LINUX_x64"
INSTALL_DIR="/System/Programs/Local/bin"
DEF_INSTDIR="$HOME/.local/bin"

# Use [ -d ] to check for directory existence directly
# Adding spaces inside brackets is mandatory in Bash
if [ -d "$INSTALL_DIR" ]; then
    echo "LDS DPATH found. Installing to $INSTALL_DIR..."
    sudo cp "$LINUX_BINARY" "$INSTALL_DIR/$BNAME" && sudo chmod +x $INSTALL_DIR/$BNAME
else
    echo "LDS DPATH not found. Defaulting to $DEF_INSTDIR..."
    sudo cp "$LINUX_BINARY" "$DEF_INSTDIR/$BNAME" && sudo chmod +x $INSTALL_DIR/$BNAME
fi
