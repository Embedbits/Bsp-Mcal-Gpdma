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
/* ========================== SYMBOLIC CONSTANTS ============================ */

/** \brief TR1 register position in transfer list for linear channels */
#define GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1         ( 0u )
/** \brief TR2 register position in transfer list for linear channels */
#define GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2         ( 1u )
/** \brief BR1 register position in transfer list for linear channels */
#define GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_BR1         ( 2u )
/** \brief SAR register position in transfer list for linear channels */
#define GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_SAR         ( 3u )
/** \brief DAR register position in transfer list for linear channels */
#define GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_DAR         ( 4u )
/** \brief LLR register position in transfer list for linear channels */
#define GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_LLR         ( 5u )

/** \brief TR1 register position in transfer list for 2D channels */
#define GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1             ( 0u )
/** \brief TR2 register position in transfer list for 2D channels */
#define GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2             ( 1u )
/** \brief BR1 register position in transfer list for 2D channels */
#define GPDMA_TRANSFER_LIST_2D_MODE_REG_BR1             ( 2u )
/** \brief SAR register position in transfer list for 2D channels */
#define GPDMA_TRANSFER_LIST_2D_MODE_REG_SAR             ( 3u )
/** \brief DAR register position in transfer list for 2D channels */
#define GPDMA_TRANSFER_LIST_2D_MODE_REG_DAR             ( 4u )
/** \brief TR3 register position in transfer list for 2D channels */
#define GPDMA_TRANSFER_LIST_2D_MODE_REG_TR3             ( 5u )
/** \brief BR2 register position in transfer list for 2D channels */
#define GPDMA_TRANSFER_LIST_2D_MODE_REG_BR2             ( 6u )
/** \brief LLR register position in transfer list for 2D channels */
#define GPDMA_TRANSFER_LIST_2D_MODE_REG_LLR             ( 7u )

/** \brief Required alignment mask of transfer list node address (32-bit words) */
#define GPDMA_TRANSFER_LIST_ADDR_ALIGN_MASK             ( 0x03u )

/** \brief Link register value of the last transfer list node (no update, no next node) */
#define GPDMA_TRANSFER_LIST_LINK_LAST                   ( 0u )

/** \brief Maximal block repeat count of 2D channel (CBR1.BRC field) */
#define GPDMA_TRANSFER_LIST_BRC_MAX                     ( DMA_CBR1_BRC_Msk >> DMA_CBR1_BRC_Pos )

/** \brief Address bits of transfer list node not stored in the link register (LLR.LA) */
#define GPDMA_TRANSFER_LIST_BASE_ADDR_MASK              ( 0xFFFF0000u )

/* =============================== MACROS =================================== */

/* ============================== TYPEDEFS ================================== */

/* ======================== FORWARD DECLARATIONS ============================ */

/* ========================== EXPORTED VARIABLES ============================ */

/* =========================== LOCAL VARIABLES ============================== */

/* ========================= EXPORTED FUNCTIONS ============================= */

