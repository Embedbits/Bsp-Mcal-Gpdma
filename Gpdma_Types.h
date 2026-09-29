/**
 * \author Mr.Nobody
 * \file Gpdma_Types.h
 * \ingroup Gpdma
 * \brief Gpdma module global types definition
 *
 * This file contains the types definitions used across the module and are 
 * available for other modules through Port file.
 *
 */

#ifndef GPDMA_GPDMA_TYPES_H
#define GPDMA_GPDMA_TYPES_H
/* ============================== INCLUDES ================================== */
#include "stdint.h"                         /* Module types definition        */
#include "Stm32_dma.h"                      /* DMA RAL functionality include  */
/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Null pointer definition */
#define GPDMA_NULL_PTR                      ( ( void* ) 0u )

/** Maximum count of transfers for single transfer */
#define GPDMA_MAX_BLOCK_LEN                 ( 0xFFFFu )

/** Maximum repetitions count of transfers for 2D transfers */
#define GPDMA_MAX_REP_BLOCK_LEN             ( 0x7FFu )

/** Maximum count of bursts */
#define GPDMA_MAX_BURST_LEN                 ( 64u )

/** Minimum count of bursts */
#define GPDMA_MIN_BURST_LEN                 ( 1u )

/** Maximum value of source address offset */
#define GPDMA_SOURCE_ADDR_OFFSET_MAX        ( 0xFFF )

/** Maximum value of transfer offset address */
#define GPDMA_TRANSFER_OFFSET_ADDR_MAX      ( 0xFFF )

/** Maximum value of repetition transfer offset address (only for 2D transfers) */
#define GPDMA_REP_TRANSFER_OFFSET_ADDR_MAX  ( 0xFFFF )

/* ========================== EXPORTED MACROS =============================== */

/* ============================== TYPEDEFS ================================== */

/** \brief Type signaling major version of SW module */
typedef uint8_t gpdma_MajorVersion_t;


/** \brief Type signaling minor version of SW module */
typedef uint8_t gpdma_MinorVersion_t;


/** \brief Type signaling patch version of SW module */
typedef uint8_t gpdma_PatchVersion_t;


/** \brief Type signaling actual version of SW module */
typedef struct
{
    gpdma_MajorVersion_t Major; /**< Major version */
    gpdma_MinorVersion_t Minor; /**< Minor version */
    gpdma_PatchVersion_t Patch; /**< Patch version */
}   gpdma_ModuleVersion_t;


/** Function status enumeration */
typedef enum
{
    GPDMA_FUNCTION_INACTIVE = 0u, /**< Function status is inactive */
    GPDMA_FUNCTION_ACTIVE         /**< Function status is active   */
}   gpdma_FunctionState_t;


/** Enumeration used to signal request processing state */
typedef enum
{
    GPDMA_REQUEST_ERROR = 0u, /**< Processing request failed  */
    GPDMA_REQUEST_OK          /**< Processing request succeed */
}   gpdma_RequestState_t;


/** Flag states enumeration */
typedef enum
{
    GPDMA_FLAG_INACTIVE = 0u, /**< Inactive flag state */
    GPDMA_FLAG_ACTIVE         /**< Active flag state   */
}   gpdma_FlagState_t;


/** Source address type representation */
typedef uint32_t gpdma_SrcAddr_t;


/** Destination address type representation */
typedef uint32_t gpdma_DstAddr_t;


/** Data address type definition */
typedef uint32_t gpdma_DataAddr_t;


/** Linked-list link register (CxLLR) value type definition */
typedef uint32_t gpdma_XferLinkReg_t;


/** \brief Data count type definition */
typedef uint16_t gpdma_ByteCnt_t;


/** \brief Burst transfer length data representation (count of data transfers
 *         during one GPDMA request).
 * - Range of values: 1 to 64
 *
 * The source and destination data transfer can wary. This type represents count
 * of this data's (size specified by \ref gpdma_DataSize_t) transfered from/into
 * GPDMA channel's FIFO. This can be useful for different speed of source and
 * destination.
 *
 * Example 1. Source is Faster than Destination
 * Scenario: You need to read a large block of data from a fast internal SRAM
 * (which supports burst reads efficiently) and send it to a communication
 * peripheral (like an I2C or UART) that can only process data one byte or one
 * word at a time.
 * Source Burst Length: Programmed for a larger burst (e.g., 4 words / 16 bytes,
 * or even up to half of the FIFO size if the memory allows it).
 * Destination Burst Length: Programmed for a single beat (burst length of 1).
 * FIFO Role: The GPDMA channel's internal FIFO acts as a buffer and rate-matcher.
 * It accumulates the incoming data from the fast source burst and then releases
 * it to the slow destination one beat at a time.
 *
 * Example 2. Source is Slower than Destination
 * Scenario: You need to collect data from a slow peripheral (e.g., an ADC
 * converting analog signals one by one, or an I2C receiving data byte by byte)
 * and efficiently write it to a fast internal SRAM or Flash memory.
 * Source Burst Length: Programmed for a single beat (burst length of 1).
 * Destination Burst Length: Programmed for a larger burst (e.g., 4 words /
 * 16 bytes for SRAM).
 * FIFO Role: The GPDMA channel's internal FIFO acts as a buffer and rate-matcher.
 * It collects the individual beats from the slow source. Once enough data has
 * accumulated in the FIFO to form the desired destination burst, the GPDMA reads
 * from the FIFO and writes the data to the fast destination in a burst.
 *
 */
typedef uint16_t gpdma_BurstLength_t;


/** \brief Type representing count of transfers in single transfer.
 *
 * Range of values: 1 to 65535
 *
 * This type specifies count of transfers (specified by \ref gpdma_DataSize_t)
 * from source to destination. Every transfer is started by request (specified
 * by \ref gpdma_PeriphReqId_t or by software request).
 */
typedef uint16_t gpdma_BlockSize_t;


/** \brief Type representing count of repetitions in single transfer (only for 2D transfers).
 *
 * This type specifies count of repetitions of single transfer (specified by
 * \ref gpdma_BlockSize_t) from source to destination. It is used only for
 * 2D transfers. This value can represent number of lines (rows) in 2D transfer.
 */
typedef uint16_t gpdma_BlockRep_t;


/** Enumeration of errors in GPDMA module */
typedef enum
{
    GPDMA_ERROR_TRANSFER      = 0x01, /**< Error during transfer             */
    GPDMA_ERROR_CONFIG_UPDATE = 0x02, /**< Error during configuration update */
    GPDMA_ERROR_CONFIG_ERROR  = 0x04, /**< Error in transfer configuration   */
    GPDMA_ERROR_TRIG_OVERRUN  = 0x08, /**< Trigger overrun error             */
    GPDMA_ERROR_SUSPENDED     = 0x10, /**< Transfer suspended error          */
    GPDMA_ERROR_ALL           = 0x1F  /**< All errors mask                   */
}   gpdma_ErrorMaskId_t;


/** DMA peripherals enumeration list */
typedef enum
{
    GPDMA_PERIPH_1 = 0, /**< DMA peripheral 1 identification */
#if defined(GPDMA2)
    GPDMA_PERIPH_2,     /**< DMA peripheral 2 identification */
#endif
    GPDMA_PERIPH_CNT
}   gpdma_PeriphId_t;


