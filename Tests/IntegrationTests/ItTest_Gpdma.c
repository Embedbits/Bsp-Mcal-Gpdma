/**
 * \author Mr.Nobody
 * \file ItTest_Gpdma.c
 * \ingroup Gpdma
 * \brief Integration tests of General purpose DMA (GPDMA) module on target.
 *
 * Gpdma module runs on the MCU together with real RCC and NVIC modules and
 * hardware. Tests verify behavior which cannot be verified by unit tests
 * (emulated registers): memory to memory transfers executed by the GPDMA
 * (data width, address increment, data operations), transfer list (linked
 * transfers) in RAM, transfer complete / half transfer / error interrupts with
 * user callbacks, channel state and priority.
 *
 * No peripheral and no external wiring is used - memory to memory transfers
 * are started by software request.
 *
 * Used resources:
 * - GPDMA1 channel 0 - polled transfers (no interrupt)
 * - GPDMA1 channel 7 - interrupt driven transfers
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "IntegrationTesting.h"             /* Integration testing on target  */
#include "Gpdma_Port.h"                     /* Module under test              */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static void It_Gpdma_Get_XferConfig      ( gpdma_TransferConfig_t * const xferConfig, const void * src, void * dst,
                                          gpdma_BlockSize_t sizeBytes, gpdma_DataSize_t dataSize );
static void It_Gpdma_Get_ChannelConfig   ( gpdma_ConfigStruct_t * const config, gpdma_ChannelId_t channelId,
                                          gpdma_TransferConfig_t * const xferConfig, gpdma_TransfersCount_t xferCnt );
static void It_Gpdma_Wait_ChannelIdle   ( gpdma_ChannelId_t channelId );
static void It_Gpdma_Wait_Callback      ( volatile const uint32_t * const callbackCnt, uint32_t expectedCnt );
static void It_Gpdma_Fill_Buffers       ( void );
static void It_Gpdma_Log                ( uint32_t event );

static void It_Gpdma_TransferCompleteCallback( void );
static void It_Gpdma_HalfTransferCallback   ( void );
static void It_Gpdma_ErrorCallback          ( gpdma_ErrorMaskId_t errorMask );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** GPDMA peripheral used by tests */
#define IT_GPDMA_PERIPH                     ( GPDMA_PERIPH_1 )

/** Channel used for polled transfers */
#define IT_GPDMA_POLL_CH                    ( GPDMA_CHANNEL_0 )

/** Channel used for interrupt driven transfers */
#define IT_GPDMA_IRQ_CH                     ( GPDMA_CHANNEL_7 )

/** Count of 32-bit words of test buffers */
#define IT_GPDMA_BUF_WORDS                  ( 64u )

/** Size of test buffers in bytes */
#define IT_GPDMA_BUF_BYTES                  ( IT_GPDMA_BUF_WORDS * 4u )

/** Pattern written into destination buffer before transfer */
#define IT_GPDMA_DST_FILL                   ( 0xA5A5A5A5u )

/** Not mapped address (source of transfer error) - reserved area behind the flash memory
 *  (STM32U5 flash has at most 4 MB at 0x08000000). 0x60000000 is FMC bank 1 on STM32U575 / U5A5
 *  and is accessed without bus error. */
#define IT_GPDMA_INVALID_ADDR               ( 0x08400000u )

/** Maximal count of wait loop iterations (timeout) */
#define IT_GPDMA_WAIT_LOOPS                 ( 1000000u )

/** Size of event log */
#define IT_GPDMA_LOG_SIZE                   ( 4u )

/** Events of event log */
#define IT_GPDMA_EV_HALF                    ( 1u )
#define IT_GPDMA_EV_COMPLETE                ( 2u )

/* ============================== MACROS ==================================== */

/* ========================== LOCAL VARIABLES =============================== */

/** Source buffer */
static uint32_t                     itGpdma_Src[ IT_GPDMA_BUF_WORDS ];

/** Destination buffer */
static uint32_t                     itGpdma_Dst[ IT_GPDMA_BUF_WORDS ];

