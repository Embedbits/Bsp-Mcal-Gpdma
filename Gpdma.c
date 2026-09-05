/**
 * \author Mr.Nobody
 * \file Gpdma.h
 * \ingroup Gpdma
 * \brief Gpdma module common functionality
 *
 */
/* ============================== INCLUDES ================================== */
#include "Gpdma.h"                          /* Self include                   */
#include "Gpdma_Port.h"                     /* Own port file include          */
#include "Gpdma_Types.h"                    /* Module types definitions       */
#include "Gpdma_Tl.h"                       /* Transfer list functionality    */
#include "Rcc_Port.h"                       /* RCC module functionality       */
#include "Nvic_Port.h"                      /* NVIC module functionality      */
#include "Stm32_dma.h"                      /* DMA RAL functionality          */
#include "Stm32_utils.h"                    /* MCU utilities functionality    */
#include "Stm32_bus.h"                      /* MCU utilities functionality    */
/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Value of major version of SW module */
#define GPDMA_MAJOR_VERSION           ( 1u )

/** Value of minor version of SW module */
#define GPDMA_MINOR_VERSION           ( 0u )

/** Value of patch version of SW module */
#define GPDMA_PATCH_VERSION           ( 0u )


/** Global Interrupt Request (IRQ) flag identification for use in \ref DMA_GET_ACTIVE_IRQ_FLAG */
#define DMA_GI_IRQ_FLAG                                 ( 0u )
/** Transfer complete Interrupt Request (IRQ) flag identification for use in \ref DMA_GET_ACTIVE_IRQ_FLAG */
#define DMA_TC_IRQ_FLAG                                 ( 1u )
/** Half transfer Interrupt Request (IRQ) flag identification for use in \ref DMA_GET_ACTIVE_IRQ_FLAG */
#define DMA_HT_IRQ_FLAG                                 ( 2u )
/** Transfer error Interrupt Request (IRQ) flag identification for use in \ref DMA_GET_ACTIVE_IRQ_FLAG */
#define DMA_TE_IRQ_FLAG                                 ( 3u )
/**
 * \brief Multiplier for use in \ref DMA_GET_ACTIVE_IRQ_FLAG
 *
 *  Multiplier value represents count of bits used for every channel in
 *  interrupt status register. Every channel has interrupt flags in same order.
 *  The reading of flag bit is done as masking by bit shifted by channel ID
 *  (0-6 for channels 1-7) multiplied by offset (count of status bits per channel)
 *  and incremented by flag ID (offset of bit within channel)
 */
#define DMA_IRQ_MULTIPLIER                              ( 4u )

/* =============================== MACROS =================================== */

/**
 * \brief Macro reading interrupt status flag.
 *
 *  Multiplier value represents count of bits used for every channel in
 *  interrupt status register. Every channel has interrupt flags in same order.
 *  The reading of flag bit is done as masking by bit shifted by channel ID
 *  (0-6 for channels 1-7) multiplied by offset (count of status bits per channel)
 *  and incremented by flag ID (offset of bit within channel)
 *
 *  Parameter DMAx is representing DMA peripheral, CHANNEL is representing
 *  channel id from \ref dma_ChannelId_t and flag can be one of
 *  \ref DMA_GI_IRQ_FLAG for channel global interrupt
 *  \ref DMA_TC_IRQ_FLAG for transfer complete flag
 *  \ref DMA_HT_IRQ_FLAG for half transfer flag
 *  \ref DMA_TE_IRQ_FLAG for transfer error flag
 */
#define GPDMA_GET_ACTIVE_IRQ_FLAG(DMAx, CHANNEL, FLAG)    ( READ_BIT( DMAx->MISR, ( 0x01 << ( ( CHANNEL * DMA_IRQ_MULTIPLIER ) + FLAG ) ) ) )

/**
 * \brief Macro clearing interrupt status flag.
 *
 *  Multiplier value represents count of bits used for every channel in
 *  interrupt status register. Every channel has interrupt flags in same order.
 *  The reading of flag bit is done as masking by bit shifted by channel ID
 *  (0-6 for channels 1-7) multiplied by offset (count of status bits per channel)
 *  and incremented by flag ID (offset of bit within channel)
 *
 *  Parameter DMAx is representing DMA peripheral, CHANNEL is representing
 *  channel id from \ref dma_ChannelId_t and flag can be one of
 *  \ref DMA_GI_IRQ_FLAG for channel global interrupt
 *  \ref DMA_TC_IRQ_FLAG for transfer complete flag
 *  \ref DMA_HT_IRQ_FLAG for half transfer flag
 *  \ref DMA_TE_IRQ_FLAG for transfer error flag
 */
#define GPDMA_CLEAR_ACTIVE_IRQ_FLAG(DMAx, CHANNEL, FLAG)  ( CLEAR_BIT( DMAx->MISR, ( 0x01 << ( ( CHANNEL * DMA_IRQ_MULTIPLIER ) + FLAG ) ) ) )


/* ============================== TYPEDEFS ================================== */

/** Type of ISR callback function */
typedef void (*gpdma_NvicIsrCallback)( void );


/** Transfers runtime values. Used for ability to append new transfers. */
typedef struct
{
    volatile gpdma_ChannelId_t      ChannelId;         /**< GPDMA channel ID. */
    volatile gpdma_XferListLock_t   TransferLockState; /**< Transfer list lock mode (specifies if user can add another transfers or not) */
    volatile gpdma_TransfersCount_t XferCount;         /**< Count of transfer to be configured. */
    volatile gpdma_DataAddr_t       FirstXferListAddr;  /**< Address of first Xfer list. */
    volatile gpdma_DataAddr_t       LastXferListAddr;  /**< Address of last Xfer list. */
}   gpdma_TransferRuntime_t;


/** \brief Structure type used for storing Interrupt Status Routine (ISR) callback's */
typedef struct
{
    gpdma_ChannelId_t     ChannelId;                /**< Channel identification       */
    gpdma_IsrCallback    *TransferCompleteCallback; /**< Transfer complete callback   */
    gpdma_IsrCallback    *HalfTransferCallback;     /**< Half transfer callback       */
    gpdma_IsrErrCallback *ErrorCallback;            /**< Transfer error callback      */
}   gpdma_IsrCallbacks_t;


/** \brief Structure type used for storing channels configuration */
typedef struct
{
    gpdma_ChannelId_t     ChannelId;          /**< Channel identification                               */
    gpdma_ChannelType_t   ChannelTypeSupport; /**< Supported channel transfer style                     */
    nvic_PeriphIrqList_t  NvicChannelIrq;     /**< Channel interrupt identification used by NVIC module */
    uint32_t              ChannelReg;         /**< Channel identification used for register operations  */
    gpdma_NvicIsrCallback ChannelIsrHandler;  /**< Channel Interrupt Status Routine (ISR) handler       */
}   gpdma_ChannelsConfig_t;

/** \brief Structure type used by DMA configuration array */
typedef struct
{
    gpdma_PeriphId_t                   GpdmaPeriphId;         /**< GPDMA bus identification   */
    rcc_PeriphId_t                     RccPeriphId;           /**< RCC bus identification     */
    DMA_TypeDef                      * DmaReg;                /**< DMA bus register address   */
    const gpdma_ChannelsConfig_t     * const ChannelsConfig;  /**< DMA channels configuration */
    volatile gpdma_IsrCallbacks_t    * const IsrCallbacks;    /**< ISR callback's array       */
    volatile gpdma_TransferRuntime_t * const TransferRuntime; /**< Transfers runtime data     */
}   gpdma_PeriphConfig_t;

/* ======================== FORWARD DECLARATIONS ============================ */

static inline void Gpdma_GlobalIsrHandler( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );

static void Gpdma_Gpdma1Channel0_IsrHandler( void );
static void Gpdma_Gpdma1Channel1_IsrHandler( void );
static void Gpdma_Gpdma1Channel2_IsrHandler( void );
static void Gpdma_Gpdma1Channel3_IsrHandler( void );
static void Gpdma_Gpdma1Channel4_IsrHandler( void );
static void Gpdma_Gpdma1Channel5_IsrHandler( void );
static void Gpdma_Gpdma1Channel6_IsrHandler( void );
static void Gpdma_Gpdma1Channel7_IsrHandler( void );
static void Gpdma_Gpdma1Channel8_IsrHandler( void );
static void Gpdma_Gpdma1Channel9_IsrHandler( void );
static void Gpdma_Gpdma1Channel10_IsrHandler( void );
static void Gpdma_Gpdma1Channel11_IsrHandler( void );
static void Gpdma_Gpdma1Channel12_IsrHandler( void );
static void Gpdma_Gpdma1Channel13_IsrHandler( void );
static void Gpdma_Gpdma1Channel14_IsrHandler( void );
static void Gpdma_Gpdma1Channel15_IsrHandler( void );
#if defined(GPDMA2)
static void Gpdma_Gpdma2Channel0_IsrHandler( void );
static void Gpdma_Gpdma2Channel1_IsrHandler( void );
static void Gpdma_Gpdma2Channel2_IsrHandler( void );
static void Gpdma_Gpdma2Channel3_IsrHandler( void );
static void Gpdma_Gpdma2Channel4_IsrHandler( void );
static void Gpdma_Gpdma2Channel5_IsrHandler( void );
static void Gpdma_Gpdma2Channel6_IsrHandler( void );
static void Gpdma_Gpdma2Channel7_IsrHandler( void );
#endif

/* ========================== EXPORTED VARIABLES ============================ */

/* =========================== LOCAL VARIABLES ============================== */

