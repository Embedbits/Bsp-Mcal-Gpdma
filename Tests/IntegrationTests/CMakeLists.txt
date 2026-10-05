#===================== Integration tests configuration ========================#
# Integration tests of Gpdma module on target (Nucleo board). Registered in module
# CMakeLists.txt by IntegrationTesting_AddPath() and created by
# IntegrationTesting_Generate() in "IntegrationTest" build
# (see EmBi_Platform/CMake/IntegrationTesting/README.md).

IntegrationTesting_Add_Test(
    NAME            Gpdma
    TEST_SOURCE     ItTest_Gpdma.c
    LINK_LIBS       Gpdma_Lib
    TIMEOUT         60
)
