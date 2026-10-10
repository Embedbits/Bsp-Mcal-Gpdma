#===================== Integration tests configuration ========================#
# Integration tests of Gpdma module on target (STM32H7R / STM32H7S board). Registered in
# module CMakeLists.txt by IntegrationTesting_AddPath() and created by
# IntegrationTesting_Generate() in "IntegrationTest" build
# (see EmBi_Platform/CMake/IntegrationTesting/README.md).
#
# The test set is independent - target interface (entry point, initialization,
# fault handlers, system reset) is implemented by ItTarget_Gpdma.c and
# all libraries of the firmware are listed here.

# StartUp calls BspMain() - the entry point of the test firmware is provided by
# the test set (BspMain.h, ItTarget_Gpdma.c)
if(NOT TARGET BspMain_Lib)
    add_library(BspMain_Lib INTERFACE)
    target_include_directories(BspMain_Lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# GPDMA1 / HPDMA1 exist only on STM32H7R / STM32H7S - classic STM32H7 lines have no tests (module is empty)
if(MCU_FAMILY_ID STREQUAL "STM32H7RSxx")

    # GPDMA1 (the controller of the test cases is selected in ItTest_Gpdma.c)
    IntegrationTesting_Add_Test(
        NAME            Gpdma
        TEST_SOURCE     ItTest_Gpdma.c
        SOURCES         ItTarget_Gpdma.c    # Target interface of the test set
        LINK_LIBS       StartUp_Lib         # Startup code (calls BspMain)
                        Nvic_Lib            # Fault handlers, system reset (ItTarget)
                        Gpdma_Lib
        TIMEOUT         60
    )

    # HPDMA1 - the same test cases on the second controller
    IntegrationTesting_Add_Test(
        NAME            Hpdma
        TEST_SOURCE     ItTest_Gpdma.c
        SOURCES         ItTarget_Gpdma.c    # Target interface of the test set
        DEFINITIONS     IT_GPDMA_HPDMA1     # GPDMA_PERIPH_2 in the test cases
        LINK_LIBS       StartUp_Lib         # Startup code (calls BspMain)
                        Nvic_Lib            # Fault handlers, system reset (ItTarget)
                        Gpdma_Lib
        TIMEOUT         60
    )

endif()
