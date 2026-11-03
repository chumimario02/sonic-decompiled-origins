#!/bin/bash
# SRB2 (Sonic Robo Blast 2) via Flatpak - RetroPie port
# Instala primero:
#   sudo apt install flatpak
#   flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
#   flatpak install -y flathub org.srb2.SRB2
exec flatpak run org.srb2.SRB2