/** Enumeration of available channels for all DMA peripherals */
typedef enum
{
    GPDMA_CHANNEL_0 = 0u, /**< DMA transfer channel 0 (FIFO size 8 bytes) linear addressing mode        */
    GPDMA_CHANNEL_1,      /**< DMA transfer channel 1 (FIFO size 8 bytes) linear addressing mode        */
    GPDMA_CHANNEL_2,      /**< DMA transfer channel 2 (FIFO size 8 bytes) linear addressing mode        */
    GPDMA_CHANNEL_3,      /**< DMA transfer channel 3 (FIFO size 8 bytes) linear addressing mode        */
    GPDMA_CHANNEL_4,      /**< DMA transfer channel 4 (FIFO size 32 bytes) linear addressing mode       */
    GPDMA_CHANNEL_5,      /**< DMA transfer channel 5 (FIFO size 32 bytes) linear addressing mode       */
    GPDMA_CHANNEL_6,      /**< DMA transfer channel 6 (FIFO size 32 bytes) linear or 2D addressing mode */
    GPDMA_CHANNEL_7,      /**< DMA transfer channel 7 (FIFO size 32 bytes) linear or 2D addressing mode */
    GPDMA_CHANNEL_CNT     /**< Count of available channels for all DMA peripherals                      */
}   gpdma_ChannelId_t;


typedef enum
{
    GPDMA_CHANNEL_LINEAR = 0u, /**< Channel support only linear transfers.   */
    GPDMA_CHANNEL_LINEAR_2D,   /**< Channel support linear and 2D transfers. */
    GPDMA_CHANNEL_OPTION_CNT   /**< Count of channel transfer options.       */
}   gpdma_ChannelType_t;


/** \brief List of available hardware events/interrupts */
typedef enum
{
    GPDMA_EVENT_TC = 0u,             /**< Transfer Complete (TC) event ID       */
    GPDMA_EVENT_HTC,                 /**< Half Transfer Complete (HTC) event ID */
    GPDMA_EVENT_TRANSFER_ERROR,      /**< Transfer Error event ID               */
    GPDMA_EVENT_SETTING_ERROR,       /**< User settings error event ID          */
    GPDMA_EVENT_LINK_TRANSFER_ERROR, /**< Link Transfer Error event ID          */
    GPDMA_EVENT_SUSPENSION_COMPLETE, /**< Suspension complete event ID          */
    GPDMA_EVENT_TRIG_OVERRUN,        /**< Trigger overrun event ID              */
}   gpdma_EventId_t;


/**
 * \brief List of available configurations for Transfer Complete (TC) and Half
 *        Transfer (HT) reactions.
 */
typedef enum
{
    GPDMA_TRANSFER_EVENT_BLOCK = 0u,    /**< Transfer Complete (TC) and Half Transfer (HT) executed within each block (defined by block size)                               */
    GPDMA_TRANSFER_EVENT_2D_BLOCK,      /**< Transfer Complete (TC) and Half Transfer (HT) executed within each 2D block (defined by block size and block repetition count) */
    GPDMA_TRANSFER_EVENT_TRANSFER,      /**< Transfer Complete (TC) and Half Transfer (HT) executed within each configured transfer (same as block or 2D block)             */
    GPDMA_TRANSFER_EVENT_LAST_TRANSFER, /**< Transfer Complete (TC) and Half Transfer (HT) executed only at last configured transfer                                        */
    GPDMA_TRANSFER_EVENT_CNT            /**< Count of transfer events.                                                                                                      */
}   gpdma_TransferEvent_t;


/** DMA data transfer direction enumeration */
typedef enum
{
    GPDMA_DIR_PERIPH_TO_MEMORY = 0u, /**< Peripheral to memory direction */
    GPDMA_DIR_MEMORY_TO_PERIPH,      /**< Memory to peripheral direction */
    GPDMA_DIR_MEMORY_TO_MEMORY,      /**< Memory to memory direction     */
    GPDMA_DIR_CNT                    /**< Count of available options     */
}   gpdma_Direction_t;


/** Peripheral address increment options enumeration */
typedef enum
{
    GPDMA_ADDR_STATIC = 0u, /**< Address will not be incremented           */
    GPDMA_ADDR_INCREMENT  , /**< Address will be incremented               */
    GPDMA_ADDR_DECREMENT  , /**< Address will be decremented (only for 2D) */
    GPDMA_ADDR_CNT          /**< Count of available options                */
}   gpdma_AddrMode_t;


/** Data size enumeration for GPDMA data transfer used for source and destination sides.
 */
typedef enum
{
    GPDMA_DATA_SIZE_8BITS = 0u, /**< Destination data transfer size : 8bits  */
    GPDMA_DATA_SIZE_16BITS,     /**< Destination data transfer size : 16bits */
    GPDMA_DATA_SIZE_32BITS,     /**< Destination data transfer size : 32bits */
    GPDMA_DATA_SIZE_CNT         /**< Count of available options              */
}   gpdma_DataSize_t;


/** Channel priority options enumeration */
typedef enum
{
    GPDMA_PRIORITY_LOW = 0u, /**< Priority level : Low       */
    GPDMA_PRIORITY_MEDIUM  , /**< Priority level : Medium    */
    GPDMA_PRIORITY_HIGH    , /**< Priority level : High      */
    GPDMA_PRIORITY_VERYHIGH, /**< Priority level : Very_High */
    GPDMA_PRIORITY_CNT       /**< Count of available options */
}   gpdma_Priority_t;


typedef enum
{
    GPDMA_PORT_0 = 0u,  /**< Port 0 shall be used for peripheral connection */
    GPDMA_PORT_1,       /**< Port 1 shall be used for memory connection     */
    GPDMA_PORT_DEFAULT, /**< If default configuration selected, port 0 will
                             be used for peripheral connection, and port 1
                             will be used for memory connection. In memory
                             to memory mode, port 0 will be used for source,
                             and port 1 for destination. */
    GPDMA_PORT_CNT      /**< Count of available options                     */
}   gpdma_PortId_t;


