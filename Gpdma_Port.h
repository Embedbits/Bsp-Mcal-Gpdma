/**
 * \author Mr.Nobody
 * \file Gpdma_Port.h
 * \ingroup Gpdma
 * \brief General-Purpose DMA module public functionality
 *
 * This file contains all available public functionality, any other files shall 
 * not used outside of the module.
 *
 */

#ifndef GPDMA_GPDMA_PORT_H
#define GPDMA_GPDMA_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================== INCLUDES ================================== */
#include "Gpdma_Types.h"                    /* Module types definition        */
/* ============================== TYPEDEFS ================================== */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/* ========================== EXPORTED MACROS =============================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */

gpdma_ModuleVersion_t   Gpdma_Get_ModuleVersion             ( void );

gpdma_RequestState_t    Gpdma_Get_DefaultConfig             ( gpdma_ConfigStruct_t * const configStruct );

gpdma_RequestState_t    Gpdma_Init                          ( gpdma_ConfigStruct_t * const configStruct );
gpdma_RequestState_t    Gpdma_Deinit                        ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
void                    Gpdma_Task                          ( void );

gpdma_RequestState_t    Gpdma_Get_XferListConfig            ( gpdma_TransferConfig_t * const transferConfig,
                                                              gpdma_TransfersCount_t transferCount,
                                                              gpdma_ChannelType_t channelType,
                                                              volatile gpdma_XferList_t * transferList );

/*------------------------- Channel state functionality ----------------------*/

gpdma_RequestState_t    Gpdma_Set_ChannelActive             ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Set_ChannelInactive           ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Get_ChannelState              ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_FunctionState_t * const channelState );

gpdma_RequestState_t    Gpdma_Set_PauseActive               ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Set_PauseInactive             ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Get_PauseState                ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_FunctionState_t * const channelState );

/*------------------- Transfer configuration functionality -------------------*/

gpdma_RequestState_t    Gpdma_Set_XferList_SrcPort          ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PortId_t sourcePort );
gpdma_RequestState_t    Gpdma_Get_XferList_SrcPort          ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PortId_t * const sourcePort );

gpdma_RequestState_t    Gpdma_Set_XferList_DestPort         ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PortId_t destPort );
gpdma_RequestState_t    Gpdma_Get_XferList_DestPort         ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PortId_t * const destPort );

gpdma_RequestState_t    Gpdma_Set_XferList_SrcDataSize      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataSize_t srcDataSize );
gpdma_RequestState_t    Gpdma_Get_XferList_SrcDataSize      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataSize_t * const srcDataSize );

gpdma_RequestState_t    Gpdma_Set_XferList_DestDataSize     ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataSize_t destDataSize );
gpdma_RequestState_t    Gpdma_Get_XferList_DestDataSize     ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataSize_t * const destDataSize );

gpdma_RequestState_t    Gpdma_Set_XferList_SrcDataOp        ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_SrcDataOp_t srcDataOp );
gpdma_RequestState_t    Gpdma_Get_XferList_SrcDataOp        ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_SrcDataOp_t * const srcDataOp );

gpdma_RequestState_t    Gpdma_Set_XferList_DestDataOp       ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DestDataOp_t destDataOp );
gpdma_RequestState_t    Gpdma_Get_XferList_DestDataOp       ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DestDataOp_t * const destDataOp );


gpdma_RequestState_t    Gpdma_Set_XferList_SrcBurstLen      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BurstLength_t srcBurstLen );
gpdma_RequestState_t    Gpdma_Get_XferList_SrcBurstLen      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BurstLength_t * const srcBurstLen );

gpdma_RequestState_t    Gpdma_Set_XferList_DestBurstLen     ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BurstLength_t destBurstLen );
gpdma_RequestState_t    Gpdma_Get_XferList_DestBurstLen     ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BurstLength_t * const destBurstLen );

