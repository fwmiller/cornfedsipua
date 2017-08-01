#!/bin/bash

# Change the installation location to your liking. Don't add a trailing
# slash at the end.
PREFIX="/usr"

echo "Installing on " $PREFIX
sleep 1

install -m 755 cornfedsip $PREFIX/bin/cornfedsip
install -m 755 cornfedsip_cli $PREFIX/bin/cornfedsip_cli
install -m 755 cornfedsip.desktop $PREFIX/share/applications/cornfedsip.desktop
install -m 755 sip_logo_32.png $PREFIX/share/pixmaps/sip_logo_32.png
install -m 755 sip_logo_48.png $PREFIX/share/pixmaps/sip_logo_48.png
mkdir -p $PREFIX/share/cornfed
install -m 755 ring.wav $PREFIX/share/cornfed/ring.wav
install -m 755 cornfedsipua.pdf $PREFIX/share/cornfed/cornfedsipua.pdf

echo "Installation is complete. If you can not find the application shortcut under the Network sub-menu on your desktop, load it from a terminal by invoking the command: cornfedsip"
