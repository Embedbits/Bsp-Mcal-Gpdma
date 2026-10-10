# Gpdma Module (MCAL)

## Overview

The `Gpdma` (General-Purpose DMA) module in MCAL provides an abstraction for configuring and managing DMA transfers on STM32 microcontrollers. It exposes public types and configuration structures for flexible DMA usage in embedded applications.

## 📘 STM32H7 specifics

Public interface of the STM32H5 module (`Dev/STM32H5`).

- **Devices** - only STM32H7R3 / H7R7 / H7S3 / H7S7 (Ral family STM32H7RS) have GPDMA. The classic STM32H7 lines
  (DMA1 / DMA2 streams + DMAMUX1, module `Dma`) have no GPDMA - `Gpdma_Lib` is empty there, unit tests are registered
  only for `MCU_FAMILY_ID` `STM32H7RSxx`.
- **Controllers** - `GPDMA_PERIPH_1` = GPDMA1, `GPDMA_PERIPH_2` = HPDMA1 (high-performance DMA), 16 channels each
  (`GPDMA_CHANNEL_0` - `GPDMA_CHANNEL_15`), channels 12 - 15 with 2D addressing, own NVIC line per channel.
- **Requests / triggers** - values are the request / trigger numbers of the controller: `GPDMA_REQ_x` / `GPDMA_TRG_x`
  for GPDMA1, `GPDMA_REQ_HPDMA1_x` / `GPDMA_TRG_HPDMA1_x` for HPDMA1 (use with `GPDMA_PERIPH_2` only). CRYP / SAES
  requests and LTDC / GPU2D triggers exist only on devices with the peripheral.
- **Default ports** (`GPDMA_PORT_DEFAULT`, ST STM32CubeH7RS examples) - GPDMA1: memories and peripherals on port 0,
  transfer list nodes loaded through port 1; HPDMA1: memories on AXI port 0, peripherals on AHB port 1, nodes loaded
  through port 0. `Gpdma_Init()` applies them to the channel and to the generated nodes; the transfer list port
  setters (`Gpdma_Set_XferList_SrcPort()` / `DestPort()`) resolve the default to port 0 (HPDMA1 nodes built without
  `Gpdma_Init()` need explicit port 1 on the peripheral side).
- **RAM location** - `Gpdma_Check_RamLocation()` checks the AXI SRAM (`RAM`) and the AHB SRAM (`RAM_D2`) regions of the
  STM32H7 linker script.
- **Cache** - Cortex-M7 D-cache: DMA buffers and transfer list nodes in cacheable memory need cache maintenance or a
  non-cacheable MPU region (ST examples use a `noncacheable_buffer` section).

Not yet tested on hardware (integration tests prepared with the IT phase on NUCLEO-H7S3L8).

## File Structure

- `Gpdma_Port.h` — Declares the public API functions for the Gpdma module.
- `Gpdma_Types.h` — Defines public types, enums, and structures used by the Gpdma API.
- `Gpdma.c` — Implements the Gpdma module functionality.

## Public API

### Public Functions

All public functions are declared in `Gpdma_Port.h`. These functions provide access to DMA configuration, transfer control, and status queries. Example functions may include:

- `Gpdma_Init(const gpdma_ConfigStruct_t* config)`
- `Gpdma_StartTransfer(gpdma_PeriphId_t periph, gpdma_ChannelId_t channel, const gpdma_TransferConfig_t* xferConfig)`
- `Gpdma_StopTransfer(gpdma_PeriphId_t periph, gpdma_ChannelId_t channel)`
- `Gpdma_GetTransferState(gpdma_PeriphId_t periph, gpdma_ChannelId_t channel)`

Refer to `Gpdma_Port.h` for the complete list and documentation of available functions.

### Public Types

All public types used by the Gpdma API are defined in `Gpdma_Types.h`. These include:

- Enumerations for DMA peripherals, channels, directions, priorities, events, and errors (e.g., `gpdma_PeriphId_t`, `gpdma_ChannelId_t`, `gpdma_Direction_t`)
- Structures for configuration (e.g., `gpdma_TransferConfig_t`, `gpdma_ConfigStruct_t`)
- Callback function pointer types for interrupts

Refer to `Gpdma_Types.h` for detailed type definitions and descriptions.

## Usage Example
   ```c
   #include "Gpdma_Port.h"
   gpdmaTransferConfigt xferConfig = { // Initialize transfer configuration fields };

   gpdmaConfigStructt dmaConfig = { .PeriphId = GPDMAPERIPH1, .ChannelId = GPDMACHANNEL0, .TransferConfig = &xferConfig, .TransfersCount = 1, // Other fields as needed };

   int main(void) { GpdmaInit(&dmaConfig); GpdmaStartTransfer(GPDMAPERIPH1, GPDMACHANNEL0, &xferConfig); // … }
   ```

## 🛠 CMake Integration

1. Include `Gpdma_Port.h` in your project.
2. Link against the Gpdma module implementation files.
3. Configure the module as needed for your hardware.

---

## License

This project is licensed under the **Creative Commons Attribution–NonCommercial 4.0 International (CC BY-NC 4.0)**.

You are free to use, modify, and share this work for **non-commercial purposes**, provided appropriate credit is given.

See [LICENSE.md](LICENSE.md) for full terms or visit [creativecommons.org/licenses/by-nc/4.0](https://creativecommons.org/licenses/by-nc/4.0/).

---

## Authors

- **Mr.Nobody** — [embedbits.com](https://embedbits.com)

Contributions are welcome! Please open a pull request.

---

## 🌐 Useful Links

- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
- [Azure DevOps](https://azure.microsoft.com/en-us/services/devops/)
- [Embedbits Github](https://github.com/Embedbits)
- [CC BY-NC 4.0 License](https://creativecommons.org/licenses/by-nc/4.0/)