/**
 * \brief Generates transfer list nodes from the user transfer configuration.
 *
 * Every transfer configuration is converted into one node of the transfer list
 * (register image loaded by the channel). Nodes are linked according to
 * \ref gpdma_XferListExecMode_t of each transfer.
 *
 * \param transferConfig [in]: Array of user transfer configurations (size \p transferCount).
 * \param transferCount  [in]: Count of transfer configurations.
 * \param channelType    [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param transferList  [out]: Transfer list (array of \p transferCount nodes) to be generated. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferListConfig( gpdma_TransferConfig_t * const transferConfig,
                                               gpdma_TransfersCount_t transferCount,
                                               gpdma_ChannelType_t channelType,
                                               volatile gpdma_XferList_t * const transferList )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_OK;

    if( ( GPDMA_NULL_PTR           != transferConfig ) &&
        ( GPDMA_NULL_PTR           != transferList   ) &&
        ( 0u                        < transferCount  ) &&
        ( GPDMA_CHANNEL_OPTION_CNT  > channelType    )    )
    {
        for( gpdma_TransfersCount_t transferId = 0u; transferCount > transferId; transferId ++ )
        {
            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_SrcPort( &transferList[ transferId ], channelType, transferConfig[ transferId ].SourcePortId, transferConfig[ transferId ].Direction );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_DestPort( &transferList[ transferId ], channelType, transferConfig[ transferId ].DestinationPortId, transferConfig[ transferId ].Direction );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_SrcDataSize( &transferList[ transferId ], channelType, transferConfig[ transferId ].SourceDataSize );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_DestDataSize( &transferList[ transferId ], channelType, transferConfig[ transferId ].DestinationDataSize );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_SrcDataOp( &transferList[ transferId ], channelType, transferConfig[ transferId ].SourceDataOp );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_DestDataOp( &transferList[ transferId ], channelType, transferConfig[ transferId ].DestinationDataOp );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_SrcBurstLen( &transferList[ transferId ], channelType, transferConfig[ transferId ].SourceBurstLength );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_DestBurstLen( &transferList[ transferId ], channelType, transferConfig[ transferId ].DestinationBurstLength );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_SrcAddrMode( &transferList[ transferId ], channelType, transferConfig[ transferId ].SourceAddrMode );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_DestAddrMode( &transferList[ transferId ], channelType, transferConfig[ transferId ].DestinationAddrMode  );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_XferCpltEvent( &transferList[ transferId ], channelType, transferConfig[ transferId ].EventMode );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_TriggerType( &transferList[ transferId ], channelType, transferConfig[ transferId ].TriggerType );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_TriggerSrc( &transferList[ transferId ], channelType, transferConfig[ transferId ].TriggerSource );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_TriggerMode( &transferList[ transferId ], channelType, transferConfig[ transferId ].TriggerMode );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_RequestMode( &transferList[ transferId ], channelType, transferConfig[ transferId ].RequestMode );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_Direction( &transferList[ transferId ], channelType, transferConfig[ transferId ].Direction );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_RequestSrc( &transferList[ transferId ], channelType, transferConfig[ transferId ].RequestSource );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_BlockSize( &transferList[ transferId ], channelType, transferConfig[ transferId ].BlockSize );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_BlockRepeatCnt( &transferList[ transferId ], channelType, transferConfig[ transferId ].BlockRepetitionCount );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_SrcAddr( &transferList[ transferId ], channelType, transferConfig[ transferId ].SourceAddr );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_DestAddr( &transferList[ transferId ], channelType, transferConfig[ transferId ].DestinationAddr );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_DstOffset2D( &transferList[ transferId ], channelType, transferConfig[ transferId ].DestinationBlockOffset2D, transferConfig[ transferId ].DestinationRepBlockOffset2D );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_SrcOffset2D( &transferList[ transferId ], channelType, transferConfig[ transferId ].SourceBlockOffset2D, transferConfig[ transferId ].SourceRepBlockOffset2D );
            }
            else
            {
                /* Error during initialization process */
            }


            if( GPDMA_REQUEST_OK == status )
            {
                if( transferCount > ( transferId + 1u  ) )
                {
                    /* Connect the next transfer configuration */

                    if( GPDMA_XFER_LIST_EXEC_SINGLE_CYCLIC == transferConfig[ transferId ].XferListExecMode )
                    {
                        /* Link current transfer configuration */
                        status = Gpdma_Set_XferList_NextXferAddr( &transferList[ transferId ], channelType, (gpdma_DataAddr_t) &transferList[ transferId ] );

                        /* Terminate configuration process. */
                        break;
                    }
                    else
                    {
                        /* Link next transfer configuration */
                        status = Gpdma_Set_XferList_NextXferAddr( &transferList[ transferId ], channelType, (gpdma_DataAddr_t) &transferList[ transferId + 1u ] );
                    }
                }
                else
                {
                    if( GPDMA_XFER_LIST_EXEC_CYCLIC_ALL == transferConfig[ transferId ].XferListExecMode )
                    {
                        /* Link first transfer configuration */
                        status = Gpdma_Set_XferList_NextXferAddr( &transferList[ transferId ], channelType, (gpdma_DataAddr_t) &transferList[ 0u ] );
                    }
                    else if( GPDMA_XFER_LIST_EXEC_SINGLE_CYCLIC == transferConfig[ transferId ].XferListExecMode )
                    {
                        /* Link current transfer configuration */
                        status = Gpdma_Set_XferList_NextXferAddr( &transferList[ transferId ], channelType, (gpdma_DataAddr_t) &transferList[ transferId ] );

                        /* Terminate configuration process. */
                        break;
                    }
                    else
                    {
                        /* No next transfer configuration */
                        status = Gpdma_Set_XferList_NextXferAddr( &transferList[ transferId ], channelType, (gpdma_DataAddr_t) 0u );
                    }
                }
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK != status )
            {
                break;
            }
            else
            {
                /* Configuration can continue */
            }
        }
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}

/*------------------- Transfer configuration functionality -------------------*/

