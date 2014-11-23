# Your Arduino environment.
ARD_REV = 105
ARD_HOME = $(HOME)/opt/arduino/default
AVR_HOME = $(ARD_HOME)/hardware/tools/avr
ARD_BIN = $(AVR_HOME)/bin
AVRDUDE = $(ARD_HOME)/hardware/tools/avrdude
AVRDUDE_CONF = $(ARD_HOME)/hardware/tools/avrdude.conf

# Your favorite serial monitor.
MON_CMD = screen
MON_SPEED = 9600

# Board settings.
BOARD = uno
PORT = /dev/ttyACM0
PROGRAMMER = arduino

# Where to find header files and libraries.
INC_DIRS = ./inc
LIB_DIRS = $(addprefix $(ARD_HOME)/libraries/, $(LIBS))
LIBS = Ethernet Ethernet/utility SPI

include ../Makefile.master
