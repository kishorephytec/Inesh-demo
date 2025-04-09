/*
 * SPDX-License-Identifier: LicenseRef-MSLA
 * Copyright (c) 2024 Silicon Laboratories Inc. (www.silabs.com)
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at [1].  This software is distributed to you in
 * Object Code format and/or Source Code format and is governed by the sections
 * of the MSLA applicable to Object Code, Source Code and Modified Open Source
 * Code. By using this software, you agree to the terms of the MSLA.
 *
 * [1]: https://www.silabs.com/about-us/legal/master-software-license-agreement
 */
#ifndef DC_WS_H
#define DC_WS_H

struct ws_frame_ctx;
struct timer_group;
struct timer_entry;
struct rcp_tx_cnf;
struct ws_ctx;
struct ws_ind;
struct dc;

void ws_on_probe_timer_timeout(struct timer_group *group, struct timer_entry *timer);

void ws_on_recv_ind(struct ws_ctx *ws, struct ws_ind *ind);
void ws_on_recv_cnf(struct ws_ctx *ws, struct ws_frame_ctx *frame_ctx, const struct rcp_tx_cnf *cnf);
void ws_recvfrom_tun(struct dc *dc);

#endif