static volatile gpdma_TransferRuntime_t     gpdma_Gpdma1RuntimeData[ GPDMA_CHANNEL_CNT ] =
{
    { .ChannelId = GPDMA_CHANNEL_0 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_1 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_2 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_3 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_4 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_5 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_6 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_7 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_8 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_9 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_10, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_11, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_12, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_13, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_14, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_15, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
};


#if defined(GPDMA2)
static volatile gpdma_TransferRuntime_t     gpdma_Gpdma2RuntimeData[ GPDMA_CHANNEL_CNT ] =
{
    { .ChannelId = GPDMA_CHANNEL_0 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_1 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_2 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_3 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_4 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_5 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_6 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_7 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_8 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_9 , .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_10, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_11, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_12, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_13, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_14, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
    { .ChannelId = GPDMA_CHANNEL_15, .TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED, .XferCount = 0u, .FirstXferListAddr = 0u, .LastXferListAddr = 0u },
};
#endif


static volatile gpdma_IsrCallbacks_t        gpdma_Gpdma1IrqCallbacks[ GPDMA_CHANNEL_CNT ] =
{
    { .ChannelId = GPDMA_CHANNEL_0  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_1  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_2  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_3  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_4  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_5  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_6  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_7  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_8  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_9  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_10 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_11 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_12 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_13 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_14 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_15 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR }
};


#if defined(GPDMA2)
static volatile gpdma_IsrCallbacks_t        gpdma_Gpdma2IrqCallbacks[ GPDMA_CHANNEL_CNT ] =
{
    { .ChannelId = GPDMA_CHANNEL_0  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_1  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_2  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_3  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_4  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_5  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_6  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_7  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_8  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_9  , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_10 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_11 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_12 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_13 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_14 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR },
    { .ChannelId = GPDMA_CHANNEL_15 , .TransferCompleteCallback = GPDMA_NULL_PTR , .HalfTransferCallback = GPDMA_NULL_PTR , .ErrorCallback = GPDMA_NULL_PTR }
};
#endif


static const gpdma_ChannelsConfig_t         gpdma_Gpdma1IrqConfig[ GPDMA_CHANNEL_CNT ] =
{
    { .ChannelId = GPDMA_CHANNEL_0 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL0 , .ChannelReg = LL_DMA_CHANNEL_0 , .ChannelIsrHandler = Gpdma_Gpdma1Channel0_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_1 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL1 , .ChannelReg = LL_DMA_CHANNEL_1 , .ChannelIsrHandler = Gpdma_Gpdma1Channel1_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_2 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL2 , .ChannelReg = LL_DMA_CHANNEL_2 , .ChannelIsrHandler = Gpdma_Gpdma1Channel2_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_3 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL3 , .ChannelReg = LL_DMA_CHANNEL_3 , .ChannelIsrHandler = Gpdma_Gpdma1Channel3_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_4 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL4 , .ChannelReg = LL_DMA_CHANNEL_4 , .ChannelIsrHandler = Gpdma_Gpdma1Channel4_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_5 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL5 , .ChannelReg = LL_DMA_CHANNEL_5 , .ChannelIsrHandler = Gpdma_Gpdma1Channel5_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_6 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL6 , .ChannelReg = LL_DMA_CHANNEL_6 , .ChannelIsrHandler = Gpdma_Gpdma1Channel6_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_7 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL7 , .ChannelReg = LL_DMA_CHANNEL_7 , .ChannelIsrHandler = Gpdma_Gpdma1Channel7_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_8 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL8 , .ChannelReg = LL_DMA_CHANNEL_8 , .ChannelIsrHandler = Gpdma_Gpdma1Channel8_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_9 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL9 , .ChannelReg = LL_DMA_CHANNEL_9 , .ChannelIsrHandler = Gpdma_Gpdma1Channel9_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_10, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL10, .ChannelReg = LL_DMA_CHANNEL_10, .ChannelIsrHandler = Gpdma_Gpdma1Channel10_IsrHandler},
    { .ChannelId = GPDMA_CHANNEL_11, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL11, .ChannelReg = LL_DMA_CHANNEL_11, .ChannelIsrHandler = Gpdma_Gpdma1Channel11_IsrHandler},
    { .ChannelId = GPDMA_CHANNEL_12, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR_2D, .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL12, .ChannelReg = LL_DMA_CHANNEL_12, .ChannelIsrHandler = Gpdma_Gpdma1Channel12_IsrHandler},
    { .ChannelId = GPDMA_CHANNEL_13, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR_2D, .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL13, .ChannelReg = LL_DMA_CHANNEL_13, .ChannelIsrHandler = Gpdma_Gpdma1Channel13_IsrHandler},
    { .ChannelId = GPDMA_CHANNEL_14, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR_2D, .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL14, .ChannelReg = LL_DMA_CHANNEL_14, .ChannelIsrHandler = Gpdma_Gpdma1Channel14_IsrHandler},
    { .ChannelId = GPDMA_CHANNEL_15, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR_2D, .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA1_CHANNEL15, .ChannelReg = LL_DMA_CHANNEL_15, .ChannelIsrHandler = Gpdma_Gpdma1Channel15_IsrHandler},
};


#if defined(GPDMA2)
static const gpdma_ChannelsConfig_t         gpdma_Gpdma2IrqConfig[ GPDMA_CHANNEL_CNT ] =
{
    { .ChannelId = GPDMA_CHANNEL_0 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL0 , .ChannelReg = LL_DMA_CHANNEL_0 , .ChannelIsrHandler = Gpdma_Gpdma2Channel0_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_1 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL1 , .ChannelReg = LL_DMA_CHANNEL_1 , .ChannelIsrHandler = Gpdma_Gpdma2Channel1_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_2 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL2 , .ChannelReg = LL_DMA_CHANNEL_2 , .ChannelIsrHandler = Gpdma_Gpdma2Channel2_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_3 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL3 , .ChannelReg = LL_DMA_CHANNEL_3 , .ChannelIsrHandler = Gpdma_Gpdma2Channel3_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_4 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL4 , .ChannelReg = LL_DMA_CHANNEL_4 , .ChannelIsrHandler = Gpdma_Gpdma2Channel4_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_5 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL5 , .ChannelReg = LL_DMA_CHANNEL_5 , .ChannelIsrHandler = Gpdma_Gpdma2Channel5_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_6 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL6 , .ChannelReg = LL_DMA_CHANNEL_6 , .ChannelIsrHandler = Gpdma_Gpdma2Channel6_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_7 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL7 , .ChannelReg = LL_DMA_CHANNEL_7 , .ChannelIsrHandler = Gpdma_Gpdma2Channel7_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_8 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL8 , .ChannelReg = LL_DMA_CHANNEL_8 , .ChannelIsrHandler = Gpdma_Gpdma2Channel8_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_9 , .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL9 , .ChannelReg = LL_DMA_CHANNEL_9 , .ChannelIsrHandler = Gpdma_Gpdma2Channel9_IsrHandler },
    { .ChannelId = GPDMA_CHANNEL_10, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL10, .ChannelReg = LL_DMA_CHANNEL_10, .ChannelIsrHandler = Gpdma_Gpdma2Channel10_IsrHandler},
    { .ChannelId = GPDMA_CHANNEL_11, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR   , .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL11, .ChannelReg = LL_DMA_CHANNEL_11, .ChannelIsrHandler = Gpdma_Gpdma2Channel11_IsrHandler},
    { .ChannelId = GPDMA_CHANNEL_12, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR_2D, .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL12, .ChannelReg = LL_DMA_CHANNEL_12, .ChannelIsrHandler = Gpdma_Gpdma2Channel12_IsrHandler},
    { .ChannelId = GPDMA_CHANNEL_13, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR_2D, .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL13, .ChannelReg = LL_DMA_CHANNEL_13, .ChannelIsrHandler = Gpdma_Gpdma2Channel13_IsrHandler},
    { .ChannelId = GPDMA_CHANNEL_14, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR_2D, .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL14, .ChannelReg = LL_DMA_CHANNEL_14, .ChannelIsrHandler = Gpdma_Gpdma2Channel14_IsrHandler},
    { .ChannelId = GPDMA_CHANNEL_15, .ChannelTypeSupport = GPDMA_CHANNEL_LINEAR_2D, .NvicChannelIrq = NVIC_PERIPH_IRQ_GPDMA2_CHANNEL15, .ChannelReg = LL_DMA_CHANNEL_15, .ChannelIsrHandler = Gpdma_Gpdma2Channel15_IsrHandler},
};
#endif


static const volatile gpdma_PeriphConfig_t  gpdma_PeriphConf[ GPDMA_PERIPH_CNT ] =
{
    { .GpdmaPeriphId = GPDMA_PERIPH_1 , .DmaReg = GPDMA1 , .RccPeriphId = RCC_PERIPH_GPDMA1 , .ChannelsConfig = gpdma_Gpdma1IrqConfig , .IsrCallbacks = gpdma_Gpdma1IrqCallbacks, .TransferRuntime = gpdma_Gpdma1RuntimeData },
#if defined(GPDMA2)
    { .GpdmaPeriphId = GPDMA_PERIPH_2 , .DmaReg = GPDMA2 , .RccPeriphId = RCC_PERIPH_GPDMA2 , .ChannelsConfig = gpdma_Gpdma2IrqConfig , .IsrCallbacks = gpdma_Gpdma2IrqCallbacks, .TransferRuntime = gpdma_Gpdma2RuntimeData },
#endif
};

/* ========================= EXPORTED FUNCTIONS ============================= */

/**
 * \brief Returns module SW version
 *
 * \return Module SW version
 */
gpdma_ModuleVersion_t Gpdma_Get_ModuleVersion( void )
{
    gpdma_ModuleVersion_t retVersion;

    retVersion.Major = GPDMA_MAJOR_VERSION;
    retVersion.Minor = GPDMA_MINOR_VERSION;
    retVersion.Patch = GPDMA_PATCH_VERSION;

    return (retVersion);
}


gpdma_RequestState_t Gpdma_Get_DefaultConfig( gpdma_ConfigStruct_t * const configStruct )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != configStruct )
    {
        configStruct->PeriphId            = GPDMA_PERIPH_1;
        configStruct->ChannelId           = GPDMA_CHANNEL_0;
        configStruct->ChannelPrio         = GPDMA_PRIORITY_LOW;

        configStruct->TransferExecMode    = GPDMA_XFER_EXEC_CONTINUOUS;
        configStruct->TransferConfig      = GPDMA_NULL_PTR;
        configStruct->TransfersCount      = 1u;
        configStruct->XferListAccessMode  = GPDMA_TRANSFER_LIST_ACCESS_APPEND;
        configStruct->XferList            = GPDMA_NULL_PTR;
        configStruct->TransferLockState   = GPDMA_TRANSFER_LIST_UNLOCKED;

        configStruct->TransferCompleteIsr = GPDMA_NULL_PTR;
        configStruct->HalfTransferIsr     = GPDMA_NULL_PTR;
        configStruct->ErrorIsr            = GPDMA_NULL_PTR;
        configStruct->ErrorMask           = 0u;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return (status);
}


/**
 * \brief Initializes module Gpdma
 *
 * This function shall call every necessary sub-module initialization function 
 * and set up all the necessary resources for the module to work.
 */
gpdma_RequestState_t Gpdma_Init( gpdma_ConfigStruct_t * const configStruct )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_OK;

    if( GPDMA_NULL_PTR != configStruct )
    {
        if( ( GPDMA_PERIPH_CNT  > configStruct->PeriphId  ) &&
            ( GPDMA_CHANNEL_CNT > configStruct->ChannelId ) &&
            ( GPDMA_NULL_PTR   != configStruct->XferList  )    )
        {
            /*----------- Check if channel was already initialized -----------*/
            if( 0u == gpdma_PeriphConf[ configStruct->PeriphId ].TransferRuntime[ configStruct->ChannelId ].XferCount )
            {
                /*--------------------- Initialize RCC block ---------------------*/
                rcc_FunctionState_t rccState = RCC_FUNCTION_INACTIVE;

                rcc_RequestState_t rccRetState = Rcc_Get_PeriphState( gpdma_PeriphConf[ configStruct->PeriphId ].RccPeriphId, &rccState );

                if( RCC_REQUEST_ERROR == rccRetState )
                {
                    /* Peripheral clock activation failed. */
                    status = GPDMA_REQUEST_ERROR;
                }
                else
                {
                    if( RCC_FUNCTION_INACTIVE == rccState )
                    {
                        rccRetState = Rcc_Set_PeriphActive( gpdma_PeriphConf[ configStruct->PeriphId ].RccPeriphId );

                        if( RCC_REQUEST_ERROR == rccRetState )
                        {
                            /* Peripheral clock activation failed. */
                            status = GPDMA_REQUEST_ERROR;
                        }
                        else
                        {
                            /* Peripheral clock activation successful. */
                        }
                    }
                    else
                    {
                        /* Peripheral clock is already active. */
                    }
                }


                /* Channel must be inactive during configuration */
                if( GPDMA_REQUEST_OK == status )
                {
                    /* Configuration shall be executed in inactive peripheral */
                    gpdma_FunctionState_t channelState = GPDMA_FUNCTION_INACTIVE;

                    status = Gpdma_Get_ChannelState( configStruct->PeriphId,
                                                     configStruct->ChannelId,
                                                     &channelState );

                    if( GPDMA_FUNCTION_ACTIVE == channelState )
                    {
                        /* Peripheral is active, initialization cannot be executed. */
                        status = GPDMA_REQUEST_ERROR;
                    }
                    else
                    {
                        /* Continue with initialization process. */
                    }
                }
                else
                {
                    /* Error during initialization process */
                }

                /* Configure channel priority */
                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_Priority( configStruct->PeriphId,
                                                 configStruct->ChannelId,
                                                 configStruct->ChannelPrio );
                }
                else
                {
                    /* Error during initialization process */
                }

                /* Configure execution step mode */
                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_XferExecMode( configStruct->PeriphId,
                                                     configStruct->ChannelId,
                                                     configStruct->TransferExecMode );
                }
                else
                {
                    /* Error during initialization process */
                }


                /*----------------- Interrupts configuration -----------------*/
                if( GPDMA_REQUEST_OK == status )
                {
                    /* Configure NVIC and GPDMA module inter-connection */
                    nvic_RequestState_t nvicRetState = NVIC_REQUEST_ERROR;

                    nvic_IsrCallback_t nvicIrqHandler = GPDMA_NULL_PTR;

                    gpdma_NvicIsrCallback isrCallback = gpdma_PeriphConf[ configStruct->PeriphId ].ChannelsConfig[ configStruct->ChannelId ].ChannelIsrHandler;

                    nvic_PeriphIrqList_t nvicIrqId = gpdma_PeriphConf[ configStruct->PeriphId ].ChannelsConfig[ configStruct->ChannelId ].NvicChannelIrq;

                    nvicRetState = Nvic_Set_PeriphIrq_Handler( nvicIrqId, nvicIrqHandler );

                    if( NVIC_REQUEST_OK != nvicRetState )
                    {
                        /* NVIC configuration error */
                        status = GPDMA_REQUEST_ERROR;
                    }
                    else
                    {
                        if( isrCallback != nvicIrqHandler )
                        {
                            nvicRetState = Nvic_Set_PeriphIrq_Handler( nvicIrqId, isrCallback );

                            if( NVIC_REQUEST_OK != nvicRetState )
                            {
                                status = GPDMA_REQUEST_ERROR;
                            }
                        }
                        else
                        {
                            /* Callback is already configured */
                        }
                    }
                }
                else
                {
                    /* Error during initialization process */
                }


                /* Configure transfer complete interrupt handling */
                if( GPDMA_REQUEST_OK == status )
                {
                    if( GPDMA_NULL_PTR != configStruct->TransferCompleteIsr )
                    {
                        status = Gpdma_Set_TransferCompleteIsrHandler( configStruct->PeriphId,
                                                                       configStruct->ChannelId,
                                                                       configStruct->TransferCompleteIsr );

                        if( GPDMA_REQUEST_OK == status )
                        {
                            status = Gpdma_Set_TransferCompleteIrqActive( configStruct->PeriphId,
                                                                          configStruct->ChannelId );
                        }
                        else
                        {
                            /* Error during initialization process */
                        }
                    }
                }
                else
                {
                    /* Error during initialization process */
                }


                /* Configure half-transfer complete interrupt handling */
                if( GPDMA_REQUEST_OK == status )
                {
                    if( GPDMA_NULL_PTR != configStruct->HalfTransferIsr )
                    {
                        status = Gpdma_Set_HalfTransferIsrHandler( configStruct->PeriphId,
                                                                   configStruct->ChannelId,
                                                                   configStruct->HalfTransferIsr );

                        if( GPDMA_REQUEST_OK == status )
                        {
                            status = Gpdma_Set_HalfTransferIrqActive( configStruct->PeriphId,
                                                                      configStruct->ChannelId );
                        }
                        else
                        {
                            /* Error during initialization process */
                        }
                    }
                }
                else
                {
                    /* Error during initialization process */
                }


                /* Configure transfer error interrupt handling */
                if( GPDMA_REQUEST_OK == status )
                {
                    if( GPDMA_NULL_PTR != configStruct->ErrorIsr )
                    {
                        status = Gpdma_Set_ErrorIsrHandler( configStruct->PeriphId,
                                                            configStruct->ChannelId,
                                                            configStruct->ErrorIsr );

                        if( ( GPDMA_REQUEST_OK     == status                                             ) &&
                            ( GPDMA_ERROR_TRANSFER == ( GPDMA_ERROR_TRANSFER & configStruct->ErrorMask ) )    )
                        {
                            status = Gpdma_Set_TransferErrorIrqActive( configStruct->PeriphId,
                                                                       configStruct->ChannelId );
                        }
                        else
                        {
                            /* Error during initialization process */
                        }

                        if( ( GPDMA_REQUEST_OK          == status                                                  ) &&
                            ( GPDMA_ERROR_CONFIG_UPDATE == ( GPDMA_ERROR_CONFIG_UPDATE & configStruct->ErrorMask ) )    )
                        {
                            status = Gpdma_Set_ConfigUpdateErrorIrqActive( configStruct->PeriphId,
                                                                           configStruct->ChannelId );
                        }
                        else
                        {
                            /* Error during initialization process */
                        }

                        if( ( GPDMA_REQUEST_OK         == status                                                 ) &&
                            ( GPDMA_ERROR_CONFIG_ERROR == ( GPDMA_ERROR_CONFIG_ERROR & configStruct->ErrorMask ) )    )
                        {
                            status = Gpdma_Set_ConfigErrorIrqActive( configStruct->PeriphId,
                                                                     configStruct->ChannelId );
                        }
                        else
                        {
                            /* Error during initialization process */
                        }

                        if( ( GPDMA_REQUEST_OK         == status                                                 ) &&
                            ( GPDMA_ERROR_TRIG_OVERRUN == ( GPDMA_ERROR_TRIG_OVERRUN & configStruct->ErrorMask ) )    )
                        {
                            status = Gpdma_Set_TriggerOverrunIrqActive( configStruct->PeriphId,
                                                                        configStruct->ChannelId );
                        }
                        else
                        {
                            /* Error during initialization process */
                        }

                        if( ( GPDMA_REQUEST_OK      == status                                              ) &&
                            ( GPDMA_ERROR_SUSPENDED == ( GPDMA_ERROR_SUSPENDED & configStruct->ErrorMask ) )    )
                        {
                            status = Gpdma_Set_SuspensionIrqActive( configStruct->PeriphId,
                                                                    configStruct->ChannelId );
                        }
                        else
                        {
                            /* Error during initialization process */
                        }
                    }
                }
                else
                {
                    /* Error during initialization process */
                }


                /*--------- Configure first transfer into registers. ---------*/
                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_TriggerType( configStruct->PeriphId,
                                                    configStruct->ChannelId,
                                                    configStruct->TransferConfig[ 0u ].TriggerType );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_TriggerSource( configStruct->PeriphId,
                                                      configStruct->ChannelId,
                                                      configStruct->TransferConfig[ 0u ].TriggerSource );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_TriggerMode( configStruct->PeriphId,
                                                    configStruct->ChannelId,
                                                    configStruct->TransferConfig[ 0u ].TriggerMode );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_RequestSource( configStruct->PeriphId,
                                                      configStruct->ChannelId,
                                                      configStruct->TransferConfig[ 0u ].RequestSource );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_SourceAddr( configStruct->PeriphId,
                                                   configStruct->ChannelId,
                                                   configStruct->TransferConfig[ 0u ].SourceAddr );
                }
                else
                {
                    /* Error during initialization process */
                }


                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_DestinationAddr( configStruct->PeriphId,
                                                        configStruct->ChannelId,
                                                        configStruct->TransferConfig[ 0u ].DestinationAddr );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_SourceAddrMode( configStruct->PeriphId,
                                                       configStruct->ChannelId,
                                                       configStruct->TransferConfig[ 0u ].SourceAddrMode );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_DestinationAddrMode( configStruct->PeriphId,
                                                            configStruct->ChannelId,
                                                            configStruct->TransferConfig[ 0u ].DestinationAddrMode );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_SourceDataSize( configStruct->PeriphId,
                                                       configStruct->ChannelId,
                                                       configStruct->TransferConfig[ 0u ].SourceDataSize );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_DestinationDataSize( configStruct->PeriphId,
                                                            configStruct->ChannelId,
                                                            configStruct->TransferConfig[ 0u ].DestinationDataSize );
                }
                else
                {

                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_BlockSize( configStruct->PeriphId,
                                                  configStruct->ChannelId,
                                                  configStruct->TransferConfig[ 0u ].BlockSize );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_BlockRepeatCount( configStruct->PeriphId,
                                                         configStruct->ChannelId,
                                                         configStruct->TransferConfig[ 0u ].BlockRepetitionCount );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_SourceBurstLength( configStruct->PeriphId,
                                                          configStruct->ChannelId,
                                                          configStruct->TransferConfig[ 0u ].SourceBurstLength );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_DestinationBurstLength( configStruct->PeriphId,
                                                               configStruct->ChannelId,
                                                               configStruct->TransferConfig[ 0u ].DestinationBurstLength );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_SourceDataOp( configStruct->PeriphId,
                                                     configStruct->ChannelId,
                                                     configStruct->TransferConfig[ 0u ].SourceDataOp );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_DestinationDataOp( configStruct->PeriphId,
                                                          configStruct->ChannelId,
                                                          configStruct->TransferConfig[ 0u ].DestinationDataOp );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_Direction( configStruct->PeriphId,
                                                  configStruct->ChannelId,
                                                  configStruct->TransferConfig[ 0u ].Direction );
                }
                else
                {
                    /* Error during initialization process */
                }

                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_ConfigurationPort( configStruct->PeriphId,
                                                          configStruct->ChannelId,
                                                          GPDMA_PORT_1 );
                }
                else
                {
                    /* Error during initialization process */
                }


                /*----------- Configure source and destination port ----------*/
                if( ( GPDMA_PORT_DEFAULT == configStruct->TransferConfig[ 0u ].SourcePortId      ) &&
                    ( GPDMA_PORT_DEFAULT == configStruct->TransferConfig[ 0u ].DestinationPortId )    )
                {
                    if( GPDMA_DIR_PERIPH_TO_MEMORY == configStruct->TransferConfig[ 0u ].Direction )
                    {
                        Gpdma_Set_SourcePort     ( configStruct->PeriphId, configStruct->ChannelId, GPDMA_PORT_0 );
                        Gpdma_Set_DestinationPort( configStruct->PeriphId, configStruct->ChannelId, GPDMA_PORT_1 );
                    }
                    else if( GPDMA_DIR_MEMORY_TO_PERIPH == configStruct->TransferConfig[ 0u ].Direction )
                    {
                        Gpdma_Set_SourcePort     ( configStruct->PeriphId, configStruct->ChannelId, GPDMA_PORT_1 );
                        Gpdma_Set_DestinationPort( configStruct->PeriphId, configStruct->ChannelId, GPDMA_PORT_0 );
                    }
                    else
                    {
                        Gpdma_Set_SourcePort     ( configStruct->PeriphId, configStruct->ChannelId, GPDMA_PORT_0 );
                        Gpdma_Set_DestinationPort( configStruct->PeriphId, configStruct->ChannelId, GPDMA_PORT_1 );
                    }
                }
                else
                {
                    Gpdma_Set_SourcePort     ( configStruct->PeriphId, configStruct->ChannelId, configStruct->TransferConfig[ 0u ].SourcePortId      );
                    Gpdma_Set_DestinationPort( configStruct->PeriphId, configStruct->ChannelId, configStruct->TransferConfig[ 0u ].DestinationPortId );
                }


                /*----------------- Configure transfer list ------------------*/
                if( GPDMA_REQUEST_OK == status )
                {
                    status = Gpdma_Set_XferListBaseAddr( configStruct->PeriphId,
                                                         configStruct->ChannelId,
                                                         (uint32_t)&configStruct->XferList[ 0u ] );
                }
                else
                {
                    /* Error during initialization process */
                }

                /*---- Generate Transfer List from required configuration ----*/
                if( GPDMA_REQUEST_OK == status )
                {
                    gpdma_ChannelType_t channelType = gpdma_PeriphConf[ configStruct->PeriphId ].ChannelsConfig[ configStruct->ChannelId ].ChannelTypeSupport;

                    status = Gpdma_Get_XferListConfig( configStruct->TransferConfig,
                                                       configStruct->TransfersCount,
                                                       channelType,
                                                       configStruct->XferList );

                    if( GPDMA_REQUEST_OK == status )
                    {
                        gpdma_PeriphConf[ configStruct->PeriphId ].TransferRuntime[ configStruct->ChannelId ].FirstXferListAddr = (uint32_t)&configStruct->XferList[ 0u ];
                        gpdma_PeriphConf[ configStruct->PeriphId ].TransferRuntime[ configStruct->ChannelId ].LastXferListAddr  = (uint32_t)&configStruct->XferList[ configStruct->TransfersCount - 1u ];
                        gpdma_PeriphConf[ configStruct->PeriphId ].TransferRuntime[ configStruct->ChannelId ].XferCount         = configStruct->TransfersCount;
                        gpdma_PeriphConf[ configStruct->PeriphId ].TransferRuntime[ configStruct->ChannelId ].TransferLockState = configStruct->TransferLockState;
                    }
                    else
                    {
                        /* Configuration failed */
                    }
                }
                else
                {
                    /* Error during initialization process */
                }
            }
            else
            {
                const gpdma_XferListLock_t lockState = gpdma_PeriphConf[ configStruct->PeriphId ].TransferRuntime[ configStruct->ChannelId ].TransferLockState;

                if( ( GPDMA_TRANSFER_LIST_ACCESS_SINGLE == configStruct->XferListAccessMode ) ||
                    ( GPDMA_TRANSFER_LIST_LOCKED        == lockState                        )    )
                {
                    /* Channel is already initialized, cannot re-initialize it */
                    status = GPDMA_REQUEST_ERROR;
                }
                else
                {
                    /* The transfer configurations already exist on current DMA
                     * channel. Check required and actual DMA configuration */

                    /* Check priority and transfer execution mode */
                    gpdma_Priority_t channelPrio = GPDMA_PRIORITY_CNT;

                    gpdma_XferExecMode_t xferExecMode = GPDMA_XFER_EXEC_MODES_CNT;

                    if( GPDMA_REQUEST_OK == status )
                    {
                        status = Gpdma_Get_Priority( configStruct->PeriphId,
                                                     configStruct->ChannelId,
                                                     &channelPrio );
                    }
                    else
                    {
                        /* Error during configuration */
                    }

                    if( GPDMA_REQUEST_OK == status )
                    {
                        status = Gpdma_Get_XferExecMode( configStruct->PeriphId,
                                                         configStruct->ChannelId,
                                                         &xferExecMode );
                    }
                    else
                    {
                        /* Error during configuration */
                    }

                    if( ( channelPrio      == configStruct->ChannelPrio      ) &&
                        ( xferExecMode     == configStruct->TransferExecMode ) &&
                        ( GPDMA_REQUEST_OK == status                         )    )
                    {
                        gpdma_ChannelType_t channelType = gpdma_PeriphConf[ configStruct->PeriphId ].ChannelsConfig[ configStruct->ChannelId ].ChannelTypeSupport;

                        if( GPDMA_REQUEST_OK == status )
                        {
                            /* Generate Xfer configuration list */
                            status = Gpdma_Get_XferListConfig( configStruct->TransferConfig,
                                                               configStruct->TransfersCount,
                                                               channelType,
                                                               configStruct->XferList );
                        }
                        else
                        {
                            /* Error during configuration */
                        }

                        if( GPDMA_REQUEST_OK == status )
                        {
                            /* Chain current Xfer configuration list to the existing Xfer list */
                            status = Gpdma_Set_XferList_NextXferAddr( &configStruct->XferList[ configStruct->TransfersCount - 1u ],
                                                                      channelType,
                                                                      gpdma_PeriphConf[ configStruct->PeriphId ].TransferRuntime[ configStruct->ChannelId ].FirstXferListAddr );
                        }
                        else
                        {
                            /* Error during configuration */
                        }
                    }
                    else
                    {
                        status = GPDMA_REQUEST_ERROR;
                    }

                    if( GPDMA_REQUEST_OK == status )
                    {
                        /* Store address of last Xfer list */
                        gpdma_PeriphConf[ configStruct->PeriphId ].TransferRuntime[ configStruct->ChannelId ].LastXferListAddr = (gpdma_DataAddr_t)&configStruct->XferList[ configStruct->TransfersCount - 1u ];

                        /* Increment count of Xfers */
                        gpdma_PeriphConf[ configStruct->PeriphId ].TransferRuntime[ configStruct->ChannelId ].XferCount += configStruct->TransfersCount;

                        /* Update lock state */
                        gpdma_PeriphConf[ configStruct->PeriphId ].TransferRuntime[ configStruct->ChannelId ].TransferLockState = configStruct->TransferLockState;
                    }
                    else
                    {
                        /* Configuration failed */
                    }
                }
            }
        }
        else
        {
            /* Incorrect configuration required. */
            status = GPDMA_REQUEST_ERROR;
        }
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief De-initializes module GPDMA
 *
 * This function shall call every necessary sub-module de-initialization function
 * and free all the resources allocated by the module.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Deinit( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        gpdma_PeriphConf[ periphId ].TransferRuntime[ channelId ].TransferLockState = GPDMA_TRANSFER_LIST_UNLOCKED;
        gpdma_PeriphConf[ periphId ].TransferRuntime[ channelId ].XferCount    = 0u;
        gpdma_PeriphConf[ periphId ].TransferRuntime[ channelId ].LastXferListAddr  = 0u;

        rcc_FunctionState_t rccState = RCC_FUNCTION_INACTIVE;

        rcc_RequestState_t rccRetState = Rcc_Get_PeriphState( gpdma_PeriphConf[ periphId ].RccPeriphId, &rccState );

        if( RCC_REQUEST_ERROR == rccRetState )
        {
            /* Peripheral clock activation failed. */
            status = GPDMA_REQUEST_ERROR;
        }
        else
        {
            if( RCC_FUNCTION_ACTIVE == rccState )
            {
                rccRetState = Rcc_Set_PeriphInactive( gpdma_PeriphConf[ periphId ].RccPeriphId );

                if( RCC_REQUEST_ERROR == rccRetState )
                {
                    /* Peripheral clock activation failed. */
                    status = GPDMA_REQUEST_ERROR;
                }
                else
                {
                    /* Peripheral clock de-activation successful. */
                    status = GPDMA_REQUEST_OK;
                }
            }
            else
            {
                /* Peripheral clock is already inactive. */
                status = GPDMA_REQUEST_OK;
            }
        }
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Main task of module GPDMA
 *
 * This function shall be called in the main loop of the application or the task
 * scheduler. It shall be called periodically, depending on the module's 
 * requirements.
 */
void Gpdma_Task( void )
{
    return;
}

/*------------------------- Channel state functionality ----------------------*/

/**
 * \brief Set DMA channel active
 *
 * \param periphId   [in]: The DMA bus identifier.
 * \param channelId  [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_ChannelActive( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        /* Enable the DMA channel */
        LL_DMA_EnableChannel( gpdma_PeriphConf[periphId].DmaReg,
                              gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Set DMA channel inactive
 *
 * \param periphId   [in]: The DMA bus identifier.
 * \param channelId  [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_ChannelInactive( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        /* Disable the DMA channel */
        LL_DMA_DisableChannel( gpdma_PeriphConf[periphId].DmaReg,
                               gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Get DMA channel state (active/inactive)
 *
 * \param periphId      [in]: The DMA bus identifier.
 * \param channelId     [in]: The DMA channel identifier.
 * \param channelState [out]: Pointer to store the current state.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_ChannelState( gpdma_PeriphId_t periphId,
                                             gpdma_ChannelId_t channelId,
                                             gpdma_FunctionState_t * const channelState )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId     ) &&
        ( GPDMA_CHANNEL_CNT > channelId    ) &&
        ( GPDMA_NULL_PTR   != channelState )    )
    {
        uint32_t regValue = LL_DMA_IsEnabledChannel( gpdma_PeriphConf[periphId].DmaReg,
                                                     gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        /* Check if the DMA channel is enabled */
        if( 0u != regValue )
        {
            *channelState = GPDMA_FUNCTION_ACTIVE;
        }
        else
        {
            *channelState = GPDMA_FUNCTION_INACTIVE;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Set DMA channel pause active
 *
 * \param periphId  [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_PauseActive( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        /* Suspend the DMA channel */
        LL_DMA_SuspendChannel( gpdma_PeriphConf[ periphId ].DmaReg,
                               gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Set DMA channel pause inactive
 *
 * \param periphId  [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_PauseInactive( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        /* Resume the DMA channel */
        LL_DMA_ResumeChannel( gpdma_PeriphConf[ periphId ].DmaReg,
                              gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Get DMA channel pause state (active/inactive)
 *
 * \param periphId      [in]: The DMA bus identifier.
 * \param channelId     [in]: The DMA channel identifier.
 * \param channelState [out]: Pointer to store the current pause state.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_PauseState( gpdma_PeriphId_t periphId,
                                           gpdma_ChannelId_t channelId,
                                           gpdma_FunctionState_t * const channelState )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT   > periphId     ) &&
         ( GPDMA_CHANNEL_CNT > channelId    ) &&
         ( GPDMA_NULL_PTR   != channelState )    )
    {
        uint32_t regValue = LL_DMA_IsSuspendedChannel( gpdma_PeriphConf[ periphId ].DmaReg,
                                                       gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        /* Check if the DMA channel is enabled */
        if( 0u != regValue )
        {
            *channelState = GPDMA_FUNCTION_ACTIVE;
        }
        else
        {
            *channelState = GPDMA_FUNCTION_INACTIVE;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/*--------------- Basic peripheral configuration functionality ---------------*/

/**
 * \brief Sets the channel priority for the DMA transfer.
 *
 * \note This can be used only if channel is NOT active.
 *
 * \param periphId    [in]: The DMA bus identifier.
 * \param channelId   [in]: The DMA channel identifier.
 * \param channelPrio [in]: Priority level.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_Priority( gpdma_PeriphId_t periphId,
                                         gpdma_ChannelId_t channelId,
                                         gpdma_Priority_t channelPrio )
{
    gpdma_RequestState_t  status       = GPDMA_REQUEST_ERROR;
    gpdma_FunctionState_t channelState = GPDMA_FUNCTION_INACTIVE;

    status = Gpdma_Get_ChannelState( periphId, channelId, &channelState );

    if( ( GPDMA_PERIPH_CNT         > periphId     ) &&
        ( GPDMA_CHANNEL_CNT        > channelId    ) &&
        ( GPDMA_PRIORITY_CNT       > channelPrio  ) &&
        ( GPDMA_REQUEST_ERROR     != status       ) &&
        ( GPDMA_FUNCTION_INACTIVE == channelState )    )
    {
        uint32_t regValue = 0u;

        if( GPDMA_PRIORITY_LOW == channelPrio )
        {
            regValue = LL_DMA_LOW_PRIORITY_LOW_WEIGHT;
        }
        else if (GPDMA_PRIORITY_MEDIUM == channelPrio)
        {
            regValue = LL_DMA_LOW_PRIORITY_MID_WEIGHT;
        }
        else if( GPDMA_PRIORITY_HIGH == channelPrio )
        {
            regValue = LL_DMA_LOW_PRIORITY_HIGH_WEIGHT;
        }
        else
        {
            regValue = LL_DMA_HIGH_PRIORITY;
        }

        LL_DMA_SetChannelPriorityLevel( gpdma_PeriphConf[ periphId ].DmaReg,
                                        gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                                        regValue );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the channel priority for the DMA transfer.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param channelPrio [out]: Pointer to priority level.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_Priority( gpdma_PeriphId_t periphId,
                                         gpdma_ChannelId_t channelId,
                                         gpdma_Priority_t * const channelPrio )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId    ) &&
        ( GPDMA_CHANNEL_CNT > channelId   ) &&
        ( GPDMA_NULL_PTR   != channelPrio )    )
    {
        uint32_t regValue = LL_DMA_GetChannelPriorityLevel( gpdma_PeriphConf[ periphId ].DmaReg,
                                                            gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if( LL_DMA_LOW_PRIORITY_LOW_WEIGHT == regValue )
        {
            *channelPrio = GPDMA_PRIORITY_LOW;
        }
        else if( LL_DMA_LOW_PRIORITY_MID_WEIGHT == regValue)
        {
            *channelPrio = GPDMA_PRIORITY_MEDIUM;
        }
        else if( LL_DMA_LOW_PRIORITY_HIGH_WEIGHT == regValue )
        {
            *channelPrio = GPDMA_PRIORITY_HIGH;
        }
        else
        {
            *channelPrio = GPDMA_PRIORITY_VERYHIGH;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the configuration list execution step mode.
 *
 * \note This can be used only if channel is NOT active.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param xferExecMode [in]: List execution step mode.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_XferExecMode( gpdma_PeriphId_t periphId,
                                             gpdma_ChannelId_t channelId,
                                             gpdma_XferExecMode_t xferExecMode )
{
    gpdma_RequestState_t  status       = GPDMA_REQUEST_ERROR;
    gpdma_FunctionState_t channelState = GPDMA_FUNCTION_INACTIVE;

    status = Gpdma_Get_ChannelState( periphId, channelId, &channelState );

    if( ( GPDMA_PERIPH_CNT           > periphId     ) &&
        ( GPDMA_CHANNEL_CNT          > channelId    ) &&
        ( GPDMA_XFER_EXEC_MODES_CNT > xferExecMode  ) &&
        ( GPDMA_REQUEST_ERROR       != status       ) &&
        ( GPDMA_FUNCTION_INACTIVE   == channelState )    )
    {
        uint32_t regValue = 0u;

        if( GPDMA_XFER_EXEC_BLOCKING == xferExecMode )
        {
            regValue = LL_DMA_LSM_1LINK_EXECUTION;
        }
        else
        {
            regValue = LL_DMA_LSM_FULL_EXECUTION;
        }

        LL_DMA_SetLinkStepMode( gpdma_PeriphConf[ periphId ].DmaReg,
                                gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                                regValue );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the configuration list execution step mode.
 *
 * \param periphId      [in]: The DMA bus identifier.
 * \param channelId     [in]: The DMA channel identifier.
 * \param xferExecMode [out]: List execution step mode.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_XferExecMode( gpdma_PeriphId_t periphId,
                                             gpdma_ChannelId_t channelId,
                                             gpdma_XferExecMode_t * const xferExecMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId     ) &&
        ( GPDMA_CHANNEL_CNT > channelId    ) &&
        ( GPDMA_NULL_PTR   != xferExecMode )    )
    {
        uint32_t regValue = LL_DMA_GetLinkStepMode( gpdma_PeriphConf[ periphId ].DmaReg,
                                                    gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if( LL_DMA_LSM_1LINK_EXECUTION == regValue )
        {
            *xferExecMode = GPDMA_XFER_EXEC_BLOCKING;
        }
        else
        {
            *xferExecMode = GPDMA_XFER_EXEC_CONTINUOUS;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Configures required port for source data connection.
 *
 * \param periphId   [in]: The DMA bus identifier.
 * \param channelId  [in]: The DMA channel identifier.
 * \param sourcePort [in]: Source port identifier. Can be one of the following:
 * - \ref GPDMA_PORT_0 : Source port 0.
 * - \ref GPDMA_PORT_1 : Source port 1.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *        otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_SourcePort( gpdma_PeriphId_t periphId,
                                           gpdma_ChannelId_t channelId,
                                           gpdma_PortId_t sourcePort )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT   > periphId   ) &&
        ( GPDMA_CHANNEL_CNT  > channelId  ) &&
        ( GPDMA_PORT_DEFAULT > sourcePort )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_PORT_0 == sourcePort )
        {
            regVal = LL_DMA_SRC_ALLOCATED_PORT0;
        }
        else
        {
            regVal = LL_DMA_SRC_ALLOCATED_PORT1;
        }

        LL_DMA_SetSrcAllocatedPort( gpdma_PeriphConf[periphId].DmaReg,
                                    gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                    regVal );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Retrieves the port for source data connection.
 *
 * \param periphId    [in]: The DMA bus identifier.
 * \param channelId   [in]: The DMA channel identifier.
 * \param sourcePort [out]: Source port identifier pointer.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *        otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_SourcePort( gpdma_PeriphId_t periphId,
                                           gpdma_ChannelId_t channelId,
                                           gpdma_PortId_t * const sourcePort )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId   ) &&
        ( GPDMA_CHANNEL_CNT > channelId  ) &&
        ( GPDMA_NULL_PTR   != sourcePort )    )
    {
        uint32_t regVal = 0u;

        regVal = LL_DMA_GetSrcAllocatedPort( gpdma_PeriphConf[periphId].DmaReg,
                                             gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        if( LL_DMA_DEST_ALLOCATED_PORT0 == regVal )
        {
            *sourcePort = GPDMA_PORT_0;
        }
        else
        {
            *sourcePort = GPDMA_PORT_1;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Configures required port for destination data connection.
 *
 * \param periphId  [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 * \param destPort  [in]: Destination port identifier. Can be one of the following:
 * - \ref GPDMA_PORT_0 : Destination port 0.
 * - \ref GPDMA_PORT_1 : Destination port 1.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *        otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_DestinationPort( gpdma_PeriphId_t periphId,
                                                gpdma_ChannelId_t channelId,
                                                gpdma_PortId_t destPort )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT   > periphId  ) &&
        ( GPDMA_CHANNEL_CNT  > channelId ) &&
        ( GPDMA_PORT_DEFAULT > destPort  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_PORT_0 == destPort )
        {
            regVal = LL_DMA_DEST_ALLOCATED_PORT0;
        }
        else
        {
            regVal = LL_DMA_DEST_ALLOCATED_PORT1;
        }

        LL_DMA_SetDestAllocatedPort( gpdma_PeriphConf[periphId].DmaReg,
                                     gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                     regVal );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Retrieves the port for destination data connection.
 *
 * \param periphId  [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 * \param destPort [out]: Destination port identifier pointer.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *        otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_DestinationPort( gpdma_PeriphId_t periphId,
                                                gpdma_ChannelId_t channelId,
                                                gpdma_PortId_t * const destPort )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId ) &&
        ( GPDMA_NULL_PTR   != destPort  )    )
    {
        uint32_t regVal = 0u;

        regVal = LL_DMA_GetDestAllocatedPort( gpdma_PeriphConf[periphId].DmaReg,
                                              gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        if( LL_DMA_DEST_ALLOCATED_PORT0 == regVal )
        {
            *destPort = GPDMA_PORT_0;
        }
        else
        {
            *destPort = GPDMA_PORT_1;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Configures required port for configuration data update connection.
 *
 * \param periphId   [in]: The DMA bus identifier.
 * \param channelId  [in]: The DMA channel identifier.
 * \param configPort [in]: Destination port identifier. Can be one of the following:
 * - \ref GPDMA_PORT_0 : Destination port 0.
 * - \ref GPDMA_PORT_1 : Destination port 1.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_ConfigurationPort( gpdma_PeriphId_t periphId,
                                                  gpdma_ChannelId_t channelId,
                                                  gpdma_PortId_t configPort )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT   > periphId   ) &&
        ( GPDMA_CHANNEL_CNT  > channelId  ) &&
        ( GPDMA_PORT_DEFAULT > configPort )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_PORT_0 == configPort )
        {
            regVal = LL_DMA_LINK_ALLOCATED_PORT0;
        }
        else
        {
            regVal = LL_DMA_LINK_ALLOCATED_PORT1;
        }

        LL_DMA_SetLinkAllocatedPort( gpdma_PeriphConf[periphId].DmaReg,
                                     gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                     regVal );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Retrieves the port for configuration data update connection.
 *
 * \param periphId    [in]: The DMA bus identifier.
 * \param channelId   [in]: The DMA channel identifier.
 * \param configPort [out]: Destination port identifier pointer.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_ConfigurationPort( gpdma_PeriphId_t periphId,
                                                  gpdma_ChannelId_t channelId,
                                                  gpdma_PortId_t * const configPort )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId ) &&
        ( GPDMA_NULL_PTR   != configPort  )    )
    {
        uint32_t regVal = 0u;

        regVal = LL_DMA_GetLinkAllocatedPort( gpdma_PeriphConf[periphId].DmaReg,
                                              gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        if( LL_DMA_LINK_ALLOCATED_PORT0 == regVal )
        {
            *configPort = GPDMA_PORT_0;
        }
        else
        {
            *configPort = GPDMA_PORT_1;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Configures base address of transfer list.
 *
 * \param periphId   [in]: The DMA bus identifier.
 * \param channelId  [in]: The DMA channel identifier.
 * \param baseAddr   [in]: Address of first transfer list.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_XferListBaseAddr( gpdma_PeriphId_t periphId,
                                                     gpdma_ChannelId_t channelId,
                                                     gpdma_DataAddr_t baseAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT   > periphId   ) &&
        ( GPDMA_CHANNEL_CNT  > channelId  )    )
    {
        LL_DMA_SetLinkedListBaseAddr( gpdma_PeriphConf[ periphId ].DmaReg,
                                      gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                                      baseAddr );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Returns base address of transfer list.
 *
 * \param periphId   [in]: The DMA bus identifier.
 * \param channelId  [in]: The DMA channel identifier.
 * \param baseAddr  [out]: Pointer to address of first transfer list.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_XferListBaseAddr( gpdma_PeriphId_t periphId,
                                                 gpdma_ChannelId_t channelId,
                                                 gpdma_DataAddr_t * const baseAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT   > periphId   ) &&
        ( GPDMA_CHANNEL_CNT  > channelId  ) &&
        ( GPDMA_NULL_PTR    != baseAddr   )    )
    {
        *baseAddr = LL_DMA_GetLinkedListBaseAddr( gpdma_PeriphConf[ periphId ].DmaReg,
                                                  gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the source transfer size.
 *
 * \param periphId               [in]: The DMA bus identifier.
 * \param channelId           [in]: The DMA channel identifier.
 * \param periphTransferSize   [in]: Source transfer data size.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_SourceDataSize( gpdma_PeriphId_t periphId,
                                               gpdma_ChannelId_t channelId,
                                               gpdma_DataSize_t srcDataSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT    > periphId    ) &&
        ( GPDMA_CHANNEL_CNT   > channelId   ) &&
        ( GPDMA_DATA_SIZE_CNT > srcDataSize )    )
    {
        uint32_t regValue = 0u;

        if( GPDMA_DATA_SIZE_8BITS == srcDataSize )
        {
            regValue = LL_DMA_SRC_DATAWIDTH_BYTE;
        }
        else if( GPDMA_DATA_SIZE_16BITS == srcDataSize )
        {
            regValue = LL_DMA_SRC_DATAWIDTH_HALFWORD;
        }
        else
        {
            regValue = LL_DMA_SRC_DATAWIDTH_WORD;
        }

        LL_DMA_SetSrcDataWidth( gpdma_PeriphConf[ periphId ].DmaReg,
                                gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                                regValue );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the source transfer size.
 *
 * \param periphId                [in]: The DMA bus identifier.
 * \param channelId            [in]: The DMA channel identifier.
 * \param periphTransferSize   [out]: Pointer to source transfer size.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_SourceDataSize( gpdma_PeriphId_t periphId,
                                               gpdma_ChannelId_t channelId,
                                               gpdma_DataSize_t * const srcDataSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId    ) &&
        ( GPDMA_CHANNEL_CNT > channelId   ) &&
        ( GPDMA_NULL_PTR   != srcDataSize )    )
    {
        uint32_t regValue = LL_DMA_GetSrcDataWidth( gpdma_PeriphConf[ periphId ].DmaReg,
                                                    gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if ( LL_DMA_SRC_DATAWIDTH_BYTE == regValue )
        {
            *srcDataSize = GPDMA_DATA_SIZE_8BITS;
        }
        else if ( LL_DMA_SRC_DATAWIDTH_HALFWORD == regValue)
        {
            *srcDataSize = GPDMA_DATA_SIZE_16BITS;
        }
        else
        {
            *srcDataSize = GPDMA_DATA_SIZE_32BITS;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the destination transfer size.
 *
 * \param periphId                [in]: The DMA bus identifier.
 * \param channelId            [in]: The DMA channel identifier.
 * \param memoryTransferSize    [in]: Destination transfer data size.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_DestinationDataSize( gpdma_PeriphId_t periphId,
                                                    gpdma_ChannelId_t channelId,
                                                    gpdma_DataSize_t destDataSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT    > periphId     ) &&
        ( GPDMA_CHANNEL_CNT   > channelId    ) &&
        ( GPDMA_DATA_SIZE_CNT > destDataSize )    )
    {
        uint32_t regValue = 0u;

        if( GPDMA_DATA_SIZE_8BITS == destDataSize )
        {
            regValue = LL_DMA_DEST_DATAWIDTH_BYTE;
        }
        else if( GPDMA_DATA_SIZE_16BITS == destDataSize )
        {
            regValue = LL_DMA_DEST_DATAWIDTH_HALFWORD;
        }
        else
        {
            regValue = LL_DMA_DEST_DATAWIDTH_WORD;
        }

        LL_DMA_SetDestDataWidth( gpdma_PeriphConf[ periphId ].DmaReg,
                                 gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                                 regValue );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the destination transfer size.
 *
 * \param periphId                 [in]: The DMA bus identifier.
 * \param channelId             [in]: The DMA channel identifier.
 * \param memoryTransferSize    [out]: Pointer to destination transfer size.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_DestinationDataSize( gpdma_PeriphId_t periphId,
                                                    gpdma_ChannelId_t channelId,
                                                    gpdma_DataSize_t * const destDataSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId     ) &&
        ( GPDMA_CHANNEL_CNT > channelId    ) &&
        ( GPDMA_NULL_PTR   != destDataSize )    )
    {
        uint32_t regValue = LL_DMA_GetDestDataWidth( gpdma_PeriphConf[ periphId ].DmaReg,
                                                     gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if ( LL_DMA_DEST_DATAWIDTH_BYTE == regValue )
        {
            *destDataSize = GPDMA_DATA_SIZE_8BITS;
        }
        else if ( LL_DMA_DEST_DATAWIDTH_HALFWORD == regValue)
        {
            *destDataSize = GPDMA_DATA_SIZE_16BITS;
        }
        else
        {
            *destDataSize = GPDMA_DATA_SIZE_32BITS;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the trigger type for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param triggerType  [in]: Trigger type. Can be one of the following values:
 *  - \ref GPDMA_TRG_NOT_USED : Trigger will not be used for transfer
 *  - \ref GPDMA_TRG_RISING   : Raising edge of trigger event will be used
 *  - \ref GPDMA_TRG_FALLING  : Falling edge of trigger event will be used
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_TriggerType ( gpdma_PeriphId_t periphId,
                                             gpdma_ChannelId_t channelId,
                                             gpdma_TrgType_t triggerType )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_TRG_TYPE_CNT > triggerType ) &&
        ( GPDMA_PERIPH_CNT   > periphId    ) &&
        ( GPDMA_CHANNEL_CNT  > channelId   )    )
    {
        uint32_t triggerTypeReg = 0u;

        if( GPDMA_TRG_RISING == triggerType )
        {
            triggerTypeReg = LL_DMA_TRIG_POLARITY_RISING;
        }
        else if( GPDMA_TRG_FALLING == triggerType )
        {
            triggerTypeReg = LL_DMA_TRIG_POLARITY_FALLING;
        }
        else
        {
            triggerTypeReg = LL_DMA_TRIG_POLARITY_MASKED;
        }

        LL_DMA_SetTriggerPolarity( gpdma_PeriphConf[periphId].DmaReg,
                                   gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                   triggerTypeReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the trigger type for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param triggerType [out]: Pointer to trigger type. Can be one of the following values:
 *  - \ref GPDMA_TRG_NOT_USED : Trigger will not be used for transfer
 *  - \ref GPDMA_TRG_RISING   : Raising edge of trigger event will be used
 *  - \ref GPDMA_TRG_FALLING  : Falling edge of trigger event will be used
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_TriggerType ( gpdma_PeriphId_t periphId,
                                             gpdma_ChannelId_t channelId,
                                             gpdma_TrgType_t * const triggerType )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR   != triggerType ) &&
        ( GPDMA_PERIPH_CNT  > periphId    ) &&
        ( GPDMA_CHANNEL_CNT > channelId   )    )
    {

        uint32_t regValue = LL_DMA_GetTriggerPolarity( gpdma_PeriphConf[periphId].DmaReg,
                                                       gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        if( LL_DMA_TRIG_POLARITY_RISING == regValue )
        {
            *triggerType = GPDMA_TRG_RISING;
        }
        else if ( LL_DMA_TRIG_POLARITY_FALLING == regValue )
        {
            *triggerType = GPDMA_TRG_FALLING;
        }
        else
        {
            *triggerType = GPDMA_TRG_NOT_USED;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the trigger source for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param triggerSrc   [in]: Trigger source identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_TriggerSource ( gpdma_PeriphId_t periphId,
                                               gpdma_ChannelId_t channelId,
                                               gpdma_TrgSrcId_t triggerSrc )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {

        LL_DMA_SetHWTrigger( gpdma_PeriphConf[periphId].DmaReg,
                             gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                             triggerSrc );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the trigger source for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param triggerSrc  [out]: Pointer to trigger source identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_TriggerSource ( gpdma_PeriphId_t periphId,
                                               gpdma_ChannelId_t channelId,
                                               gpdma_TrgSrcId_t * const triggerSrc )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId   ) &&
        ( GPDMA_CHANNEL_CNT > channelId  ) &&
        ( GPDMA_NULL_PTR   != triggerSrc )    )
    {

        *triggerSrc = LL_DMA_GetHWTrigger( gpdma_PeriphConf[periphId].DmaReg,
                                           gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Configures the system reaction to trigger signal.
 *
 * \param periphId    [in]: The DMA bus identifier.
 * \param channelId   [in]: The DMA channel identifier.
 * \param triggerMode [in]: Required trigger mode. Can be one of following:
 *  -\ref GPDMA_TRIGGER_BLOCK    : Trigger is needed for each block transfer.
 *                                 If 2D transfer is used, next row needs another
 *                                 trigger.
 *  -\ref GPDMA_TRIGGER_2D_BLOCK : Trigger is needed for each 2D block transfer.
 *                                 All columns and rows will be transfered after
 *                                 single trigger.
 *  -\ref GPDMA_TRIGGER_TRANSFER : Trigger is needed for each transfer change.
 *                                 This is available only if multiple linked
 *                                 transfers are used.
 *  -\ref GPDMA_TRIGGER_SINGLE   : Trigger is needed for each single transfer.
 *                                 Every transfer (whole burst count) from/to
 *                                 memory/peripheral need trigger for execution.
 *                                 This mode is like double condition for transfer
 *                                 request.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_TriggerMode( gpdma_PeriphId_t periphId,
                                            gpdma_ChannelId_t channelId,
                                            gpdma_TriggerMode_t triggerMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT       > periphId    ) &&
        ( GPDMA_CHANNEL_CNT      > channelId   )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_TRIGGER_BLOCK == triggerMode )
        {
            regVal = LL_DMA_TRIGM_BLK_TRANSFER;

            status = GPDMA_REQUEST_OK;
        }
        else if( GPDMA_TRIGGER_2D_BLOCK == triggerMode )
        {
            if( GPDMA_CHANNEL_LINEAR_2D == gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelTypeSupport )
            {
                regVal = LL_DMA_TRIGM_RPT_BLK_TRANSFER;

                status = GPDMA_REQUEST_OK;
            }
            else
            {
                status = GPDMA_REQUEST_ERROR;
            }
        }
        else if( GPDMA_TRIGGER_TRANSFER == triggerMode )
        {
            regVal = LL_DMA_TRIGM_LLI_LINK_TRANSFER;

            status = GPDMA_REQUEST_OK;
        }
        else if( GPDMA_TRIGGER_SINGLE == triggerMode )
        {
            regVal = LL_DMA_TRIGM_SINGLBURST_TRANSFER;

            status = GPDMA_REQUEST_OK;
        }
        else
        {
            regVal = LL_DMA_TRIGM_BLK_TRANSFER;

            status = GPDMA_REQUEST_OK;
        }

        if( GPDMA_REQUEST_OK == status )
        {
            LL_DMA_SetTriggerMode( gpdma_PeriphConf[periphId].DmaReg,
                                   gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                   regVal );
        }
        else
        {
            /* Incorrect trigger mode selected */
        }
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Returns the system reaction to trigger signal.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param triggerMode [out]: Required trigger mode. Can be one of following:
 *  -\ref GPDMA_TRIGGER_BLOCK    : Trigger is needed for each block transfer.
 *                                 If 2D transfer is used, next row needs another
 *                                 trigger.
 *  -\ref GPDMA_TRIGGER_2D_BLOCK : Trigger is needed for each 2D block transfer.
 *                                 All columns and rows will be transfered after
 *                                 single trigger.
 *  -\ref GPDMA_TRIGGER_TRANSFER : Trigger is needed for each transfer change.
 *                                 This is available only if multiple linked
 *                                 transfers are used.
 *  -\ref GPDMA_TRIGGER_SINGLE   : Trigger is needed for each single transfer.
 *                                 Every transfer (whole burst count) from/to
 *                                 memory/peripheral need trigger for execution.
 *                                 This mode is like double condition for transfer
 *                                 request.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_TriggerMode( gpdma_PeriphId_t periphId,
                                            gpdma_ChannelId_t channelId,
                                            gpdma_TriggerMode_t * const triggerMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId    ) &&
        ( GPDMA_CHANNEL_CNT > channelId   ) &&
        ( GPDMA_NULL_PTR   != triggerMode )    )
    {
        uint32_t regVal = LL_DMA_GetTriggerMode( gpdma_PeriphConf[periphId].DmaReg,
                                                 gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        if( LL_DMA_TRIGM_BLK_TRANSFER == regVal )
        {
            *triggerMode = GPDMA_TRIGGER_BLOCK;

            status = GPDMA_REQUEST_OK;
        }
        else if( LL_DMA_TRIGM_RPT_BLK_TRANSFER == regVal )
        {
            *triggerMode = GPDMA_TRIGGER_2D_BLOCK;

            status = GPDMA_REQUEST_OK;
        }
        else if( LL_DMA_TRIGM_LLI_LINK_TRANSFER == regVal )
        {
            *triggerMode = GPDMA_TRIGGER_TRANSFER;

            status = GPDMA_REQUEST_OK;
        }
        else
        {
            *triggerMode = GPDMA_TRIGGER_SINGLE;

            status = GPDMA_REQUEST_OK;
        }
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the request source for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param requestSrc   [in]: Peripheral request source identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_RequestSource ( gpdma_PeriphId_t periphId,
                                               gpdma_ChannelId_t channelId,
                                               gpdma_PeriphReqId_t requestSrc )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_SetPeriphRequest( gpdma_PeriphConf[periphId].DmaReg,
                                 gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                 requestSrc );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the request source for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param requestSrc  [out]: Pointer to peripheral request source identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_RequestSource ( gpdma_PeriphId_t periphId,
                                                gpdma_ChannelId_t channelId,
                                                gpdma_PeriphReqId_t * const requestSrc )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId   ) &&
        ( GPDMA_CHANNEL_CNT > channelId  ) &&
        ( GPDMA_NULL_PTR   != requestSrc )    )
    {

        *requestSrc = LL_DMA_GetPeriphRequest( gpdma_PeriphConf[periphId].DmaReg,
                                               gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}

/*---------------------------- Transfer Direction --------------------------*/

/**
 * \brief Set direction of the DMA transfer
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 * \param direction  [in]: Transfer direction to configure.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_Direction( gpdma_PeriphId_t periphId,
                                          gpdma_ChannelId_t channelId,
                                          gpdma_Direction_t direction )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId ) &&
        ( GPDMA_DIR_CNT     > direction )    )
    {
        uint32_t regValue = 0u;

        if( GPDMA_DIR_PERIPH_TO_MEMORY == direction )
        {
            regValue = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
        }
        else if (GPDMA_DIR_MEMORY_TO_PERIPH == direction)
        {
            regValue = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
        }
        else
        {
            regValue = LL_DMA_DIRECTION_MEMORY_TO_MEMORY;
        }

        LL_DMA_SetDataTransferDirection( gpdma_PeriphConf[ periphId ].DmaReg,
                                         gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                                         regValue );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }


    return ( status );
}


/**
 * \brief Get direction of the DMA transfer
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 * \param direction [out]: Pointer to store the current transfer direction.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_Direction( gpdma_PeriphId_t periphId,
                                          gpdma_ChannelId_t channelId,
                                          gpdma_Direction_t * const direction )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId ) &&
        ( GPDMA_NULL_PTR   != direction )    )
    {
        uint32_t regValue = LL_DMA_GetDataTransferDirection( gpdma_PeriphConf[ periphId ].DmaReg,
                                                             gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if ( LL_DMA_DIRECTION_PERIPH_TO_MEMORY == regValue)
        {
            *direction = GPDMA_DIR_PERIPH_TO_MEMORY;
        }
        else if ( LL_DMA_DIRECTION_MEMORY_TO_PERIPH == regValue)
        {
            *direction = GPDMA_DIR_MEMORY_TO_PERIPH;
        }
        else
        {
            *direction = GPDMA_DIR_MEMORY_TO_MEMORY;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}

/*---------------------------- Address Configuration ------------------------*/

/**
 * \brief Set source address for DMA transfer
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 * \param periphAddr [in]: Source (peripheral) address.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_SourceAddr( gpdma_PeriphId_t periphId,
                                           gpdma_ChannelId_t channelId,
                                           gpdma_SrcAddr_t sourceAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_SetSrcAddress( gpdma_PeriphConf[ periphId ].DmaReg,
                              gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                              sourceAddr );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Get source address for DMA transfer
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 * \param periphAddr [out]: Pointer to store source address.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_SourceAddr( gpdma_PeriphId_t periphId,
                                           gpdma_ChannelId_t channelId,
                                           gpdma_SrcAddr_t * const sourceAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId   ) &&
        ( GPDMA_CHANNEL_CNT > channelId  ) &&
        ( GPDMA_NULL_PTR   != sourceAddr )    )
    {
        *sourceAddr = LL_DMA_GetSrcAddress( gpdma_PeriphConf[ periphId ].DmaReg,
                                            gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Set destination address for DMA transfer
 *
 * \param periphId      [in]: The DMA bus identifier.
 * \param channelId  [in]: The DMA channel identifier.
 * \param memoryAddr  [in]: Destination (memory) address.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_DestinationAddr( gpdma_PeriphId_t periphId,
                                                gpdma_ChannelId_t channelId,
                                                gpdma_DstAddr_t destAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_SetDestAddress( gpdma_PeriphConf[ periphId ].DmaReg,
                               gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                               destAddr );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Get destination address for DMA transfer
 *
 * \param periphId      [in]: The DMA bus identifier.
 * \param channelId  [in]: The DMA channel identifier.
 * \param memoryAddr [out]: Pointer to store destination address.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_DestinationAddr( gpdma_PeriphId_t periphId,
                                                gpdma_ChannelId_t channelId,
                                                gpdma_DstAddr_t * const destAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId ) &&
        ( GPDMA_NULL_PTR   != destAddr  )    )
    {
        *destAddr = LL_DMA_GetDestAddress( gpdma_PeriphConf[ periphId ].DmaReg,
                                           gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the address increment mode for the source.
 *
 * \param periphId         [in]: The DMA bus identifier.
 * \param channelId     [in]: The DMA channel identifier.
 * \param srcAddrMode    [in]: Address increment mode for the source.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_SourceAddrMode( gpdma_PeriphId_t periphId,
                                               gpdma_ChannelId_t channelId,
                                               gpdma_AddrMode_t srcAddrMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId    ) &&
        ( GPDMA_CHANNEL_CNT > channelId   ) &&
        ( GPDMA_ADDR_CNT    > srcAddrMode )    )
    {
        uint32_t regValue = 0u;

        if( GPDMA_ADDR_INCREMENT != srcAddrMode )
        {
            regValue = LL_DMA_SRC_FIXED;
        }
        else
        {
            regValue = LL_DMA_SRC_INCREMENT;
        }

        LL_DMA_SetSrcIncMode( gpdma_PeriphConf[ periphId ].DmaReg,
                              gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                              regValue );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the address increment mode for the source.
 *
 * \param periphId           [in]: The DMA bus identifier.
 * \param channelId       [in]: The DMA channel identifier.
 * \param srcAddrMode     [out]: Pointer to address increment mode.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_SourceAddrMode( gpdma_PeriphId_t periphId,
                                               gpdma_ChannelId_t channelId,
                                               gpdma_AddrMode_t * const srcAddrMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId    ) &&
        ( GPDMA_CHANNEL_CNT > channelId   ) &&
        ( GPDMA_NULL_PTR   != srcAddrMode )    )
    {
        uint32_t regValue = LL_DMA_GetSrcIncMode( gpdma_PeriphConf[ periphId ].DmaReg,
                                                  gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if ( LL_DMA_SRC_INCREMENT == regValue )
        {
            *srcAddrMode = GPDMA_ADDR_INCREMENT;
        }
        else
        {
            *srcAddrMode = GPDMA_ADDR_STATIC;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the address increment mode for the destination.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param destAddrMode [in]: Address increment mode for the destination:
 *  - \ref GPDMA_ADDR_INCREMENT : Address will be incremented
 *  - \ref GPDMA_ADDR_STATIC    : Address will not be changed
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_DestinationAddrMode( gpdma_PeriphId_t periphId,
                                                    gpdma_ChannelId_t channelId,
                                                    gpdma_AddrMode_t destAddrMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId       ) &&
        ( GPDMA_CHANNEL_CNT > channelId   ) &&
        ( GPDMA_ADDR_CNT    > destAddrMode )    )
    {
        uint32_t regValue = 0u;

        if( GPDMA_ADDR_INCREMENT != destAddrMode )
        {
            regValue = LL_DMA_DEST_FIXED;
        }
        else
        {
            regValue = LL_DMA_DEST_INCREMENT;
        }

        LL_DMA_SetDestIncMode( gpdma_PeriphConf[ periphId ].DmaReg,
                               gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                               regValue );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the address increment mode for the destination.
 *
 * \param periphId      [in]: The DMA bus identifier.
 * \param channelId     [in]: The DMA channel identifier.
 * \param destAddrMode [out]: Pointer to address increment mode:
 *  - \ref GPDMA_ADDR_INCREMENT : Address will be incremented
 *  - \ref GPDMA_ADDR_STATIC    : Address will not be changed
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_DestinationAddrMode( gpdma_PeriphId_t periphId,
                                                    gpdma_ChannelId_t channelId,
                                                    gpdma_AddrMode_t * const destAddrMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId       ) &&
        ( GPDMA_CHANNEL_CNT > channelId   ) &&
        ( GPDMA_NULL_PTR   != destAddrMode )    )
    {
        uint32_t regValue = LL_DMA_GetDestIncMode( gpdma_PeriphConf[ periphId ].DmaReg,
                                                   gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if ( LL_DMA_DEST_INCREMENT == regValue )
        {
            *destAddrMode = GPDMA_ADDR_INCREMENT;
        }
        else
        {
            *destAddrMode = GPDMA_ADDR_STATIC;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Configures length of burst transfers for the source.
 *
 * The source and destination data transfer can wary. The count of this data's
 * (size specified by \ref gpdma_DataSize_t) transfered from/into GPDMA channel's
 * FIFO can be configured with regards of application. This can be useful for
 * different speed of source and destination.
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
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param srcBurstLen  [in]: Source burst length to configure.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_SourceBurstLength( gpdma_PeriphId_t periphId,
                                                  gpdma_ChannelId_t channelId,
                                                  gpdma_BurstLength_t srcBurstLen )
{
    gpdma_RequestState_t status   = GPDMA_REQUEST_ERROR;
    gpdma_BurstLength_t  burstLen = 0u;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        if( ( GPDMA_MAX_BURST_LEN >= srcBurstLen ) &&
            ( GPDMA_MIN_BURST_LEN <= srcBurstLen )    )
        {
            burstLen = srcBurstLen - 1u;
        }
        else
        {
            burstLen = 0u;
        }

        LL_DMA_SetSrcBurstLength( gpdma_PeriphConf[ periphId ].DmaReg,
                                  gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                                  burstLen );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Returns length of burst transfers for the source.
 *
 * The source and destination data transfer can wary. The count of this data's
 * (size specified by \ref gpdma_DataSize_t) transfered from/into GPDMA channel's
 * FIFO can be configured with regards of application. This can be useful for
 * different speed of source and destination.
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
 * \param periphId      [in]: The DMA bus identifier.
 * \param channelId     [in]: The DMA channel identifier.
 * \param srcBurstLen  [out]: Configured source burst length.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_SourceBurstLength( gpdma_PeriphId_t periphId,
                                                  gpdma_ChannelId_t channelId,
                                                  gpdma_BurstLength_t * const srcBurstLen )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId    ) &&
        ( GPDMA_CHANNEL_CNT > channelId   ) &&
        ( GPDMA_NULL_PTR   != srcBurstLen )    )
    {
        uint32_t regVal = LL_DMA_GetSrcBurstLength( gpdma_PeriphConf[ periphId ].DmaReg,
                                                    gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        *srcBurstLen = regVal + 1u;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Configures length of burst transfers for the destination.
 *
 * The source and destination data transfer can wary. The count of this data's
 * (size specified by \ref gpdma_DataSize_t) transfered from/into GPDMA channel's
 * FIFO can be configured with regards of application. This can be useful for
 * different speed of source and destination.
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
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param destBurstLen [in]: Destination burst length to configure.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_DestinationBurstLength( gpdma_PeriphId_t periphId,
                                                       gpdma_ChannelId_t channelId,
                                                       gpdma_BurstLength_t destBurstLen )
{
    gpdma_RequestState_t status   = GPDMA_REQUEST_ERROR;
    gpdma_BurstLength_t  burstLen = 0u;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        if( ( GPDMA_MAX_BURST_LEN >= destBurstLen ) &&
            ( GPDMA_MIN_BURST_LEN <= destBurstLen )    )
        {
            burstLen = destBurstLen - 1u;
        }
        else
        {
            burstLen = 0u;
        }

        LL_DMA_SetDestBurstLength( gpdma_PeriphConf[ periphId ].DmaReg,
                                   gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                                   burstLen );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Returns length of burst transfers for the destination.
 *
 * The source and destination data transfer can wary. The count of this data's
 * (size specified by \ref gpdma_DataSize_t) transfered from/into GPDMA channel's
 * FIFO can be configured with regards of application. This can be useful for
 * different speed of source and destination.
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
 * \param periphId      [in]: The DMA bus identifier.
 * \param channelId     [in]: The DMA channel identifier.
 * \param destBurstLen [out]: Configured destination burst length.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_DestinationBurstLength( gpdma_PeriphId_t periphId,
                                                       gpdma_ChannelId_t channelId,
                                                       gpdma_BurstLength_t * const destBurstLen )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId    ) &&
        ( GPDMA_CHANNEL_CNT > channelId   ) &&
        ( GPDMA_NULL_PTR   != destBurstLen )    )
    {
        uint32_t regVal = LL_DMA_GetDestBurstLength( gpdma_PeriphConf[ periphId ].DmaReg,
                                                     gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        *destBurstLen = regVal + 1u;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}



/**
 * \brief Sets the data count for the DMA transfer.
 *
 * \note This can be used only if channel is NOT active.
 *
 * \param periphId  [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 * \param dataCount [in]: Number of data items to transfer. Max value is 0xFFFF (65535).
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_BlockSize( gpdma_PeriphId_t periphId,
                                          gpdma_ChannelId_t channelId,
                                          gpdma_BlockSize_t blockSize )
{
    gpdma_RequestState_t  status       = GPDMA_REQUEST_ERROR;
    gpdma_FunctionState_t channelState = GPDMA_FUNCTION_INACTIVE;

    status = Gpdma_Get_ChannelState( periphId, channelId, &channelState );

    if( ( GPDMA_PERIPH_CNT         > periphId     ) &&
        ( GPDMA_CHANNEL_CNT        > channelId    ) &&
        ( GPDMA_REQUEST_ERROR     != status       ) &&
        ( GPDMA_FUNCTION_INACTIVE == channelState )    )
    {
        LL_DMA_SetBlkDataLength( gpdma_PeriphConf[ periphId ].DmaReg,
                                 gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                                 blockSize );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the data count for the DMA transfer.
 *
 * \param periphId   [in]: The DMA bus identifier.
 * \param channelId  [in]: The DMA channel identifier.
 * \param dataCount [out]: Pointer to number of data items to transfer.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_BlockSize( gpdma_PeriphId_t periphId,
                                          gpdma_ChannelId_t channelId,
                                          gpdma_BlockSize_t * const blockSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId ) &&
        ( GPDMA_NULL_PTR   != blockSize )    )
    {
        uint32_t regVal = LL_DMA_GetBlkDataLength( gpdma_PeriphConf[ periphId ].DmaReg,
                                                   gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        *blockSize = (gpdma_BlockSize_t)regVal;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the repetition count of data blocks for the DMA transfer (only for 2D transfers).
 *
 * \note This can be used only if channel is NOT active.
 * \note This option is available only for 2D transfers.
 *
 * \param periphId    [in]: The DMA bus identifier.
 * \param channelId   [in]: The DMA channel identifier.
 * \param blockRepCnt [in]: Count of block transfers repetitions. Max value is 0x7FF (2047).
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_BlockRepeatCount( gpdma_PeriphId_t periphId,
                                                gpdma_ChannelId_t channelId,
                                                gpdma_BlockRep_t blockRepCnt )
{
    gpdma_RequestState_t  status       = GPDMA_REQUEST_ERROR;
    gpdma_FunctionState_t channelState = GPDMA_FUNCTION_INACTIVE;

    status = Gpdma_Get_ChannelState( periphId, channelId, &channelState );
    gpdma_ChannelType_t availableTransferStyle = gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelTypeSupport;

    if( ( GPDMA_PERIPH_CNT         > periphId               ) &&
        ( GPDMA_CHANNEL_CNT        > channelId              ) &&
        ( GPDMA_MAX_REP_BLOCK_LEN >= blockRepCnt            ) &&
        ( GPDMA_REQUEST_ERROR     != status                 ) &&
        ( GPDMA_FUNCTION_INACTIVE == channelState           ) &&
        ( GPDMA_CHANNEL_LINEAR_2D == availableTransferStyle )    )
    {
        LL_DMA_SetBlkRptCount( gpdma_PeriphConf[ periphId ].DmaReg,
                               gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg,
                               blockRepCnt );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        if( 0u != blockRepCnt )
        {
            status = GPDMA_REQUEST_ERROR;
        }
        else
        {
            status = GPDMA_REQUEST_OK;
        }
    }

    return ( status );
}


/**
 * \brief Gets the block repetition counter for the DMA transfer.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param blockRepCnt [out]: Pointer to count of block transfers to be executed.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_BlockRepeatCount( gpdma_PeriphId_t periphId,
                                                gpdma_ChannelId_t channelId,
                                                gpdma_BlockRep_t * const blockRepCnt )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId    ) &&
        ( GPDMA_CHANNEL_CNT > channelId   ) &&
        ( GPDMA_NULL_PTR   != blockRepCnt )    )
    {
        uint32_t regVal = LL_DMA_GetBlkRptCount( gpdma_PeriphConf[ periphId ].DmaReg,
                                                 gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        *blockRepCnt = (gpdma_BlockRep_t)regVal;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the source data operation for the DMA transfer.
 *
 * \param periphId  [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 * \param srcDataOp [in]: Source data operation to configure.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_SourceDataOp( gpdma_PeriphId_t periphId,
                                             gpdma_ChannelId_t channelId,
                                             gpdma_SrcDataOp_t srcDataOp )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT      > periphId  ) &&
        ( GPDMA_CHANNEL_CNT     > channelId ) &&
        ( GPDMA_SRC_DATA_OP_CNT > srcDataOp )    )
    {
        if( GPDMA_SRC_DATA_PRESERVE == srcDataOp )
        {
            LL_DMA_SetSrcByteExchange( gpdma_PeriphConf[periphId].DmaReg,
                                       gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                       LL_DMA_SRC_BYTE_PRESERVE );
        }
        else
        {
            LL_DMA_SetSrcByteExchange( gpdma_PeriphConf[periphId].DmaReg,
                                       gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                       LL_DMA_SRC_BYTE_EXCHANGE );
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Returns source data operation settings for the DMA transfer.
 *
 * \param periphId  [in]: The DMA bus identifier.
 * \param channelId [in]: The DMA channel identifier.
 * \param srcDataOp [out]: Pointer to source data operation settings.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_SourceDataOp( gpdma_PeriphId_t periphId,
                                             gpdma_ChannelId_t channelId,
                                             gpdma_SrcDataOp_t * const srcDataOp )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId ) &&
        ( GPDMA_NULL_PTR   != srcDataOp )    )
    {
        uint32_t regVal = LL_DMA_GetSrcByteExchange( gpdma_PeriphConf[periphId].DmaReg,
                                                     gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        if( LL_DMA_SRC_BYTE_PRESERVE == regVal )
        {
            *srcDataOp = GPDMA_SRC_DATA_PRESERVE;
        }
        else
        {
            *srcDataOp = GPDMA_SRC_DATA_BYTE_SWAP;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the destination data operation for the DMA transfer.
 *
 * \param periphId   [in]: The DMA bus identifier.
 * \param channelId  [in]: The DMA channel identifier.
 * \param destDataOp [in]: Destination data operation to configure.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_DestinationDataOp( gpdma_PeriphId_t periphId,
                                                  gpdma_ChannelId_t channelId,
                                                  gpdma_DestDataOp_t destDataOp )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT       > periphId   ) &&
        ( GPDMA_CHANNEL_CNT      > channelId  ) &&
        ( GPDMA_DEST_DATA_OP_CNT > destDataOp )    )
    {
        if( GPDMA_DEST_DATA_PRESERVE == destDataOp )
        {
            LL_DMA_SetDestByteExchange( gpdma_PeriphConf[periphId].DmaReg,
                                        gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                        LL_DMA_DEST_BYTE_PRESERVE );

            LL_DMA_SetDestHWordExchange( gpdma_PeriphConf[periphId].DmaReg,
                                         gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                         LL_DMA_DEST_HALFWORD_PRESERVE );
        }
        else if( GPDMA_DEST_DATA_2BYTES_SWAP == destDataOp )
        {
            LL_DMA_SetDestByteExchange( gpdma_PeriphConf[periphId].DmaReg,
                                        gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                        LL_DMA_DEST_BYTE_PRESERVE );

            LL_DMA_SetDestHWordExchange( gpdma_PeriphConf[periphId].DmaReg,
                                         gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                         LL_DMA_DEST_HALFWORD_EXCHANGE );
        }
        else if( GPDMA_DEST_DATA_BYTE_SWAP == destDataOp )
        {
            LL_DMA_SetDestByteExchange( gpdma_PeriphConf[periphId].DmaReg,
                                        gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                        LL_DMA_DEST_BYTE_EXCHANGE );

            LL_DMA_SetDestHWordExchange( gpdma_PeriphConf[periphId].DmaReg,
                                         gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                         LL_DMA_DEST_HALFWORD_PRESERVE );
        }
        else
        {
            LL_DMA_SetDestByteExchange( gpdma_PeriphConf[periphId].DmaReg,
                                        gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                        LL_DMA_DEST_BYTE_EXCHANGE );

            LL_DMA_SetDestHWordExchange( gpdma_PeriphConf[periphId].DmaReg,
                                         gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg,
                                         LL_DMA_DEST_HALFWORD_EXCHANGE );
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Returns destination data operation settings for the DMA transfer.
 *
 * \param periphId    [in]: The DMA bus identifier.
 * \param channelId   [in]: The DMA channel identifier.
 * \param destDataOp [out]: Pointer to destination data operation settings.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_DestinationDataOp( gpdma_PeriphId_t periphId,
                                                  gpdma_ChannelId_t channelId,
                                                  gpdma_DestDataOp_t * const destDataOp )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT       > periphId   ) &&
        ( GPDMA_CHANNEL_CNT      > channelId  )    )
    {
        uint32_t byteExchangeRegval = LL_DMA_GetDestByteExchange( gpdma_PeriphConf[periphId].DmaReg,
                                                                  gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        uint32_t halfWordExchangeRegVal = LL_DMA_GetDestHWordExchange( gpdma_PeriphConf[periphId].DmaReg,
                                                                       gpdma_PeriphConf[periphId].ChannelsConfig[channelId].ChannelReg );

        if( ( LL_DMA_DEST_BYTE_PRESERVE     == byteExchangeRegval     ) &&
            ( LL_DMA_DEST_HALFWORD_PRESERVE == halfWordExchangeRegVal )    )
        {
            *destDataOp = GPDMA_DEST_DATA_PRESERVE;
        }
        else if( ( LL_DMA_DEST_BYTE_PRESERVE     == byteExchangeRegval     ) &&
                 ( LL_DMA_DEST_HALFWORD_PRESERVE != halfWordExchangeRegVal )    )
        {
            *destDataOp = GPDMA_DEST_DATA_2BYTES_SWAP;
        }
        else if( ( LL_DMA_DEST_BYTE_PRESERVE     != byteExchangeRegval     ) &&
                 ( LL_DMA_DEST_HALFWORD_PRESERVE == halfWordExchangeRegVal )    )
        {
            *destDataOp = GPDMA_DEST_DATA_BYTE_SWAP;
        }
        else
        {
            *destDataOp = GPDMA_DEST_DATA_BYTE_2BYTES_SWAP;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}

/*------------------------- Interrupts functionality -------------------------*/

/**
 * \brief Sets the interrupt active state for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_InterruptActive ( gpdma_PeriphId_t periphId,
                                                 gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        nvic_RequestState_t retState = Nvic_Set_PeriphIrq_Active( gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].NvicChannelIrq );

        if( NVIC_REQUEST_ERROR != retState )
        {
            status = GPDMA_REQUEST_OK;
        }
        else
        {
            status = GPDMA_REQUEST_ERROR;
        }
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the interrupt inactive state for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_InterruptInactive ( gpdma_PeriphId_t periphId,
                                                   gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        nvic_RequestState_t retState = Nvic_Set_PeriphIrq_Inactive( gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].NvicChannelIrq );

        if( NVIC_REQUEST_ERROR != retState )
        {
            status = GPDMA_REQUEST_OK;
        }
        else
        {
            status = GPDMA_REQUEST_ERROR;
        }
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Activates the transfer complete interrupt for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_TransferCompleteIrqActive ( gpdma_PeriphId_t periphId,
                                                           gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_EnableIT_TC( gpdma_PeriphConf[ periphId ].DmaReg,
                            gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Deactivates the transfer complete interrupt for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_TransferCompleteIrqInactive ( gpdma_PeriphId_t periphId,
                                                             gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_DisableIT_TC( gpdma_PeriphConf[ periphId ].DmaReg,
                             gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the ISR handler for transfer complete event.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param irqHandler   [in]: Callback function for the ISR.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_TransferCompleteIsrHandler ( gpdma_PeriphId_t periphId,
                                                            gpdma_ChannelId_t channelId,
                                                            gpdma_IsrCallback * const irqHandler )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId   ) &&
        ( GPDMA_CHANNEL_CNT > channelId  ) &&
        ( GPDMA_NULL_PTR   != irqHandler )    )
    {
        gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].TransferCompleteCallback = irqHandler;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the ISR handler for transfer complete event.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param irqHandler  [out]: Pointer to store the callback function.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_TransferCompleteIsrHandler ( gpdma_PeriphId_t periphId,
                                                            gpdma_ChannelId_t channelId,
                                                            gpdma_IsrCallback ** const irqHandler )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId   ) &&
        ( GPDMA_CHANNEL_CNT > channelId  ) &&
        ( GPDMA_NULL_PTR   != irqHandler )    )
    {
        *irqHandler = gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].TransferCompleteCallback;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Activates the half transfer interrupt for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_HalfTransferIrqActive ( gpdma_PeriphId_t periphId,
                                                       gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_EnableIT_HT( gpdma_PeriphConf[ periphId ].DmaReg,
                            gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Deactivates the half transfer interrupt for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_HalfTransferIrqInactive ( gpdma_PeriphId_t periphId,
                                                         gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_DisableIT_HT( gpdma_PeriphConf[ periphId ].DmaReg,
                             gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the ISR handler for half transfer event.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param irqHandler   [in]: Callback function for the ISR.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_HalfTransferIsrHandler ( gpdma_PeriphId_t periphId,
                                                        gpdma_ChannelId_t channelId,
                                                        gpdma_IsrCallback * const irqHandler )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId   ) &&
        ( GPDMA_CHANNEL_CNT > channelId  ) &&
        ( GPDMA_NULL_PTR   != irqHandler )    )
    {
        gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].HalfTransferCallback = irqHandler;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the ISR handler for half transfer event.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param irqHandler  [out]: Pointer to store the callback function.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_HalfTransferIsrHandler ( gpdma_PeriphId_t periphId,
                                                        gpdma_ChannelId_t channelId,
                                                        gpdma_IsrCallback ** const irqHandler )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId   ) &&
        ( GPDMA_CHANNEL_CNT > channelId  ) &&
        ( GPDMA_NULL_PTR   != irqHandler )    )
    {
        *irqHandler = gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].HalfTransferCallback;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Activates the transfer error interrupt for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_TransferErrorIrqActive ( gpdma_PeriphId_t periphId,
                                                        gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_EnableIT_DTE( gpdma_PeriphConf[ periphId ].DmaReg,
                             gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Deactivates the transfer error interrupt for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_TransferErrorIrqInactive ( gpdma_PeriphId_t periphId,
                                                          gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_DisableIT_DTE( gpdma_PeriphConf[ periphId ].DmaReg,
                              gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the ISR handler for transfer error event.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param irqHandler   [in]: Callback function for the ISR.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_ErrorIsrHandler ( gpdma_PeriphId_t periphId,
                                                 gpdma_ChannelId_t channelId,
                                                 gpdma_IsrErrCallback * const irqHandler )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId   ) &&
        ( GPDMA_CHANNEL_CNT > channelId  ) &&
        ( GPDMA_NULL_PTR   != irqHandler )    )
    {
        gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback = irqHandler;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Gets the ISR handler for transfer error event.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 * \param irqHandler  [out]: Pointer to store the callback function.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_ErrorIsrHandler ( gpdma_PeriphId_t periphId,
                                                 gpdma_ChannelId_t channelId,
                                                 gpdma_IsrErrCallback ** const irqHandler )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId   ) &&
        ( GPDMA_CHANNEL_CNT > channelId  ) &&
        ( GPDMA_NULL_PTR   != irqHandler )    )
    {
        *irqHandler = gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback;

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the suspension interrupt active state for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_SuspensionIrqActive( gpdma_PeriphId_t periphId,
                                                    gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_EnableIT_SUSP( gpdma_PeriphConf[ periphId ].DmaReg,
                              gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the suspension interrupt inactive state for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_SuspensionIrqInactive( gpdma_PeriphId_t periphId,
                                                      gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_DisableIT_SUSP( gpdma_PeriphConf[ periphId ].DmaReg,
                               gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the configuration error interrupt active state for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_ConfigErrorIrqActive( gpdma_PeriphId_t periphId,
                                                     gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_EnableIT_USE( gpdma_PeriphConf[ periphId ].DmaReg,
                             gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the configuration error interrupt inactive state for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_ConfigErrorIrqInactive( gpdma_PeriphId_t periphId,
                                                       gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_DisableIT_USE( gpdma_PeriphConf[ periphId ].DmaReg,
                              gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the configuration update error interrupt active state for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_ConfigUpdateErrorIrqActive( gpdma_PeriphId_t periphId,
                                                           gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_EnableIT_ULE( gpdma_PeriphConf[ periphId ].DmaReg,
                             gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the configuration update error interrupt inactive state for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_ConfigUpdateErrorIrqInactive( gpdma_PeriphId_t periphId,
                                                             gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_DisableIT_ULE( gpdma_PeriphConf[ periphId ].DmaReg,
                              gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the trigger overrun interrupt active state for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_TriggerOverrunIrqActive( gpdma_PeriphId_t periphId,
                                                        gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_EnableIT_TO( gpdma_PeriphConf[ periphId ].DmaReg,
                            gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Sets the trigger overrun interrupt inactive state for the specified DMA channel.
 *
 * \param periphId     [in]: The DMA bus identifier.
 * \param channelId    [in]: The DMA channel identifier.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Set_TriggerOverrunIrqInactive( gpdma_PeriphId_t periphId,
                                                          gpdma_ChannelId_t channelId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_PERIPH_CNT  > periphId  ) &&
        ( GPDMA_CHANNEL_CNT > channelId )    )
    {
        LL_DMA_DisableIT_TO( gpdma_PeriphConf[ periphId ].DmaReg,
                             gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


/**
 * \brief Checks if selected address is located in FLASH memory.
 *
 * If address is located in FLASH or RAM, the recommended port is PORT1,
 * otherwise PORT0 shall be used for peripheral addressing. For memory to memory
 * transfer, port orientation is irrelevant. However, the source shall use PORT1
 * and destination shall use PORT0.
 */
gpdma_FlagState_t Gpdma_Check_FlashLocation( uint32_t addr )
{
    gpdma_FlagState_t flagState = GPDMA_FLAG_INACTIVE;

    extern uint32_t _flash_start;
    extern uint32_t _flash_end;

    if( ( (uint32_t)&_flash_start <= addr ) &&
        ( (uint32_t)&_flash_end    > addr )    )
    {
        flagState = GPDMA_FLAG_ACTIVE;
    }
    else
    {
        flagState = GPDMA_FLAG_INACTIVE;
    }

    return ( flagState );
}


/**
 * \brief Checks if selected address is located in RAM memory.
 *
 * If address is located in FLASH or RAM, the recommended port is PORT1,
 * otherwise PORT0 shall be used for peripheral addressing. For memory to memory
 * transfer, port orientation is irrelevant. However, the source shall use PORT1
 * and destination shall use PORT0.
 */
gpdma_FlagState_t Gpdma_Check_RamLocation( uint32_t addr )
{
    gpdma_FlagState_t flagState = GPDMA_FLAG_INACTIVE;

    extern uint32_t _ram_start;
    extern uint32_t _ram_end;
    extern uint32_t _ram4_start;
    extern uint32_t _ram4_end;

    if( ( ( (uint32_t)&_ram_start  <= addr ) &&
          ( (uint32_t)&_ram_end     > addr )    ) ||
        ( ( (uint32_t)&_ram4_start <= addr ) &&
          ( (uint32_t)&_ram4_end    > addr )    )    )
    {
        flagState = GPDMA_FLAG_ACTIVE;
    }
    else
    {
        flagState = GPDMA_FLAG_INACTIVE;
    }

    return ( flagState );
}

/* =========================== LOCAL FUNCTIONS ============================== */


/* =========================== INTERRUPT HANDLERS =========================== */

/**
 * \brief Common handler for GPDMA interrupts.
 *
 * \param periphId     [in]: The GPDMA bus identifier.
 * \param channelId [in]: The GPDMA channel identifier.
 */
static inline void Gpdma_GlobalIsrHandler( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId )
{
    uint32_t dma_base_addr = (uint32_t)gpdma_PeriphConf[ periphId ].DmaReg;
    uint32_t irqReg        = READ_REG( ( (DMA_Channel_TypeDef *)(dma_base_addr + LL_DMA_CH_OFFSET_TAB[gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg] ) )->CSR );

    if( 0u != ( irqReg & DMA_CSR_TCF ) )
    {
        LL_DMA_ClearFlag_TC( gpdma_PeriphConf[ periphId ].DmaReg,
                             gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if( GPDMA_NULL_PTR != gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].TransferCompleteCallback )
        {
            gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].TransferCompleteCallback();
        }
        else
        {
            /* Interrupt callback is not configured */
        }
    }

    if( 0u != ( irqReg & DMA_CFCR_HTF ) )
    {
        LL_DMA_ClearFlag_HT( gpdma_PeriphConf[ periphId ].DmaReg,
                             gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if( GPDMA_NULL_PTR != gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].HalfTransferCallback )
        {
            gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].HalfTransferCallback();
        }
        else
        {
            /* Interrupt callback is not configured */
        }
    }

    if( 0u != ( irqReg & DMA_CFCR_DTEF ) )
    {
        LL_DMA_ClearFlag_DTE( gpdma_PeriphConf[ periphId ].DmaReg,
                              gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if( GPDMA_NULL_PTR != gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback )
        {
            gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback( GPDMA_ERROR_TRANSFER );
        }
        else
        {
            /* Interrupt callback is not configured */
        }
    }

    if( 0u != ( irqReg & DMA_CFCR_USEF ) )
    {
        LL_DMA_ClearFlag_USE( gpdma_PeriphConf[ periphId ].DmaReg,
                              gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if( GPDMA_NULL_PTR != gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback )
        {
            gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback( GPDMA_ERROR_CONFIG_ERROR );
        }
        else
        {
            /* Interrupt callback is not configured */
        }
    }

    if( 0u != ( irqReg & DMA_CFCR_ULEF ) )
    {
        LL_DMA_ClearFlag_ULE( gpdma_PeriphConf[ periphId ].DmaReg,
                              gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if( GPDMA_NULL_PTR != gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback )
        {
            gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback( GPDMA_ERROR_CONFIG_UPDATE );
        }
        else
        {
            /* Interrupt callback is not configured */
        }
    }

    if( 0u != ( irqReg & DMA_CFCR_SUSPF ) )
    {
        LL_DMA_ClearFlag_SUSP( gpdma_PeriphConf[ periphId ].DmaReg,
                               gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if( GPDMA_NULL_PTR != gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback )
        {
            gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback( GPDMA_ERROR_SUSPENDED );
        }
        else
        {
            /* Interrupt callback is not configured */
        }
    }

    if( 0u != ( irqReg & DMA_CFCR_TOF ) )
    {
        LL_DMA_ClearFlag_TO( gpdma_PeriphConf[ periphId ].DmaReg,
                             gpdma_PeriphConf[ periphId ].ChannelsConfig[ channelId ].ChannelReg );

        if( GPDMA_NULL_PTR != gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback )
        {
            gpdma_PeriphConf[ periphId ].IsrCallbacks[ channelId ].ErrorCallback( GPDMA_ERROR_TRIG_OVERRUN );
        }
        else
        {
            /* Interrupt callback is not configured */
        }
    }
}

static void Gpdma_Gpdma1Channel0_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_0 );
}

static void Gpdma_Gpdma1Channel1_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_1 );
}


static void Gpdma_Gpdma1Channel2_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_2 );
}


static void Gpdma_Gpdma1Channel3_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_3 );
}


static void Gpdma_Gpdma1Channel4_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_4 );
}


static void Gpdma_Gpdma1Channel5_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_5 );
}


static void Gpdma_Gpdma1Channel6_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_6 );
}


