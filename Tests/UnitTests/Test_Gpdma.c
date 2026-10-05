/**
 * \author Mr.Nobody
 * \file Test_Gpdma.c
 * \ingroup Gpdma
 * \brief Unit tests of General Purpose DMA (GPDMA) module.
 *
 * Gpdma.c and Gpdma_Tl.c are compiled unchanged with real LL drivers. GPDMA
 * registers are emulated by RegMem, transfer lists (linked-list nodes) are placed
 * in emulated SRAM (32-bit addresses as on MCU). RCC and NVIC modules are mocked,
 * channel ISR registered in NVIC is captured by stub and called directly.
 *
 * \note Flag clear register (CFCR) is write-1-to-clear on HW, emulated register
 *       keeps written value - tests check that the flag bit was written.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "UtCommon.h"                       /* Common test helpers            */
#include "RegMem.h"                         /* Register memory emulation      */
#include "Gpdma_Port.h"                     /* Module under test              */
#include "MockRcc_Port.h"                   /* RCC module mock                */
#include "MockNvic_Port.h"                  /* NVIC module mock               */
#include "Stm32_dma.h"                      /* GPDMA registers definition     */
#include "Stm32_usart.h"                    /* USART1 data register address   */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static nvic_RequestState_t  Ut_Gpdma_NvicSetHandlerStub ( nvic_PeriphIrqList_t irqId, const nvic_IsrCallback_t irqHandler, int callCnt );
static void                 Ut_Gpdma_TcCallback         ( void );
static void                 Ut_Gpdma_HtCallback         ( void );
static void                 Ut_Gpdma_ErrCallback        ( gpdma_ErrorMaskId_t errorId );
static void                 Ut_Gpdma_Get_XferConfig     ( gpdma_TransferConfig_t * const xferConfig );
static void                 Ut_Gpdma_Get_Config         ( gpdma_ConfigStruct_t * const config, gpdma_ChannelId_t channelId, uint32_t sramOffset );
static void                 Ut_Gpdma_Expect_ClockActivation( rcc_FunctionState_t clockState );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Channel used by register tests - supports 2D addressing */
#define UT_GPDMA_CH_2D                      ( GPDMA_CHANNEL_6 )

/** Channel used by initialization tests - linear addressing only */
#define UT_GPDMA_CH_LINEAR                  ( GPDMA_CHANNEL_2 )

/** Source buffer address in emulated SRAM */
#define UT_GPDMA_SRC_ADDR                   ( REGMEM_SRAM_BASE + 0x1000u )

/** Offset of transfer list of the first channel in emulated SRAM */
#define UT_GPDMA_LIST_OFFSET_A              ( 0x8000u )

/** Offset of transfer list of the second channel in emulated SRAM */
#define UT_GPDMA_LIST_OFFSET_B              ( 0x8400u )

/** Block size used by tests (bytes) */
#define UT_GPDMA_BLOCK_SIZE                 ( 16u )

/** Mask of upper half of 32-bit address (linked-list base address) */
#define UT_GPDMA_UPPER_HALF_MASK            ( 0xFFFF0000u )

/* ============================== MACROS ==================================== */

/** Channel register block of GPDMA1 */
#define UT_GPDMA1_CH( channel )             ( (DMA_Channel_TypeDef *)( (uintptr_t)GPDMA1_Channel0 + \
                                              ( (uintptr_t)GPDMA1_Channel1 - (uintptr_t)GPDMA1_Channel0 ) * (uint32_t)( channel ) ) )

/** Set - Get round trip of channel configuration function pair */
#define UT_GPDMA_ROUNDTRIP( setFn, getFn, type, value )                                             \
    do                                                                                              \
    {                                                                                               \
        type readValue = (type)0;                                                                   \
        TEST_ASSERT_EQUAL_MESSAGE( GPDMA_REQUEST_OK, setFn( GPDMA_PERIPH_1, UT_GPDMA_CH_2D, ( value ) ), #setFn );      \
        TEST_ASSERT_EQUAL_MESSAGE( GPDMA_REQUEST_OK, getFn( GPDMA_PERIPH_1, UT_GPDMA_CH_2D, &readValue ), #getFn );     \
        TEST_ASSERT_EQUAL_MESSAGE( ( value ), readValue, #getFn " value" );                         \
    } while( 0 )

/** Set - Get round trip of transfer list node function pair (node in emulated SRAM) */
#define UT_GPDMA_LIST_ROUNDTRIP( channelType, setFn, getFn, type, value )                           \
    do                                                                                              \
    {                                                                                               \
        volatile gpdma_XferList_t * const node = REGMEM_SRAM_PTR( volatile gpdma_XferList_t, UT_GPDMA_LIST_OFFSET_A ); \
        type readValue = (type)0;                                                                   \
        TEST_ASSERT_EQUAL_MESSAGE( GPDMA_REQUEST_OK, setFn( node, ( channelType ), ( value ) ), #setFn );          \
        TEST_ASSERT_EQUAL_MESSAGE( GPDMA_REQUEST_OK, getFn( node, ( channelType ), &readValue ), #getFn );         \
        TEST_ASSERT_EQUAL_MESSAGE( ( value ), readValue, #getFn " value" );                         \
    } while( 0 )

/* ========================= EXPORTED VARIABLES ============================= */

/**
 * Linker script symbols referenced by Gpdma.c (FLASH / RAM location checks used
 * for default port selection). Host variables - addresses do not match MCU
 * memory map, port selection is not checked by these tests.
 */
uint32_t _flash_start;
uint32_t _flash_end;
uint32_t _ram_start;
uint32_t _ram_end;
uint32_t _ram4_start;
uint32_t _ram4_end;

/* ========================== LOCAL VARIABLES =============================== */

static nvic_IsrCallback_t   utGpdma_ChannelIsr;
static uint32_t             utGpdma_TcCnt;
static uint32_t             utGpdma_HtCnt;
static uint32_t             utGpdma_ErrCnt;
static gpdma_ErrorMaskId_t  utGpdma_LastError;

/** Transfer configurations (configuration is copied into list nodes by Init) */
static gpdma_TransferConfig_t utGpdma_XferConfig[ 1 ];

/* ============================ TEST FIXTURE ================================ */

void setUp( void )
{
    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Reset() );

    utGpdma_ChannelIsr = NULL;
    utGpdma_TcCnt      = 0u;
    utGpdma_HtCnt      = 0u;
    utGpdma_ErrCnt     = 0u;
    utGpdma_LastError  = GPDMA_ERROR_ALL;

    Nvic_Set_PeriphIrq_Handler_Stub( Ut_Gpdma_NvicSetHandlerStub );
}


void tearDown( void )
{
    /* Mocks are verified by generated runner */
}

/* ========================== MODULE VERSION ================================ */

/**
 * \brief   Gpdma_Get_ModuleVersion() returns version of the module.
 *
 * \details Reads the module version structure.
 *
 * \par Expected results
 * - Major version is 1.
 */
void Ut_Gpdma_Get_ModuleVersion_ReturnsVersion( void )
{
    gpdma_ModuleVersion_t version = Gpdma_Get_ModuleVersion();

    TEST_ASSERT_EQUAL_UINT8( 1u, version.Major );
}


/**
 * \brief   Gpdma_Get_DefaultConfig() fills default channel configuration.
 *
 * \details Reads default configuration, then calls the function with NULL.
 *
 * \par Expected results
 * - GPDMA_REQUEST_OK, GPDMA1, channel 0, list access mode append, no transfer
 *   list, no transfer complete callback.
 * - NULL pointer: GPDMA_REQUEST_ERROR.
 */
void Ut_Gpdma_Get_DefaultConfig_ReturnsDefaults( void )
{
    gpdma_ConfigStruct_t config;

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_DefaultConfig( &config ) );

    TEST_ASSERT_EQUAL( GPDMA_PERIPH_1,                    config.PeriphId );
    TEST_ASSERT_EQUAL( GPDMA_CHANNEL_0,                   config.ChannelId );
    TEST_ASSERT_EQUAL( GPDMA_TRANSFER_LIST_ACCESS_APPEND, config.XferListAccessMode );
    TEST_ASSERT_NULL( config.XferList );
    TEST_ASSERT_NULL( config.TransferCompleteIsr );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Get_DefaultConfig( NULL ) );
}

/* ============================ CHANNEL STATE =============================== */

/**
 * \brief   Gpdma_Set_ChannelActive() / Gpdma_Set_ChannelInactive() control channel enable.
 *
 * \details Activates channel 3, reads its state, then deactivates it.
 *
 * \par Expected results
 * - Activation: CCR.EN of channel 3 set, neighbour channel 2 untouched, state ACTIVE.
 * - Deactivation (suspension reported by CSR.SUSPF): CCR.SUSP and CCR.RESET set
 *   (EN is cleared by HW), SUSPF clear written.
 */
void Ut_Gpdma_Set_ChannelActiveInactive_TogglesEnableBit( void )
{
    gpdma_FunctionState_t state = GPDMA_FUNCTION_INACTIVE;

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( GPDMA_PERIPH_1, GPDMA_CHANNEL_3 ) );
    TEST_ASSERT_BITS_HIGH( DMA_CCR_EN, UT_GPDMA1_CH( GPDMA_CHANNEL_3 )->CCR );
    TEST_ASSERT_BITS_LOW( DMA_CCR_EN, UT_GPDMA1_CH( GPDMA_CHANNEL_2 )->CCR );    /* Neighbour channel */
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_ChannelState( GPDMA_PERIPH_1, GPDMA_CHANNEL_3, &state ) );
    TEST_ASSERT_EQUAL( GPDMA_FUNCTION_ACTIVE, state );

    /* Emulated HW: suspension is effective, channel is reset after it, EN is cleared by HW */
    UT_GPDMA1_CH( GPDMA_CHANNEL_3 )->CSR = DMA_CSR_SUSPF;

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelInactive( GPDMA_PERIPH_1, GPDMA_CHANNEL_3 ) );
    TEST_ASSERT_BITS_HIGH( DMA_CCR_SUSP | DMA_CCR_RESET, UT_GPDMA1_CH( GPDMA_CHANNEL_3 )->CCR );
    TEST_ASSERT_BITS_HIGH( DMA_CFCR_SUSPF, UT_GPDMA1_CH( GPDMA_CHANNEL_3 )->CFCR );
}


