/** ========================================================================= *
 *
 * @file hal_gpio.h
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Port implementation of SDK GPIO HAL
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "stm32l0xx.h"
#include "stm32l0xx_ll_gpio.h"
#include "util/util.h"

/* Defines ================================================================== */
#define GPIO_PORT_STRUCT_IMPL \
    GPIO_TypeDef * port; \
    uint32_t pin;

/* Macros =================================================================== */
#define GPIO_GET_BIND_PORT(...) UTIL_GET_ARG1(__VA_ARGS__)
#define GPIO_GET_BIND_PIN(...) UTIL_GET_ARG2(__VA_ARGS__)

#define GPIO_PORT_EXPAND_TYPE(gpio) \
    gpio.port, gpio.pin

#define GPIO_PORT_BIND(...) \
    __VA_ARGS__ ## _GPIO_Port, __VA_ARGS__ ## _Pin

#define GPIO_PORT_TYPE_BIND(...) \
    (gpio_t){.port = __VA_ARGS__ ## _GPIO_Port, .pin = __VA_ARGS__ ## _Pin}

#define GPIO_PORT_TO_TYPE(...) \
    (gpio_t){.port = GPIO_GET_BIND_PORT(__VA_ARGS__), .pin = GPIO_GET_BIND_PIN(__VA_ARGS__)}

#define GPIO_PORT_READ(...) \
    LL_GPIO_IsInputPinSet(__VA_ARGS__)

#define GPIO_PORT_SET(...) \
    LL_GPIO_SetOutputPin(__VA_ARGS__)

#define GPIO_PORT_CLEAR(...) \
    LL_GPIO_ResetOutputPin(__VA_ARGS__)

#define GPIO_PORT_TOGGLE(...) \
    LL_GPIO_TogglePin(__VA_ARGS__)

#define GPIO_PORT_SET_PIN_MODE(port, pin, mode) \
    LL_GPIO_SetPinMode(port, pin,                 \
      mode == GPIO_PIN_MODE_INPUT                 \
        ? LL_GPIO_MODE_INPUT                      \
        : LL_GPIO_MODE_OUTPUT)

/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Shared functions ========================================================= */

#ifdef __cplusplus
}
#endif
