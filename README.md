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
cmake --preset l476-debug
cmake --build --preset l476-debug
```

Flash the NUCLEO-L476RG:

```bash
STM32_Programmer_CLI \
    -c port=SWD \
    -w build/l476-debug/nucleo_hal.elf \
    -v \
    -rst
```
## Board and MCU selection

The l476-debug and l476-hardware-test presets select `BOARD=NUCLEO_L476RG`. To select
it explicitly, configure with `cmake --preset l476-debug -DBOARD=NUCLEO_L476RG`.
Unknown board names are rejected during configuration. The f446-debug preset
reports an error until F446 board and MCU support is implemented.

- `bsp/common/` contains peripheral drivers and shared clock utilities.
- `bsp/nucleo_l476rg/` contains the board API, device instances, clock setup,
  interrupt routing, and build configuration in `target.cmake`.
- `mcal/stm32l4/` contains unmodified CubeMX-generated sources and ST drivers.

Applications include `board.hpp` and use `platform::board`; the selected board's
include directory supplies that header. Sensor drivers link only to `bsp_common`.
CMake propagates `BSP_BOARD_NUCLEO_L476RG`, `BSP_MCU_STM32L476XX`,
`BSP_MCU_FAMILY_STM32L4`, and ST's `STM32L476xx` definition to dependent targets.
`bsp/common/inc/mcu.hpp` maps those MCU definitions to HAL/LL header macros;
for example, SPI includes `BSP_LL_DMA_HEADER` and `BSP_LL_SPI_HEADER`.

To add another board, add its BSP directory with the same board API and its
own target configuration, then register it in the `cmake/boards.cmake` board selection.
The target configuration selects its MCAL directory, startup file, linker
script, architecture flags, and compile definitions. For another MCU family,
add its header mapping and adapt common drivers where its peripheral APIs
differ. Only the NUCLEO-L476RG is currently implemented. Interrupt handlers are
linked directly into the executable so they override the startup's weak aliases.

## Hardware test firmware

Each file in `tests/hardware/` is a standalone firmware entry point with its own
`int main()`. Select one with `FIRMWARE_TEST`, using the filename without `.cpp`.
The selected test replaces all `appl/src/` sources and shares the BSP,
MCAL, interrupt handlers, and sensor library with the application.

Build the UART polling smoke test:

```bash
cmake --preset l476-hardware-test -DFIRMWARE_TEST=uart
cmake --build --preset l476-hardware-test
```

Build the UART DMA smoke test:

```bash
cmake --preset l476-hardware-test -DFIRMWARE_TEST=uart_dma
cmake --build --preset l476-hardware-test
```

Flash the selected test:

```bash
STM32_Programmer_CLI -c port=SWD -w build/l476-hardware-test/nucleo_hal.elf -v -rst
```

Open the ST-LINK virtual COM port at **115200 baud, 8 data bits, no parity,
one stop bit, no flow control**. Both tests should print one complete line per
second and toggle the status LED. The DMA test prints a failure message and
holds the LED on if a transfer returns an error; a missing completion interrupt
can leave it waiting indefinitely with the LED stopped. These are manual board
smoke tests; compiling them does not verify physical UART output or DMA behavior.

To add a test, create `tests/hardware/my_feature.cpp` with its own `main()`, call
`platform::board::initialize()`, and configure with
`-DFIRMWARE_TEST=my_feature`. No CMake source-list edit is needed. Unknown names
fail during configuration. Reconfigure whenever you change the selected test;
`cmake --build` uses the last configured selection. The l476-hardware-test preset
defaults to `uart`, so pass the desired name each time you configure it.

Build the actual application again:

```bash
cmake --preset l476-debug
cmake --build --preset l476-debug
```

The l476-debug preset explicitly clears `FIRMWARE_TEST`. Application artifacts stay
in `build/l476-debug/`; test artifacts stay in `build/l476-hardware-test/`. To debug a hardware test, use `build/l476-hardware-test/nucleo_hal.elf` and a build task
that runs `cmake --build --preset l476-hardware-test`.

## SPI hardware-in-the-loop test (ESP32-S3 slave)

`tests/hardware/spi_test.cpp` uses the existing blocking `SpiDevice`/`Spi`
transfer path. The ESP32 must already be running slave firmware implementing
the four-byte protocol described below; this test supplies the STM32 side.

Power both boards independently over USB and connect these 3.3 V signals:

| Signal | STM32L476RG | ESP32-S3 |
| --- | --- | --- |
| SCK | PB3 | GPIO4 |
| MOSI | PB5 | GPIO5 |
| MISO | PB4 | GPIO6 |
| CS | PA4 | GPIO7 |
| Ground | GND | GND |

The test uses the standard SPI1 configuration through `board::initialize()`:
full-duplex, 8-bit, MSB-first, Mode 3 at 2.5 MHz (80 MHz peripheral clock
divided by 32), with PB3/PB4/PB5 for SCK/MISO/MOSI. PA4 is the active-low
GPIO chip-select output. One CS assertion covers all four bytes; the driver
waits for SPI to become idle before releasing CS on success. The ESP32 test
additionally requests a minimum 5 us CS hold after the transfer through
`SpiDevice`'s `chip_select_hold_us` constructor argument. The delay uses the
platform's Cortex-M4 cycle counter; interrupts can extend it. Other devices
default to zero added hold time.
All GPIO and peripheral setup lives in `bsp`; the test uses
`board::spi1()`, `board::pins::pa4`, and `SpiDevice` for transactions, sharing
the application's SPI1 configuration. Disconnect the ADXL345 and other SPI
devices for this test because PA4 is also the application's ADXL345 chip select.
Configure the ESP32 slave for Mode 3 before running this test.

Build and flash from the repository root:

```bash
cmake --preset l476-hardware-test -DFIRMWARE_TEST=spi_test
cmake --build --preset l476-hardware-test
STM32_Programmer_CLI -c port=SWD -w build/l476-hardware-test/nucleo_hal.elf -v -rst
```

1. Open the STM32 ST-LINK virtual COM port at **115200 baud, 8N1, no flow control**.
   Reset the STM32 if you missed the startup message.
2. Reset the ESP32 and wait until its SPI slave has queued the initial response
   `5A 10 00 00` and is ready to receive. It must use Mode 3 and four-byte transfers.
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
readiness, the previous-transaction response rule, Mode 3, common ground, and
MOSI/MISO wiring. With a logic analyzer, expect CS low across exactly 32 clocks
per request and SCK idle high. PASS validates the protocol responses; verify LED
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
