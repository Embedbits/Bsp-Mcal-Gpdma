# Gpdma Module (MCAL)

## Overview

The `Gpdma` (General-Purpose DMA) module in MCAL provides an abstraction for configuring and managing DMA transfers on STM32 microcontrollers. It exposes public types and configuration structures for flexible DMA usage in embedded applications.

## STM32H5 Specifics

### Channels

Every STM32H5 device has **GPDMA1** and **GPDMA2** (`GPDMA_PERIPH_1`, `GPDMA_PERIPH_2`) with the same channel implementation. The count of channels and the channels with 2D addressing depend on the device line, `gpdma_ChannelId_t` holds exactly the channels of the line:

| Device lines | Channels | FIFO | Addressing |
|---|---|---|---|
| STM32H5E4 / H5E5 / H5F4 / H5F5 (RM0517 Rev 1, table 126 "GPDMA1/2 channel implementation") | `GPDMA_CHANNEL_0` - `GPDMA_CHANNEL_7` | 8 bytes (2 words) | linear |
| | `GPDMA_CHANNEL_8` - `GPDMA_CHANNEL_9` | 32 bytes (8 words) | linear |
| | `GPDMA_CHANNEL_10` - `GPDMA_CHANNEL_11` | 32 bytes (8 words) | linear or 2D |
| other STM32H5 lines (RM0481 Rev 5, table 140; RM0492 Rev 3, table 85) | `GPDMA_CHANNEL_0` - `GPDMA_CHANNEL_3` | 8 bytes (2 words) | linear |
| | `GPDMA_CHANNEL_4` - `GPDMA_CHANNEL_5` | 32 bytes (8 words) | linear |
| | `GPDMA_CHANNEL_6` - `GPDMA_CHANNEL_7` | 32 bytes (8 words) | linear or 2D |

The 2D settings of a channel (`Gpdma_Set_BlockRepeatCount()` with a count other than 0, trigger mode `GPDMA_TRIGGER_2D_BLOCK`) are refused on the linear channels; `Gpdma_Init()` builds the transfer list of a linear channel without the 2D registers (block repetition and 2D offsets are not written). The interrupt of every channel is a separate vector (`NVIC_PERIPH_IRQ_GPDMAx_CHANNEL0` - `NVIC_PERIPH_IRQ_GPDMAx_CHANNEL7`, on STM32H5E4 / H5E5 / H5F4 / H5F5 also `NVIC_PERIPH_IRQ_GPDMAx_CHANNEL8` - `NVIC_PERIPH_IRQ_GPDMAx_CHANNEL11`). The channels 8 - 11 are guarded by the CMSIS macro `GPDMA1_Channel8`.

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