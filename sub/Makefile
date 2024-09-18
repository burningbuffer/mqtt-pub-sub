CFLAGS = -Wall -Wextra -std=c99

PKG_CONFIG_FLAGS = `pkg-config --cflags --libs gtk+-3.0 libmosquitto`

build:
	gcc ./*.c $(CFLAGS) $(PKG_CONFIG_FLAGS) -o sub