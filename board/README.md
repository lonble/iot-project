# karen

This is an embedded program written for development boards based on RIOT OS. It communicates via the 802.15.4 protocol to collect RSSI and LQI data.

## Build Instructions

**Operating System**: **Linux** is required

**Dependencies**:

- `arm-none-eabi-gcc`, compiler
- `arm-none-eabi-newlib`, standard libc
- `python3`
- `python3-pyserial`
- `python3-psutil`
- `python3-twisted`

### Prerequisites

1. Run the following to grant **serial port** access to the current user.

```sh
sudo usermod -aG dialout <username>
```

2. Enable **udisks automounting** in your system settings.

The specific steps depend on your distribution. If you cannot find it, press the **Reset** button twice before flashing the board, then mount it manually.

3. Run the following command to synchronize the **system time**, especially if you are **dual-booting** Windows and Linux on the same device.

```sh
sudo systemctl enable --now systemd-timesyncd.service
```

### Build and Flash

1. Clone the repository and remember to pull the **submodules** as well.

```sh
git clone --recurse-submodules `https://github.com/lonble/iot-project.git`
```

If you have already cloned the main repository but forgot to include the submodules, you can use the following command.

```sh
git submodule update --init --recursive
```

2. `cd` into the `board` directory.

```sh
cd iot-project/board
```

3. Make sure the board is connected, then run the command below to build and flash.

```sh
make flash
```

## User Guide

### Features Overview
The board has a power LED, two custom LEDs, a reset button, and a custom button.

When the **custom button** is pressed, the board will send high-frequency IPv6 packets via the 802.15.4 protocol to our dedicated multicast group for **30 minutes**. You can press the button again to terminate the process early.

During the sending process, **LED 0** will remain lit; you can use it to determine whether the board is currently sending data.

Upon receiving a network packet, the board will print the source **IPv6 address, RSSI, and LQI**, and **LED 1** will light up.

The duration of **LED 1** within a cycle is correlated with the packet loss rate: it remains steady on if there is no packet loss, flashes if some packets are lost, and turns off if no data is received. This allows you to visually assess the network quality.

### Data Collection Process

1. **Determine Maximum Transmission Range**

The development board has been set to its maximum transmit power according to project requirements. One tester acts as the sender, staying stationary and pressing the send button. The other two testers act as receivers, gradually moving away from the sender until LED 1 is nearly extinguished. The distance at this point is the maximum transmission range. The sender should stop transmitting at this time.

2. **Collect Data**

Receivers should ensure their boards are connected and navigate to the `board` directory. Then, run the following command to start **pyterm**.

```sh
make term
```

Once pyterm is running on all receiving devices, the sender can press the send button; data will then be printed within pyterm. pyterm will display the log file's location upon startup; locate this file and push it to the repository.

3. **Swap roles and change locations to repeat the experiment**