/**
 * \brief Configures source allocated port in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param sourcePort   [in]: Source allocated port, value from \ref gpdma_PortId_t.
 * \param direction    [in]: Transfer direction (used for \ref GPDMA_PORT_DEFAULT resolution).
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_SrcPort( volatile gpdma_XferList_t * const transferList,
                                                 gpdma_ChannelType_t channelType,
                                                 gpdma_PortId_t sourcePort,
                                                 gpdma_Direction_t direction )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != transferList )
    {
        uint32_t regVal = 0u;

        if( GPDMA_PORT_0 == sourcePort )
        {
            regVal = LL_DMA_SRC_ALLOCATED_PORT0;
        }
        else if( GPDMA_PORT_1 == sourcePort )
        {
            regVal = LL_DMA_SRC_ALLOCATED_PORT1;
        }
        else
        {
            if( GPDMA_DIR_MEMORY_TO_PERIPH == direction )
            {
                regVal = LL_DMA_SRC_ALLOCATED_PORT1;
            }
            else
            {
                regVal = LL_DMA_SRC_ALLOCATED_PORT0;
            }
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ], DMA_CTR1_SAP_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ], DMA_CTR1_SAP_Msk, regVal );
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
 * \brief Returns source allocated port stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param sourcePort  [out]: Pointer to store source allocated port. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_SrcPort( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PortId_t * const sourcePort )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != sourcePort   )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_SAP_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_SAP_Msk;
        }

        if( LL_DMA_SRC_ALLOCATED_PORT0 == regVal )
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
 * \brief Configures destination allocated port in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destPort     [in]: Destination allocated port, value from \ref gpdma_PortId_t.
 * \param direction    [in]: Transfer direction (used for \ref GPDMA_PORT_DEFAULT resolution).
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_DestPort( volatile gpdma_XferList_t * const transferList,
                                                  gpdma_ChannelType_t channelType,
                                                  gpdma_PortId_t destPort,
                                                  gpdma_Direction_t direction )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != transferList )
    {
        uint32_t regVal = 0u;

        if( GPDMA_PORT_0 == destPort )
        {
            regVal = LL_DMA_DEST_ALLOCATED_PORT0;
        }
        else if( GPDMA_PORT_1 == destPort )
        {
            regVal = LL_DMA_DEST_ALLOCATED_PORT1;
        }
        else
        {
            if( GPDMA_DIR_MEMORY_TO_PERIPH == direction )
            {
                regVal = LL_DMA_DEST_ALLOCATED_PORT0;
            }
            else
            {
                regVal = LL_DMA_DEST_ALLOCATED_PORT1;
            }
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ], DMA_CTR1_DAP_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ], DMA_CTR1_DAP_Msk, regVal );
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
 * \brief Returns destination allocated port stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destPort    [out]: Pointer to store destination allocated port. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_DestPort( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PortId_t * const destPort )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != destPort     )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_DAP_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_DAP_Msk;
        }

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
 * \brief Configures source data width in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param srcDataSize  [in]: Source data width, value from \ref gpdma_DataSize_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_SrcDataSize( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataSize_t srcDataSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR     != transferList ) &&
        ( GPDMA_DATA_SIZE_CNT > srcDataSize  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_DATA_SIZE_8BITS == srcDataSize )
        {
            regVal = LL_DMA_SRC_DATAWIDTH_BYTE;
        }
        else if( GPDMA_DATA_SIZE_16BITS == srcDataSize )
        {
            regVal = LL_DMA_SRC_DATAWIDTH_HALFWORD;
        }
        else
        {
            regVal = LL_DMA_SRC_DATAWIDTH_WORD;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ], DMA_CTR1_SDW_LOG2_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ], DMA_CTR1_SDW_LOG2_Msk, regVal );
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
 * \brief Returns source data width stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param srcDataSize [out]: Pointer to store source data width. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_SrcDataSize( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataSize_t * const srcDataSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != srcDataSize  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_SDW_LOG2_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_SDW_LOG2_Msk;
        }

        if ( LL_DMA_SRC_DATAWIDTH_BYTE == regVal )
        {
            *srcDataSize = GPDMA_DATA_SIZE_8BITS;
        }
        else if ( LL_DMA_SRC_DATAWIDTH_HALFWORD == regVal )
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
 * \brief Configures destination data width in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destDataSize [in]: Destination data width, value from \ref gpdma_DataSize_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_DestDataSize( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataSize_t destDataSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR     != transferList ) &&
        ( GPDMA_DATA_SIZE_CNT > destDataSize )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_DATA_SIZE_8BITS == destDataSize )
        {
            regVal = LL_DMA_DEST_DATAWIDTH_BYTE;
        }
        else if( GPDMA_DATA_SIZE_16BITS == destDataSize )
        {
            regVal = LL_DMA_DEST_DATAWIDTH_HALFWORD;
        }
        else
        {
            regVal = LL_DMA_DEST_DATAWIDTH_WORD;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ], DMA_CTR1_DDW_LOG2_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ], DMA_CTR1_DDW_LOG2_Msk, regVal );
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
 * \brief Returns destination data width stored in the transfer list node.
 *
 * \param transferList  [in]: Transfer list node.
 * \param channelType   [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destDataSize [out]: Pointer to store destination data width. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_DestDataSize( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataSize_t * const destDataSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != destDataSize )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_DDW_LOG2_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_DDW_LOG2_Msk;
        }

        if ( LL_DMA_DEST_DATAWIDTH_BYTE == regVal )
        {
            *destDataSize = GPDMA_DATA_SIZE_8BITS;
        }
        else if ( LL_DMA_DEST_DATAWIDTH_HALFWORD == regVal )
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
 * \brief Configures source data handling operation in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param srcDataOp    [in]: Source data handling operation, value from \ref gpdma_SrcDataOp_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_SrcDataOp( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_SrcDataOp_t srcDataOp )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR       != transferList ) &&
        ( GPDMA_SRC_DATA_OP_CNT > srcDataOp    )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_SRC_DATA_PRESERVE == srcDataOp )
        {
            regVal = LL_DMA_SRC_BYTE_PRESERVE;
        }
        else
        {
            regVal = LL_DMA_SRC_BYTE_EXCHANGE;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ], DMA_CTR1_SBX_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ], DMA_CTR1_SBX_Msk, regVal );
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
 * \brief Returns source data handling operation stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param srcDataOp   [out]: Pointer to store source data handling operation. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_SrcDataOp( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_SrcDataOp_t * const srcDataOp )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != srcDataOp    )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_SBX_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_SBX_Msk;
        }

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
 * \brief Configures destination data handling operation in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destDataOp   [in]: Destination data handling operation, value from \ref gpdma_DestDataOp_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_DestDataOp( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DestDataOp_t destDataOp )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR        != transferList ) &&
        ( GPDMA_DEST_DATA_OP_CNT > destDataOp   )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_DEST_DATA_PRESERVE == destDataOp )
        {
            regVal = LL_DMA_DEST_BYTE_PRESERVE | LL_DMA_DEST_HALFWORD_PRESERVE;
        }
        else if( GPDMA_DEST_DATA_2BYTES_SWAP == destDataOp )
        {
            regVal = LL_DMA_DEST_BYTE_PRESERVE | LL_DMA_DEST_HALFWORD_EXCHANGE;
        }
        else if( GPDMA_DEST_DATA_BYTE_SWAP == destDataOp )
        {
            regVal = LL_DMA_DEST_BYTE_EXCHANGE | LL_DMA_DEST_HALFWORD_PRESERVE;
        }
        else
        {
            regVal = LL_DMA_DEST_BYTE_EXCHANGE | LL_DMA_DEST_BYTE_EXCHANGE;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ], DMA_CTR1_DHX_Msk | DMA_CTR1_DBX_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ], DMA_CTR1_DHX_Msk | DMA_CTR1_DBX_Msk, regVal );
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
 * \brief Returns destination data handling operation stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destDataOp  [out]: Pointer to store destination data handling operation. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_DestDataOp( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DestDataOp_t * const destDataOp )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != destDataOp   )    )
    {
        uint32_t dbxRegVal = 0u;
        uint32_t dhxRegVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            dbxRegVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_DBX_Msk;
            dhxRegVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_DHX_Msk;
        }
        else
        {
            dbxRegVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_DBX_Msk;
            dhxRegVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_DHX_Msk;
        }

        if( ( LL_DMA_DEST_BYTE_PRESERVE     == dbxRegVal ) &&
            ( LL_DMA_DEST_HALFWORD_PRESERVE == dhxRegVal )    )
        {
            *destDataOp = GPDMA_DEST_DATA_PRESERVE;
        }
        else if( ( LL_DMA_DEST_BYTE_PRESERVE     == dbxRegVal ) &&
                 ( LL_DMA_DEST_HALFWORD_PRESERVE != dhxRegVal )    )
        {
            *destDataOp = GPDMA_DEST_DATA_2BYTES_SWAP;
        }
        else if( ( LL_DMA_DEST_BYTE_PRESERVE     != dbxRegVal ) &&
                 ( LL_DMA_DEST_HALFWORD_PRESERVE == dhxRegVal )    )
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


/**
 * \brief Configures source burst length in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param srcBurstLen  [in]: Source burst length (1 - 64).
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_SrcBurstLen( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BurstLength_t srcBurstLen )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR      != transferList ) &&
        ( GPDMA_MAX_BURST_LEN >= srcBurstLen  ) &&
        ( GPDMA_MIN_BURST_LEN <= srcBurstLen  )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ], DMA_CTR1_SBL_1_Msk, ( ( srcBurstLen - 1u )  << DMA_CTR1_SBL_1_Pos ) );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ], DMA_CTR1_SBL_1_Msk, ( ( srcBurstLen - 1u )  << DMA_CTR1_SBL_1_Pos ) );
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
 * \brief Returns source burst length stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param srcBurstLen [out]: Pointer to store source burst length (1 - 64). Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_SrcBurstLen( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BurstLength_t * const srcBurstLen )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != srcBurstLen  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_SBL_1_Pos;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_SBL_1_Pos;
        }

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
 * \brief Configures destination burst length in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destBurstLen [in]: Destination burst length (1 - 64).
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_DestBurstLen( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BurstLength_t destBurstLen )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR      != transferList ) &&
        ( GPDMA_MAX_BURST_LEN >= destBurstLen ) &&
        ( GPDMA_MIN_BURST_LEN <= destBurstLen )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ], DMA_CTR1_DBL_1_Msk, ( ( destBurstLen - 1u )  << DMA_CTR1_DBL_1_Pos ) );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ], DMA_CTR1_DBL_1_Msk, ( ( destBurstLen - 1u )  << DMA_CTR1_DBL_1_Pos ) );
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
 * \brief Returns destination burst length stored in the transfer list node.
 *
 * \param transferList  [in]: Transfer list node.
 * \param channelType   [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destBurstLen [out]: Pointer to store destination burst length (1 - 64). Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_DestBurstLen( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BurstLength_t * const destBurstLen )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != destBurstLen )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_DBL_1_Pos;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_DBL_1_Pos;
        }

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
 * \brief Configures source address mode (fixed / increment) in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param srcAddrMode  [in]: Source address mode, value from \ref gpdma_AddrMode_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_SrcAddrMode( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_AddrMode_t srcAddrMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_ADDR_CNT  > srcAddrMode  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_ADDR_INCREMENT != srcAddrMode )
        {
            regVal = LL_DMA_SRC_FIXED;
        }
        else
        {
            regVal = LL_DMA_SRC_INCREMENT;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ], DMA_CTR1_SINC_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ], DMA_CTR1_SINC_Msk, regVal );
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
 * \brief Returns source address mode (fixed / increment) stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param srcAddrMode [out]: Pointer to store source address mode. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_SrcAddrMode( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_AddrMode_t * const srcAddrMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != srcAddrMode  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_SINC_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_SINC_Msk;
        }

        if ( LL_DMA_SRC_INCREMENT == regVal )
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
 * \brief Configures destination address mode (fixed / increment) in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destAddrMode [in]: Destination address mode, value from \ref gpdma_AddrMode_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_DestAddrMode( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_AddrMode_t destAddrMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_ADDR_CNT  > destAddrMode )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_ADDR_INCREMENT != destAddrMode )
        {
            regVal = LL_DMA_DEST_FIXED;
        }
        else
        {
            regVal = LL_DMA_DEST_INCREMENT;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ], DMA_CTR1_DINC_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ], DMA_CTR1_DINC_Msk, regVal );
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
 * \brief Returns destination address mode (fixed / increment) stored in the transfer list node.
 *
 * \param transferList  [in]: Transfer list node.
 * \param channelType   [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destAddrMode [out]: Pointer to store destination address mode. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_DestAddrMode( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_AddrMode_t * const destAddrMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != destAddrMode )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR1_DINC_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR1_DINC_Msk;
        }

        if ( LL_DMA_DEST_INCREMENT == regVal )
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
 * \brief Configures transfer complete event mode in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param eventId      [in]: Transfer complete event mode, value from \ref gpdma_TransferEvent_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_XferCpltEvent( volatile gpdma_XferList_t * const transferList,
                                                       gpdma_ChannelType_t channelType,
                                                       gpdma_TransferEvent_t eventId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != transferList )
    {
        uint32_t regVal = 0u;

        if( GPDMA_TRANSFER_EVENT_BLOCK != eventId )
        {
            regVal = LL_DMA_TCEM_BLK_TRANSFER;
        }
        else if( GPDMA_TRANSFER_EVENT_2D_BLOCK != eventId )
        {
            regVal = LL_DMA_TCEM_RPT_BLK_TRANSFER;
        }
        else if( GPDMA_TRANSFER_EVENT_TRANSFER != eventId )
        {
            regVal = LL_DMA_TCEM_EACH_LLITEM_TRANSFER;
        }
        else
        {
            regVal = LL_DMA_TCEM_LAST_LLITEM_TRANSFER;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ], DMA_CTR2_TCEM_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ], DMA_CTR2_TCEM_Msk, regVal );
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
 * \brief Returns transfer complete event mode stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param eventId     [out]: Pointer to store transfer complete event mode. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_XferCpltEvent( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TransferEvent_t * const eventId )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != transferList )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ] & DMA_CTR2_TCEM_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ] & DMA_CTR2_TCEM_Msk;
        }

        if( LL_DMA_TCEM_BLK_TRANSFER != regVal )
        {
            *eventId = GPDMA_TRANSFER_EVENT_BLOCK;
        }
        else if( LL_DMA_TCEM_RPT_BLK_TRANSFER != regVal )
        {
            *eventId = GPDMA_TRANSFER_EVENT_2D_BLOCK;
        }
        else if( LL_DMA_TCEM_EACH_LLITEM_TRANSFER != regVal )
        {
            *eventId = GPDMA_TRANSFER_EVENT_TRANSFER;
        }
        else
        {
            *eventId = GPDMA_TRANSFER_EVENT_LAST_TRANSFER;
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
 * \brief Configures trigger type (polarity) in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param triggerType  [in]: Trigger type, value from \ref gpdma_TrgType_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_TriggerType( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgType_t triggerType )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR    != transferList ) &&
        ( GPDMA_TRG_TYPE_CNT > triggerType  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_TRG_NOT_USED == triggerType )
        {
            regVal = LL_DMA_TRIG_POLARITY_MASKED;
        }
        else if( GPDMA_TRG_RISING == triggerType )
        {
            regVal = LL_DMA_TRIG_POLARITY_RISING;
        }
        else
        {
            regVal = LL_DMA_TRIG_POLARITY_FALLING;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ], DMA_CTR2_TRIGPOL_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ], DMA_CTR2_TRIGPOL_Msk, regVal );
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
 * \brief Returns trigger type (polarity) stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param triggerType [out]: Pointer to store trigger type. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_TriggerType( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgType_t * const triggerType )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != triggerType  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ] & DMA_CTR2_TRIGPOL_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ] & DMA_CTR2_TRIGPOL_Msk;
        }

        if( LL_DMA_TRIG_POLARITY_MASKED == regVal )
        {
            *triggerType = GPDMA_TRG_NOT_USED;
        }
        else if( LL_DMA_TRIG_POLARITY_RISING == regVal )
        {
            *triggerType = GPDMA_TRG_RISING;
        }
        else
        {
            *triggerType = GPDMA_TRG_FALLING;
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
 * \brief Configures trigger source in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param triggerSrc   [in]: Trigger source, value from \ref gpdma_TrgSrcId_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_TriggerSrc( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgSrcId_t triggerSrc )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != transferList )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ], DMA_CTR2_TRIGSEL_Msk, ( (uint32_t)triggerSrc << DMA_CTR2_TRIGSEL_Pos ) );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ], DMA_CTR2_TRIGSEL_Msk, ( (uint32_t)triggerSrc << DMA_CTR2_TRIGSEL_Pos ) );
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
 * \brief Returns trigger source stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param triggerSrc  [out]: Pointer to store trigger source. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_TriggerSrc( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgSrcId_t * const triggerSrc )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != triggerSrc   )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *triggerSrc = (gpdma_TrgSrcId_t)( ( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ] & DMA_CTR2_TRIGSEL_Msk ) >> DMA_CTR2_TRIGSEL_Pos );
        }
        else
        {
            *triggerSrc = (gpdma_TrgSrcId_t)( ( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ] & DMA_CTR2_TRIGSEL_Msk ) >> DMA_CTR2_TRIGSEL_Pos );
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
 * \brief Configures trigger mode in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param triggerMode  [in]: Trigger mode, value from \ref gpdma_TriggerMode_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_TriggerMode( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TriggerMode_t triggerMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR        != transferList ) &&
        ( GPDMA_TRIGGER_MODE_CNT > triggerMode  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_TRIGGER_BLOCK == triggerMode )
        {
            regVal = LL_DMA_TRIGM_BLK_TRANSFER;
        }
        else if( GPDMA_TRIGGER_2D_BLOCK == triggerMode )
        {
            regVal = LL_DMA_TRIGM_RPT_BLK_TRANSFER;
        }
        else if( GPDMA_TRIGGER_TRANSFER == triggerMode )
        {
            regVal = LL_DMA_TRIGM_LLI_LINK_TRANSFER;
        }
        else
        {
            regVal = LL_DMA_TRIGM_SINGLBURST_TRANSFER;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ], DMA_CTR2_TRIGM_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ], DMA_CTR2_TRIGM_Msk, regVal );
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
 * \brief Returns trigger mode stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param triggerMode [out]: Pointer to store trigger mode. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_TriggerMode( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TriggerMode_t * const triggerMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != triggerMode  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ] & DMA_CTR2_TRIGM_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ] & DMA_CTR2_TRIGM_Msk;
        }

        if( LL_DMA_TRIGM_BLK_TRANSFER == regVal )
        {
            *triggerMode = GPDMA_TRIGGER_BLOCK;
        }
        else if( LL_DMA_TRIGM_RPT_BLK_TRANSFER == regVal )
        {
            *triggerMode = GPDMA_TRIGGER_2D_BLOCK;
        }
        else if( LL_DMA_TRIGM_LLI_LINK_TRANSFER == regVal )
        {
            *triggerMode = GPDMA_TRIGGER_TRANSFER;
        }
        else
        {
            *triggerMode = GPDMA_TRIGGER_SINGLE;
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
 * \brief Configures peripheral request mode in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param requestMode  [in]: Peripheral request mode, value from \ref gpdma_PeriphReqMode_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_RequestMode( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PeriphReqMode_t requestMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR           != transferList ) &&
        ( GPDMA_PERIPH_REQ_MODE_CNT > requestMode  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_PERIPH_REQ_SINGLE == requestMode )
        {
            regVal = LL_DMA_HWREQUEST_SINGLEBURST;
        }
        else
        {
            regVal = LL_DMA_HWREQUEST_BLK;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ], DMA_CTR2_BREQ_Msk, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ], DMA_CTR2_BREQ_Msk, regVal );
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
 * \brief Returns peripheral request mode stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param requestMode [out]: Pointer to store peripheral request mode. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_RequestMode( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PeriphReqMode_t * const requestMode )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != transferList )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ] & DMA_CTR2_BREQ_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ] & DMA_CTR2_BREQ_Msk;
        }

        if( LL_DMA_HWREQUEST_SINGLEBURST == regVal )
        {
            *requestMode = GPDMA_PERIPH_REQ_SINGLE;
        }
        else
        {
            *requestMode = GPDMA_PERIPH_REQ_BLOCK;
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
 * \brief Configures transfer direction in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param direction    [in]: Transfer direction (used for \ref GPDMA_PORT_DEFAULT resolution).
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_Direction( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_Direction_t direction )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_DIR_CNT   > direction    )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_DIR_PERIPH_TO_MEMORY == direction )
        {
            regVal = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
        }
        else if (GPDMA_DIR_MEMORY_TO_PERIPH == direction)
        {
            regVal = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
        }
        else
        {
            regVal = LL_DMA_DIRECTION_MEMORY_TO_MEMORY;
        }

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ], DMA_CTR2_DREQ | DMA_CTR2_SWREQ, regVal );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ], DMA_CTR2_DREQ | DMA_CTR2_SWREQ, regVal );
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
 * \brief Returns transfer direction stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param direction   [out]: Pointer to store transfer direction (used for \ref GPDMA_PORT_DEFAULT resolution). Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_Direction( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_Direction_t * const direction )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != direction    )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ] & ( DMA_CTR2_DREQ | DMA_CTR2_SWREQ );
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ] & ( DMA_CTR2_DREQ | DMA_CTR2_SWREQ );
        }

        if( LL_DMA_DIRECTION_PERIPH_TO_MEMORY == regVal )
        {
            *direction = GPDMA_DIR_PERIPH_TO_MEMORY;
        }
        else if( LL_DMA_DIRECTION_MEMORY_TO_PERIPH == regVal )
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


/**
 * \brief Configures peripheral request source in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param requestSrc   [in]: Peripheral request source, value from \ref gpdma_PeriphReqId_t.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_RequestSrc( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PeriphReqId_t requestSrc )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != transferList )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ], DMA_CTR2_REQSEL_Msk, (uint32_t)requestSrc );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ], DMA_CTR2_REQSEL_Msk, (uint32_t)requestSrc );
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
 * \brief Returns peripheral request source stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param requestSrc  [out]: Pointer to store peripheral request source. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_RequestSrc( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PeriphReqId_t * const requestSrc )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != requestSrc   )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *requestSrc = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ] & DMA_CTR2_REQSEL_Msk;
        }
        else
        {
            *requestSrc = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ] & DMA_CTR2_REQSEL_Msk;
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
 * \brief Configures block size in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param blockSize    [in]: Block size in bytes.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_BlockSize( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BlockSize_t blockSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR     != transferList ) &&
        ( GPDMA_MAX_BLOCK_LEN > blockSize    )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_BR1 ], DMA_CBR1_BNDT_Msk, (uint32_t)blockSize );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_BR1 ], DMA_CBR1_BNDT_Msk, (uint32_t)blockSize );
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
 * \brief Returns block size stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param blockSize   [out]: Pointer to store block size in bytes. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_BlockSize( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BlockSize_t * const blockSize )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != blockSize    )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *blockSize = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_BR1 ] & DMA_CBR1_BNDT_Msk;
        }
        else
        {
            *blockSize = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_BR1 ] & DMA_CBR1_BNDT_Msk;
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
 * \brief Configures block repeat count (2D channels only) in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param blockRepCnt  [in]: Block repeat count.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_BlockRepeatCnt( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BlockRep_t blockRepCnt )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR     != transferList ) &&
        ( GPDMA_TRANSFER_LIST_BRC_MAX >= blockRepCnt )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_BR1 ], DMA_CBR1_BRC_Msk, ( (uint32_t)blockRepCnt << DMA_CBR1_BRC_Pos ) );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_BR1 ], DMA_CBR1_BRC_Msk, 0u );
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
 * \brief Returns block repeat count (2D channels only) stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param blockRepCnt [out]: Pointer to store block repeat count. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_BlockRepeatCnt( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BlockRep_t * const blockRepCnt )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != blockRepCnt  )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *blockRepCnt = (gpdma_BlockRep_t)( ( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_BR1 ] & DMA_CBR1_BRC_Msk ) >> DMA_CBR1_BRC_Pos );
        }
        else
        {
            *blockRepCnt = (gpdma_BlockRep_t)( ( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_BR1 ] & DMA_CBR1_BRC_Msk ) >> DMA_CBR1_BRC_Pos );
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
 * \brief Configures source address in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param sourceAddr   [in]: Source address.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_SrcAddr( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_SrcAddr_t sourceAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != transferList )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_SAR ], DMA_CSAR_SA_Msk, (uint32_t)sourceAddr );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_SAR ], DMA_CSAR_SA_Msk, (uint32_t)sourceAddr );
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
 * \brief Returns source address stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param sourceAddr  [out]: Pointer to store source address. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_SrcAddr( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_SrcAddr_t * const sourceAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != sourceAddr   )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *sourceAddr = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_SAR ] & DMA_CSAR_SA_Msk;
        }
        else
        {
            *sourceAddr = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_SAR ] & DMA_CSAR_SA_Msk;;
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
 * \brief Configures destination address in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destAddr     [in]: Destination address.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_DestAddr( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DstAddr_t destAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != transferList )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_DAR ], DMA_CDAR_DA_Msk, (uint32_t)destAddr );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_DAR ], DMA_CDAR_DA_Msk, (uint32_t)destAddr );
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
 * \brief Returns destination address stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destAddr    [out]: Pointer to store destination address. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_DestAddr( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DstAddr_t * const destAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != destAddr     )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *destAddr = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_DAR ] & DMA_CDAR_DA_Msk;
        }
        else
        {
            *destAddr = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_DAR ] & DMA_CDAR_DA_Msk;;
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
 * \brief Configures destination block offsets (2D channels only) in the transfer list node.
 *
 * \param transferList   [in]: Transfer list node.
 * \param channelType    [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param blockOffset    [in]: Block offset in bytes.
 * \param repBlockOffset [in]: Repeated block offset in bytes.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_DstOffset2D( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_ByteCnt_t blockOffset, gpdma_ByteCnt_t repBlockOffset )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR                    != transferList   ) &&
        ( GPDMA_TRANSFER_OFFSET_ADDR_MAX     > blockOffset    ) &&
        ( GPDMA_REP_TRANSFER_OFFSET_ADDR_MAX > repBlockOffset )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR3 ], DMA_CTR3_DAO_Msk, (uint32_t)( blockOffset << DMA_CTR3_DAO_Pos ) );

            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_BR2 ], DMA_CBR2_BRDAO, (uint32_t)( repBlockOffset << DMA_CBR2_BRDAO_Pos ) );
        }
        else
        {
            /* This settings is not available for linear transfer channels */
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
 * \brief Returns destination block offsets (2D channels only) stored in the transfer list node.
 *
 * \param transferList    [in]: Transfer list node.
 * \param channelType     [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param blockOffset    [out]: Pointer to store block offset in bytes. Must not be NULL.
 * \param repBlockOffset [out]: Pointer to store repeated block offset in bytes. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_DstOffset2D( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_ByteCnt_t * const blockOffset, gpdma_ByteCnt_t * const repBlockOffset )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList   ) &&
        ( GPDMA_NULL_PTR != blockOffset    ) &&
        ( GPDMA_NULL_PTR != repBlockOffset )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *blockOffset = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR3 ] & DMA_CTR3_DAO_Msk;

            *repBlockOffset = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_BR2 ] & DMA_CBR2_BRDAO;
        }
        else
        {
            *blockOffset = 0u;

            *repBlockOffset = 0u;
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
 * \brief Configures source block offsets (2D channels only) in the transfer list node.
 *
 * \param transferList   [in]: Transfer list node.
 * \param channelType    [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param blockOffset    [in]: Block offset in bytes.
 * \param repBlockOffset [in]: Repeated block offset in bytes.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_SrcOffset2D( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_ByteCnt_t blockOffset, gpdma_ByteCnt_t repBlockOffset )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR                    != transferList   ) &&
        ( GPDMA_TRANSFER_OFFSET_ADDR_MAX     > blockOffset    ) &&
        ( GPDMA_REP_TRANSFER_OFFSET_ADDR_MAX > repBlockOffset )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR3 ], DMA_CTR3_SAO_Msk, (uint32_t)( blockOffset << DMA_CTR3_SAO_Pos ) );

            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_BR2 ], DMA_CBR2_BRSAO, (uint32_t)( repBlockOffset << DMA_CBR2_BRSAO_Pos ) );
        }
        else
        {
            /* This settings is not available for linear transfer channels */
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
 * \brief Returns source block offsets (2D channels only) stored in the transfer list node.
 *
 * \param transferList    [in]: Transfer list node.
 * \param channelType     [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param blockOffset    [out]: Pointer to store block offset in bytes. Must not be NULL.
 * \param repBlockOffset [out]: Pointer to store repeated block offset in bytes. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Get_XferList_SrcOffset2D( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_ByteCnt_t * const blockOffset, gpdma_ByteCnt_t * const repBlockOffset )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList   ) &&
        ( GPDMA_NULL_PTR != blockOffset    ) &&
        ( GPDMA_NULL_PTR != repBlockOffset )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *blockOffset = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR3 ] & DMA_CTR3_SAO_Msk;

            *repBlockOffset = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_BR2 ] & DMA_CBR2_BRSAO;
        }
        else
        {
            *blockOffset = 0u;

            *repBlockOffset = 0u;
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
 * \brief Configures address of the next transfer list node (link register) in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param nextAddr     [in]: Address of the next node (0 - last node), 32-bit aligned.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 */
gpdma_RequestState_t Gpdma_Set_XferList_NextXferAddr( volatile gpdma_XferList_t * const transferList,
                                                      gpdma_ChannelType_t channelType,
                                                      gpdma_DataAddr_t nextAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList                                     ) &&
        ( 0u             == ( nextAddr & GPDMA_TRANSFER_LIST_ADDR_ALIGN_MASK ) )    )
    {
        if( 0u == nextAddr )
        {
            /* Last node - update flags and link address must be all zero,
             * otherwise the channel loads the next node from list offset 0 */
            if( GPDMA_CHANNEL_LINEAR_2D == channelType )
            {
                transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_LLR ] = GPDMA_TRANSFER_LIST_LINK_LAST;
            }
            else
            {
                transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_LLR ] = GPDMA_TRANSFER_LIST_LINK_LAST;
            }
        }
        else if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            uint32_t regUpdateMask = LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 | LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CTR3 | LL_DMA_UPDATE_CBR2 | LL_DMA_UPDATE_CLLR;

            uint32_t destAddrOffset = nextAddr & DMA_CLLR_LA_Msk;

            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_LLR ],
                        DMA_CLLR_LA_Msk  |
                        DMA_CLLR_ULL_Msk |
                        DMA_CLLR_UB2_Msk |
                        DMA_CLLR_UT3_Msk |
                        DMA_CLLR_UDA_Msk |
                        DMA_CLLR_USA_Msk |
                        DMA_CLLR_UB1_Msk |
                        DMA_CLLR_UT2_Msk |
                        DMA_CLLR_UT1_Msk,
                        (uint32_t)( regUpdateMask | destAddrOffset ) );
        }
        else
        {
            uint32_t regUpdateMask = LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 | LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CLLR;

            uint32_t destAddrOffset = nextAddr & DMA_CLLR_LA_Msk;

            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_LLR ],
                        DMA_CLLR_LA_Msk  |
                        DMA_CLLR_ULL_Msk |
                        DMA_CLLR_UDA_Msk |
                        DMA_CLLR_USA_Msk |
                        DMA_CLLR_UB1_Msk |
                        DMA_CLLR_UT2_Msk |
                        DMA_CLLR_UT1_Msk,
                        (uint32_t)( regUpdateMask | destAddrOffset ) );
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
 * \brief Returns address of the next transfer list node (link register) stored in the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to, value from \ref gpdma_ChannelType_t.
 * \param destAddr    [out]: Pointer to store address of the next node (0 - last node). Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPDMA_REQUEST_OK if request was
 *         success, otherwise returns \ref GPDMA_REQUEST_ERROR.
 *
 * \note Link register holds only lower 16 bits of the address, upper 16 bits are taken
 *       from the node address (all nodes of the list are in the same 64 kB region).
 */
