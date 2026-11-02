#!/bin/bash
# Sonic 2 (RSDKv4) - RetroPie port
GAMEDIR="/home/pi/SonicOrigins"
cd "$GAMEDIR" || exit 1
exec ./launcher --launch 1
