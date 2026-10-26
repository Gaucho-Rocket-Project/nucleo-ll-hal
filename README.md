# Nucleo Hardware Abstraction Layer (HAL)

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
The `f446-debug` and `f446-hardware-test` presets select `BOARD=NUCLEO_F446RE`.
Unknown board names are rejected during configuration.

```bash
cmake --preset f446-debug
cmake --build --preset f446-debug
```

- `bsp/common/` contains peripheral drivers and shared clock utilities.
- `bsp/nucleo_l476rg/` and `bsp/nucleo_f446re/` contain each board's API, device instances, clock setup,
  interrupt routing, and build configuration in `target.cmake`.
- `mcal/stm32l4/` and `mcal/stm32f4/` contain unmodified CubeMX-generated sources and ST drivers.

Applications include `board.hpp` and use `bsp::board`; the selected board's
include directory supplies that header. Sensor drivers link only to `bsp_common`.
CMake propagates `BSP_BOARD_NUCLEO_L476RG`, `BSP_MCU_STM32L476XX`,
`BSP_MCU_FAMILY_STM32L4`, and ST's `STM32L476xx` definition to dependent targets
for L476. F446 uses `BSP_BOARD_NUCLEO_F446RE`, `BSP_MCU_STM32F446XX`,
`BSP_MCU_FAMILY_STM32F4`, and `STM32F446xx`.
`bsp/common/inc/mcu.hpp` maps those MCU definitions to HAL/LL header macros;
for example, SPI includes `BSP_LL_DMA_HEADER` and `BSP_LL_SPI_HEADER`.

To add another board, add its BSP directory with the same board API and its
own target configuration, then register it in the `cmake/boards.cmake` board selection.
The target configuration selects its MCAL directory, startup file, linker
script, architecture flags, and compile definitions. For another MCU family,
add its header mapping and adapt common drivers where its peripheral APIs
differ. NUCLEO-L476RG and NUCLEO-F446RE are implemented. Interrupt handlers are
linked directly into the executable so they override the startup's weak aliases.
For F446, source-specific compiler definitions rename CubeMX's peripheral IRQ
stubs, letting BSP handlers own those vectors while retaining generated fault
and SysTick handlers. No generated files need editing.

| Configuration | NUCLEO-L476RG | NUCLEO-F446RE |
| --- | --- | --- |
| Core / APB1 / APB2 | 80 / 80 / 80 MHz | 84 / 42 / 84 MHz |
| SPI1 SCK / MISO / MOSI | PB3 / PB4 / PB5 | PB3 / PA6 / PA7 |
| SPI1 chip select | PA4 | PA4 (configured by BSP) |
| SPI1 mode / clock | Mode 3 / 2.5 MHz | Mode 3 / 2.625 MHz |
| SPI1 RX / TX DMA | DMA1 channels 2 / 3 | DMA2 streams 0 / 3, channel 3 requests |
| USART2 TX DMA | DMA1 channel 7 | DMA1 stream 6, channel 4 requests |
| LED / button | PA5 / PC13 | PA5 / PC13 |
| CAN1 RX / TX | PB8 / PB9 | PB8 / PB9 |

F446 uses the checked-in CubeMX HSI/PLL clock settings at 84 MHz. Its BSP changes
SPI1 from CubeMX's Mode 0 to Mode 3 before enabling it to support the existing
ADXL345 application. Both boards enable the HAL 1 ms tick before CAN initialization
and retain CubeMX's 500 kbit/s CAN timing. F446 is configured for internal
loopback; the current L476 generated configuration uses normal bus mode.
`can_test` expects loopback, so L476 requires a matching CubeMX configuration
before that test can pass on hardware.
The common DMA layer handles L4 channels and F4 streams, including waiting for
F4 stream disable, clearing stale flags, and routing direct-mode/FIFO errors.

## Hardware test firmware

Each file in `tests/hardware/` is a standalone firmware entry point with its own
`int main()`. Select one with `FIRMWARE_TEST`, using the filename without `.cpp`.
The selected test replaces all `appl/src/` sources and shares the BSP,
MCAL, interrupt handlers, and sensor library with the application.

Use `f446-hardware-test` instead of `l476-hardware-test` for F446, and flash
`build/f446-hardware-test/nucleo_hal.elf`. Both support `uart`, `uart_dma`,
`spi_test`, and `can_test`. Builds alone do not validate board behavior.

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
`bsp::board::initialize()`, and configure with
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

| Signal | STM32L476RG | STM32F446RE | ESP32-S3 |
| --- | --- | --- | --- |
| SCK | PB3 | PB3 | GPIO4 |
| MOSI | PB5 | PA7 | GPIO5 |
| MISO | PB4 | PA6 | GPIO6 |
| CS | PA4 | PA4 | GPIO7 |
| Ground | GND | GND | GND |

The test uses the standard SPI1 configuration through `board::initialize()`:
full-duplex, 8-bit, MSB-first, Mode 3 at 2.5 MHz (80 MHz peripheral clock
divided by 32), with PB3/PB4/PB5 for SCK/MISO/MOSI. PA4 is the active-low
GPIO chip-select output. F446 uses the pins in the table above at 2.625 MHz
(84 MHz peripheral clock divided by 32), also Mode 3. One CS assertion covers all four bytes; the driver
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

## Renode CAN loopback simulation

From the repository root, build and launch the F446 loopback test:

```bash
cmake --preset f446-hardware-test -DFIRMWARE_TEST=can_test
cmake --build --preset f446-hardware-test
renode --console --disable-gui simulation/renode/can_loopback.resc -e start
```

In another terminal, run `tail -F /tmp/nucleo-can.log`. Expected output is
`CAN1 classic loopback test` followed by repeated `PASS: 0x123 AB CD` lines.
Use `pause`, `start`, and `quit` in the Renode Monitor to control execution.
The script's reset macro reloads the ELF and reapplies the workaround on
`machine Reset`. Quit an existing simulation before launching a fresh instance.

The Renode 1.17 STMCAN model does not acknowledge initialization when both
INRQ and SLEEP are set. The checked-in HAL requests initialization before
clearing sleep, so the unadjusted model causes `HAL_CAN_ERROR_TIMEOUT` and
enters `Error_Handler()` before UART logging starts. The script wakes the
simulated CAN controller before firmware runs (MCR = 0x00010000). It does
not change the firmware or fabricate received frames. UART logging uses
immediate flushing so output is visible while the simulation is running.

This smoke test exercises the firmware's polling CAN loopback against the
Renode model; it does not validate physical bus signaling, DMA, or all F446
peripherals. Generic F4 model warnings about SYSCFG, RCC, and DMA can still
appear. The separate `can_network.resc` is a Linux SocketCAN experiment,
not this native loopback test.
