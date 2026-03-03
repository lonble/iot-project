# name of your application
APPLICATION = karen

# If no BOARD is found in the environment, use this default:
BOARD ?= adafruit-feather-nrf52840-sense

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/RIOT

# specify source files
SRC += main.c
SRC += net.c
SRC += control.c

# enable thread flags
USEMODULE += core_thread_flags

# Enable timer
USEMODULE += ztimer
USEMODULE += ztimer_periodic
USEMODULE += ztimer_msec

# enable default networking devides in the platform
USEMODULE += netdev_default

# automatically initialize network interfaces for enabled devices
USEMODULE += auto_init_gnrc_netif

# enable single interface optimization
USEMODULE += gnrc_netif_single

# add IPv6 support
USEMODULE += gnrc_ipv6_default

# add modules to control LEDs and to handle interrupts
USEMODULE += periph_gpio
USEMODULE += periph_gpio_irq

# enable shell and built-in commands
USEMODULE += shell
USEMODULE += shell_cmds_default

# disable echo of shell command
CFLAGS += -DCONFIG_SHELL_NO_ECHO=1
# disable shell's prompt because of bug
CFLAGS += -DCONFIG_SHELL_NO_PROMPT=1
# default STDIO_RX_BUFSIZE is 64, increase it to match SHELL_DEFAULT_BUFSIZE
CFLAGS += -DSTDIO_RX_BUFSIZE=128

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
