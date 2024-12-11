/*
 * Copyright (c) 2024 Roman Vasut
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
#include <arch/mm/as.h>
#include <genarch/mm/page_pt.h>

#if defined(CONFIG_ASID) && defined(CONFIG_ASID_FIFO)
#include <genarch/mm/asid_fifo.h>
#include <arch/asm.h>
#include <arch/mm/tlb.h>

void as_install_arch(as_t *as)
{
	if (!pcid_is_enabled())
		return;

	pcid_set_no_invalidate(as->asid);
}

void as_deinstall_arch(as_t *as)
{
	tlb_invalidate_asid(as->asid);
}
#endif

/** Architecture dependent address space init. */
void as_arch_init(void)
{
	as_operations = &as_pt_operations;
#if defined(CONFIG_ASID) && defined(CONFIG_ASID_FIFO)
	asid_fifo_init();
#endif
}

/** @}
 */
