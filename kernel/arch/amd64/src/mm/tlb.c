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
#include <mm/tlb.h>
#include <arch/mm/tlb.h>
#include <arch/mm/asid.h>
#include <arch/asm.h>
#include <typedefs.h>
#include <mm/asid.h>

#if defined(CONFIG_ASID) && defined(CONFIG_ASID_FIFO)
#include <arch/cpuid.h>

#define CR3_PCID_MASK ((1 << CR3_PCID_BITS) - 1)
#define CR3_INVALIDATE_TARGET_MASK (0x7fffffffffffffff) /* bit 63 */

static bool in_long_mode()
{
	return read_msr(AMD_MSR_EFER) & AMD_LMA;
}

static bool invpcid_supported()
{
	cpu_info_t cpuid;
	cpuid_ext_flags(INTEL_CPUID_FEATURES, &cpuid);
	return cpuid.cpuid_ebx & (1 << 10);
}

static bool pcid_supported()
{

	if (!has_cpuid())
		return false;

	cpu_info_t info;
	cpuid(INTEL_CPUID_STANDARD, &info);

	bool pcid_feature_supported = (info.cpuid_ecx & (1 << 17)) != 0;

	return pcid_feature_supported && in_long_mode() && !pcid_is_enabled();
}

static bool pcid_enable()
{
	if (pcid_supported()) {
		write_cr4(read_cr4() | CR4_PCIDE);
		return true;
	}
	return false;
}

static asid_t pcid_get_current()
{
	return read_cr3() & CR3_PCID_MASK;
}

static void pcid_set_invalidate(asid_t pcid)
{
	/*
	 * clear bit 63 to invalidate the target PCID
	 * MOV to CR3 with PCIDE = 1 and bit 63 of operand = 0
	 * => invalidates all TLB entries assoc. with the PCID in bits 0-11 (exc. global)
	 */
	write_cr3(((read_cr3() & (~CR3_PCID_MASK)) | (pcid & CR3_PCID_MASK)) & CR3_INVALIDATE_TARGET_MASK);
}

inline void pcid_set_no_invalidate(asid_t pcid)
{
	/*
	 * set bit 63 to prevent invalidation
	 * see pcid_set_invalidate above
	 * => insn NOT required to invalidate anything
	 */
	write_cr3(((read_cr3() & (~CR3_PCID_MASK)) | (pcid & CR3_PCID_MASK)) | (~CR3_INVALIDATE_TARGET_MASK));
}

static void pcid_invalidate_no_invpcid_assume_pcide(asid_t pcid)
{
	asid_t curr_pcid = pcid_get_current();
	/* Without INVPCID, we must first invalidate the supplied PCID */
	pcid_set_invalidate(pcid);
	/* and restore the PCID of the caller */
	pcid_set_no_invalidate(curr_pcid);
}

static void pcid_invalidate_assume_pcide(asid_t pcid)
{
	if (invpcid_supported()) {
		invpcid(pcid, 0, INVPCID_TYPE_SINGLE_PCID);
	} else {
		pcid_invalidate_no_invpcid_assume_pcide(pcid);
	}
}
#endif

/** Invalidate all entries in TLB. */
void tlb_invalidate_all(void)
{
#if defined(CONFIG_ASID) && defined(CONFIG_ASID_FIFO)
	if (pcid_is_enabled() && invpcid_supported()) {
		invpcid(0, 0, INVPCID_TYPE_EVERYTHING);
		return;
	}
#endif
	write_cr3(read_cr3());
}

/** Invalidate all entries in TLB that belong to specified address space.
 *
 * @param asid This parameter is ignored as the architecture doesn't support it.
 */
void tlb_invalidate_asid(asid_t asid)
{
#if defined(CONFIG_ASID) && defined(CONFIG_ASID_FIFO)
	if (pcid_is_enabled()) {
		pcid_invalidate_assume_pcide(asid);
		return;
	}
#endif
	/* (MOV to CR3 with PCIDE = 0) invalidates all except global mappings */
	tlb_invalidate_all();
}

/** Invalidate TLB entries for specified page range belonging to specified address space.
 *
 * @param asid This parameter is ignored as the architecture doesn't support it.
 * @param page Address of the first page whose entry is to be invalidated.
 * @param cnt Number of entries to invalidate.
 */
void tlb_invalidate_pages(asid_t asid, uintptr_t page, size_t cnt)
{
#if defined(CONFIG_ASID) && defined(CONFIG_ASID_FIFO)
	asid_t curr_pcid = ASID_INVALID;
	if (pcid_is_enabled() && !invpcid_supported()) {
		curr_pcid = pcid_get_current();
		if (curr_pcid != asid) {
			/*
			 * set PCID to target PCID
			 * - INVLPG invalidates TLB entries ...corresponding to the addr... & current PCID
			 */
			pcid_set_no_invalidate(asid);
		}
	}
#endif
	/*
	 * INVPCID expects canonical address, at the same time INVLPG:
	 * "if the memory address is in non-canonical form. In this case, INVLPG is the same as a NOP."
	 * therefore assuming we always work with canonical addresses
	 */
	unsigned int i;
	for (i = 0; i < cnt; i++) {
#if defined(CONFIG_ASID) && defined(CONFIG_ASID_FIFO)
		if (pcid_is_enabled() && invpcid_supported()) {
			invpcid(asid, page + i * PAGE_SIZE, INVPCID_TYPE_SINGLE_ADDR);
			continue;
		}
#endif

		invlpg(page + i * PAGE_SIZE);
	}
#if defined(CONFIG_ASID) && defined(CONFIG_ASID_FIFO)
	if (pcid_is_enabled() && !invpcid_supported() && curr_pcid != asid) {
		pcid_set_no_invalidate(curr_pcid);
	}
#endif
}

void tlb_arch_init(void)
{
#if defined(CONFIG_ASID) && defined(CONFIG_ASID_FIFO)
	if (pcid_supported()) {
		pcid_enable();
	}
#endif
}

void tlb_print(void)
{
#if defined(CONFIG_ASID)
	bool pcide = pcid_is_enabled();
	printf("PCID CPU feature is %s\n", (pcide ? "Enabled" : "Disabled"));
	printf("INVPCID instruction is %s\n", ((pcide && invpcid_supported()) ? "Available" : "Not Available"));
#endif
#if defined(CONFIG_ASID_FIFO)
	printf("Number of slots in ASID Queue (may not be used): %d\n", ASIDS_ALLOCABLE);
	printf("ASID fallback value: %d\n", ASID_FALLBACK_NO_SIDE_EFFECT);
	printf("Using ASID Queue: %s\n", (asid_force_fallback() ? "N" : "Y"));
#endif
}

/** @}
 */
