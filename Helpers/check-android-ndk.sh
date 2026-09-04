#!/usr/bin/env bash
if [[ -d .NDK ]]; then
		echo "NDK found, proceeding with compilation..."
else
	echo "NDK not found, please ensure the NDK is located at .NDK/android-ndk-r27d. Please download so and place it on the expected directory."
    exit 1
fi