/**
 * \brief   Gpdma_Set_ChannelInactive() does not reset enabled channel before its
 *          suspension is effective.
 *
 * \details Enabled channel never reports suspension (CSR.SUSPF stays 0) - e.g. a burst
 *          transfer that does not finish.
 *
 * \par Expected results
 * - GPDMA_REQUEST_ERROR, CCR.SUSP set, CCR.RESET not written (HW would ignore it).
 */
void Ut_Gpdma_Set_ChannelInactive_SuspendNotEffective_ReturnsError( void )
{
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelActive( GPDMA_PERIPH_1, GPDMA_CHANNEL_3 ) );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_ChannelInactive( GPDMA_PERIPH_1, GPDMA_CHANNEL_3 ) );
    TEST_ASSERT_BITS_HIGH( DMA_CCR_SUSP,  UT_GPDMA1_CH( GPDMA_CHANNEL_3 )->CCR );
    TEST_ASSERT_BITS_LOW ( DMA_CCR_RESET, UT_GPDMA1_CH( GPDMA_CHANNEL_3 )->CCR );
}


/**
 * \brief   Gpdma_Set_ChannelInactive() resets disabled channel without suspension.
 *
 * \details Channel 3 is not enabled.
 *
 * \par Expected results
 * - GPDMA_REQUEST_OK, CCR.RESET set, CCR.SUSP not set.
 */
void Ut_Gpdma_Set_ChannelInactive_DisabledChannel_ResetOnly( void )
{
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ChannelInactive( GPDMA_PERIPH_1, GPDMA_CHANNEL_3 ) );
    TEST_ASSERT_BITS_HIGH( DMA_CCR_RESET, UT_GPDMA1_CH( GPDMA_CHANNEL_3 )->CCR );
    TEST_ASSERT_BITS_LOW ( DMA_CCR_SUSP,  UT_GPDMA1_CH( GPDMA_CHANNEL_3 )->CCR );
}


/**
 * \brief   Gpdma_Set_PauseActive() / Gpdma_Set_PauseInactive() control channel suspend.
 *
 * \details Pauses channel 1, reads pause state, then resumes the channel.
 *
 * \par Expected results
 * - Pause: CCR.SUSP set, pause state ACTIVE.
 * - Resume: CCR.SUSP cleared.
 */
void Ut_Gpdma_Set_PauseActiveInactive_TogglesSuspendBit( void )
{
    gpdma_FunctionState_t state = GPDMA_FUNCTION_INACTIVE;

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_PauseActive( GPDMA_PERIPH_1, GPDMA_CHANNEL_1 ) );
    TEST_ASSERT_BITS_HIGH( DMA_CCR_SUSP, UT_GPDMA1_CH( GPDMA_CHANNEL_1 )->CCR );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_PauseState( GPDMA_PERIPH_1, GPDMA_CHANNEL_1, &state ) );
    TEST_ASSERT_EQUAL( GPDMA_FUNCTION_ACTIVE, state );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_PauseInactive( GPDMA_PERIPH_1, GPDMA_CHANNEL_1 ) );
    TEST_ASSERT_BITS_LOW( DMA_CCR_SUSP, UT_GPDMA1_CH( GPDMA_CHANNEL_1 )->CCR );
}


/**
 * \brief   Channel functions reject invalid arguments.
 *
 * \details Calls channel functions with invalid peripheral, invalid channel, NULL
 *          read pointer, invalid priority / data size / direction / trigger type
 *          and burst length 0 and above maximum.
 *
 * \par Expected results
 * - GPDMA_REQUEST_ERROR is returned in all cases.
 * - CCR, CTR1 and CTR2 of channel 0 are not written.
 */
