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
#ifndef FUZZ_REPLAY_H
#define FUZZ_REPLAY_H

#include <stdint.h>

struct fuzz_ctxt;
struct iobuf_read;
struct rcp;
struct wsbr_ctxt;

void fuzz_ind_replay_timers(struct rcp *rcp, struct iobuf_read *buf);
void fuzz_trigger_timer(struct fuzz_ctxt *ctxt);

#endif
