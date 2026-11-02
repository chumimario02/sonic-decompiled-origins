#!/bin/bash
# Sonic CD (RSDKv3) - RetroPie port
GAMEDIR="/home/pi/SonicOrigins"
cd "$GAMEDIR" || exit 1
exec ./launcher --launch 3
