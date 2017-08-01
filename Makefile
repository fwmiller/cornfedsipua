include MakeConfig

TARBALL_NAME = cornfedsipua-1.1.6.tar.gz

all:	
	cd src && $(MAKE)
	cd stun && $(MAKE)
	cd codec && $(MAKE)
ifeq ($(CODEC_G729),-D_CODEC_G729)
	cd codec/codec_g729 && $(MAKE)
endif
	cd hist && $(MAKE)
	cd cli && $(MAKE)
	cd gnome && $(MAKE)

tarball: all
	mkdir cornfedsipua
	cp install/README cornfedsipua/README
	cp gnome/sip cornfedsipua/cornfedsip
	cp cli/sip cornfedsipua/cornfedsip_cli
	cp install/cornfedsip.desktop cornfedsipua/cornfedsip.desktop
	cp install/sip_logo_32.png cornfedsipua/sip_logo_32.png
	cp install/sip_logo_48.png cornfedsipua/sip_logo_48.png
	cp install/install.sh cornfedsipua/install.sh
	cp install/uninstall.sh cornfedsipua/uninstall.sh
	cp wav/ring.wav cornfedsipua/ring.wav
	cp doc/user/user.pdf cornfedsipua/cornfedsipua.pdf
	tar czvf $(TARBALL_NAME) cornfedsipua

clean:
	cd src; make clean
	cd stun; make clean
	cd codec; make clean
ifeq ($(CODEC_G729),-D_CODEC_G729)
	cd codec/codec_g729; make clean
endif
	cd hist; make clean
	cd cli; make clean
	cd gnome; make clean
	rm -fr cornfedsipua
	rm -f $(TARBALL_NAME)

distclean: clean
	rm -f cornfedsipua.tgz
	rm -fr cornfedsipua

wc: clean
	wc -l include/*.h src/*.c stun/*.h stun/*.c codec/*.c cli/*.h cli/*.c hist/*.c gnome/*.h gnome/*.c