typedef enum gpdma_PeriphReqId_t
{
    GPDMA_REQ_ADC1            = LL_GPDMA1_REQUEST_ADC1         , /**< GPDMA1 HW request is ADC1          */
#if defined (ADC2)
    GPDMA_REQ_ADC2            = LL_GPDMA1_REQUEST_ADC2         , /**< GPDMA1 HW request is ADC2          */
#endif /* ADC2 */
    GPDMA_REQ_DAC1_CH1        = LL_GPDMA1_REQUEST_DAC1_CH1     , /**< GPDMA1 HW request is DAC1_CH1      */
    GPDMA_REQ_DAC1_CH2        = LL_GPDMA1_REQUEST_DAC1_CH2     , /**< GPDMA1 HW request is DAC1_CH2      */
    GPDMA_REQ_TIM6_UP         = LL_GPDMA1_REQUEST_TIM6_UP      , /**< GPDMA1 HW request is TIM6_UP       */
    GPDMA_REQ_TIM7_UP         = LL_GPDMA1_REQUEST_TIM7_UP      , /**< GPDMA1 HW request is TIM7_UP       */
    GPDMA_REQ_SPI1_RX         = LL_GPDMA1_REQUEST_SPI1_RX      , /**< GPDMA1 HW request is SPI1_RX       */
    GPDMA_REQ_SPI1_TX         = LL_GPDMA1_REQUEST_SPI1_TX      , /**< GPDMA1 HW request is SPI1_TX       */
    GPDMA_REQ_SPI2_RX         = LL_GPDMA1_REQUEST_SPI2_RX      , /**< GPDMA1 HW request is SPI2_RX       */
    GPDMA_REQ_SPI2_TX         = LL_GPDMA1_REQUEST_SPI2_TX      , /**< GPDMA1 HW request is SPI2_TX       */
    GPDMA_REQ_SPI3_RX         = LL_GPDMA1_REQUEST_SPI3_RX      , /**< GPDMA1 HW request is SPI3_RX       */
    GPDMA_REQ_SPI3_TX         = LL_GPDMA1_REQUEST_SPI3_TX      , /**< GPDMA1 HW request is SPI3_TX       */
    GPDMA_REQ_I2C1_RX         = LL_GPDMA1_REQUEST_I2C1_RX      , /**< GPDMA1 HW request is I2C1_RX       */
    GPDMA_REQ_I2C1_TX         = LL_GPDMA1_REQUEST_I2C1_TX      , /**< GPDMA1 HW request is I2C1_TX       */
    GPDMA_REQ_I2C2_RX         = LL_GPDMA1_REQUEST_I2C2_RX      , /**< GPDMA1 HW request is I2C2_RX       */
    GPDMA_REQ_I2C2_TX         = LL_GPDMA1_REQUEST_I2C2_TX      , /**< GPDMA1 HW request is I2C2_TX       */
#if defined (I2C3)
    GPDMA_REQ_I2C3_RX         = LL_GPDMA1_REQUEST_I2C3_RX      , /**< GPDMA1 HW request is I2C3_RX       */
    GPDMA_REQ_I2C3_TX         = LL_GPDMA1_REQUEST_I2C3_TX      , /**< GPDMA1 HW request is I2C3_TX       */
#endif /* I2C3 */
    GPDMA_REQ_USART1_RX       = LL_GPDMA1_REQUEST_USART1_RX    , /**< GPDMA1 HW request is USART1_RX     */
    GPDMA_REQ_USART1_TX       = LL_GPDMA1_REQUEST_USART1_TX    , /**< GPDMA1 HW request is USART1_TX     */
    GPDMA_REQ_USART2_RX       = LL_GPDMA1_REQUEST_USART2_RX    , /**< GPDMA1 HW request is USART2_RX     */
    GPDMA_REQ_USART2_TX       = LL_GPDMA1_REQUEST_USART2_TX    , /**< GPDMA1 HW request is USART2_TX     */
    GPDMA_REQ_USART3_RX       = LL_GPDMA1_REQUEST_USART3_RX    , /**< GPDMA1 HW request is USART3_RX     */
    GPDMA_REQ_USART3_TX       = LL_GPDMA1_REQUEST_USART3_TX    , /**< GPDMA1 HW request is USART3_TX     */
#if defined (UART4)
    GPDMA_REQ_UART4_RX        = LL_GPDMA1_REQUEST_UART4_RX     , /**< GPDMA1 HW request is UART4_RX      */
    GPDMA_REQ_UART4_TX        = LL_GPDMA1_REQUEST_UART4_TX     , /**< GPDMA1 HW request is UART4_TX      */
#endif /* UART4 */
#if defined (UART4)
    GPDMA_REQ_UART5_RX        = LL_GPDMA1_REQUEST_UART5_RX     , /**< GPDMA1 HW request is UART5_RX      */
    GPDMA_REQ_UART5_TX        = LL_GPDMA1_REQUEST_UART5_TX     , /**< GPDMA1 HW request is UART5_TX      */
#endif /* UART5 */
#if defined (UART4)
    GPDMA_REQ_USART6_RX       = LL_GPDMA1_REQUEST_USART6_RX    , /**< GPDMA1 HW request is USART6_RX     */
    GPDMA_REQ_USART6_TX       = LL_GPDMA1_REQUEST_USART6_TX    , /**< GPDMA1 HW request is USART6_TX     */
#endif /* USART6 */
#if defined (UART7)
    GPDMA_REQ_UART7_RX        = LL_GPDMA1_REQUEST_UART7_RX     , /**< GPDMA1 HW request is UART7_RX      */
    GPDMA_REQ_UART7_TX        = LL_GPDMA1_REQUEST_UART7_TX     , /**< GPDMA1 HW request is UART7_TX      */
#endif /* UART7 */
#if defined (UART8)
    GPDMA_REQ_UART8_RX        = LL_GPDMA1_REQUEST_UART8_RX     , /**< GPDMA1 HW request is UART8_RX      */
    GPDMA_REQ_UART8_TX        = LL_GPDMA1_REQUEST_UART8_TX     , /**< GPDMA1 HW request is UART8_TX      */
#endif /* UART8 */
#if defined (UART9)
    GPDMA_REQ_UART9_RX        = LL_GPDMA1_REQUEST_UART9_RX     , /**< GPDMA1 HW request is UART9_RX      */
    GPDMA_REQ_UART9_TX        = LL_GPDMA1_REQUEST_UART9_TX     , /**< GPDMA1 HW request is UART9_TX      */
#endif /* UART9 */
#if defined (USART10)
    GPDMA_REQ_USART10_RX      = LL_GPDMA1_REQUEST_USART10_RX   , /**< GPDMA1 HW request is USART10_RX    */
    GPDMA_REQ_USART10_TX      = LL_GPDMA1_REQUEST_USART10_TX   , /**< GPDMA1 HW request is USART10_TX    */
#endif /* USART10 */
#if defined (USART11)
    GPDMA_REQ_USART11_RX      = LL_GPDMA1_REQUEST_USART11_RX   , /**< GPDMA1 HW request is USART11_RX    */
    GPDMA_REQ_USART11_TX      = LL_GPDMA1_REQUEST_USART11_TX   , /**< GPDMA1 HW request is USART11_TX    */
#endif /* USART11 */
#if defined (UART12)
    GPDMA_REQ_UART12_RX       = LL_GPDMA1_REQUEST_UART12_RX    , /**< GPDMA1 HW request is UART12_RX     */
    GPDMA_REQ_UART12_TX       = LL_GPDMA1_REQUEST_UART12_TX    , /**< GPDMA1 HW request is UART12_TX     */
#endif /* UART12 */
    GPDMA_REQ_LPUART1_RX      = LL_GPDMA1_REQUEST_LPUART1_RX   , /**< GPDMA1 HW request is LPUART1_RX    */
    GPDMA_REQ_LPUART1_TX      = LL_GPDMA1_REQUEST_LPUART1_TX   , /**< GPDMA1 HW request is LPUART1_TX    */
#if defined (SPI4)
    GPDMA_REQ_SPI4_RX         = LL_GPDMA1_REQUEST_SPI4_RX      , /**< GPDMA1 HW request is SPI4_RX       */
    GPDMA_REQ_SPI4_TX         = LL_GPDMA1_REQUEST_SPI4_TX      , /**< GPDMA1 HW request is SPI4_TX       */
#endif /* SPI4 */
#if defined (SPI5)
    GPDMA_REQ_SPI5_RX         = LL_GPDMA1_REQUEST_SPI5_RX      , /**< GPDMA1 HW request is SPI5_RX       */
    GPDMA_REQ_SPI5_TX         = LL_GPDMA1_REQUEST_SPI5_TX      , /**< GPDMA1 HW request is SPI5_TX       */
#endif /* SPI5 */
#if defined (SPI6)
    GPDMA_REQ_SPI6_RX         = LL_GPDMA1_REQUEST_SPI6_RX      , /**< GPDMA1 HW request is SPI6_RX       */
    GPDMA_REQ_SPI6_TX         = LL_GPDMA1_REQUEST_SPI6_TX      , /**< GPDMA1 HW request is SPI6_TX       */
#endif /* SPI6 */
#if defined (SAI1)
    GPDMA_REQ_SAI1_A          = LL_GPDMA1_REQUEST_SAI1_A       , /**< GPDMA1 HW request is SAI1_A        */
    GPDMA_REQ_SAI1_B          = LL_GPDMA1_REQUEST_SAI1_B       , /**< GPDMA1 HW request is SAI1_B        */
#endif /* SAI1 */
#if defined (SAI2)
    GPDMA_REQ_SAI2_A          = LL_GPDMA1_REQUEST_SAI2_A       , /**< GPDMA1 HW request is SAI2_A        */
    GPDMA_REQ_SAI2_B          = LL_GPDMA1_REQUEST_SAI2_B       , /**< GPDMA1 HW request is SAI2_B        */
#endif /* SAI2 */
#if defined (OCTOSPI1)
    GPDMA_REQ_OCTOSPI1        = LL_GPDMA1_REQUEST_OCTOSPI1     , /**< GPDMA1 HW request is OCTOSPI1      */
#endif /* OCTOSPI1 */
    GPDMA_REQ_TIM1_CH1        = LL_GPDMA1_REQUEST_TIM1_CH1     , /**< GPDMA1 HW request is TIM1_CH1      */
    GPDMA_REQ_TIM1_CH2        = LL_GPDMA1_REQUEST_TIM1_CH2     , /**< GPDMA1 HW request is TIM1_CH2      */
    GPDMA_REQ_TIM1_CH3        = LL_GPDMA1_REQUEST_TIM1_CH3     , /**< GPDMA1 HW request is TIM1_CH3      */
    GPDMA_REQ_TIM1_CH4        = LL_GPDMA1_REQUEST_TIM1_CH4     , /**< GPDMA1 HW request is TIM1_CH4      */
    GPDMA_REQ_TIM1_UP         = LL_GPDMA1_REQUEST_TIM1_UP      , /**< GPDMA1 HW request is TIM1_UP       */
    GPDMA_REQ_TIM1_TRIG       = LL_GPDMA1_REQUEST_TIM1_TRIG    , /**< GPDMA1 HW request is TIM1_TRIG     */
    GPDMA_REQ_TIM1_COM        = LL_GPDMA1_REQUEST_TIM1_COM     , /**< GPDMA1 HW request is TIM1_COM      */
#if defined (TIM8)
    GPDMA_REQ_TIM8_CH1        = LL_GPDMA1_REQUEST_TIM8_CH1     , /**< GPDMA1 HW request is TIM8_CH1      */
    GPDMA_REQ_TIM8_CH2        = LL_GPDMA1_REQUEST_TIM8_CH2     , /**< GPDMA1 HW request is TIM8_CH2      */
    GPDMA_REQ_TIM8_CH3        = LL_GPDMA1_REQUEST_TIM8_CH3     , /**< GPDMA1 HW request is TIM8_CH3      */
    GPDMA_REQ_TIM8_CH4        = LL_GPDMA1_REQUEST_TIM8_CH4     , /**< GPDMA1 HW request is TIM8_CH4      */
    GPDMA_REQ_TIM8_UP         = LL_GPDMA1_REQUEST_TIM8_UP      , /**< GPDMA1 HW request is TIM8_UP       */
    GPDMA_REQ_TIM8_TRIG       = LL_GPDMA1_REQUEST_TIM8_TRIG    , /**< GPDMA1 HW request is TIM8_TRIG     */
    GPDMA_REQ_TIM8_COM        = LL_GPDMA1_REQUEST_TIM8_COM     , /**< GPDMA1 HW request is TIM8_COM      */
#endif /* TIM8 */
    GPDMA_REQ_TIM2_CH1        = LL_GPDMA1_REQUEST_TIM2_CH1     , /**< GPDMA1 HW request is TIM2_CH1      */
    GPDMA_REQ_TIM2_CH2        = LL_GPDMA1_REQUEST_TIM2_CH2     , /**< GPDMA1 HW request is TIM2_CH2      */
    GPDMA_REQ_TIM2_CH3        = LL_GPDMA1_REQUEST_TIM2_CH3     , /**< GPDMA1 HW request is TIM2_CH3      */
    GPDMA_REQ_TIM2_CH4        = LL_GPDMA1_REQUEST_TIM2_CH4     , /**< GPDMA1 HW request is TIM2_CH4      */
    GPDMA_REQ_TIM2_UP         = LL_GPDMA1_REQUEST_TIM2_UP      , /**< GPDMA1 HW request is TIM2_UP       */
    GPDMA_REQ_TIM3_CH1        = LL_GPDMA1_REQUEST_TIM3_CH1     , /**< GPDMA1 HW request is TIM3_CH1      */
    GPDMA_REQ_TIM3_CH2        = LL_GPDMA1_REQUEST_TIM3_CH2     , /**< GPDMA1 HW request is TIM3_CH2      */
    GPDMA_REQ_TIM3_CH3        = LL_GPDMA1_REQUEST_TIM3_CH3     , /**< GPDMA1 HW request is TIM3_CH3      */
    GPDMA_REQ_TIM3_CH4        = LL_GPDMA1_REQUEST_TIM3_CH4     , /**< GPDMA1 HW request is TIM3_CH4      */
    GPDMA_REQ_TIM3_UP         = LL_GPDMA1_REQUEST_TIM3_UP      , /**< GPDMA1 HW request is TIM3_UP       */
    GPDMA_REQ_TIM3_TRIG       = LL_GPDMA1_REQUEST_TIM3_TRIG    , /**< GPDMA1 HW request is TIM3_TRIG     */
#if defined (TIM4)
    GPDMA_REQ_TIM4_CH1        = LL_GPDMA1_REQUEST_TIM4_CH1     , /**< GPDMA1 HW request is TIM4_CH1      */
    GPDMA_REQ_TIM4_CH2        = LL_GPDMA1_REQUEST_TIM4_CH2     , /**< GPDMA1 HW request is TIM4_CH2      */
    GPDMA_REQ_TIM4_CH3        = LL_GPDMA1_REQUEST_TIM4_CH3     , /**< GPDMA1 HW request is TIM4_CH3      */
    GPDMA_REQ_TIM4_CH4        = LL_GPDMA1_REQUEST_TIM4_CH4     , /**< GPDMA1 HW request is TIM4_CH4      */
    GPDMA_REQ_TIM4_UP         = LL_GPDMA1_REQUEST_TIM4_UP      , /**< GPDMA1 HW request is TIM4_UP       */
#endif /* TIM4 */
#if defined (TIM5)
    GPDMA_REQ_TIM5_CH1        = LL_GPDMA1_REQUEST_TIM5_CH1     , /**< GPDMA1 HW request is TIM5_CH1      */
    GPDMA_REQ_TIM5_CH2        = LL_GPDMA1_REQUEST_TIM5_CH2     , /**< GPDMA1 HW request is TIM5_CH2      */
    GPDMA_REQ_TIM5_CH3        = LL_GPDMA1_REQUEST_TIM5_CH3     , /**< GPDMA1 HW request is TIM5_CH3      */
    GPDMA_REQ_TIM5_CH4        = LL_GPDMA1_REQUEST_TIM5_CH4     , /**< GPDMA1 HW request is TIM5_CH4      */
    GPDMA_REQ_TIM5_UP         = LL_GPDMA1_REQUEST_TIM5_UP      , /**< GPDMA1 HW request is TIM5_UP       */
    GPDMA_REQ_TIM5_TRIG       = LL_GPDMA1_REQUEST_TIM5_TRIG    , /**< GPDMA1 HW request is TIM5_TRIG     */
#endif /* TIM5 */
#if defined (TIM15)
    GPDMA_REQ_TIM15_CH1       = LL_GPDMA1_REQUEST_TIM15_CH1    , /**< GPDMA1 HW request is TIM15_CH1     */
    GPDMA_REQ_TIM15_UP        = LL_GPDMA1_REQUEST_TIM15_UP     , /**< GPDMA1 HW request is TIM15_UP      */
    GPDMA_REQ_TIM15_TRIG      = LL_GPDMA1_REQUEST_TIM15_TRIG   , /**< GPDMA1 HW request is TIM15_TRIG    */
    GPDMA_REQ_TIM15_COM       = LL_GPDMA1_REQUEST_TIM15_COM    , /**< GPDMA1 HW request is TIM15_COM     */
#endif /* TIM15 */
#if defined (TIM16)
    GPDMA_REQ_TIM16_CH1       = LL_GPDMA1_REQUEST_TIM16_CH1    , /**< GPDMA1 HW request is TIM16_CH1     */
    GPDMA_REQ_TIM16_UP        = LL_GPDMA1_REQUEST_TIM16_UP     , /**< GPDMA1 HW request is TIM16_UP      */
#endif /* TIM16 */
#if defined (TIM17)
    GPDMA_REQ_TIM17_CH1       = LL_GPDMA1_REQUEST_TIM17_CH1    , /**< GPDMA1 HW request is TIM17_CH1     */
    GPDMA_REQ_TIM17_UP        = LL_GPDMA1_REQUEST_TIM17_UP     , /**< GPDMA1 HW request is TIM17_UP      */
#endif /* TIM17 */
    GPDMA_REQ_LPTIM1_IC1      = LL_GPDMA1_REQUEST_LPTIM1_IC1   , /**< GPDMA1 HW request is LPTIM1_IC1    */
    GPDMA_REQ_LPTIM1_IC2      = LL_GPDMA1_REQUEST_LPTIM1_IC2   , /**< GPDMA1 HW request is LPTIM1_IC2    */
    GPDMA_REQ_LPTIM1_UE       = LL_GPDMA1_REQUEST_LPTIM1_UE    , /**< GPDMA1 HW request is LPTIM1_UE     */
    GPDMA_REQ_LPTIM2_IC1      = LL_GPDMA1_REQUEST_LPTIM2_IC1   , /**< GPDMA1 HW request is LPTIM2_IC1    */
    GPDMA_REQ_LPTIM2_IC2      = LL_GPDMA1_REQUEST_LPTIM2_IC2   , /**< GPDMA1 HW request is LPTIM2_IC2    */
    GPDMA_REQ_LPTIM2_UE       = LL_GPDMA1_REQUEST_LPTIM2_UE    , /**< GPDMA1 HW request is LPTIM2_UE     */
#if defined (DCMI)
    GPDMA_REQ_DCMI            = LL_GPDMA1_REQUEST_DCMI         , /**< GPDMA1 HW request is DCMI          */
#endif /* DCMI */
#if defined (AES)
    GPDMA_REQ_AES_OUT         = LL_GPDMA1_REQUEST_AES_OUT      , /**< GPDMA1 HW request is AES_OUT       */
    GPDMA_REQ_AES_IN          = LL_GPDMA1_REQUEST_AES_IN       , /**< GPDMA1 HW request is AES_IN        */
#endif /* AES */
    GPDMA_REQ_HASH_IN         = LL_GPDMA1_REQUEST_HASH_IN      , /**< GPDMA1 HW request is HASH_IN       */
#if defined (UCPD1)
    GPDMA_REQ_UCPD1_RX        = LL_GPDMA1_REQUEST_UCPD1_RX     , /**< GPDMA1 HW request is UCPD1_RX      */
    GPDMA_REQ_UCPD1_TX        = LL_GPDMA1_REQUEST_UCPD1_TX     , /**< GPDMA1 HW request is UCPD1_TX      */
#endif /* UCPD1 */
#if defined (CORDIC)
    GPDMA_REQ_CORDIC_READ     = LL_GPDMA1_REQUEST_CORDIC_READ  , /**< GPDMA1 HW request is CORDIC_READ   */
    GPDMA_REQ_CORDIC_WRITE    = LL_GPDMA1_REQUEST_CORDIC_WRITE , /**< GPDMA1 HW request is CORDIC_WRITE  */
#endif /* CORDIC */
#if defined (FMAC)
    GPDMA_REQ_FMAC_READ       = LL_GPDMA1_REQUEST_FMAC_READ    , /**< GPDMA1 HW request is FMAC_READ     */
    GPDMA_REQ_FMAC_WRITE      = LL_GPDMA1_REQUEST_FMAC_WRITE   , /**< GPDMA1 HW request is FMAC_WRITE    */
#endif /* FMAC */
#if defined (SAES)
    GPDMA_REQ_SAES_OUT        = LL_GPDMA1_REQUEST_SAES_OUT     , /**< GPDMA1 HW request is SAES_OUT      */
    GPDMA_REQ_SAES_IN         = LL_GPDMA1_REQUEST_SAES_IN      , /**< GPDMA1 HW request is SAES_IN       */
#endif /* SAES */
    GPDMA_REQ_I3C1_RX         = LL_GPDMA1_REQUEST_I3C1_RX      , /**< GPDMA1 HW request is I3C1_RX       */
    GPDMA_REQ_I3C1_TX         = LL_GPDMA1_REQUEST_I3C1_TX      , /**< GPDMA1 HW request is I3C1_TX       */
    GPDMA_REQ_I3C1_TC         = LL_GPDMA1_REQUEST_I3C1_TC      , /**< GPDMA1 HW request is I3C1_TC       */
    GPDMA_REQ_I3C1_RS         = LL_GPDMA1_REQUEST_I3C1_RS      , /**< GPDMA1 HW request is I3C1_RS       */
#if defined (I2C4)
    GPDMA_REQ_I2C4_RX         = LL_GPDMA1_REQUEST_I2C4_RX      , /**< GPDMA1 HW request is I2C4_RX       */
    GPDMA_REQ_I2C4_TX         = LL_GPDMA1_REQUEST_I2C4_TX      , /**< GPDMA1 HW request is I2C4_TX       */
#endif /* I2C4 */
#if defined (LPTIM3)
    GPDMA_REQ_LPTIM3_IC1      = LL_GPDMA1_REQUEST_LPTIM3_IC1   , /**< GPDMA1 HW request is LPTIM3_IC1    */
    GPDMA_REQ_LPTIM3_IC2      = LL_GPDMA1_REQUEST_LPTIM3_IC2   , /**< GPDMA1 HW request is LPTIM3_IC2    */
    GPDMA_REQ_LPTIM3_UE       = LL_GPDMA1_REQUEST_LPTIM3_UE    , /**< GPDMA1 HW request is LPTIM3_UE     */
#endif /* LPTIM3 */
#if defined (LPTIM5)
    GPDMA_REQ_LPTIM5_IC1      = LL_GPDMA1_REQUEST_LPTIM5_IC1   , /**< GPDMA1 HW request is LPTIM5_IC1    */
    GPDMA_REQ_LPTIM5_IC2      = LL_GPDMA1_REQUEST_LPTIM5_IC2   , /**< GPDMA1 HW request is LPTIM5_IC2    */
    GPDMA_REQ_LPTIM5_UE       = LL_GPDMA1_REQUEST_LPTIM5_UE    , /**< GPDMA1 HW request is LPTIM5_UE     */
#endif /* LPTIM5 */
#if defined (LPTIM6)
    GPDMA_REQ_LPTIM6_IC1      = LL_GPDMA1_REQUEST_LPTIM6_IC1   , /**< GPDMA1 HW request is LPTIM6_IC1    */
    GPDMA_REQ_LPTIM6_IC2      = LL_GPDMA1_REQUEST_LPTIM6_IC2   , /**< GPDMA1 HW request is LPTIM6_IC2    */
    GPDMA_REQ_LPTIM6_UE       = LL_GPDMA1_REQUEST_LPTIM6_UE    , /**< GPDMA1 HW request is LPTIM6_UE     */
#endif /* LPTIM6 */
#if defined (I3C2)
    GPDMA_REQ_I3C2_RX         = LL_GPDMA1_REQUEST_I3C2_RX      , /**< GPDMA1 HW request is I3C2_RX       */
    GPDMA_REQ_I3C2_TX         = LL_GPDMA1_REQUEST_I3C2_TX      , /**< GPDMA1 HW request is I3C2_TX       */
    GPDMA_REQ_I3C2_TC         = LL_GPDMA1_REQUEST_I3C2_TC      , /**< GPDMA1 HW request is I3C2_TC       */
    GPDMA_REQ_I3C2_RS         = LL_GPDMA1_REQUEST_I3C2_RS      , /**< GPDMA1 HW request is I3C2_RS       */
#endif /* I3C2 */
}   gpdma_PeriphReqId_t;


