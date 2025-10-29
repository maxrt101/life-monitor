/** ========================================================================= *
*
 * @file sh_cmd_adc.c
 * @date 07-03-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief 'adc' CLI Command implementation
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "shell/shell.h"
#include "shell/shell_util.h"
#include "log/log.h"
#include "bsp.h"

/* Defines ================================================================== */
#define LOG_TAG SHELL

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
static int8_t cmd_adc(shell_t * sh, uint8_t argc, const char ** argv) {
  uint32_t vref, vbat;
  int32_t temp;

  SHELL_ERR_REPORT_RETURN(bsp_adc_get_vrefint(&vref), "bsp_adc_get_vrefint");
  SHELL_ERR_REPORT_RETURN(bsp_adc_get_vbat(&vbat), "bsp_adc_get_vbat");
  SHELL_ERR_REPORT_RETURN(bsp_adc_get_temp(&temp), "bsp_adc_get_temp");

  log_info("vref: %d mv", vref);
  log_info("vbat: %d mv", vbat);
  log_info("temp: %d C", temp);

  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(adc, cmd_adc, "Get vref, vbat & temp from ADC");
