SRC_DIR      := .
BUILD_DIR    := build
TARGET       := $(BUILD_DIR)/mon-on
SRC          := $(SRC_DIR)/main.c
SERVICE_SRC  := $(SRC_DIR)/mon-on.service
CC           := gcc
CFLAGS       := -Wall -Wextra -O2
LIBS         := -ludev -lX11 -lXrandr
BINDIR       ?= /usr/bin
SYSTEMD_DIR  ?= /etc/systemd/system

.PHONY: all clean install uninstall

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -rf $(BUILD_DIR)

install: all
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/mon-on
	install -d $(SYSTEMD_DIR)
	install -m 644 $(SERVICE_SRC) $(SYSTEMD_DIR)/mon-on.service

uninstall:
	rm -f $(BINDIR)/mon-on
	rm -f $(SYSTEMD_DIR)/mon-on.service
