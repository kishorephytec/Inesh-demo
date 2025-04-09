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
#ifndef AUTHENTICATOR_EAP_H
#define AUTHENTICATOR_EAP_H

#include <stddef.h>

struct auth_ctx;
struct auth_supp_ctx;
struct pktbuf;

void auth_eap_recv(struct auth_ctx *auth, struct auth_supp_ctx *supp, const void *buf, size_t buf_len);
void auth_eap_send(struct auth_ctx *auth, struct auth_supp_ctx *supp, struct pktbuf *pktbuf);
void auth_eap_send_request_identity(struct auth_ctx *auth, struct auth_supp_ctx *supp);
void auth_eap_send_failure(struct auth_ctx *auth, struct auth_supp_ctx *supp);
void auth_supp_eap_init(struct auth_ctx *auth, struct auth_supp_ctx *supp);

#endif
