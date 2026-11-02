#!/bin/bash
# Sonic 1 (RSDKv4) - RetroPie port
GAMEDIR="/home/pi/SonicOrigins"
cd "$GAMEDIR" || exit 1
exec ./launcher --launch 0
