/*
 * Copyright (c) 2005 Jakub Jermar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup kernel_amd64_mm
 * @{
 */
/** @file
 * @ingroup kernel_amd64_mm
 */

#ifndef KERN_amd64_ASID_H_
#define KERN_amd64_ASID_H_

#include <stdint.h>
#include <arch/asm.h>

typedef int32_t asid_t;

#if defined(CONFIG_ASID) && defined(CONFIG_ASID_FIFO)
#define CR3_PCID_BITS 12
#define ASID_MAX_ARCH (1 << CR3_PCID_BITS) /* 4096 */

/* "is enabled" instead of "enabled" to avoid name simialrity with "pcid_enable" */
#define pcid_is_enabled() (read_cr4() & CR4_PCIDE)

#define asid_force_fallback() (!pcid_is_enabled())

#include <genarch/mm/asid_fifo.h>
#else
#define ASID_MAX_ARCH  3

#define asid_get()  (ASID_START + 1)
#define asid_put(asid)
#endif
#endif

/** @}
 */