/**
 * \brief List of possible options for request response.
 *
 */
typedef enum
{
    GPDMA_PERIPH_REQ_SINGLE = 0u, /**< Peripheral request starts single transfer     */
    GPDMA_PERIPH_REQ_BLOCK,       /**< Peripheral request starts whole block at once */
    GPDMA_PERIPH_REQ_MODE_CNT     /**< Count of available peripheral request modes   */
}   gpdma_PeriphReqMode_t;


/** \brief List of possible trigger sources */
typedef enum gpdma_TrgSrcId_t
{
    GPDMA_TRG_EXTI_LINE0     = LL_GPDMA1_TRIGGER_EXTI_LINE0     , /**< GPDMA1 HW Trigger signal is EXTI_LINE0         */
    GPDMA_TRG_EXTI_LINE1     = LL_GPDMA1_TRIGGER_EXTI_LINE1     , /**< GPDMA1 HW Trigger signal is EXTI_LINE1         */
    GPDMA_TRG_EXTI_LINE2     = LL_GPDMA1_TRIGGER_EXTI_LINE2     , /**< GPDMA1 HW Trigger signal is EXTI_LINE2         */
    GPDMA_TRG_EXTI_LINE3     = LL_GPDMA1_TRIGGER_EXTI_LINE3     , /**< GPDMA1 HW Trigger signal is EXTI_LINE3         */
    GPDMA_TRG_EXTI_LINE4     = LL_GPDMA1_TRIGGER_EXTI_LINE4     , /**< GPDMA1 HW Trigger signal is EXTI_LINE4         */
    GPDMA_TRG_EXTI_LINE5     = LL_GPDMA1_TRIGGER_EXTI_LINE5     , /**< GPDMA1 HW Trigger signal is EXTI_LINE5         */
    GPDMA_TRG_EXTI_LINE6     = LL_GPDMA1_TRIGGER_EXTI_LINE6     , /**< GPDMA1 HW Trigger signal is EXTI_LINE6         */
    GPDMA_TRG_EXTI_LINE7     = LL_GPDMA1_TRIGGER_EXTI_LINE7     , /**< GPDMA1 HW Trigger signal is EXTI_LINE7         */
    GPDMA_TRG_TAMP_TRG1      = LL_GPDMA1_TRIGGER_TAMP_TRG1      , /**< GPDMA1 HW Trigger signal is TAMP_TRG1          */
    GPDMA_TRG_TAMP_TRG2      = LL_GPDMA1_TRIGGER_TAMP_TRG2      , /**< GPDMA1 HW Trigger signal is TAMP_TRG2          */
#if defined (TAMP_CR1_TAMP3E)
    GPDMA_TRG_TAMP_TRG3      = LL_GPDMA1_TRIGGER_TAMP_TRG3      , /**< GPDMA1 HW Trigger signal is TAMP_TRG3          */
#endif /* TAMP_CR1_TAMP3E */
    GPDMA_TRG_LPTIM1_CH1     = LL_GPDMA1_TRIGGER_LPTIM1_CH1     , /**< GPDMA1 HW Trigger signal is LPTIM1_CH1         */
    GPDMA_TRG_LPTIM1_CH2     = LL_GPDMA1_TRIGGER_LPTIM1_CH2     , /**< GPDMA1 HW Trigger signal is LPTIM1_CH2         */
    GPDMA_TRG_LPTIM2_CH1     = LL_GPDMA1_TRIGGER_LPTIM2_CH1     , /**< GPDMA1 HW Trigger signal is LPTIM2_CH1         */
    GPDMA_TRG_LPTIM2_CH2     = LL_GPDMA1_TRIGGER_LPTIM2_CH2     , /**< GPDMA1 HW Trigger signal is LPTIM2_CH2         */
    GPDMA_TRG_RTC_ALRA_TRG   = LL_GPDMA1_TRIGGER_RTC_ALRA_TRG   , /**< GPDMA1 HW Trigger signal is RTC_ALRA_TRG       */
    GPDMA_TRG_RTC_ALRB_TRG   = LL_GPDMA1_TRIGGER_RTC_ALRB_TRG   , /**< GPDMA1 HW Trigger signal is RTC_ALRB_TRG       */
    GPDMA_TRG_RTC_WUT_TRG    = LL_GPDMA1_TRIGGER_RTC_WUT_TRG    , /**< GPDMA1 HW Trigger signal is RTC_WUT_TRG        */
    GPDMA_TRG_GPDMA1_CH0_TCF = LL_GPDMA1_TRIGGER_GPDMA1_CH0_TCF , /**< GPDMA1 HW Trigger signal is GPDMA1_CH0_TCF     */
    GPDMA_TRG_GPDMA1_CH1_TCF = LL_GPDMA1_TRIGGER_GPDMA1_CH1_TCF , /**< GPDMA1 HW Trigger signal is GPDMA1_CH1_TCF     */
    GPDMA_TRG_GPDMA1_CH2_TCF = LL_GPDMA1_TRIGGER_GPDMA1_CH2_TCF , /**< GPDMA1 HW Trigger signal is GPDMA1_CH2_TCF     */
    GPDMA_TRG_GPDMA1_CH3_TCF = LL_GPDMA1_TRIGGER_GPDMA1_CH3_TCF , /**< GPDMA1 HW Trigger signal is GPDMA1_CH3_TCF     */
    GPDMA_TRG_GPDMA1_CH4_TCF = LL_GPDMA1_TRIGGER_GPDMA1_CH4_TCF , /**< GPDMA1 HW Trigger signal is GPDMA1_CH4_TCF     */
    GPDMA_TRG_GPDMA1_CH5_TCF = LL_GPDMA1_TRIGGER_GPDMA1_CH5_TCF , /**< GPDMA1 HW Trigger signal is GPDMA1_CH5_TCF     */
    GPDMA_TRG_GPDMA1_CH6_TCF = LL_GPDMA1_TRIGGER_GPDMA1_CH6_TCF , /**< GPDMA1 HW Trigger signal is GPDMA1_CH6_TCF     */
    GPDMA_TRG_GPDMA1_CH7_TCF = LL_GPDMA1_TRIGGER_GPDMA1_CH7_TCF , /**< GPDMA1 HW Trigger signal is GPDMA1_CH7_TCF     */
    GPDMA_TRG_GPDMA2_CH0_TCF = LL_GPDMA1_TRIGGER_GPDMA2_CH0_TCF , /**< GPDMA1 HW Trigger signal is GPDMA2_CH0_TCF     */
    GPDMA_TRG_GPDMA2_CH1_TCF = LL_GPDMA1_TRIGGER_GPDMA2_CH1_TCF , /**< GPDMA1 HW Trigger signal is GPDMA2_CH1_TCF     */
    GPDMA_TRG_GPDMA2_CH2_TCF = LL_GPDMA1_TRIGGER_GPDMA2_CH2_TCF , /**< GPDMA1 HW Trigger signal is GPDMA2_CH2_TCF     */
    GPDMA_TRG_GPDMA2_CH3_TCF = LL_GPDMA1_TRIGGER_GPDMA2_CH3_TCF , /**< GPDMA1 HW Trigger signal is GPDMA2_CH3_TCF     */
    GPDMA_TRG_GPDMA2_CH4_TCF = LL_GPDMA1_TRIGGER_GPDMA2_CH4_TCF , /**< GPDMA1 HW Trigger signal is GPDMA2_CH4_TCF     */
    GPDMA_TRG_GPDMA2_CH5_TCF = LL_GPDMA1_TRIGGER_GPDMA2_CH5_TCF , /**< GPDMA1 HW Trigger signal is GPDMA2_CH5_TCF     */
    GPDMA_TRG_GPDMA2_CH6_TCF = LL_GPDMA1_TRIGGER_GPDMA2_CH6_TCF , /**< GPDMA1 HW Trigger signal is GPDMA2_CH6_TCF     */
    GPDMA_TRG_GPDMA2_CH7_TCF = LL_GPDMA1_TRIGGER_GPDMA2_CH7_TCF , /**< GPDMA1 HW Trigger signal is GPDMA2_CH7_TCF     */
    GPDMA_TRG_TIM2_TRGO      = LL_GPDMA1_TRIGGER_TIM2_TRGO      , /**< GPDMA1 HW Trigger signal is TIM2_TRGO          */
#if defined (TIM15)
    GPDMA_TRG_TIM15_TRGO     = LL_GPDMA1_TRIGGER_TIM15_TRGO     , /**< GPDMA1 HW Trigger signal is TIM15_TRGO         */
#endif /* TIM15 */
#if defined (TIM12)
    GPDMA_TRG_TIM12_TRGO     = LL_GPDMA1_TRIGGER_TIM12_TRGO     , /**< GPDMA1 HW Trigger signal is TIM12_TRGO         */
#endif /* TIM12 */
#if defined (LPTIM3)
    GPDMA_TRG_LPTIM3_CH1     = LL_GPDMA1_TRIGGER_LPTIM3_CH1     , /**< GPDMA1 HW Trigger signal is LPTIM3_CH1         */
    GPDMA_TRG_LPTIM3_CH2     = LL_GPDMA1_TRIGGER_LPTIM3_CH2     , /**< GPDMA1 HW Trigger signal is LPTIM3_CH2         */
#endif /* LPTIM3 */
#if defined (LPTIM4)
    GPDMA_TRG_LPTIM4_AIT     = LL_GPDMA1_TRIGGER_LPTIM4_AIT     , /**< GPDMA1 HW Trigger signal is LPTIM4_AIT         */
#endif /* LPTIM4 */
#if defined (LPTIM5)
    GPDMA_TRG_LPTIM5_CH1     = LL_GPDMA1_TRIGGER_LPTIM5_CH1     , /**< GPDMA1 HW Trigger signal is LPTIM5_CH1         */
    GPDMA_TRG_LPTIM5_CH2     = LL_GPDMA1_TRIGGER_LPTIM5_CH2     , /**< GPDMA1 HW Trigger signal is LPTIM5_CH2         */
#endif /* LPTIM5 */
#if defined (LPTIM6)
    GPDMA_TRG_LPTIM6_CH1     = LL_GPDMA1_TRIGGER_LPTIM6_CH1     , /**< GPDMA1 HW Trigger signal is LPTIM6_CH1         */
    GPDMA_TRG_LPTIM6_CH2     = LL_GPDMA1_TRIGGER_LPTIM6_CH2     , /**< GPDMA1 HW Trigger signal is LPTIM6_CH2         */
#endif /* LPTIM6 */
#if defined (COMP1)
    GPDMA_TRG_COMP1_OUT      = LL_GPDMA1_TRIGGER_COMP1_OUT      , /**< GPDMA1 HW Trigger signal is COMP1_OUT          */
#endif /* COMP1 */
#if defined (STM32H503xx) || defined(STM32H523xx) || defined(STM32H533xx)
    GPDMA_TRG_EVENTOUT       = LL_GPDMA1_TRIGGER_EVENTOUT       , /**< GPDMA1 HW Trigger signal is EVENTOUT           */
#endif /* STM32H503xx || STM32H523xx || STM32H533xx */
}   gpdma_TrgSrcId_t;


