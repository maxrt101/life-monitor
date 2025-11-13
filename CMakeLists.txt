# =========================================================================
#
# @file CMakeLists.txt
# @date 27-10-2025
# @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
#
# @brief LifeMonitor project
#
# =========================================================================

cmake_minimum_required(VERSION 3.27)

# Required variables
set(PROJECT_DIR          "${CMAKE_CURRENT_LIST_DIR}")
set(SDK_DIR              "${CMAKE_CURRENT_LIST_DIR}/sdk")
set(PROJECT_NAME         "LifeMonitor")
set(PROJECT_VER_SW_MAJOR 0)
set(PROJECT_VER_SW_MINOR 1)
set(PROJECT_VER_SW_PATCH 0)

# Additional configuration
set(PROJECT_VERBOSE 0)
set(GENERATE_HEX    1)
set(GENERATE_BIN    1)
set(GENERATE_MAP    1)
set(SHOW_SIZE       1)

# Feature flags
set(FEATURES
    # Network
    "PROJECT_DEVICE_MAC=0xDEADBEEF"
    "NET_USE_FULL_CHANNEL_TABLE=0"
    "USE_NET_PACKET_DUMP=1"

    # Application
    "USE_MOCK_STORAGE=1"
    "USE_WDG=0"
    "USE_ULTRALOWPOWER=0"

    # Logs
    "USE_COLOR_LOG=1"
    "USE_PRINT_STARTUP_INFO=1"

    # VFS
    "USE_GLOBAL_VFS=1"
    "VFS_IOCTL_CMD_PORT=VFS_IOCTL_RESET_DEVICE, VFS_IOCTL_WRITE_DETECTED, VFS_IOCTL_WRITE_DETECTED_CLEAR"

    # OS Allocator
    "USE_OS_HEAP_DEFRAG_ON_FREE=1"
    "USE_OS_HEAP_DEFRAG_ON_NO_MEM=1"

    # OS Tasks
    "USE_OS_STACK_CHECK=1"
    "USE_OS_STAT=1"
    "OS_STAT_TRACE_TASK_STACK=1"
    "OS_SLEEP_MODE=OS_POWER_MODE_AUTO"

    # Debug Log Configuration (Modules)
    "LOG_ENABLE_main=1"
    "LOG_ENABLE_shell=1"
    "LOG_ENABLE_app=1"
    "LOG_ENABLE_net=1"
    "LOG_ENABLE_storage=1"
    "LOG_ENABLE_pulse=1"
    "LOG_ENABLE_gps=1"
    "LOG_ENABLE_btn=1"
    "LOG_ENABLE_led=1"
    "LOG_ENABLE_pwm=0"
    "LOG_ENABLE_os=1"
    "LOG_ENABLE_irq=1"
    "LOG_ENABLE_rst=1"
    "LOG_ENABLE_heap=1"
    "LOG_ENABLE_swdg=1"
    "LOG_ENABLE_swdt=1"
    "LOG_ENABLE_sx1278=0"
    "LOG_ENABLE_ds28=0"
    "LOG_ENABLE_aht=0"
    "LOG_ENABLE_max3010x=1"
    "LOG_ENABLE_nvm_crc=0"
    "LOG_ENABLE_hal_nvm=0"
    "LOG_ENABLE_hal_spi=0"
    "LOG_ENABLE_hal_uart=0"
    "LOG_ENABLE_hal_wdt=0"
    "LOG_ENABLE_hal_1w=0"

    # Debug Log Configuration
    "SHELL_DEBUG_PRINT_TOKENS=0"
    "SHELL_DEBUG_DEBUG_TOKENS_HEX=0"
    "USE_RTC_IRQ_DEBUG_LOG=0"
    "USE_RTC_DEBUG_LOG=0"
    "USE_OS_TRACE_CYCLE=0"
    "USE_OS_TRACE_TASK_KILL=0"
    "USE_OS_TRACE_TASK_YIELD=0"
    "USE_OS_TRACE_TASK_SWITCH=0"
    "USE_OS_TRACE_TASK_HANDLE=0"
    "USE_OS_TRACE_SETJMP=0"
    "USE_OS_TRACE_MUTEX=0"
    "USE_OS_TRACE_EVENT=0"
)

# Include utils
include(${SDK_DIR}/toolchain/project.cmake)
include(${SDK_DIR}/toolchain/utils.cmake)

# Setup project
project_init()
project(${PROJECT_NAME} C ASM)

# Add sdk
project_add_inc_recursive("${SDK_DIR}")
project_add_inc_dirs(
        "${SDK_DIR}"
        "${SDK_DIR}/hal"
        "${SDK_DIR}/lib"
        "${SDK_DIR}/drv"
)
project_add_src_files(
        "${SDK_DIR}/drv/trx/trx.c"
)
project_add_src_recursive(
        "${SDK_DIR}/hal"
        "${SDK_DIR}/lib"
        "${SDK_DIR}/drv/mpu6050"
        "${SDK_DIR}/drv/max3010x"
        "${SDK_DIR}/drv/trx/sx1278"
)

execute_process(
    COMMAND bash -c "echo -n $(whoami)"
    OUTPUT_VARIABLE PROJECT_COMPILED_BY
)

execute_process(
    COMMAND bash -c "git log -n 1 --pretty=format:%h"
    OUTPUT_VARIABLE PROJECT_COMMIT
)

# Add defines
project_add_define(
    # Project Info
    "PROJECT_NAME=\"${PROJECT_NAME}\""
    "PROJECT_VERSION=\"${PROJECT_VER_SW_MAJOR}.${PROJECT_VER_SW_MINOR}.${PROJECT_VER_SW_PATCH}\""
    "PROJECT_VERSION_SW_MAJOR=${PROJECT_VER_SW_MAJOR}"
    "PROJECT_VERSION_SW_MINOR=${PROJECT_VER_SW_MINOR}"
    "PROJECT_VERSION_SW_PATCH=${PROJECT_VER_SW_PATCH}"
    "PROJECT_COMPILED_BY=\"${PROJECT_COMPILED_BY}\""
    "PROJECT_COMPILED_WITH=\"${CMAKE_C_COMPILER_ID}\""
    "PROJECT_COMMIT=\"${PROJECT_COMMIT}\""
)

if(CMAKE_BUILD_TYPE STREQUAL DEBUG)
    list(APPEND FEATURES
        "USE_OS_SLEEP_AFTER_CYCLE=0"
        "USE_SHELL_ENV=1"
        "USE_SHELL_HISTORY=1"
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL RELEASE)
    list(APPEND FEATURES
        "USE_OS_SLEEP_AFTER_CYCLE=1"
        "USE_SHELL_ENV=0"
        "USE_SHELL_HISTORY=0"
    )
endif()

# Parse feature flags
feature_parse_all(${FEATURES})

# Add project headers and sources
project_add_inc_dirs("${PROJECT_DIR}" "${PROJECT_DIR}/src")

project_add_src_recursive("${PROJECT_DIR}/src")

project_add_ld_paths("${SDK_DIR}/lib/shell/")

project_finish()