void Ut_Gpdma_ChannelFunctions_InvalidArgs_ReturnError( void )
{
    gpdma_FunctionState_t state = GPDMA_FUNCTION_INACTIVE;
    gpdma_Priority_t      prio  = GPDMA_PRIORITY_LOW;

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_ChannelActive( GPDMA_PERIPH_CNT, GPDMA_CHANNEL_0 ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_ChannelActive( GPDMA_PERIPH_1,   GPDMA_CHANNEL_CNT ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Get_ChannelState( GPDMA_PERIPH_1,    GPDMA_CHANNEL_0, NULL ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Get_ChannelState( GPDMA_PERIPH_1,    GPDMA_CHANNEL_CNT, &state ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_Priority( GPDMA_PERIPH_1,        GPDMA_CHANNEL_0, GPDMA_PRIORITY_CNT ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Get_Priority( GPDMA_PERIPH_1,        GPDMA_CHANNEL_CNT, &prio ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_SourceDataSize( GPDMA_PERIPH_1,  GPDMA_CHANNEL_0, GPDMA_DATA_SIZE_CNT ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_Direction( GPDMA_PERIPH_1,       GPDMA_CHANNEL_0, GPDMA_DIR_CNT ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_TriggerType( GPDMA_PERIPH_1,     GPDMA_CHANNEL_0, GPDMA_TRG_TYPE_CNT ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_SourceBurstLength( GPDMA_PERIPH_1, GPDMA_CHANNEL_0, GPDMA_MAX_BURST_LEN + 1u ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_SourceBurstLength( GPDMA_PERIPH_1, GPDMA_CHANNEL_0, 0u ) );

    /* No register written by rejected requests */
    TEST_ASSERT_EQUAL_HEX32( 0u, UT_GPDMA1_CH( GPDMA_CHANNEL_0 )->CCR );
    TEST_ASSERT_EQUAL_HEX32( 0u, UT_GPDMA1_CH( GPDMA_CHANNEL_0 )->CTR1 );
    TEST_ASSERT_EQUAL_HEX32( 0u, UT_GPDMA1_CH( GPDMA_CHANNEL_0 )->CTR2 );
}

/* ======================== CHANNEL CONFIGURATION =========================== */

/**
 * \brief   All channel configuration setters and getters are consistent.
 *
 * \details For every Set / Get pair of channel 6 (2D capable) writes a non-default
 *          value and reads it back: priority, execution mode, source / destination /
 *          configuration port, data sizes, data operations, trigger type / source /
 *          mode, request source, direction, addresses, address modes, burst
 *          lengths, block size, block repeat count and list base address.
 *
 * \par Expected results
 * - Every setter and getter returns GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_ChannelConfig_SetGet_RoundTrip( void )
{
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_Priority,               Gpdma_Get_Priority,               gpdma_Priority_t,      GPDMA_PRIORITY_HIGH );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_XferExecMode,           Gpdma_Get_XferExecMode,           gpdma_XferExecMode_t,  GPDMA_XFER_EXEC_CONTINUOUS );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_SourcePort,             Gpdma_Get_SourcePort,             gpdma_PortId_t,        GPDMA_PORT_1 );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_DestinationPort,        Gpdma_Get_DestinationPort,        gpdma_PortId_t,        GPDMA_PORT_1 );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_ConfigurationPort,      Gpdma_Get_ConfigurationPort,      gpdma_PortId_t,        GPDMA_PORT_1 );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_SourceDataSize,         Gpdma_Get_SourceDataSize,         gpdma_DataSize_t,      GPDMA_DATA_SIZE_32BITS );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_DestinationDataSize,    Gpdma_Get_DestinationDataSize,    gpdma_DataSize_t,      GPDMA_DATA_SIZE_16BITS );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_SourceDataOp,           Gpdma_Get_SourceDataOp,           gpdma_SrcDataOp_t,     GPDMA_SRC_DATA_BYTE_SWAP );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_DestinationDataOp,      Gpdma_Get_DestinationDataOp,      gpdma_DestDataOp_t,    GPDMA_DEST_DATA_BYTE_2BYTES_SWAP );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_TriggerType,            Gpdma_Get_TriggerType,            gpdma_TrgType_t,       GPDMA_TRG_FALLING );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_TriggerSource,          Gpdma_Get_TriggerSource,          gpdma_TrgSrcId_t,      GPDMA_TRG_EXTI_LINE3 );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_TriggerMode,            Gpdma_Get_TriggerMode,            gpdma_TriggerMode_t,   GPDMA_TRIGGER_SINGLE );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_RequestSource,          Gpdma_Get_RequestSource,          gpdma_PeriphReqId_t,   GPDMA_REQ_USART1_TX );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_Direction,              Gpdma_Get_Direction,              gpdma_Direction_t,     GPDMA_DIR_MEMORY_TO_PERIPH );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_SourceAddr,             Gpdma_Get_SourceAddr,             gpdma_SrcAddr_t,       UT_GPDMA_SRC_ADDR );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_DestinationAddr,        Gpdma_Get_DestinationAddr,        gpdma_DstAddr_t,       UT_GPDMA_SRC_ADDR + 0x100u );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_SourceAddrMode,         Gpdma_Get_SourceAddrMode,         gpdma_AddrMode_t,      GPDMA_ADDR_INCREMENT );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_DestinationAddrMode,    Gpdma_Get_DestinationAddrMode,    gpdma_AddrMode_t,      GPDMA_ADDR_INCREMENT );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_SourceBurstLength,      Gpdma_Get_SourceBurstLength,      gpdma_BurstLength_t,   4u );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_DestinationBurstLength, Gpdma_Get_DestinationBurstLength, gpdma_BurstLength_t,   GPDMA_MAX_BURST_LEN );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_BlockSize,              Gpdma_Get_BlockSize,              gpdma_BlockSize_t,     UT_GPDMA_BLOCK_SIZE );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_BlockRepeatCount,       Gpdma_Get_BlockRepeatCount,       gpdma_BlockRep_t,      3u );
    UT_GPDMA_ROUNDTRIP( Gpdma_Set_XferListBaseAddr,       Gpdma_Get_XferListBaseAddr,       gpdma_DataAddr_t,      REGMEM_SRAM_BASE );
}


/**
 * \brief   Channel configuration setters write the right registers.
 *
 * \details Sets source address, block size, source increment and request source
 *          USART1 TX of channel 6.
 *
 * \par Expected results
 * - CSAR = source address, CBR1.BNDT = 16, CTR1.SINC set, CTR2.REQSEL = USART1 TX.
 * - Other channel (7) is not written.
 */
void Ut_Gpdma_ChannelConfig_WritesRegisters( void )
{
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_SourceAddr( GPDMA_PERIPH_1, UT_GPDMA_CH_2D, UT_GPDMA_SRC_ADDR ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_BlockSize( GPDMA_PERIPH_1, UT_GPDMA_CH_2D, UT_GPDMA_BLOCK_SIZE ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_SourceAddrMode( GPDMA_PERIPH_1, UT_GPDMA_CH_2D, GPDMA_ADDR_INCREMENT ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_RequestSource( GPDMA_PERIPH_1, UT_GPDMA_CH_2D, GPDMA_REQ_USART1_TX ) );

    TEST_ASSERT_EQUAL_HEX32( UT_GPDMA_SRC_ADDR,   UT_GPDMA1_CH( UT_GPDMA_CH_2D )->CSAR );
    TEST_ASSERT_EQUAL_HEX32( UT_GPDMA_BLOCK_SIZE, UT_GPDMA1_CH( UT_GPDMA_CH_2D )->CBR1 & DMA_CBR1_BNDT );
    TEST_ASSERT_BITS_HIGH( DMA_CTR1_SINC,         UT_GPDMA1_CH( UT_GPDMA_CH_2D )->CTR1 );
    TEST_ASSERT_EQUAL_HEX32( LL_GPDMA1_REQUEST_USART1_TX, UT_GPDMA1_CH( UT_GPDMA_CH_2D )->CTR2 & DMA_CTR2_REQSEL );

    /* Other channels are not touched */
    TEST_ASSERT_EQUAL_HEX32( 0u, UT_GPDMA1_CH( GPDMA_CHANNEL_7 )->CSAR );
}


/**
 * \brief   Channel configuration is rejected while transfer is in progress.
 *
 * \details Presets CCR.EN of channel 6 and sets priority.
 *
 * \par Expected results
 * - GPDMA_REQUEST_ERROR is returned, CCR keeps only EN.
 */
void Ut_Gpdma_ChannelConfig_ChannelActive_RejectedWithoutWrite( void )
{
    UT_GPDMA1_CH( UT_GPDMA_CH_2D )->CCR = DMA_CCR_EN;   /* Transfer in progress */

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_Priority( GPDMA_PERIPH_1, UT_GPDMA_CH_2D, GPDMA_PRIORITY_VERYHIGH ) );

    TEST_ASSERT_EQUAL_HEX32( DMA_CCR_EN, UT_GPDMA1_CH( UT_GPDMA_CH_2D )->CCR );
}

/* ============================== INTERRUPTS ================================ */

/**
 * \brief   Interrupt enable functions control channel interrupt bits.
 *
 * \details Enables all interrupts of channel 4 (TC, HT, DTE, USE, ULE, TO, SUSP),
 *          then disables transfer complete interrupt.
 *
 * \par Expected results
 * - CCR contains exactly all 7 interrupt enable bits.
 * - After disable: TCIE cleared, HTIE stays set.
 */
void Ut_Gpdma_IrqActiveInactive_TogglesEnableBits( void )
{
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_TransferCompleteIrqActive  ( GPDMA_PERIPH_1, GPDMA_CHANNEL_4 ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_HalfTransferIrqActive      ( GPDMA_PERIPH_1, GPDMA_CHANNEL_4 ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_TransferErrorIrqActive     ( GPDMA_PERIPH_1, GPDMA_CHANNEL_4 ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ConfigErrorIrqActive       ( GPDMA_PERIPH_1, GPDMA_CHANNEL_4 ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ConfigUpdateErrorIrqActive ( GPDMA_PERIPH_1, GPDMA_CHANNEL_4 ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_TriggerOverrunIrqActive    ( GPDMA_PERIPH_1, GPDMA_CHANNEL_4 ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_SuspensionIrqActive        ( GPDMA_PERIPH_1, GPDMA_CHANNEL_4 ) );

    TEST_ASSERT_EQUAL_HEX32( DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_DTEIE | DMA_CCR_USEIE |
                             DMA_CCR_ULEIE | DMA_CCR_TOIE | DMA_CCR_SUSPIE,
                             UT_GPDMA1_CH( GPDMA_CHANNEL_4 )->CCR );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_TransferCompleteIrqInactive( GPDMA_PERIPH_1, GPDMA_CHANNEL_4 ) );
    TEST_ASSERT_BITS_LOW( DMA_CCR_TCIE, UT_GPDMA1_CH( GPDMA_CHANNEL_4 )->CCR );
    TEST_ASSERT_BITS_HIGH( DMA_CCR_HTIE, UT_GPDMA1_CH( GPDMA_CHANNEL_4 )->CCR );
}


/**
 * \brief   Callback setters and getters are consistent.
 *
 * \details Sets and reads back transfer complete, half transfer and error callback
 *          of channel 5, then uses invalid channel and NULL read pointer.
 *
 * \par Expected results
 * - Read callbacks equal the set ones.
 * - Invalid channel and NULL pointer: GPDMA_REQUEST_ERROR.
 */
void Ut_Gpdma_IsrHandlers_SetGet_RoundTrip( void )
{
    gpdma_IsrCallback    *callback    = NULL;
    gpdma_IsrErrCallback *errCallback = NULL;

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_TransferCompleteIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_5, Ut_Gpdma_TcCallback ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_TransferCompleteIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_5, &callback ) );
    TEST_ASSERT_EQUAL_PTR( Ut_Gpdma_TcCallback, callback );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_HalfTransferIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_5, Ut_Gpdma_HtCallback ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_HalfTransferIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_5, &callback ) );
    TEST_ASSERT_EQUAL_PTR( Ut_Gpdma_HtCallback, callback );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_ErrorIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_5, Ut_Gpdma_ErrCallback ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_ErrorIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_5, &errCallback ) );
    TEST_ASSERT_EQUAL_PTR( Ut_Gpdma_ErrCallback, errCallback );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_TransferCompleteIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_CNT, Ut_Gpdma_TcCallback ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Get_TransferCompleteIsrHandler( GPDMA_PERIPH_1, GPDMA_CHANNEL_5, NULL ) );
}

/* =========================== TRANSFER LIST ================================ */

/**
 * \brief   Gpdma_Set_XferList_SrcDataSize() / Gpdma_Get_XferList_SrcDataSize() round trip.
 *
 * \details Writes source data size 16 bit to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_SrcDataSize_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_SrcDataSize, Gpdma_Get_XferList_SrcDataSize, gpdma_DataSize_t, GPDMA_DATA_SIZE_16BITS );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_SrcDataSize, Gpdma_Get_XferList_SrcDataSize, gpdma_DataSize_t, GPDMA_DATA_SIZE_16BITS );
}


/**
 * \brief   Gpdma_Set_XferList_DestDataSize() / Gpdma_Get_XferList_DestDataSize() round trip.
 *
 * \details Writes destination data size 32 bit to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_DestDataSize_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_DestDataSize, Gpdma_Get_XferList_DestDataSize, gpdma_DataSize_t, GPDMA_DATA_SIZE_32BITS );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_DestDataSize, Gpdma_Get_XferList_DestDataSize, gpdma_DataSize_t, GPDMA_DATA_SIZE_32BITS );
}


/**
 * \brief   Gpdma_Set_XferList_SrcDataOp() / Gpdma_Get_XferList_SrcDataOp() round trip.
 *
 * \details Writes source data operation byte swap to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_SrcDataOp_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_SrcDataOp, Gpdma_Get_XferList_SrcDataOp, gpdma_SrcDataOp_t, GPDMA_SRC_DATA_BYTE_SWAP );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_SrcDataOp, Gpdma_Get_XferList_SrcDataOp, gpdma_SrcDataOp_t, GPDMA_SRC_DATA_BYTE_SWAP );
}


/**
 * \brief   Gpdma_Set_XferList_DestDataOp() / Gpdma_Get_XferList_DestDataOp() round trip.
 *
 * \details Writes destination data operation byte swap to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_DestDataOp_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_DestDataOp, Gpdma_Get_XferList_DestDataOp, gpdma_DestDataOp_t, GPDMA_DEST_DATA_BYTE_SWAP );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_DestDataOp, Gpdma_Get_XferList_DestDataOp, gpdma_DestDataOp_t, GPDMA_DEST_DATA_BYTE_SWAP );
}


/**
 * \brief   Gpdma_Set_XferList_SrcAddrMode() / Gpdma_Get_XferList_SrcAddrMode() round trip.
 *
 * \details Writes source address mode increment to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_SrcAddrMode_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_SrcAddrMode, Gpdma_Get_XferList_SrcAddrMode, gpdma_AddrMode_t, GPDMA_ADDR_INCREMENT );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_SrcAddrMode, Gpdma_Get_XferList_SrcAddrMode, gpdma_AddrMode_t, GPDMA_ADDR_INCREMENT );
}


/**
 * \brief   Gpdma_Set_XferList_DestAddrMode() / Gpdma_Get_XferList_DestAddrMode() round trip.
 *
 * \details Writes destination address mode increment to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_DestAddrMode_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_DestAddrMode, Gpdma_Get_XferList_DestAddrMode, gpdma_AddrMode_t, GPDMA_ADDR_INCREMENT );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_DestAddrMode, Gpdma_Get_XferList_DestAddrMode, gpdma_AddrMode_t, GPDMA_ADDR_INCREMENT );
}


/**
 * \brief   Gpdma_Set_XferList_TriggerType() / Gpdma_Get_XferList_TriggerType() round trip.
 *
 * \details Writes trigger type rising edge to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_TriggerType_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_TriggerType, Gpdma_Get_XferList_TriggerType, gpdma_TrgType_t, GPDMA_TRG_RISING );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_TriggerType, Gpdma_Get_XferList_TriggerType, gpdma_TrgType_t, GPDMA_TRG_RISING );
}


/**
 * \brief   Gpdma_Set_XferList_TriggerSrc() / Gpdma_Get_XferList_TriggerSrc() round trip.
 *
 * \details Writes trigger source EXTI line 5 to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_TriggerSrc_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_TriggerSrc, Gpdma_Get_XferList_TriggerSrc, gpdma_TrgSrcId_t, GPDMA_TRG_EXTI_LINE5 );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_TriggerSrc, Gpdma_Get_XferList_TriggerSrc, gpdma_TrgSrcId_t, GPDMA_TRG_EXTI_LINE5 );
}


/**
 * \brief   Gpdma_Set_XferList_TriggerMode() / Gpdma_Get_XferList_TriggerMode() round trip.
 *
 * \details Writes trigger mode 2D block to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_TriggerMode_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_TriggerMode, Gpdma_Get_XferList_TriggerMode, gpdma_TriggerMode_t, GPDMA_TRIGGER_2D_BLOCK );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_TriggerMode, Gpdma_Get_XferList_TriggerMode, gpdma_TriggerMode_t, GPDMA_TRIGGER_2D_BLOCK );
}


/**
 * \brief   Gpdma_Set_XferList_RequestMode() / Gpdma_Get_XferList_RequestMode() round trip.
 *
 * \details Writes request mode block to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_RequestMode_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_RequestMode, Gpdma_Get_XferList_RequestMode, gpdma_PeriphReqMode_t, GPDMA_PERIPH_REQ_BLOCK );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_RequestMode, Gpdma_Get_XferList_RequestMode, gpdma_PeriphReqMode_t, GPDMA_PERIPH_REQ_BLOCK );
}


/**
 * \brief   Gpdma_Set_XferList_RequestSrc() / Gpdma_Get_XferList_RequestSrc() round trip.
 *
 * \details Writes request source SPI1 RX to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_RequestSrc_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_RequestSrc, Gpdma_Get_XferList_RequestSrc, gpdma_PeriphReqId_t, GPDMA_REQ_SPI1_RX );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_RequestSrc, Gpdma_Get_XferList_RequestSrc, gpdma_PeriphReqId_t, GPDMA_REQ_SPI1_RX );
}


/**
 * \brief   Gpdma_Set_XferList_BlockSize() / Gpdma_Get_XferList_BlockSize() round trip.
 *
 * \details Writes block size 16 bytes to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_BlockSize_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_BlockSize, Gpdma_Get_XferList_BlockSize, gpdma_BlockSize_t, UT_GPDMA_BLOCK_SIZE );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_BlockSize, Gpdma_Get_XferList_BlockSize, gpdma_BlockSize_t, UT_GPDMA_BLOCK_SIZE );
}


/**
 * \brief   Gpdma_Set_XferList_BlockRepeatCnt() / Gpdma_Get_XferList_BlockRepeatCnt() round trip.
 *
 * \details Writes block repeat count 5 to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK.
 * - GPDMA_CHANNEL_LINEAR: read value 0 (block repeat count used only by 2D channel).
 * - GPDMA_CHANNEL_LINEAR_2D: read value equals written value.
 * - Maximal value 0x7FF (11-bit BRC field) is accepted, 0x800 returns GPDMA_REQUEST_ERROR.
 */
void Ut_Gpdma_XferList_BlockRepeatCnt_RoundTrip( void )
{
    volatile gpdma_XferList_t * const node = REGMEM_SRAM_PTR( volatile gpdma_XferList_t, UT_GPDMA_LIST_OFFSET_A );
    gpdma_BlockRep_t                  readValue = 0xFFu;

    /* Block repeat count is not used by linear channel - field is written 0 */
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_XferList_BlockRepeatCnt( node, GPDMA_CHANNEL_LINEAR, 5u ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_XferList_BlockRepeatCnt( node, GPDMA_CHANNEL_LINEAR, &readValue ) );
    TEST_ASSERT_EQUAL_UINT32( 0u, readValue );

    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_BlockRepeatCnt, Gpdma_Get_XferList_BlockRepeatCnt, gpdma_BlockRep_t, 5u );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_BlockRepeatCnt, Gpdma_Get_XferList_BlockRepeatCnt, gpdma_BlockRep_t, 0x7FFu );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_XferList_BlockRepeatCnt( node, GPDMA_CHANNEL_LINEAR_2D, 0x800u ) );
}


/**
 * \brief   Gpdma_Set_XferList_SrcAddr() / Gpdma_Get_XferList_SrcAddr() round trip.
 *
 * \details Writes source address (emulated SRAM) to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_SrcAddr_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_SrcAddr, Gpdma_Get_XferList_SrcAddr, gpdma_SrcAddr_t, UT_GPDMA_SRC_ADDR );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_SrcAddr, Gpdma_Get_XferList_SrcAddr, gpdma_SrcAddr_t, UT_GPDMA_SRC_ADDR );
}


/**
 * \brief   Gpdma_Set_XferList_DestAddr() / Gpdma_Get_XferList_DestAddr() round trip.
 *
 * \details Writes destination address (source + 0x40) to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_DestAddr_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_DestAddr, Gpdma_Get_XferList_DestAddr, gpdma_DstAddr_t, ( UT_GPDMA_SRC_ADDR + 0x40u ) );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_DestAddr, Gpdma_Get_XferList_DestAddr, gpdma_DstAddr_t, ( UT_GPDMA_SRC_ADDR + 0x40u ) );
}


/**
 * \brief   Gpdma_Set_XferList_NextXferAddr() / Gpdma_Get_XferList_NextXferAddr() round trip.
 *
 * \details Writes address of the next node to a transfer list node in emulated SRAM
 *          by the setter and reads it back by the getter. Done for channel type
 *          GPDMA_CHANNEL_LINEAR and GPDMA_CHANNEL_LINEAR_2D.
 *
 * \par Expected results
 * - Setter and getter return GPDMA_REQUEST_OK, read value equals written value.
 */
void Ut_Gpdma_XferList_NextXferAddr_RoundTrip( void )
{
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR,    Gpdma_Set_XferList_NextXferAddr, Gpdma_Get_XferList_NextXferAddr, gpdma_DataAddr_t, ( REGMEM_SRAM_BASE + UT_GPDMA_LIST_OFFSET_A + 0x20u ) );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_NextXferAddr, Gpdma_Get_XferList_NextXferAddr, gpdma_DataAddr_t, ( REGMEM_SRAM_BASE + UT_GPDMA_LIST_OFFSET_A + 0x20u ) );
}


/**
 * \brief   Transfer list node functions reject invalid arguments.
 *
 * \details Calls source data size setter / getter with NULL node, invalid data
 *          size and NULL read pointer.
 *
 * \par Expected results
 * - GPDMA_REQUEST_ERROR is returned in all cases.
 */
void Ut_Gpdma_XferList_InvalidArgs_ReturnError( void )
{
    volatile gpdma_XferList_t * const node = REGMEM_SRAM_PTR( volatile gpdma_XferList_t, UT_GPDMA_LIST_OFFSET_A );
    gpdma_DataSize_t dataSize = GPDMA_DATA_SIZE_8BITS;

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_XferList_SrcDataSize( NULL, GPDMA_CHANNEL_LINEAR,     GPDMA_DATA_SIZE_8BITS ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_XferList_SrcDataSize( node, GPDMA_CHANNEL_LINEAR,     GPDMA_DATA_SIZE_CNT ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Get_XferList_SrcDataSize( node, GPDMA_CHANNEL_LINEAR,     NULL ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Get_XferList_SrcDataSize( NULL, GPDMA_CHANNEL_LINEAR,     &dataSize ) );
}


/**
 * \brief   Burst length of transfer list node - round trip (known defect).
 *
 * \details Writes source burst length 8 and destination burst length 2 to a node
 *          in emulated SRAM and reads them back. Ignored unless known defects are run.
 *
 * \par Expected results
 * - Read burst lengths equal written ones (fails now - getters mask by bit
 *   position instead of mask and do not shift).
 */
void Ut_Gpdma_XferList_BurstLength_SetGet_RoundTrip( void )
{
    UT_KNOWN_DEFECT( "Gpdma_Get_XferList_SrcBurstLen / DestBurstLen mask node register with DMA_CTR1_xBL_1_Pos "
                     "instead of _Msk and do not shift - wrong burst length is returned" );

    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_SrcBurstLen,  Gpdma_Get_XferList_SrcBurstLen,  gpdma_BurstLength_t, 8u );
    UT_GPDMA_LIST_ROUNDTRIP( GPDMA_CHANNEL_LINEAR_2D, Gpdma_Set_XferList_DestBurstLen, Gpdma_Get_XferList_DestBurstLen, gpdma_BurstLength_t, 2u );
}


/**
 * \brief   Transfer complete event mode of transfer list node - round trip (known defect).
 *
 * \details For every transfer event mode writes it to a node and reads it back.
 *          Ignored unless known defects are run.
 *
 * \par Expected results
 * - Read event mode equals written one (fails now - inverted conditions).
 */
void Ut_Gpdma_XferList_XferCpltEvent_WritesTcemAndRoundTrips( void )
{
    volatile gpdma_XferList_t * const node = REGMEM_SRAM_PTR( volatile gpdma_XferList_t, UT_GPDMA_LIST_OFFSET_A );

    UT_KNOWN_DEFECT( "Gpdma_Set/Get_XferList_XferCpltEvent use inverted conditions (!= instead of ==) - "
                     "wrong TCEM is written and read for every event mode" );

    for( uint32_t eventId = 0u; GPDMA_TRANSFER_EVENT_CNT > eventId; eventId++ )
    {
        gpdma_TransferEvent_t readEvent = GPDMA_TRANSFER_EVENT_CNT;

        TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Set_XferList_XferCpltEvent( node, GPDMA_CHANNEL_LINEAR_2D, (gpdma_TransferEvent_t)eventId ) );
        TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_XferList_XferCpltEvent( node, GPDMA_CHANNEL_LINEAR_2D, &readEvent ) );
        TEST_ASSERT_EQUAL( eventId, readEvent );
    }
}


/**
 * \brief   Transfer list functions reject invalid channel type (known defect).
 *
 * \details Sets source data size of a node with channel type GPDMA_CHANNEL_OPTION_CNT.
 *          Ignored unless known defects are run.
 *
 * \par Expected results
 * - GPDMA_REQUEST_ERROR (fails now - invalid type is handled as linear).
 */
void Ut_Gpdma_XferList_InvalidChannelType_ReturnsError( void )
{
    volatile gpdma_XferList_t * const node = REGMEM_SRAM_PTR( volatile gpdma_XferList_t, UT_GPDMA_LIST_OFFSET_A );

    UT_KNOWN_DEFECT( "Gpdma_Set/Get_XferList_* do not validate channelType (GPDMA_CHANNEL_OPTION_CNT is handled as linear)" );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Set_XferList_SrcDataSize( node, GPDMA_CHANNEL_OPTION_CNT, GPDMA_DATA_SIZE_8BITS ) );
}