gpdma_RequestState_t Gpdma_Get_XferList_NextXferAddr( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataAddr_t * const destAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != destAddr     )    )
    {
        uint32_t nextAddrOffset = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            nextAddrOffset = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_LLR ] & DMA_CLLR_LA_Msk;
        }
        else
        {
            nextAddrOffset = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_LLR ] & DMA_CLLR_LA_Msk;
        }

        if( 0u == nextAddrOffset )
        {
            /* Last node of the list */
            *destAddr = 0u;
        }
        else
        {
            /* Nodes of one list are in the same 64 kB region (CLBAR) - upper address bits are taken from the node */
            *destAddr = ( (uint32_t)(uintptr_t)transferList & GPDMA_TRANSFER_LIST_BASE_ADDR_MASK ) | nextAddrOffset;
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
 * \brief Returns complete link register value (update flags and next node
 *        address) of the transfer list node.
 *
 * \param transferList [in]: Transfer list node.
 * \param channelType  [in]: Type of the channel the transfer list belongs to.
 * \param linkReg     [out]: Link register value to be loaded into channel CxLLR.
 *
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpdma_RequestState_t Gpdma_Get_XferList_LinkReg( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_XferLinkReg_t * const linkReg )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != linkReg      )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *linkReg = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_LLR ];
        }
        else
        {
            *linkReg = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_LLR ];
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}

/* =========================== LOCAL FUNCTIONS ============================== */

/* =========================== INTERRUPT HANDLERS =========================== */

/* ================================ TASKS =================================== */