/** \brief Trigger type enumeration */
typedef enum
{
    GPDMA_TRG_NOT_USED = 0u, /**< Trigger will not be used for transfer      */
    GPDMA_TRG_RISING,        /**< Raising edge of trigger event will be used */
    GPDMA_TRG_FALLING,       /**< Falling edge of trigger event will be used */
    GPDMA_TRG_TYPE_CNT       /**< Count of available trigger types           */
}   gpdma_TrgType_t;


/** \brief Trigger modes enumeration */
typedef enum
{
    GPDMA_TRIGGER_BLOCK = 0u, /**< Trigger is needed for each block transfer.
                                   If 2D transfer is used, next row needs another
                                   trigger. */
    GPDMA_TRIGGER_2D_BLOCK,   /**< Trigger is needed for each 2D block transfer.
                                   All columns and rows will be transfered after
                                   single trigger. */
    GPDMA_TRIGGER_TRANSFER,   /**< Trigger is needed for each transfer change.
                                   This is available only if multiple linked
                                   transfers are used. */
    GPDMA_TRIGGER_SINGLE,     /**< Trigger is needed for each single transfer.
                                   Every transfer (whole burst count) from/to
                                   memory/peripheral need trigger for execution.
                                   This mode is like double condition for transfer
                                   request. */
    GPDMA_TRIGGER_MODE_CNT    /**< Count of available trigger modes */
}   gpdma_TriggerMode_t;