gpdma_RequestState_t    Gpdma_Set_XferList_SrcAddrMode      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_AddrMode_t srcAddrMode );
gpdma_RequestState_t    Gpdma_Get_XferList_SrcAddrMode      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_AddrMode_t * const srcAddrMode );

gpdma_RequestState_t    Gpdma_Set_XferList_DestAddrMode     ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_AddrMode_t destAddrMode );
gpdma_RequestState_t    Gpdma_Get_XferList_DestAddrMode     ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_AddrMode_t * const destAddrMode );


gpdma_RequestState_t    Gpdma_Set_XferList_XferCpltEvent    ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TransferEvent_t eventId );
gpdma_RequestState_t    Gpdma_Get_XferList_XferCpltEvent    ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TransferEvent_t * const eventId );

gpdma_RequestState_t    Gpdma_Set_XferList_TriggerType      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgType_t triggerType );
gpdma_RequestState_t    Gpdma_Get_XferList_TriggerType      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgType_t * const triggerType );

gpdma_RequestState_t    Gpdma_Set_XferList_TriggerSrc       ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgSrcId_t triggerSrc );
gpdma_RequestState_t    Gpdma_Get_XferList_TriggerSrc       ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgSrcId_t * const triggerSrc );

gpdma_RequestState_t    Gpdma_Set_XferList_TriggerMode      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TriggerMode_t triggerMode );
gpdma_RequestState_t    Gpdma_Get_XferList_TriggerMode      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TriggerMode_t * const triggerMode );

gpdma_RequestState_t    Gpdma_Set_XferList_RequestMode      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PeriphReqMode_t requestMode );
gpdma_RequestState_t    Gpdma_Get_XferList_RequestMode      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PeriphReqMode_t * const requestMode );

gpdma_RequestState_t    Gpdma_Set_XferList_Direction        ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_Direction_t direction );
gpdma_RequestState_t    Gpdma_Get_XferList_Direction        ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_Direction_t * const direction );

gpdma_RequestState_t    Gpdma_Set_XferList_RequestSrc       ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PeriphReqId_t requestSrc );
gpdma_RequestState_t    Gpdma_Get_XferList_RequestSrc       ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PeriphReqId_t * const requestSrc );


gpdma_RequestState_t    Gpdma_Set_XferList_BlockSize        ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BlockSize_t blockSize );
gpdma_RequestState_t    Gpdma_Get_XferList_BlockSize        ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BlockSize_t * const blockSize );

gpdma_RequestState_t    Gpdma_Set_XferList_BlockRepeatCnt   ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BlockRep_t blockRepCnt );
gpdma_RequestState_t    Gpdma_Get_XferList_BlockRepeatCnt   ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BlockRep_t * const blockRepCnt );


gpdma_RequestState_t    Gpdma_Set_XferList_SrcAddr          ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_SrcAddr_t sourceAddr );
gpdma_RequestState_t    Gpdma_Get_XferList_SrcAddr          ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_SrcAddr_t * const sourceAddr );

gpdma_RequestState_t    Gpdma_Set_XferList_DestAddr         ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DstAddr_t destAddr );
gpdma_RequestState_t    Gpdma_Get_XferList_DestAddr         ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DstAddr_t * const destAddr );


gpdma_RequestState_t    Gpdma_Set_XferList_DstOffset2D      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_ByteCnt_t blockOffset, gpdma_ByteCnt_t repBlockOffset );
gpdma_RequestState_t    Gpdma_Get_XferList_DstOffset2D      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_ByteCnt_t * const blockOffset, gpdma_ByteCnt_t * const repBlockOffset );

gpdma_RequestState_t    Gpdma_Set_XferList_SrcOffset2D      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_ByteCnt_t blockOffset, gpdma_ByteCnt_t repBlockOffset );
gpdma_RequestState_t    Gpdma_Get_XferList_SrcOffset2D      ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_ByteCnt_t * const blockOffset, gpdma_ByteCnt_t * const repBlockOffset );


