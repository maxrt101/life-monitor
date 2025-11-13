# =========================================================================
#
# @file board.cmake
# @date 20-07-2024
# @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
#
# @brief Board definition for Generic STM32L051
#
# =========================================================================

include_guard(GLOBAL)

####################    VARIABLES    ####################
set(BOARD_NAME       "STM32L051")
set(BOARD_VERSION    "0.1")
set(PROJECT_VER_HW   0)
set(MCU              "STM32L051C8T6")
set(CMAKE_C_STANDARD 17)

set(BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}")

####################    COMPILER    ####################
include(${SDK_DIR}/toolchain/compiler.cmake)

compiler_setup(GCC arm-none-eabi)

####################    PLATFORM    ####################
project_setup_platform(STM32L0xx)

####################    OPTIONS     ####################
project_add_define(
    # Needed by ST HAL/LL
    "STM32L051xx"

    # Application
    "USE_LED_ERROR_ON_ABORT=1"

    # Console
    "CONSOLE_UART_INDEX=1"
    "CONSOLE_FILE=\"/dev/console\""

    # Workarounds
    "USE_SPI_FIRST_READ_OUT_OF_SYNC_FIX=1"

    # Generic Peripherals
    "BSP_LED_COUNT=1"
    "BSP_LED_PULSE=0"
    "BSP_LED_TRX=0"
    "BSP_LED_ERR=0"
    "BSP_BTN_COUNT=1"
    "BSP_BTN_MAIN=0"
    "BSP_I2C_RECV_TIMEOUT=100"
    "BSP_SPI_RECV_TIMEOUT=100"

    # SX1278
    "HAS_TRX_SX1278_SUPPORT=1"

    # Power
    "BAT_VOLTAGE_SCALER=3"
    "BAT_DEAD_VOLTAGE_MV=1500"
    "BAT_FULL_VOLTAGE_MV=5400"

    # Board logs
    "LOG_ENABLE_BSP=1"
    "LOG_ENABLE_BSP_ADC=0"

    # Board Info
    "BOARD_NAME=\"${BOARD_NAME}\""
    "BOARD_VERSION=\"${BOARD_VERSION}\""
    "PROJECT_VERSION_HW=${PROJECT_VER_HW}"
    "MCU=\"${MCU}\""
)

project_add_compile_options(ALL
        -mcpu=cortex-m0plus
        -mthumb
        -mfloat-abi=soft

        --specs=nano.specs

        -masm-syntax-unified

        -ffunction-sections
        -fdata-sections
        -fshort-enums
)

project_add_compile_options(ALL
        -Os -ggdb
)

project_add_compile_options(DEBUG
        -g3 -DDEBUG
)

project_add_link_options(ALL
        -mcpu=cortex-m0plus
        -specs=nano.specs
        -ffunction-sections -fdata-sections
        -Wl,--gc-sections,--sort-common
        -Wl,--start-group -lc -lm -Wl,--end-group
)

####################   SOURCES    ####################
project_add_inc_dirs(
        "${BOARD_DIR}/bsp"
        "${BOARD_DIR}/bsp/hal"
        "${BOARD_DIR}/bsp/port"
        "${BOARD_DIR}/Core/Inc"
)

project_add_inc_recursive("${BOARD_DIR}")
project_add_src_recursive("${BOARD_DIR}")
project_add_src_recursive("${BOARD_DIR}/Core")
project_add_src_files("${BOARD_DIR}/Core/Startup/startup_stm32l051c8tx.s")

project_add_ld_scripts("${BOARD_DIR}/STM32L051C8TX.ld")

message(STATUS "Using ${BOARD_NAME} board (${BOARD_DIR})")