/** \brief Destination data operations enumeration */
typedef enum
{
    GPDMA_DEST_DATA_PRESERVE = 0u,    /**< Destination data preserved (not changed)                                             */
    GPDMA_DEST_DATA_2BYTES_SWAP,      /**< 2 upper bytes are swapped with 2 lower bytes (ignored if destination size is 16bits) */
    GPDMA_DEST_DATA_BYTE_SWAP,        /**< Two consequent bytes are swapped (ignored if destination size is byte)               */
    GPDMA_DEST_DATA_BYTE_2BYTES_SWAP, /**< Combination of Double-Byte and Byte Swap (two previous options)                      */
    GPDMA_DEST_DATA_OP_CNT            /**< Count of destination data operation settings                                         */
}   gpdma_DestDataOp_t;


/** \brief Source data operations enumeration */
typedef enum
{
    GPDMA_SRC_DATA_PRESERVE = 0u, /**< Source data preserved (not changed)                                    */
    GPDMA_SRC_DATA_BYTE_SWAP,     /**< Two consequent bytes are swapped (ignored if destination size is byte) */
    GPDMA_SRC_DATA_OP_CNT         /**< Count of source data operation settings                                */
}   gpdma_SrcDataOp_t;


/**
 * \brief Transfer execution mode enumeration (if multiple transfers are used)
 */
