#!/bin/bash
# Sonic 1 Forever (RSDKv4 Team Forever) - RetroPie port
GAMEDIR="/home/pi/SonicOrigins"
cd "$GAMEDIR" || exit 1
exec ./launcher --launch 8