/** Second destination buffer (linked transfer) */
static uint32_t                     itGpdma_Dst2[ IT_GPDMA_BUF_WORDS ];

/** Transfer list of the channel (static, see \ref gpdma_ConfigStruct_t) */
static gpdma_XferList_t             itGpdma_XferList[ 2u ];

/** Count of callback calls */
static volatile uint32_t            itGpdma_CompleteCnt;
static volatile uint32_t            itGpdma_HalfCnt;
static volatile uint32_t            itGpdma_ErrorCnt;

/** Error mask of the last error callback */
static volatile gpdma_ErrorMaskId_t itGpdma_ErrorMask;

/** Event log of callbacks */
static volatile uint32_t            itGpdma_EventLog[ IT_GPDMA_LOG_SIZE ];
static volatile uint32_t            itGpdma_EventCnt;

/* ============================= TEST SETUP ================================= */

void setUp( void )
{
    itGpdma_CompleteCnt = 0u;
    itGpdma_HalfCnt     = 0u;
    itGpdma_ErrorCnt    = 0u;
    itGpdma_ErrorMask   = (gpdma_ErrorMaskId_t)0u;
    itGpdma_EventCnt    = 0u;

    It_Gpdma_Fill_Buffers();
}


void tearDown( void )
{
    /* Every test case runs after system reset */
}

/* =============================== TESTS ==================================== */

/*----------------------------- Memory to memory -----------------------------*/

/**
 * \brief   Memory to memory transfer with 32-bit data is executed by GPDMA.
 *
 * \details Initializes channel 0 with one memory to memory transfer of 256 bytes
 *          (32-bit data, both addresses incremented), enables the channel and
 *          waits until it is disabled by HW.
 *
 * \par Expected results
 * - Destination buffer equals source buffer (64 words).
 */
void It_Gpdma_Set_ChannelActive_MemToMem32Bit_DataCopied( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;

    It_Gpdma_Get_XferConfig( &xferConfig, itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_BYTES, GPDMA_DATA_SIZE_32BITS );
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_POLL_CH, &xferConfig, 1u );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH ) );
    It_Gpdma_Wait_ChannelIdle( IT_GPDMA_POLL_CH );

    TEST_ASSERT_EQUAL_HEX32_ARRAY( itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_WORDS );
}


/**
 * \brief   8-bit transfer of odd size copies only required bytes.
 *
 * \details Transfers 37 bytes with 8-bit data size on channel 0.
 *
 * \par Expected results
 * - First 37 destination bytes equal the source.
 * - Bytes 37 and 38 keep fill pattern 0xA5.
 */
void It_Gpdma_Set_ChannelActive_MemToMem8BitOddSize_OnlyRequiredBytesCopied( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;
    const uint8_t * const  srcBytes = (const uint8_t *)itGpdma_Src;
    const uint8_t * const  dstBytes = (const uint8_t *)itGpdma_Dst;

    It_Gpdma_Get_XferConfig( &xferConfig, itGpdma_Src, itGpdma_Dst, 37u, GPDMA_DATA_SIZE_8BITS );
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_POLL_CH, &xferConfig, 1u );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH ) );
    It_Gpdma_Wait_ChannelIdle( IT_GPDMA_POLL_CH );

    TEST_ASSERT_EQUAL_HEX8_ARRAY( srcBytes, dstBytes, 37u );
    TEST_ASSERT_EQUAL_HEX8( 0xA5u, dstBytes[ 37u ] );
    TEST_ASSERT_EQUAL_HEX8( 0xA5u, dstBytes[ 38u ] );
}


/**
 * \brief   Static destination address - every word is written to the same address.
 *
 * \details Transfers 64 words with destination address mode static.
 *
 * \par Expected results
 * - 1st destination word contains the last source word.
 * - 2nd destination word keeps fill pattern 0xA5A5A5A5.
 */