/* ============================ INITIALIZATION ============================== */

/**
 * \brief   Gpdma_Init() configures channel for single memory to USART1 transfer.
 *
 * \details Initializes channel 2 with one transfer (memory -> USART1 TDR, 16 bytes,
 *          source increment), list in emulated SRAM. GPDMA1 clock is inactive -
 *          activation in RCC is expected. ISR is captured by NVIC stub.
 *
 * \par Expected results
 * - GPDMA_REQUEST_OK.
 * - CSAR = source buffer, CDAR = USART1 TDR, CBR1.BNDT = 16, REQSEL = USART1 TX,
 *   DREQ set (destination request), SINC set.
 * - CLBAR = upper half of list address.
 * - TCIE and DTEIE set, channel not enabled (started by application).
 * - Channel ISR registered in NVIC.
 */
void Ut_Gpdma_Init_SingleTransfer_ConfiguresChannel( void )
{
    gpdma_ConfigStruct_t config;

    Ut_Gpdma_Get_Config( &config, UT_GPDMA_CH_LINEAR, UT_GPDMA_LIST_OFFSET_A );
    Ut_Gpdma_Expect_ClockActivation( RCC_FUNCTION_INACTIVE );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    TEST_ASSERT_EQUAL_HEX32( UT_GPDMA_SRC_ADDR,                        UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CSAR );
    TEST_ASSERT_EQUAL_HEX32( (uint32_t)(uintptr_t)&USART1->TDR,         UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CDAR );
    TEST_ASSERT_EQUAL_HEX32( UT_GPDMA_BLOCK_SIZE,                      UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CBR1 & DMA_CBR1_BNDT );
    TEST_ASSERT_EQUAL_HEX32( LL_GPDMA1_REQUEST_USART1_TX,              UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CTR2 & DMA_CTR2_REQSEL );
    TEST_ASSERT_BITS_HIGH( DMA_CTR2_DREQ,                              UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CTR2 );   /* Destination (peripheral) request */
    TEST_ASSERT_BITS_HIGH( DMA_CTR1_SINC,                              UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CTR1 );
    TEST_ASSERT_EQUAL_HEX32( REGMEM_SRAM_BASE & UT_GPDMA_UPPER_HALF_MASK, UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CLBAR & DMA_CLBAR_LBA );
    TEST_ASSERT_BITS_HIGH( DMA_CCR_TCIE | DMA_CCR_DTEIE,               UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CCR );
    TEST_ASSERT_BITS_LOW( DMA_CCR_EN,                                  UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CCR );    /* Started by application */
    TEST_ASSERT_NOT_NULL( utGpdma_ChannelIsr );
}