typedef enum
{
    GPDMA_XFER_EXEC_BLOCKING = 0u, /**< Transfer will be finished after execution.    */
    GPDMA_XFER_EXEC_CONTINUOUS,    /**< Next transfer will be executed automatically. */
    GPDMA_XFER_EXEC_MODES_CNT      /**< Count of transfer execution modes.            */
}   gpdma_XferExecMode_t;


typedef enum
{
    /** Transfer list has to have only one transfer configuration.
     *  If another transfers exists in required channel, the request
     *  will be ignored. */
    GPDMA_TRANSFER_LIST_ACCESS_SINGLE = 0u,
    /** Transfer will be appended to the existing list if possible.
     *  Transfer will be added as the first if selected channel is
     *  not yet used. */
    GPDMA_TRANSFER_LIST_ACCESS_APPEND,
    /** Count of available transfer creation list. */
    GPDMA_TRANSFER_LIST_CNT
}   gpdma_XferListAccessMode_t;


typedef enum
{
    GPDMA_TRANSFER_LIST_LOCKED = 0u, /**< Transfer list is locked and new transfers cannot be added. */
    GPDMA_TRANSFER_LIST_UNLOCKED,    /**< Transfer list is free to add new transfers into queue.     */
    GPDMA_TRANSFER_LIST_LOCK_CNT     /**< Count of transfer lock options.                            */
}   gpdma_XferListLock_t;