void It_Gpdma_Set_ChannelActive_StaticDestination_LastWordStored( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;

    It_Gpdma_Get_XferConfig( &xferConfig, itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_BYTES, GPDMA_DATA_SIZE_32BITS );
    xferConfig.DestinationAddrMode = GPDMA_ADDR_STATIC;
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_POLL_CH, &xferConfig, 1u );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH ) );
    It_Gpdma_Wait_ChannelIdle( IT_GPDMA_POLL_CH );

    TEST_ASSERT_EQUAL_HEX32( itGpdma_Src[ IT_GPDMA_BUF_WORDS - 1u ], itGpdma_Dst[ 0u ] );
    TEST_ASSERT_EQUAL_HEX32( IT_GPDMA_DST_FILL, itGpdma_Dst[ 1u ] );
}


/**
 * \brief   Destination data operation "2 bytes swap" swaps half-words of a word.
 *
 * \details Transfers 4 words (16 bytes) with destination data operation
 *          GPDMA_DEST_DATA_2BYTES_SWAP.
 *
 * \par Expected results
 * - Every destination word = source word with swapped upper and lower half-word.
 */
void It_Gpdma_Set_ChannelActive_DestHalfwordSwap_WordHalvesSwapped( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;

    It_Gpdma_Get_XferConfig( &xferConfig, itGpdma_Src, itGpdma_Dst, 16u, GPDMA_DATA_SIZE_32BITS );
    xferConfig.DestinationDataOp = GPDMA_DEST_DATA_2BYTES_SWAP;
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_POLL_CH, &xferConfig, 1u );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH ) );
    It_Gpdma_Wait_ChannelIdle( IT_GPDMA_POLL_CH );

    for( uint32_t wordIdx = 0u; 4u > wordIdx; wordIdx++ )
    {
        const uint32_t srcWord = itGpdma_Src[ wordIdx ];

        TEST_ASSERT_EQUAL_HEX32( ( srcWord << 16u ) | ( srcWord >> 16u ), itGpdma_Dst[ wordIdx ] );
    }
}


/**
 * \brief   Transfer list with two linked transfers is executed completely.
 *
 * \details Initializes channel 0 with two transfers: 1st half of source into the
 *          1st destination buffer, 2nd half into the 2nd destination buffer
 *          (linked list nodes in RAM), enables the channel and waits for its end.
 *
 * \par Expected results
 * - 1st buffer contains the 1st half of source, 2nd buffer the 2nd half.
 * - 1st buffer behind the copied half keeps fill pattern (no overrun).
 */
void It_Gpdma_Init_TwoLinkedTransfers_BothBuffersCopied( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig[ 2u ];

    for( uint32_t wordIdx = 0u; IT_GPDMA_BUF_WORDS > wordIdx; wordIdx++ )
    {
        itGpdma_Dst2[ wordIdx ] = IT_GPDMA_DST_FILL;
    }

    /* First half of source into first buffer, second half into second buffer */
    It_Gpdma_Get_XferConfig( &xferConfig[ 0u ], &itGpdma_Src[ 0u ], itGpdma_Dst, IT_GPDMA_BUF_BYTES / 2u, GPDMA_DATA_SIZE_32BITS );
    It_Gpdma_Get_XferConfig( &xferConfig[ 1u ], &itGpdma_Src[ IT_GPDMA_BUF_WORDS / 2u ], itGpdma_Dst2, IT_GPDMA_BUF_BYTES / 2u, GPDMA_DATA_SIZE_32BITS );
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_POLL_CH, xferConfig, 2u );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH ) );
    It_Gpdma_Wait_ChannelIdle( IT_GPDMA_POLL_CH );

    TEST_ASSERT_EQUAL_HEX32_ARRAY( &itGpdma_Src[ 0u ], itGpdma_Dst,  IT_GPDMA_BUF_WORDS / 2u );
    TEST_ASSERT_EQUAL_HEX32_ARRAY( &itGpdma_Src[ IT_GPDMA_BUF_WORDS / 2u ], itGpdma_Dst2, IT_GPDMA_BUF_WORDS / 2u );
    TEST_ASSERT_EQUAL_HEX32( IT_GPDMA_DST_FILL, itGpdma_Dst[ IT_GPDMA_BUF_WORDS / 2u ] );
}

/*-------------------------------- Interrupts --------------------------------*/