/**
 * \brief   Gpdma_Init() rejects invalid configuration.
 *
 * \details Calls Gpdma_Init() with NULL configuration, without transfer list and
 *          with invalid channel.
 *
 * \par Expected results
 * - GPDMA_REQUEST_ERROR is returned in all cases, RCC and NVIC are not called.
 */
void Ut_Gpdma_Init_InvalidConfig_ReturnsErrorWithoutAccess( void )
{
    gpdma_ConfigStruct_t config;

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Init( NULL ) );

    Ut_Gpdma_Get_Config( &config, UT_GPDMA_CH_LINEAR, UT_GPDMA_LIST_OFFSET_A );
    config.XferList = NULL;
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Init( &config ) );

    Ut_Gpdma_Get_Config( &config, UT_GPDMA_CH_LINEAR, UT_GPDMA_LIST_OFFSET_A );
    config.ChannelId = GPDMA_CHANNEL_CNT;
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Init( &config ) );
}


/**
 * \brief   Gpdma_Init() is rejected for channel with transfer in progress.
 *
 * \details Presets CCR.EN of channel 2 (clock already active) and initializes it.
 *
 * \par Expected results
 * - GPDMA_REQUEST_ERROR, CSAR is not written.
 */
void Ut_Gpdma_Init_ChannelActive_ReturnsError( void )
{
    gpdma_ConfigStruct_t config;

    Ut_Gpdma_Get_Config( &config, UT_GPDMA_CH_LINEAR, UT_GPDMA_LIST_OFFSET_A );
    UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CCR = DMA_CCR_EN;

    Ut_Gpdma_Expect_ClockActivation( RCC_FUNCTION_ACTIVE );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( 0u, UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CSAR );
}


