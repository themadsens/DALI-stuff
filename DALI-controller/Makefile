# vim: set ft=make:
#


# Name of the program and source .ino (previously .pde) file.
# No extension here (e.g. PROJECT = Blink).
PROJECT =	$(shell basename `pwd`)

# Project version. Only used for packing the source into an archive.
VERSION =	1.0

# Arduino model. E.g. atmega328, mega, mega2560, uno.
# Valid model names can be found in $(ARDUINO_DIR)/hardware/arduino/boards.txt
# This must be set to a valid model name.
#ARDUINO_MODEL = atmega328
#ARDUINO_MODEL = nano328  # Is set to a 168 CPU
#ARDUINO_MODEL = mega
ARDUINO_MODEL = nano

# USB port the Arduino board is connected to.
# Linux: e.g. /dev/ttyUSB0, or /dev/ttyACM0 for the Uno.
# BSD:   e.g. /dev/cuaU0
# It is a good idea to use udev rules to create a device name that is constant,
# based on the serial number etc. of the USB device.
# See e.g. 97-avr-ftdi.rules from the same download location as this Makefile.
#PORT =		/dev/ttyUSB0
PORT =		$(shell echo /dev/cu.wchusbserial*)

# Arduino variant (for Arduino 1.0+). Makefile.inc makes educated guess from ARDUINO_MODEL
# Directory containing the pins_arduino.h file.
#ARDUINO_VARIANT=$(ARDUINO_DIR)/hardware/arduino/variants/standard

# Standard Arduino libraries used, e.g. EEPROM, LiquidCrystal.
# Give the name of the directory containing the library source files.
ARDUINO_LIBS =
#ARDUINO_LIBS += EEPROM
#ARDUINO_LIBS +=	MaxMatrix
#ARDUINO_LIBS +=	OneWire

# User libraries (in ~/sketchbook/libraries/).
# Give the name of the directory containing the library source files.
USER_LIBS =	
#USER_LIBS +=	MaxMatrix
#USER_LIBS +=	OneWire

# Additional pre-compiled libraries to link with.
# Always leave the math (m) library last!
# The Arduino core library is automatically linked in.
# If the library is in a location the compiler doesn't already know, also
# give the directory with -L.
# Note this is dealing with real libraries (libXXX.a), not Arduino "libraries"!
LDLIBS =
LDLIBS	 +=-lm

UPLOAD_RATE         = 57600
MCU                 = atmega328p
F_CPU               = 16000000L
AVRDUDE_PROGRAMMER  = arduino

-include ../../arduino-playground/Makefile.conf
-include ../../arduino-playground/Makefile.inc

connect:
	cu -s $(UPLOAD_RATE) -l $(PORT)

talk: history.txt
	rlwrap --always-readline  --command-name dali --file . --history-filename history.txt \
	    cu -h -s $(UPLOAD_RATE) -l $(PORT)
history.txt:
	touch $@