/**
 * \brief   Transfer complete interrupt calls user callback once.
 *
 * \details Initializes channel 7 with transfer complete callback, activates its
 *          interrupt, enables the channel and waits for the callback and channel end.
 *
 * \par Expected results
 * - Transfer complete callback called exactly once, no error callback.
 * - Destination buffer equals source buffer.
 */
void It_Gpdma_Init_TransferCompleteIsr_CallbackCalledOnce( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;

    It_Gpdma_Get_XferConfig( &xferConfig, itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_BYTES, GPDMA_DATA_SIZE_32BITS );
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_IRQ_CH, &xferConfig, 1u );
    config.TransferCompleteIsr = It_Gpdma_TransferCompleteCallback;
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_InterruptActive( IT_GPDMA_PERIPH, IT_GPDMA_IRQ_CH ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( IT_GPDMA_PERIPH, IT_GPDMA_IRQ_CH ) );
    It_Gpdma_Wait_Callback( &itGpdma_CompleteCnt, 1u );
    It_Gpdma_Wait_ChannelIdle( IT_GPDMA_IRQ_CH );

    TEST_ASSERT_EQUAL_UINT32( 1u, itGpdma_CompleteCnt );
    TEST_ASSERT_EQUAL_UINT32( 0u, itGpdma_ErrorCnt );
    TEST_ASSERT_EQUAL_HEX32_ARRAY( itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_WORDS );
}


/**
 * \brief   Half transfer callback precedes transfer complete callback.
 *
 * \details Initializes channel 7 with half transfer and transfer complete
 *          callbacks, enables interrupt and channel, waits for complete callback.
 *          Callbacks are recorded in event log.
 *
 * \par Expected results
 * - Half transfer and complete callbacks called once each.
 * - Event log: half transfer first, then transfer complete.
 */
void It_Gpdma_Init_HalfAndCompleteIsr_HalfTransferBeforeComplete( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;

    It_Gpdma_Get_XferConfig( &xferConfig, itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_BYTES, GPDMA_DATA_SIZE_32BITS );
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_IRQ_CH, &xferConfig, 1u );
    config.TransferCompleteIsr = It_Gpdma_TransferCompleteCallback;
    config.HalfTransferIsr     = It_Gpdma_HalfTransferCallback;
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_InterruptActive( IT_GPDMA_PERIPH, IT_GPDMA_IRQ_CH ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( IT_GPDMA_PERIPH, IT_GPDMA_IRQ_CH ) );
    It_Gpdma_Wait_Callback( &itGpdma_CompleteCnt, 1u );

    TEST_ASSERT_EQUAL_UINT32( 1u, itGpdma_HalfCnt );
    TEST_ASSERT_EQUAL_UINT32( 1u, itGpdma_CompleteCnt );
    TEST_ASSERT_EQUAL_UINT32( 2u, itGpdma_EventCnt );

    TEST_ASSERT_EQUAL_UINT32( IT_GPDMA_EV_HALF,     itGpdma_EventLog[ 0u ] );
    TEST_ASSERT_EQUAL_UINT32( IT_GPDMA_EV_COMPLETE, itGpdma_EventLog[ 1u ] );
}


/**
 * \brief   Transfer from not mapped address reports transfer error.
 *
 * \details Initializes channel 7 with source address 0x08400000 (not mapped),
 *          error callback with mask GPDMA_ERROR_TRANSFER and complete callback,
 *          enables interrupt and channel and waits for error callback.
 *
 * \par Expected results
 * - Error callback called once with GPDMA_ERROR_TRANSFER.
 * - Transfer complete callback is not called.
 */
