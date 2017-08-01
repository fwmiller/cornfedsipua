#!/bin/bash

# Change the installation location to your liking. Don't add a trailing
# slash at the end.
PREFIX="/usr"

echo "Removing Cornfed SIP User Agent installation"
sleep 1

rm -f $PREFIX/bin/cornfedsip
rm -f $PREFIX/bin/cornfedsip_cli
rm -f $PREFIX/share/applications/cornfedsip.desktop
rm -f $PREFIX/share/pixmaps/sip_logo_32.png
rm -f $PREFIX/share/pixmaps/sip_logo_48.png
rm -fr $PREFIX/share/cornfed

echo "Installation removed"
