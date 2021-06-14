INCLUDEPATH +=  $$PWD

HEADERS +=      $$PWD/nfc.h \
                $$PWD/nfc-emulation.h \
                $$PWD/nfc-types.h

LIBS +=         $$PWD/libnfc.a \
                $$PWD/libusb.a

CONFIG *=       c++11
