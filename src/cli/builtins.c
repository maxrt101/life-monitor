/** ========================================================================= *
*
 * @file builtins.c
 * @date 30-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Builtin commands inclusion into shell
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "shell/shell.h"
#include "shell/shell_builtins.h"

/* Defines ================================================================== */
#define LOG_TAG shell

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
SHELL_DECLARE_COMMAND(abort, builtin_abort, "Aborts app");
SHELL_DECLARE_COMMAND(echo, builtin_echo, "Echoes arguments");
SHELL_DECLARE_COMMAND(exec, builtin_exec, "Execute subcommand");
SHELL_DECLARE_COMMAND(exit, builtin_exit, "Exit from shell");
SHELL_DECLARE_COMMAND(help, builtin_help, "Shows all commands");
SHELL_DECLARE_COMMAND(if, builtin_if, "Conditional execution");
SHELL_DECLARE_COMMAND(log, builtin_log, "Log");
SHELL_DECLARE_COMMAND(mem, builtin_mem, "Memory usage and reading");
SHELL_DECLARE_COMMAND(pwr, builtin_pwr, "Power Mode control");
SHELL_DECLARE_COMMAND(repeat, builtin_repeat, "Repeat subcommand");
SHELL_DECLARE_COMMAND(reset, builtin_reset, "Reset device (restart)");
SHELL_DECLARE_COMMAND(sh, builtin_sh, "Shell control");
SHELL_DECLARE_COMMAND(sleep, builtin_sleep, "Sleep ms");
SHELL_DECLARE_COMMAND(task, builtin_task, "Task control");
SHELL_DECLARE_COMMAND(time, builtin_time, "Reports system time (ms tick)");
SHELL_DECLARE_COMMAND(tty, builtin_tty, "TTY control");

#if USE_SHELL_HISTORY
SHELL_DECLARE_COMMAND(history, builtin_history, "Show history");
#endif

#if USE_GLOBAL_VFS
SHELL_DECLARE_COMMAND(cat, builtin_cat, "Print file to console from VFS");
SHELL_DECLARE_COMMAND(hexdump, builtin_hexdump, "File hexdump");
SHELL_DECLARE_COMMAND(ls, builtin_ls, "List dir in VFS");
SHELL_DECLARE_COMMAND(mkdir, builtin_mkdir, "Creates directories in VFS");
SHELL_DECLARE_COMMAND(rm, builtin_rm, "Remove file in VFS");
SHELL_DECLARE_COMMAND(touch, builtin_touch, "Creates file in VFS");
SHELL_DECLARE_COMMAND(write, builtin_write, "Writes to file in VFS");
#endif

#if USE_SHELL_ENV
SHELL_DECLARE_COMMAND(check, builtin_check, "Comparison between 2 ints");
SHELL_DECLARE_COMMAND(expr, builtin_expr, "Integer arithmetic");
SHELL_DECLARE_COMMAND(set, builtin_set, "Set variable");
SHELL_DECLARE_COMMAND(unset, builtin_unset, "Unset variable");
SHELL_DECLARE_COMMAND(env, builtin_env, "Prints env (variables)");
SHELL_DECLARE_COMMAND(input, builtin_input, "User input into variable");
#endif

/* Private functions ======================================================== */
/* Shared functions ========================================================= */