gpdma_RequestState_t    Gpdma_Set_XferList_NextXferAddr     ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataAddr_t destAddr );
gpdma_RequestState_t    Gpdma_Get_XferList_NextXferAddr     ( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataAddr_t * const destAddr );

/*-------------------------- Primitive functionality -------------------------*/

gpdma_RequestState_t    Gpdma_Set_Priority                  ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_Priority_t channelPrio );
gpdma_RequestState_t    Gpdma_Get_Priority                  ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_Priority_t * const channelPrio );

gpdma_RequestState_t    Gpdma_Set_XferExecMode              ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_XferExecMode_t xferExecMode );
gpdma_RequestState_t    Gpdma_Get_XferExecMode              ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_XferExecMode_t * const xferExecMode );

gpdma_RequestState_t    Gpdma_Set_SourcePort                ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_PortId_t sourcePort );
gpdma_RequestState_t    Gpdma_Get_SourcePort                ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_PortId_t * const sourcePort );

gpdma_RequestState_t    Gpdma_Set_DestinationPort           ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_PortId_t destPort );
gpdma_RequestState_t    Gpdma_Get_DestinationPort           ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_PortId_t * const destPort );

gpdma_RequestState_t    Gpdma_Set_ConfigurationPort         ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_PortId_t configPort );
gpdma_RequestState_t    Gpdma_Get_ConfigurationPort         ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_PortId_t * const configPort );

gpdma_RequestState_t    Gpdma_Set_XferListBaseAddr          ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_DataAddr_t baseAddr );
gpdma_RequestState_t    Gpdma_Get_XferListBaseAddr          ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_DataAddr_t * const baseAddr );


gpdma_RequestState_t    Gpdma_Set_SourceDataSize            ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_DataSize_t srcDataSize );
gpdma_RequestState_t    Gpdma_Get_SourceDataSize            ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_DataSize_t * const srcDataSize );

gpdma_RequestState_t    Gpdma_Set_DestinationDataSize       ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_DataSize_t destDataSize );
gpdma_RequestState_t    Gpdma_Get_DestinationDataSize       ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_DataSize_t * const destDataSize );

gpdma_RequestState_t    Gpdma_Set_SourceDataOp              ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_SrcDataOp_t srcDataOp );
gpdma_RequestState_t    Gpdma_Get_SourceDataOp              ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_SrcDataOp_t * const srcDataOp );

gpdma_RequestState_t    Gpdma_Set_DestinationDataOp         ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_DestDataOp_t destDataOp );
gpdma_RequestState_t    Gpdma_Get_DestinationDataOp         ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_DestDataOp_t * const destDataOp );


gpdma_RequestState_t    Gpdma_Set_TriggerType               ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_TrgType_t triggerType );
gpdma_RequestState_t    Gpdma_Get_TriggerType               ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_TrgType_t * const triggerType );

gpdma_RequestState_t    Gpdma_Set_TriggerSource             ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_TrgSrcId_t triggerSrc );
gpdma_RequestState_t    Gpdma_Get_TriggerSource             ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_TrgSrcId_t * const triggerSrc );

gpdma_RequestState_t    Gpdma_Set_TriggerMode               ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_TriggerMode_t triggerMode );
gpdma_RequestState_t    Gpdma_Get_TriggerMode               ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_TriggerMode_t * const triggerMode );

gpdma_RequestState_t    Gpdma_Set_RequestSource             ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_PeriphReqId_t requestSrc );
gpdma_RequestState_t    Gpdma_Get_RequestSource             ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_PeriphReqId_t * const requestSrc );


gpdma_RequestState_t    Gpdma_Set_Direction                 ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_Direction_t direction );
gpdma_RequestState_t    Gpdma_Get_Direction                 ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_Direction_t * const direction );


gpdma_RequestState_t    Gpdma_Set_SourceAddr                ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_SrcAddr_t sourceAddr );
gpdma_RequestState_t    Gpdma_Get_SourceAddr                ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_SrcAddr_t * const sourceAddr );

