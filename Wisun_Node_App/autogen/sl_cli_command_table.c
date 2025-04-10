/***************************************************************************//**
 * @file sl_cli_command_table.c
 * @brief Declarations of relevant command structs for cli framework.
 * @version x.y.z
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <stdlib.h>

#include "sl_cli_config.h"
#include "sl_cli_command.h"
#include "sl_cli_arguments.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   TEMPLATED FILE   ******************************
 ******************************************************************************/

/*******************************************************************************
 * Example syntax (.slcc or .slcp) for populating this file:
 *
 *   template_contribution:
 *     - name: cli_command          # Register a command
 *       value:
 *         name: status             # Name of command
 *         handler: status_command  # Function to be called. Must be defined
 *         help: "Prints status"    # Optional help description
 *         shortcuts:               # Optional shorcut list
 *           - name: st
 *         argument:                # Argument list, if apliccable
 *           - type: uint8          # Variable type
 *             help: "Channel"      # Optional description
 *           - type: string
 *             help: "Text"
 *     - name: cli_group            # Register a group
 *       value:
 *         name: shell              # Group name
 *         help: "Shell commands"   # Optional help description
 *         shortcuts:               # Optional shorcuts
 *           - name: sh
 *     - name: cli_command
 *       value:
 *         name: repeat
 *         handler: repeat_cmd
 *         help: "Repeat commands"
 *         shortcuts:
 *           - name: r
 *           - name: rep
 *         group: shell            # Associate command with group
 *         argument:
 *           - type: string
 *             help: "Text"
 *           - type: additional
 *             help: "More text"
 *
 * For subgroups, an optional unique id can be used to allow a particular name to
 * be used more than once. In the following case, from the command line the
 * following commands are available:
 *
 * >  root_1 shell status
 * >  root_2 shell status
 *
 *     - name: cli_group            # Register a group
 *       value:
 *         name: root_1             # Group name
 *
 *     - name: cli_group            # Register a group
 *       value:
 *         name: root_2             # Group name
 *
 *    - name: cli_group             # Register a group
 *       value:
 *         name: shell              # Group name
 *         id: shell_root_1         # Optional unique id for group
 *         group: root_1            # Add group to root_1 group
 *
 *    - name: cli_group             # Register a group
 *       value:
 *         name: shell              # Group name
 *         id: shell_root_2         # Optional unique id for group
 *         group: root_2            # Add group to root_1 group
 *
 *    - name: cli_command           # Register a command
 *       value:
 *         name: status
 *         handler: status_1
 *         group: shell_root_1      # id of subgroup
 *
 *    - name: cli_command           # Register a command
 *       value:
 *         name: status
 *         handler: status_2
 *         group: shell_root_2      # id of subgroup
 *
 ******************************************************************************/

// Provide function declarations
void app_join_fan10(sl_cli_command_arg_t *arguments);
void app_join_fan11(sl_cli_command_arg_t *arguments);
void app_join_explicit(sl_cli_command_arg_t *arguments);
void app_disconnect(sl_cli_command_arg_t *arguments);
void app_about(sl_cli_command_arg_t *arguments);
void app_set(sl_cli_command_arg_t *arguments);
void app_get(sl_cli_command_arg_t *arguments);
void app_save(sl_cli_command_arg_t *arguments);
void app_reset(sl_cli_command_arg_t *arguments);
void app_ping(sl_cli_command_arg_t *arguments);
void app_udp_client(sl_cli_command_arg_t *arguments);
void app_socket_close(sl_cli_command_arg_t *arguments);
void app_socket_write(sl_cli_command_arg_t *arguments);
void app_socket_read(sl_cli_command_arg_t *arguments);