/** Type representing count of transfers configured through type \ref gpdma_TransferConfig_t */
typedef uint32_t gpdma_TransfersCount_t;


/** \brief Transfer list execution mode enumeration */
typedef enum
{
    /** Transfer list will be executed once and then will be stopped. */
    GPDMA_XFER_LIST_EXEC_ONCE = 0u,
    /** Whole transfer list will be executed cyclically.
     * User select if X-fer list will be locked or not. */
    GPDMA_XFER_LIST_EXEC_CYCLIC_ALL,
    /** Single transfer will be executed cyclically.
     * The X-fer list will be blocked after adding this configuration. All
     * previous X-fers in X-fer list will be executed once. */
    GPDMA_XFER_LIST_EXEC_SINGLE_CYCLIC,
    /** Count of transfer list execution modes. */
    GPDMA_XFER_LIST_EXEC_CNT
}   gpdma_XferListExecMode_t;


/** Interrupt callback function pointer type */
typedef void ( gpdma_IsrCallback )( void );


/** Error interrupt callback function pointer type */
typedef void ( gpdma_IsrErrCallback )( gpdma_ErrorMaskId_t );


/** \brief Internal structure used for list of transfers.
 *
 * \warning User shall not access values of this structure/array! The values
 *          will be configured by GPDMA MCAL handler.
 *
 * \warning Variable created for this structure/array MUST be static!
 */
typedef struct
{
    volatile uint32_t Register[ 8u ];
}   gpdma_XferList_t;


/** \brief configuration structure of single transfer.
 *
 * User can configure multiple transfers by using this structure for array
 * definition. If single transfer will be used, the count of transfers must be
 * set to 0.
 */
typedef struct
{
    gpdma_Direction_t        Direction;
    gpdma_XferListExecMode_t XferListExecMode;              /**< Execution mode of transfer list */

    gpdma_TransferEvent_t    EventMode;

    gpdma_TrgType_t          TriggerType;
    gpdma_TrgSrcId_t         TriggerSource;
    gpdma_TriggerMode_t      TriggerMode;

    gpdma_PeriphReqId_t      RequestSource;
    gpdma_PeriphReqMode_t    RequestMode;

    gpdma_BlockSize_t        BlockSize;                     /**< Size of each block in bytes */
    gpdma_BlockRep_t         BlockRepetitionCount;          /**< Count of block repetitions. This is available only for 2D transfers. For linear transfers this value must be 0. */

    gpdma_SrcAddr_t          SourceAddr;
    gpdma_DataSize_t         SourceDataSize;
    gpdma_BurstLength_t      SourceBurstLength;
    gpdma_AddrMode_t         SourceAddrMode;
    gpdma_PortId_t           SourcePortId;
    gpdma_SrcDataOp_t        SourceDataOp;                  /**< Source data operation settings */
    gpdma_ByteCnt_t          SourceBlockOffset2D;
    gpdma_ByteCnt_t          SourceRepBlockOffset2D;

    gpdma_DstAddr_t          DestinationAddr;
    gpdma_DataSize_t         DestinationDataSize;
    gpdma_BurstLength_t      DestinationBurstLength;
    gpdma_AddrMode_t         DestinationAddrMode;
    gpdma_PortId_t           DestinationPortId;
    gpdma_DestDataOp_t       DestinationDataOp;             /**< Destination data operation settings */
    gpdma_ByteCnt_t          DestinationBlockOffset2D;
    gpdma_ByteCnt_t          DestinationRepBlockOffset2D;

}   gpdma_TransferConfig_t;


typedef struct
{
    gpdma_PeriphId_t            PeriphId;            /**< GPDMA peripheral ID. */
    gpdma_ChannelId_t           ChannelId;           /**< GPDMA channel ID. */
    gpdma_Priority_t            ChannelPrio;         /**< CPDMA channel priority configuration. */

    gpdma_XferExecMode_t        TransferExecMode;    /**< Transfer execution mode (if multiple transfers used) */
    gpdma_TransferConfig_t     *TransferConfig;      /**< User configuration of transfers to be executed by GPDMA channel. */
    gpdma_TransfersCount_t      TransfersCount;      /**< Count of transfer to be configured. */
    gpdma_XferListAccessMode_t  XferListAccessMode;  /**< Accessibility of current transfer configuration. */
    volatile gpdma_XferList_t  *XferList;            /**< Internally used transfer list. User shall not change the values. Array must be size of \ref TransfersCount and must be static. */
    gpdma_XferListLock_t        TransferLockState;   /**< Transfer list lock mode (specifies if user can add another transfers or not) */

    gpdma_IsrCallback          *TransferCompleteIsr; /**< Transfer complete interrupt service routine. If configured, the IRQ will be activated. */
    gpdma_IsrCallback          *HalfTransferIsr;     /**< Half transfer complete interrupt service routine. If configured, the IRQ will be activated. */
    gpdma_IsrErrCallback       *ErrorIsr;            /**< Transfer error interrupt service routine. If configured, the IRQ will be activated. */
    gpdma_ErrorMaskId_t         ErrorMask;           /**< Error activation bit mask. */
}   gpdma_ConfigStruct_t;

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */


#endif /* GPDMA_GPDMA_TYPES_H */
