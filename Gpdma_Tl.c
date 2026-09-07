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

/* =============================== MACROS =================================== */

/* ============================== TYPEDEFS ================================== */

/* ======================== FORWARD DECLARATIONS ============================ */

/* ========================== EXPORTED VARIABLES ============================ */

/* =========================== LOCAL VARIABLES ============================== */

/* ========================= EXPORTED FUNCTIONS ============================= */

gpdma_RequestState_t Gpdma_Get_XferListConfig( gpdma_TransferConfig_t * const transferConfig,
                                               gpdma_TransfersCount_t transferCount,
                                               gpdma_ChannelType_t channelType,
                                               volatile gpdma_XferList_t * const transferList )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR           != transferConfig ) &&
        ( GPDMA_NULL_PTR           != transferConfig ) &&
        ( 0u                        < transferCount  ) &&
        ( GPDMA_CHANNEL_OPTION_CNT  > channelType    )    )
    {
        for( gpdma_TransfersCount_t transferId = 0u; transferCount > transferId; transferId ++ )
        {
            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_SrcPort( &transferList[ transferId ], channelType, transferConfig[ transferId ].SourcePortId );
            }
            else
            {
                /* Error during initialization process */
            }

            if( GPDMA_REQUEST_OK == status )
            {
                status = Gpdma_Set_XferList_DestPort( &transferList[ transferId ], channelType, transferConfig[ transferId ].DestinationPortId );
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
                        status = Gpdma_Set_XferList_NextXferAddr( &transferList[ transferId ], channelType, (gpdma_DataAddr_t) &transferConfig[ transferId ] );

                        /* Terminate configuration process. */
                        break;
                    }
                    else
                    {
                        /* Link next transfer configuration */
                        status = Gpdma_Set_XferList_NextXferAddr( &transferList[ transferId ], channelType, (gpdma_DataAddr_t) &transferConfig[ transferId + 1u ] );
                    }
                }
                else
                {
                    if( GPDMA_XFER_LIST_EXEC_CYCLIC_ALL == transferConfig[ transferId ].XferListExecMode )
                    {
                        /* Link first transfer configuration */
                        status = Gpdma_Set_XferList_NextXferAddr( &transferList[ transferId ], channelType, (gpdma_DataAddr_t) &transferConfig[ 0u ] );
                    }
                    else if( GPDMA_XFER_LIST_EXEC_SINGLE_CYCLIC == transferConfig[ transferId ].XferListExecMode )
                    {
                        /* Link current transfer configuration */
                        status = Gpdma_Set_XferList_NextXferAddr( &transferList[ transferId ], channelType, (gpdma_DataAddr_t) &transferConfig[ 0u ] );

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

gpdma_RequestState_t Gpdma_Set_XferList_SrcPort( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PortId_t sourcePort )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR    != transferList ) &&
        ( GPDMA_PORT_DEFAULT > sourcePort   )    )
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


gpdma_RequestState_t Gpdma_Set_XferList_DestPort( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_PortId_t destPort )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR    != transferList ) &&
        ( GPDMA_PORT_DEFAULT > destPort     )    )
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


gpdma_RequestState_t Gpdma_Get_XferList_TriggerType( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgType_t * const triggerType )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != triggerType  )    )
    {
        uint32_t regVal = 0u;

        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR1 ] & DMA_CTR2_TRIGPOL_Msk;
        }
        else
        {
            regVal = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR1 ] & DMA_CTR2_TRIGPOL_Msk;
        }

        if( LL_DMA_TRIG_POLARITY_MASKED == regVal )
        {
            *triggerType = GPDMA_TRG_NOT_USED;
        }
        else if( LL_DMA_TRIG_POLARITY_MASKED == regVal )
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



gpdma_RequestState_t Gpdma_Set_XferList_TriggerSrc( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgSrcId_t triggerSrc )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( GPDMA_NULL_PTR != transferList )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ], DMA_CTR2_TRIGSEL_Msk, triggerSrc );
        }
        else
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ], DMA_CTR2_TRIGSEL_Msk, triggerSrc );
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


gpdma_RequestState_t Gpdma_Get_XferList_TriggerSrc( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_TrgSrcId_t * const triggerSrc )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != triggerSrc   )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *triggerSrc = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_TR2 ] & DMA_CTR2_TRIGSEL_Msk;
        }
        else
        {
            *triggerSrc = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_TR2 ] & DMA_CTR2_TRIGSEL_Msk;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}



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


gpdma_RequestState_t Gpdma_Set_XferList_BlockRepeatCnt( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BlockRep_t blockRepCnt )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR     != transferList ) &&
        ( GPDMA_MAX_BLOCK_LEN > blockRepCnt  )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            MODIFY_REG( transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_BR1 ], DMA_CBR1_BRC_Msk, (uint32_t)blockRepCnt );
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


gpdma_RequestState_t Gpdma_Get_XferList_BlockRepeatCnt( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_BlockRep_t * const blockRepCnt )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != blockRepCnt  )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *blockRepCnt = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_BR1 ] & DMA_CBR1_BRC_Msk;
        }
        else
        {
            *blockRepCnt = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_BR1 ] & DMA_CBR1_BRC_Msk;
        }

        status = GPDMA_REQUEST_OK;
    }
    else
    {
        status = GPDMA_REQUEST_ERROR;
    }

    return ( status );
}


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


gpdma_RequestState_t Gpdma_Set_XferList_NextXferAddr( volatile gpdma_XferList_t * const transferList,
                                                      gpdma_ChannelType_t channelType,
                                                      gpdma_DataAddr_t nextAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList      ) &&
        ( 0u             != ( nextAddr & 0x03 ) )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
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


gpdma_RequestState_t Gpdma_Get_XferList_NextXferAddr( volatile gpdma_XferList_t * const transferList, gpdma_ChannelType_t channelType, gpdma_DataAddr_t * const destAddr )
{
    gpdma_RequestState_t status = GPDMA_REQUEST_ERROR;

    if( ( GPDMA_NULL_PTR != transferList ) &&
        ( GPDMA_NULL_PTR != destAddr     )    )
    {
        if( GPDMA_CHANNEL_LINEAR_2D == channelType )
        {
            *destAddr = transferList->Register[ GPDMA_TRANSFER_LIST_2D_MODE_REG_LLR ] & DMA_CLLR_LA_Msk;
        }
        else
        {
            *destAddr = transferList->Register[ GPDMA_TRANSFER_LIST_LINEAR_MODE_REG_LLR ] & DMA_CLLR_LA_Msk;
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