// Command structs. Names are in the format : cli_cmd_{command group name}_{command name}
// In order to support hyphen in command and group name, every occurence of it while
// building struct names will be replaced by "_hyphen_"
static const sl_cli_command_info_t cli_cmd_wisun_join_fan10 = \
  SL_CLI_COMMAND(app_join_fan10,
                 "Connect to a Wi-SUN network using FAN1.0 settings",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_join_fan11 = \
  SL_CLI_COMMAND(app_join_fan11,
                 "Connect to a Wi-SUN network using FAN1.1 settings",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_join_explicit = \
  SL_CLI_COMMAND(app_join_explicit,
                 "Connect to a Wi-SUN network using explicit PHY settings",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_disconnect = \
  SL_CLI_COMMAND(app_disconnect,
                 "Disconnect from the Wi-SUN network",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd__about = \
  SL_CLI_COMMAND(app_about,
                 "Get info about the running app",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_set = \
  SL_CLI_COMMAND(app_set,
                 "Set a variable",
                  "empty | help | [string] Key [string] Value" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_WILDCARD, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_get = \
  SL_CLI_COMMAND(app_get,
                 "Get a variable",
                  "empty | help | [string] Key" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_WILDCARD, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_save = \
  SL_CLI_COMMAND(app_save,
                 "Save variables to non-volatile storage",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_reset = \
  SL_CLI_COMMAND(app_reset,
                 "Reset variables to default settings",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_ping = \
  SL_CLI_COMMAND(app_ping,
                 "Ping a remote host",
                  "Remote address" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_STRING, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_udp_client = \
  SL_CLI_COMMAND(app_udp_client,
                 "Open a UDP client socket",
                  "",
                 {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_socket_close = \
  SL_CLI_COMMAND(app_socket_close,
                 "Close a socket",
                  "Socket Id" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT32, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_socket_write = \
  SL_CLI_COMMAND(app_socket_write,
                 "Write to a socket",
                  "Socket Id" SL_CLI_UNIT_SEPARATOR "Remote address" SL_CLI_UNIT_SEPARATOR "Remote port" SL_CLI_UNIT_SEPARATOR "Data to write" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT32, SL_CLI_ARG_STRING, SL_CLI_ARG_UINT16, SL_CLI_ARG_STRING, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cli_cmd_wisun_socket_read = \
  SL_CLI_COMMAND(app_socket_read,
                 "Read from a socket",
                  "Socket Id" SL_CLI_UNIT_SEPARATOR "Amount of bytes to read" SL_CLI_UNIT_SEPARATOR,
                 {SL_CLI_ARG_UINT32, SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });


// Create group command tables and structs if cli_groups given
// in template. Group name is suffixed with _group_table for tables
// and group commands are cli_cmd_grp_( group name )
static const sl_cli_command_entry_t wisun_group_table[] = {
  { "join_fan10", &cli_cmd_wisun_join_fan10, false },
  { "j10", &cli_cmd_wisun_join_fan10, true },
  { "join_fan11", &cli_cmd_wisun_join_fan11, false },
  { "j11", &cli_cmd_wisun_join_fan11, true },
  { "join_explicit", &cli_cmd_wisun_join_explicit, false },
  { "je", &cli_cmd_wisun_join_explicit, true },
  { "disconnect", &cli_cmd_wisun_disconnect, false },
  { "d", &cli_cmd_wisun_disconnect, true },
  { "set", &cli_cmd_wisun_set, false },
  { "s", &cli_cmd_wisun_set, true },
  { "get", &cli_cmd_wisun_get, false },
  { "g", &cli_cmd_wisun_get, true },
  { "save", &cli_cmd_wisun_save, false },
  { "a", &cli_cmd_wisun_save, true },
  { "reset", &cli_cmd_wisun_reset, false },
  { "r", &cli_cmd_wisun_reset, true },
  { "ping", &cli_cmd_wisun_ping, false },
  { "p", &cli_cmd_wisun_ping, true },
  { "udp_client", &cli_cmd_wisun_udp_client, false },
  { "uc", &cli_cmd_wisun_udp_client, true },
  { "socket_close", &cli_cmd_wisun_socket_close, false },
  { "sc", &cli_cmd_wisun_socket_close, true },
  { "socket_write", &cli_cmd_wisun_socket_write, false },
  { "sw", &cli_cmd_wisun_socket_write, true },
  { "socket_read", &cli_cmd_wisun_socket_read, false },
  { "sr", &cli_cmd_wisun_socket_read, true },
  { NULL, NULL, false },
};
static const sl_cli_command_info_t cli_cmd_grp_wisun = \
  SL_CLI_COMMAND_GROUP(wisun_group_table, "Wi-SUN commands");

// Create root command table
const sl_cli_command_entry_t sl_cli_default_command_table[] = {
  { "about", &cli_cmd__about, false },
  { "a", &cli_cmd__about, true },
  { "wisun", &cli_cmd_grp_wisun, false },
  { "w", &cli_cmd_grp_wisun, true },
  { NULL, NULL, false },
};


#ifdef __cplusplus
}
#endif