/**
 * \brief   Gpdma_Init() with single list access mode refuses second initialization.
 *
 * \details Initializes channel 2 with GPDMA_TRANSFER_LIST_ACCESS_SINGLE twice.
 *
 * \par Expected results
 * - First call GPDMA_REQUEST_OK, second call GPDMA_REQUEST_ERROR (channel already
 *   owns a transfer list).
 */
void Ut_Gpdma_Init_SecondSingleAccess_ReturnsError( void )
{
    gpdma_ConfigStruct_t config;

    Ut_Gpdma_Get_Config( &config, UT_GPDMA_CH_LINEAR, UT_GPDMA_LIST_OFFSET_A );
    config.XferListAccessMode = GPDMA_TRANSFER_LIST_ACCESS_SINGLE;
    Ut_Gpdma_Expect_ClockActivation( RCC_FUNCTION_INACTIVE );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    /* Channel already owns a transfer list */
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Init( &config ) );
}

/* ========================= INTERRUPT HANDLING ============================= */

/**
 * \brief   Channel ISR handles transfer complete and half transfer.
 *
 * \details Initializes channel 2 with TC and HT callbacks, presets CSR TCF and HTF
 *          and calls captured ISR.
 *
 * \par Expected results
 * - TC and HT callbacks called once, error callback not called.
 * - CFCR is written per flag in event order (HT before TC) - emulated register
 *   keeps the last one (TCF).
 */