static void Gpdma_Gpdma1Channel7_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_7 );
}


static void Gpdma_Gpdma1Channel8_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_8 );
}


static void Gpdma_Gpdma1Channel9_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_9 );
}


static void Gpdma_Gpdma1Channel10_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_10 );
}


static void Gpdma_Gpdma1Channel11_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_11 );
}


static void Gpdma_Gpdma1Channel12_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_12 );
}


static void Gpdma_Gpdma1Channel13_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_13 );
}


static void Gpdma_Gpdma1Channel14_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_14 );
}


static void Gpdma_Gpdma1Channel15_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_15 );
}


#if defined(GPDMA2)
static void Gpdma_Gpdma2Channel0_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_2, GPDMA_CHANNEL_0 );
}


static void Gpdma_Gpdma2Channel1_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_2, GPDMA_CHANNEL_1 );
}


static void Gpdma_Gpdma2Channel2_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_2, GPDMA_CHANNEL_2 );
}


static void Gpdma_Gpdma2Channel3_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_2, GPDMA_CHANNEL_3 );
}


static void Gpdma_Gpdma2Channel4_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_2, GPDMA_CHANNEL_4 );
}


static void Gpdma_Gpdma2Channel5_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_2, GPDMA_CHANNEL_5 );
}


static void Gpdma_Gpdma2Channel6_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_2, GPDMA_CHANNEL_6 );
}


static void Gpdma_Gpdma2Channel7_IsrHandler( void )
{
    Gpdma_GlobalIsrHandler( GPDMA_PERIPH_2, GPDMA_CHANNEL_7 );
}

#endif

/* ================================ TASKS =================================== */