void It_Gpdma_Init_ErrorIsr_InvalidSourceAddress_TransferErrorReported( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;

    It_Gpdma_Get_XferConfig( &xferConfig, (const void *)IT_GPDMA_INVALID_ADDR, itGpdma_Dst, 16u, GPDMA_DATA_SIZE_32BITS );
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_IRQ_CH, &xferConfig, 1u );
    config.TransferCompleteIsr = It_Gpdma_TransferCompleteCallback;
    config.ErrorIsr            = It_Gpdma_ErrorCallback;
    config.ErrorMask           = GPDMA_ERROR_TRANSFER;
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_InterruptActive( IT_GPDMA_PERIPH, IT_GPDMA_IRQ_CH ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( IT_GPDMA_PERIPH, IT_GPDMA_IRQ_CH ) );
    It_Gpdma_Wait_Callback( &itGpdma_ErrorCnt, 1u );
    It_Gpdma_Wait_ChannelIdle( IT_GPDMA_IRQ_CH );

    TEST_ASSERT_EQUAL_UINT32( 1u, itGpdma_ErrorCnt );
    TEST_ASSERT_EQUAL( GPDMA_ERROR_TRANSFER, itGpdma_ErrorMask );
    TEST_ASSERT_EQUAL_UINT32( 0u, itGpdma_CompleteCnt );
}

/*---------------------- Channel state and configuration ---------------------*/

/**
 * \brief   Channel state reflects enabled channel, transfer starts only by enable.
 *
 * \details Initializes channel 0, reads its state and destination, enables the
 *          channel, waits until it is finished and reads the state again.
 *
 * \par Expected results
 * - After initialization: state INACTIVE and destination not written.
 * - After transfer: state INACTIVE (channel disabled by HW).
 */
void It_Gpdma_Get_ChannelState_ActiveUntilTransferFinished( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;
    gpdma_FunctionState_t  state = GPDMA_FUNCTION_ACTIVE;

    It_Gpdma_Get_XferConfig( &xferConfig, itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_BYTES, GPDMA_DATA_SIZE_32BITS );
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_POLL_CH, &xferConfig, 1u );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_ChannelState( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH, &state ) );
    TEST_ASSERT_EQUAL( GPDMA_FUNCTION_INACTIVE, state );

    /* Nothing is transferred before the channel is enabled */
    TEST_ASSERT_EQUAL_HEX32( IT_GPDMA_DST_FILL, itGpdma_Dst[ 0u ] );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH ) );
    It_Gpdma_Wait_ChannelIdle( IT_GPDMA_POLL_CH );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_ChannelState( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH, &state ) );
    TEST_ASSERT_EQUAL( GPDMA_FUNCTION_INACTIVE, state );
}


/**
 * \brief   All channel priorities are written and read back on target.
 *
 * \details Initializes channel 0 and sets every priority value.
 *
 * \par Expected results
 * - Every Set / Get returns GPDMA_REQUEST_OK and read priority equals set one.
 */
void It_Gpdma_Set_Priority_AllValues_ReadBack( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;
    gpdma_Priority_t       priority = GPDMA_PRIORITY_CNT;

    It_Gpdma_Get_XferConfig( &xferConfig, itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_BYTES, GPDMA_DATA_SIZE_32BITS );
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_POLL_CH, &xferConfig, 1u );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    for( gpdma_Priority_t prioIdx = GPDMA_PRIORITY_LOW; GPDMA_PRIORITY_CNT > prioIdx; prioIdx++ )
    {
        TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_Priority( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH, prioIdx ) );
        TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_Priority( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH, &priority ) );
        TEST_ASSERT_EQUAL( prioIdx, priority );
    }
}


/**
 * \brief   Channel with single list access is released by Gpdma_Deinit().
 *
 * \details
 * 1. Initializes channel 0 with single list access mode twice.
 * 2. Deinitializes the channel and initializes it again.
 * 3. Executes the transfer.
 *
 * \par Expected results
 * 1. First call GPDMA_REQUEST_OK, second GPDMA_REQUEST_ERROR.
 * 2. GPDMA_REQUEST_OK for deinitialization and new initialization.
 * 3. Destination buffer equals source buffer.
 */
void It_Gpdma_Init_AlreadyInitializedSingleAccess_ReturnsError( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;

    It_Gpdma_Get_XferConfig( &xferConfig, itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_BYTES, GPDMA_DATA_SIZE_32BITS );
    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_POLL_CH, &xferConfig, 1u );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Init( &config ) );

    /* Deinit releases the channel - initialization is possible again */
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Deinit( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( IT_GPDMA_PERIPH, IT_GPDMA_POLL_CH ) );
    It_Gpdma_Wait_ChannelIdle( IT_GPDMA_POLL_CH );
    TEST_ASSERT_EQUAL_HEX32_ARRAY( itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_WORDS );
}