void Ut_Gpdma_Isr_TransferCompleteAndHalf_CallsCallbacksAndClearsFlags( void )
{
    gpdma_ConfigStruct_t config;

    Ut_Gpdma_Get_Config( &config, UT_GPDMA_CH_LINEAR, UT_GPDMA_LIST_OFFSET_A );
    config.HalfTransferIsr = Ut_Gpdma_HtCallback;
    Ut_Gpdma_Expect_ClockActivation( RCC_FUNCTION_INACTIVE );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CSR = DMA_CSR_TCF | DMA_CSR_HTF;

    utGpdma_ChannelIsr();

    TEST_ASSERT_EQUAL_UINT32( 1u, utGpdma_TcCnt );
    TEST_ASSERT_EQUAL_UINT32( 1u, utGpdma_HtCnt );
    TEST_ASSERT_EQUAL_UINT32( 0u, utGpdma_ErrCnt );

    /* CFCR is written per flag (WRITE_REG), HT before TC - emulated register keeps the last one */
    TEST_ASSERT_EQUAL_HEX32( DMA_CFCR_TCF, UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CFCR );
}


/**
 * \brief   Channel ISR reports transfer error.
 *
 * \details Initializes channel 2 with error mask GPDMA_ERROR_TRANSFER, presets CSR
 *          DTEF and calls captured ISR.
 *
 * \par Expected results
 * - TC callback not called, error callback once with GPDMA_ERROR_TRANSFER.
 * - DTEF is cleared (CFCR.DTEF written).
 */
void Ut_Gpdma_Isr_TransferError_CallsErrorCallback( void )
{
    gpdma_ConfigStruct_t config;

    Ut_Gpdma_Get_Config( &config, UT_GPDMA_CH_LINEAR, UT_GPDMA_LIST_OFFSET_A );
    Ut_Gpdma_Expect_ClockActivation( RCC_FUNCTION_INACTIVE );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &config ) );

    UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CSR = DMA_CSR_DTEF;

    utGpdma_ChannelIsr();

    TEST_ASSERT_EQUAL_UINT32( 0u, utGpdma_TcCnt );
    TEST_ASSERT_EQUAL_UINT32( 1u, utGpdma_ErrCnt );
    TEST_ASSERT_EQUAL( GPDMA_ERROR_TRANSFER, utGpdma_LastError );
    TEST_ASSERT_BITS_HIGH( DMA_CFCR_DTEF, UT_GPDMA1_CH( UT_GPDMA_CH_LINEAR )->CFCR );
}

/* =========================== DEINITIALIZATION ============================= */