gpdma_RequestState_t    Gpdma_Set_DestinationAddr           ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_DstAddr_t destAddr );
gpdma_RequestState_t    Gpdma_Get_DestinationAddr           ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_DstAddr_t * const destAddr );


gpdma_RequestState_t    Gpdma_Set_SourceAddrMode            ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_AddrMode_t srcAddrMode );
gpdma_RequestState_t    Gpdma_Get_SourceAddrMode            ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_AddrMode_t * const srcAddrMode );

gpdma_RequestState_t    Gpdma_Set_DestinationAddrMode       ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_AddrMode_t destAddrMode );
gpdma_RequestState_t    Gpdma_Get_DestinationAddrMode       ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_AddrMode_t * const destAddrMode );


gpdma_RequestState_t    Gpdma_Set_SourceBurstLength         ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_BurstLength_t srcBurstLen );
gpdma_RequestState_t    Gpdma_Get_SourceBurstLength         ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_BurstLength_t * const srcBurstLen );

gpdma_RequestState_t    Gpdma_Set_DestinationBurstLength    ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_BurstLength_t destBurstLen );
gpdma_RequestState_t    Gpdma_Get_DestinationBurstLength    ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_BurstLength_t * const destBurstLen );


gpdma_RequestState_t    Gpdma_Set_BlockSize                 ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_BlockSize_t blockSize );
gpdma_RequestState_t    Gpdma_Get_BlockSize                 ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_BlockSize_t * const blockSize );

gpdma_RequestState_t    Gpdma_Set_BlockRepeatCount          ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_BlockRep_t blockRepCnt );
gpdma_RequestState_t    Gpdma_Get_BlockRepeatCount          ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_BlockRep_t * const blockRepCnt );

/*------------------------- Interrupts functionality -------------------------*/

gpdma_RequestState_t    Gpdma_Set_InterruptActive             ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Set_InterruptInactive           ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );


gpdma_RequestState_t    Gpdma_Set_TransferCompleteIrqActive   ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Set_TransferCompleteIrqInactive ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );

gpdma_RequestState_t    Gpdma_Set_TransferCompleteIsrHandler  ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_IsrCallback * const irqHandler );
gpdma_RequestState_t    Gpdma_Get_TransferCompleteIsrHandler  ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_IsrCallback ** const irqHandler );


gpdma_RequestState_t    Gpdma_Set_HalfTransferIrqActive       ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Set_HalfTransferIrqInactive     ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );

gpdma_RequestState_t    Gpdma_Set_HalfTransferIsrHandler      ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_IsrCallback * const irqHandler );
gpdma_RequestState_t    Gpdma_Get_HalfTransferIsrHandler      ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_IsrCallback ** const irqHandler );


gpdma_RequestState_t    Gpdma_Set_ErrorIsrHandler             ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_IsrErrCallback * const irqHandler );
gpdma_RequestState_t    Gpdma_Get_ErrorIsrHandler             ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId, gpdma_IsrErrCallback ** const irqHandler );

gpdma_RequestState_t    Gpdma_Set_TransferErrorIrqActive      ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Set_TransferErrorIrqInactive    ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );

gpdma_RequestState_t    Gpdma_Set_ConfigErrorIrqActive        ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Set_ConfigErrorIrqInactive      ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );

gpdma_RequestState_t    Gpdma_Set_ConfigUpdateErrorIrqActive  ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Set_ConfigUpdateErrorIrqInactive( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );

gpdma_RequestState_t    Gpdma_Set_TriggerOverrunIrqActive     ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Set_TriggerOverrunIrqInactive   ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );

gpdma_RequestState_t    Gpdma_Set_SuspensionIrqActive         ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );
gpdma_RequestState_t    Gpdma_Set_SuspensionIrqInactive       ( gpdma_PeriphId_t periphId, gpdma_ChannelId_t channelId );

#ifdef __cplusplus
}
#endif

#endif /* GPDMA_GPDMA_PORT_H */