/**
 * \brief   Gpdma_Init() on target rejects invalid configurations.
 *
 * \details Calls Gpdma_Init() with NULL configuration, without transfer list and
 *          with invalid channel.
 *
 * \par Expected results
 * - GPDMA_REQUEST_ERROR is returned in all cases.
 */
void It_Gpdma_Init_InvalidConfig_ReturnsError( void )
{
    gpdma_ConfigStruct_t   config;
    gpdma_TransferConfig_t xferConfig;

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Init( GPDMA_NULL_PTR ) );

    It_Gpdma_Get_XferConfig( &xferConfig, itGpdma_Src, itGpdma_Dst, IT_GPDMA_BUF_BYTES, GPDMA_DATA_SIZE_32BITS );

    It_Gpdma_Get_ChannelConfig( &config, IT_GPDMA_POLL_CH, &xferConfig, 1u );
    config.XferList = GPDMA_NULL_PTR;
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Init( &config ) );

    It_Gpdma_Get_ChannelConfig( &config, GPDMA_CHANNEL_CNT, &xferConfig, 1u );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Init( &config ) );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/**
 * \brief Fills memory to memory transfer configuration (software request, no trigger).
 *
 * \param xferConfig [out]: Transfer configuration
 * \param src         [in]: Source address
 * \param dst         [in]: Destination address
 * \param sizeBytes   [in]: Block size in bytes
 * \param dataSize    [in]: Source and destination data size
 */
static void It_Gpdma_Get_XferConfig( gpdma_TransferConfig_t * const xferConfig, const void * src, void * dst,
                                    gpdma_BlockSize_t sizeBytes, gpdma_DataSize_t dataSize )
{
    xferConfig->Direction                   = GPDMA_DIR_MEMORY_TO_MEMORY;
    xferConfig->XferListExecMode            = GPDMA_XFER_LIST_EXEC_ONCE;
    xferConfig->EventMode                   = GPDMA_TRANSFER_EVENT_BLOCK;
    xferConfig->TriggerType                 = GPDMA_TRG_NOT_USED;
    xferConfig->TriggerSource               = (gpdma_TrgSrcId_t)0u;
    xferConfig->TriggerMode                 = GPDMA_TRIGGER_BLOCK;
    xferConfig->RequestSource               = (gpdma_PeriphReqId_t)0u;
    xferConfig->RequestMode                 = GPDMA_PERIPH_REQ_SINGLE;
    xferConfig->BlockSize                   = sizeBytes;
    xferConfig->BlockRepetitionCount        = 0u;
    xferConfig->SourceAddr                  = (gpdma_SrcAddr_t)src;
    xferConfig->SourceDataSize              = dataSize;
    xferConfig->SourceBurstLength           = 1u;
    xferConfig->SourceAddrMode              = GPDMA_ADDR_INCREMENT;
    xferConfig->SourcePortId                = GPDMA_PORT_DEFAULT;
    xferConfig->SourceDataOp                = GPDMA_SRC_DATA_PRESERVE;
    xferConfig->SourceBlockOffset2D         = 0u;
    xferConfig->SourceRepBlockOffset2D      = 0u;
    xferConfig->DestinationAddr             = (gpdma_DstAddr_t)dst;
    xferConfig->DestinationDataSize         = dataSize;
    xferConfig->DestinationBurstLength      = 1u;
    xferConfig->DestinationAddrMode         = GPDMA_ADDR_INCREMENT;
    xferConfig->DestinationPortId           = GPDMA_PORT_DEFAULT;
    xferConfig->DestinationDataOp           = GPDMA_DEST_DATA_PRESERVE;
    xferConfig->DestinationBlockOffset2D    = 0u;
    xferConfig->DestinationRepBlockOffset2D = 0u;
}