/**
 * \brief   Gpdma_Deinit() keeps GPDMA clock while other channel is used (known defect).
 *
 * \details Initializes channels 2 and 3, deinitializes channel 2. Only clock state
 *          read is allowed. Ignored unless known defects are run.
 *
 * \par Expected results
 * - GPDMA_REQUEST_OK and GPDMA1 clock is not disabled (fails now - clock of whole
 *   peripheral is disabled, channel itself is not disabled).
 */
void Ut_Gpdma_Deinit_OtherChannelInUse_KeepsPeripheralClock( void )
{
    gpdma_ConfigStruct_t configA;
    gpdma_ConfigStruct_t configB;

    UT_KNOWN_DEFECT( "Gpdma_Deinit disables RCC clock of whole GPDMA peripheral although other channels are initialized, "
                     "the channel itself (EN, interrupts) is not disabled" );

    Ut_Gpdma_Get_Config( &configA, GPDMA_CHANNEL_2, UT_GPDMA_LIST_OFFSET_A );
    Ut_Gpdma_Get_Config( &configB, GPDMA_CHANNEL_3, UT_GPDMA_LIST_OFFSET_B );

    Ut_Gpdma_Expect_ClockActivation( RCC_FUNCTION_INACTIVE );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &configA ) );
    Ut_Gpdma_Expect_ClockActivation( RCC_FUNCTION_ACTIVE );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Init( &configB ) );

    /* Clock state may be read, but GPDMA1 clock must not be disabled (channel 3 in use) */
    Rcc_Get_PeriphState_IgnoreAndReturn( RCC_REQUEST_OK );

    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Deinit( GPDMA_PERIPH_1, GPDMA_CHANNEL_2 ) );
}


/**
 * \brief   Gpdma_Deinit() rejects invalid arguments.
 *
 * \details Calls Gpdma_Deinit() with invalid peripheral and invalid channel.
 *
 * \par Expected results
 * - GPDMA_REQUEST_ERROR is returned in both cases.
 */
void Ut_Gpdma_Deinit_InvalidArgs_ReturnsError( void )
{
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Deinit( GPDMA_PERIPH_CNT, GPDMA_CHANNEL_0 ) );
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_ERROR, Gpdma_Deinit( GPDMA_PERIPH_1,   GPDMA_CHANNEL_CNT ) );
}

/* =========================== LOCAL FUNCTIONS ============================== */

/** Stub of Nvic_Set_PeriphIrq_Handler - stores registered channel ISR */
static nvic_RequestState_t Ut_Gpdma_NvicSetHandlerStub( nvic_PeriphIrqList_t irqId, const nvic_IsrCallback_t irqHandler, int callCnt )
{
    (void)irqId;
    (void)callCnt;

    utGpdma_ChannelIsr = irqHandler;

    return ( NVIC_REQUEST_OK );
}


static void Ut_Gpdma_TcCallback( void )
{
    utGpdma_TcCnt++;
}


static void Ut_Gpdma_HtCallback( void )
{
    utGpdma_HtCnt++;
}


static void Ut_Gpdma_ErrCallback( gpdma_ErrorMaskId_t errorId )
{
    utGpdma_ErrCnt++;
    utGpdma_LastError = errorId;
}


/** Returns memory to USART1 transfer configuration */
static void Ut_Gpdma_Get_XferConfig( gpdma_TransferConfig_t * const xferConfig )
{
    *xferConfig = (gpdma_TransferConfig_t){ 0 };

    xferConfig->Direction              = GPDMA_DIR_MEMORY_TO_PERIPH;
    xferConfig->XferListExecMode       = GPDMA_XFER_LIST_EXEC_ONCE;
    xferConfig->EventMode              = GPDMA_TRANSFER_EVENT_BLOCK;
    xferConfig->TriggerType            = GPDMA_TRG_NOT_USED;
    xferConfig->TriggerSource          = GPDMA_TRG_EXTI_LINE0;
    xferConfig->TriggerMode            = GPDMA_TRIGGER_BLOCK;
    xferConfig->RequestSource          = GPDMA_REQ_USART1_TX;
    xferConfig->RequestMode            = GPDMA_PERIPH_REQ_SINGLE;
    xferConfig->BlockSize              = UT_GPDMA_BLOCK_SIZE;
    xferConfig->BlockRepetitionCount   = 0u;
    xferConfig->SourceAddr             = UT_GPDMA_SRC_ADDR;
    xferConfig->SourceDataSize         = GPDMA_DATA_SIZE_8BITS;
    xferConfig->SourceBurstLength      = 1u;
    xferConfig->SourceAddrMode         = GPDMA_ADDR_INCREMENT;
    xferConfig->SourcePortId           = GPDMA_PORT_DEFAULT;
    xferConfig->SourceDataOp           = GPDMA_SRC_DATA_PRESERVE;
    xferConfig->DestinationAddr        = (gpdma_DstAddr_t)(uintptr_t)&USART1->TDR;
    xferConfig->DestinationDataSize    = GPDMA_DATA_SIZE_8BITS;
    xferConfig->DestinationBurstLength = 1u;
    xferConfig->DestinationAddrMode    = GPDMA_ADDR_STATIC;
    xferConfig->DestinationPortId      = GPDMA_PORT_DEFAULT;
    xferConfig->DestinationDataOp      = GPDMA_DEST_DATA_PRESERVE;
}


/** Returns channel configuration with single transfer, list in emulated SRAM */
static void Ut_Gpdma_Get_Config( gpdma_ConfigStruct_t * const config, gpdma_ChannelId_t channelId, uint32_t sramOffset )
{
    TEST_ASSERT_EQUAL( GPDMA_REQUEST_OK, Gpdma_Get_DefaultConfig( config ) );

    Ut_Gpdma_Get_XferConfig( &utGpdma_XferConfig[ 0 ] );

    config->ChannelId           = channelId;
    config->TransferConfig      = utGpdma_XferConfig;
    config->TransfersCount      = 1u;
    config->XferList            = REGMEM_SRAM_PTR( volatile gpdma_XferList_t, sramOffset );
    config->TransferCompleteIsr = Ut_Gpdma_TcCallback;
    config->ErrorIsr            = Ut_Gpdma_ErrCallback;
    config->ErrorMask           = GPDMA_ERROR_TRANSFER;
}


/** Expects GPDMA1 clock state request (and activation if clock is inactive) */
static void Ut_Gpdma_Expect_ClockActivation( rcc_FunctionState_t clockState )
{
    static rcc_FunctionState_t state;

    state = clockState;

    Rcc_Get_PeriphState_ExpectAndReturn( RCC_PERIPH_GPDMA1, NULL, RCC_REQUEST_OK );
    Rcc_Get_PeriphState_IgnoreArg_funcState();
    Rcc_Get_PeriphState_ReturnThruPtr_funcState( &state );

    if( RCC_FUNCTION_INACTIVE == clockState )
    {
        Rcc_Set_PeriphActive_ExpectAndReturn( RCC_PERIPH_GPDMA1, RCC_REQUEST_OK );
    }
    else
    {
        /* Clock already active */
    }
}
