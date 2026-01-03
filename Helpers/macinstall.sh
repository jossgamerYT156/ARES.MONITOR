# 
#   Helpers/macinstall.sh
# helper script to copy files locally to a $PATH available directory to $USER.
# 
#!/usr/bin/env bash

# Define binaries and install directory.
BINARY_X86="bin/ARES.MON.MACOS_x64"
INSTALL_DIR="$HOME/.local/bin"

# Determine Architecture, so we can flip off arm64 users, yes, we do not use that at LDS LLC.
## Comment out the arm if block if you wish to accept the risk of your actions.
ARCH=$(uname -m)
echo "Detected architecture: $ARCH"

if [ "$ARCH" == "arm64" ]; then
    echo arm64 architecture is NOT supported by ARES.
else
    SOURCE_BIN=$BINARY_X86
fi

# Ensure the destination exists
if [ ! -d "$INSTALL_DIR" ]; then
    echo "Creating $INSTALL_DIR..."
    sudo mkdir -p "$INSTALL_DIR"
fi

echo "Installing $SOURCE_BIN to $INSTALL_DIR..."
cp "$SOURCE_BIN" "$INSTALL_DIR/ares-mon"
sudo chmod +x "$INSTALL_DIR/ares-mon"

echo "Installation complete."