/**
 * \brief Fills channel configuration with test transfer list.
 *
 * \param config     [out]: Channel configuration
 * \param channelId   [in]: GPDMA channel
 * \param xferConfig  [in]: Transfer configuration(s)
 * \param xferCnt     [in]: Count of transfers (max. 2)
 */
static void It_Gpdma_Get_ChannelConfig( gpdma_ConfigStruct_t * const config, gpdma_ChannelId_t channelId,
                                       gpdma_TransferConfig_t * const xferConfig, gpdma_TransfersCount_t xferCnt )
{
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_DefaultConfig( config ) );

    config->PeriphId           = IT_GPDMA_PERIPH;
    config->ChannelId          = channelId;
    config->TransferExecMode   = GPDMA_XFER_EXEC_CONTINUOUS;
    config->TransferConfig     = xferConfig;
    config->TransfersCount     = xferCnt;
    config->XferListAccessMode = GPDMA_TRANSFER_LIST_ACCESS_SINGLE;
    config->XferList           = itGpdma_XferList;
}


/**
 * \brief Waits until the channel is disabled by hardware (transfer finished) or timeout.
 *
 * \param channelId [in]: GPDMA channel
 */
static void It_Gpdma_Wait_ChannelIdle( gpdma_ChannelId_t channelId )
{
    gpdma_FunctionState_t state = GPDMA_FUNCTION_ACTIVE;

    for( uint32_t loopIdx = 0u; ( IT_GPDMA_WAIT_LOOPS > loopIdx ) && ( GPDMA_FUNCTION_INACTIVE != state ); loopIdx++ )
    {
        TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_ChannelState( IT_GPDMA_PERIPH, channelId, &state ) );
    }

    TEST_ASSERT_EQUAL_MESSAGE( GPDMA_FUNCTION_INACTIVE, state, "Channel not finished" );
}


/**
 * \brief Waits until callback is called required count of times (or timeout).
 *
 * \param callbackCnt [in]: Callback call counter
 * \param expectedCnt [in]: Required count of calls
 */
static void It_Gpdma_Wait_Callback( volatile const uint32_t * const callbackCnt, uint32_t expectedCnt )
{
    for( volatile uint32_t loopIdx = 0u; ( IT_GPDMA_WAIT_LOOPS > loopIdx ) && ( expectedCnt > *callbackCnt ); loopIdx++ )
    {
        /* Busy wait */
    }
}


/**
 * \brief Fills source with pattern and destinations with \ref IT_GPDMA_DST_FILL.
 */
static void It_Gpdma_Fill_Buffers( void )
{
    for( uint32_t wordIdx = 0u; IT_GPDMA_BUF_WORDS > wordIdx; wordIdx++ )
    {
        itGpdma_Src [ wordIdx ] = 0x01020304u + ( wordIdx * 0x11111111u );
        itGpdma_Dst [ wordIdx ] = IT_GPDMA_DST_FILL;
        itGpdma_Dst2[ wordIdx ] = IT_GPDMA_DST_FILL;
    }
}


/**
 * \brief Stores event into the event log.
 *
 * \param event [in]: Event identification
 */
static void It_Gpdma_Log( uint32_t event )
{
    if( IT_GPDMA_LOG_SIZE > itGpdma_EventCnt )
    {
        itGpdma_EventLog[ itGpdma_EventCnt ] = event;
    }
    else
    {
        /* Log is full */
    }

    itGpdma_EventCnt++;
}


/**
 * \brief Transfer complete callback.
 */
static void It_Gpdma_TransferCompleteCallback( void )
{
    itGpdma_CompleteCnt++;
    It_Gpdma_Log( IT_GPDMA_EV_COMPLETE );
}


/**
 * \brief Half transfer callback.
 */
static void It_Gpdma_HalfTransferCallback( void )
{
    itGpdma_HalfCnt++;
    It_Gpdma_Log( IT_GPDMA_EV_HALF );
}


/**
 * \brief Error callback.
 *
 * \param errorMask [in]: Reported error
 */
static void It_Gpdma_ErrorCallback( gpdma_ErrorMaskId_t errorMask )
{
    itGpdma_ErrorMask = errorMask;
    itGpdma_ErrorCnt++;
}
