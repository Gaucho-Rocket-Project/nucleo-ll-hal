# Nucleo-l476rg Hardware Abstraction Layer (HAL)

## Prerequisites

Install the following tools:

- CMake
- Ninja
- Arm GNU Toolchain
- STM32CubeProgrammer

## Build and Flash

Run all commands from the repository root.

Configure the Debug build:

```bash
cmake --preset Debug
cmake --build --preset Debug
```

Flash the NUCLEO-L476RG:

```bash
STM32_Programmer_CLI \
    -c port=SWD \
    -w build/Debug/nucleo_hal.elf \
    -v \
    -rst
```
## Hardware test firmware

Each file in `tests/hardware/` is a standalone firmware entry point with its own
`int main()`. Select one with `FIRMWARE_TEST`, using the filename without `.cpp`.
The selected test replaces all `appl/src/` sources and shares the platform,
MCAL, interrupt handlers, and sensor library with the application.

Build the UART polling smoke test:

```bash
cmake --preset HardwareTest -DFIRMWARE_TEST=uart
cmake --build --preset HardwareTest
```

Build the UART DMA smoke test:

```bash
cmake --preset HardwareTest -DFIRMWARE_TEST=uart_dma
cmake --build --preset HardwareTest
```

Flash the selected test:

```bash
STM32_Programmer_CLI -c port=SWD -w build/HardwareTest/nucleo_hal.elf -v -rst
```

Open the ST-LINK virtual COM port at **115200 baud, 8 data bits, no parity,
one stop bit, no flow control**. Both tests should print one complete line per
second and toggle the status LED. The DMA test prints a failure message and
holds the LED on if a transfer returns an error; a missing completion interrupt
can leave it waiting indefinitely with the LED stopped. These are manual board
smoke tests; compiling them does not verify physical UART output or DMA behavior.

To add a test, create `tests/hardware/my_feature.cpp` with its own `main()`, call
`platform::nucleo_l476rg::initialize()`, and configure with
`-DFIRMWARE_TEST=my_feature`. No CMake source-list edit is needed. Unknown names
fail during configuration. Reconfigure whenever you change the selected test;
`cmake --build` uses the last configured selection. The HardwareTest preset
defaults to `uart`, so pass the desired name each time you configure it.

Build the actual application again:

```bash
cmake --preset Debug
cmake --build --preset Debug
```

The Debug preset explicitly clears `FIRMWARE_TEST`. Application artifacts stay
in `build/Debug/`; test artifacts stay in `build/HardwareTest/`. The existing
VS Code launch configuration still builds and launches the Debug application.
To debug a hardware test, use `build/HardwareTest/nucleo_hal.elf` and a build task
that runs `cmake --build --preset HardwareTest`.

## SPI hardware-in-the-loop test (ESP32-S3 slave)

`tests/hardware/spi_test.cpp` uses the existing blocking `SpiDevice`/`Spi`
transfer path. The ESP32 must already be running slave firmware implementing
the four-byte protocol described below; this test supplies the STM32 side.

Power both boards independently over USB and connect these 3.3 V signals:

| Signal | STM32L476RG | ESP32-S3 |
| --- | --- | --- |
| SCK | PA5 | GPIO4 |
| MOSI | PA7 | GPIO5 |
| MISO | PA6 | GPIO6 |
| CS | PB6 | GPIO7 |
| Ground | GND | GND |

The test selects the platform's `Spi1Profile::arduino_mode0` board configuration
through `board::initialize()`. The platform configures SPI1 for full-duplex,
8-bit, MSB-first, Mode 0 at 625 kHz
(80 MHz peripheral clock divided by 128). PB6 is an active-low GPIO output.
One CS assertion covers all four bytes; the driver waits for SPI to become idle
before releasing CS on success. The ESP32 test additionally requests a minimum
5 us CS hold after the transfer through `SpiDevice`'s `chip_select_hold_us`
constructor argument. The delay uses the platform's Cortex-M4 cycle counter;
interrupts can extend it. Other devices default to zero added hold time.
All GPIO and peripheral setup lives in
`platform`; the test uses `board::spi1()`, `board::pins::pb6`, and `SpiDevice`
for transactions. The default board initialization retains the application's
CubeMX settings. PA5 is also the Nucleo status LED pin, so this test does not use the
status LED for reporting. Disconnect other SPI devices for this test.

Build and flash from the repository root:

```bash
cmake --preset HardwareTest -DFIRMWARE_TEST=spi_test
cmake --build --preset HardwareTest
STM32_Programmer_CLI -c port=SWD -w build/HardwareTest/nucleo_hal.elf -v -rst
```

1. Open the STM32 ST-LINK virtual COM port at **115200 baud, 8N1, no flow control**.
   Reset the STM32 if you missed the startup message.
2. Reset the ESP32 and wait until its SPI slave has queued the initial response
   `5A 10 00 00` and is ready to receive. It must use Mode 0 and four-byte transfers.
3. Press the STM32 USER button once to start the test.
4. Read the TX, RX, EXPECT, and OK lines. A successful run ends with
   `PASS: READY and all five command acknowledgments verified`.
5. Before each retry, reset the ESP32 again so its response pipeline starts at
   READY; then press USER. Resetting just the STM32 does not reset the slave.

Requests are `A5 COMMAND 00 SEQUENCE`. The sequence is RED/1, GREEN/2, BLUE/3,
WHITE/4, OFF/5, with at least 10 ms between transfers. The first received frame
must be `5A 10 00 00`; later frames must be `5A 00 PREVIOUS_COMMAND PREVIOUS_SEQUENCE`.
A sixth OFF/6 request clocks out the acknowledgment for OFF/5. Its own
acknowledgment is intentionally not read. For example:

```text
TX:     A5 02 00 02
RX:     5A 00 01 01
EXPECT: 5A 00 01 01
OK
```

The ESP32 LED should follow red, green, blue, white, off. At the initial 10 ms
gap, colors change too quickly for reliable visual inspection; increase
`transaction_gap_ms` in the test to 500 or 1000 for visual checking. UART logging
also adds time between transactions.

A mismatch prints the actual and expected frames and stops the run. Check slave
readiness, the previous-transaction response rule, Mode 0, common ground, and
MOSI/MISO wiring. With a logic analyzer, expect CS low across exactly 32 clocks
per request and SCK idle low. PASS validates the protocol responses; verify LED
behavior on the ESP32 separately. Building alone does not validate hardware.

For last-bit errors, the 5 us CS hold is a timing mitigation to verify on the
boards, not a confirmed hardware fix. Espressif's
[SPI slave sender example](https://github.com/espressif/esp-idf/blob/master/examples/peripherals/spi_slave/sender/main/app_main.c)
also extends CS after the transfer to avoid losing the last bit.
Rebuild and flash the test above, reset the ESP32, and rerun several times.
Look for the new CS hold banner to confirm the updated firmware is running.
For comparison, set `chip_select_hold_us` in `spi_test.cpp` to `0U` and rebuild.
If the mismatch persists, compare the ESP32's received request and prepared
response to distinguish MOSI corruption from a MISO/response issue. A logic
analyzer can verify the final clock edge precedes CS rising by at least 5 us.
