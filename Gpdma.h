/**
 * \author Mr.Nobody
 * \file Gpdma.h
 * \ingroup Gpdma
 * \brief Gpdma module common functionality header file.
 *
 * This file contains the common functionality used internally by the module,
 * and shall provide interface between the module and the application.
 *
 */

#ifndef GPDMA_GPDMA_H
#define GPDMA_GPDMA_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================= INCLUDES =================================== */
#include "Gpdma_Types.h"                    /* Module types definition        */
/* ============================= TYPEDEFS =================================== */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

gpdma_FlagState_t       Gpdma_Check_FlashLocation   ( uint32_t addr );
gpdma_FlagState_t       Gpdma_Check_RamLocation     ( uint32_t addr );

#ifdef __cplusplus
}
#endif

#endif /* GPDMA_GPDMA_H */
