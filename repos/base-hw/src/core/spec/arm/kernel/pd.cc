/*
 * \brief   Kernel backend for protection domains
 * \author  Stefan Kalkowski
 * \date    2015-03-20
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <util/bit_allocator.h>

/* core includes */
#include <assert.h>
#include <kernel/pd.h>

using Asid_allocator = Genode::Bit_allocator<256>;

static Asid_allocator &alloc() {
	return *unmanaged_singleton<Asid_allocator>(); }


Kernel::Pd::Pd(Kernel::Pd::Table * const table,
               Genode::Platform_pd * const platform_pd)
: Kernel::Cpu::Pd((Genode::uint8_t)alloc().alloc()),
  _table(table), _platform_pd(platform_pd)
{
	capid_t invalid = _capid_alloc.alloc();
	assert(invalid == cap_id_invalid());
}


Kernel::Pd::~Pd() {

	while (Object_identity_reference *oir = _cap_tree.first())
		oir->~Object_identity_reference();

	/* clean up buffers of memory management */
	Cpu * const cpu  = cpu_pool()->cpu(Cpu::executing_id());
	cpu->clean_invalidate_data_cache();
	cpu->invalidate_instr_cache();
	cpu->invalidate_tlb_by_pid(asid);
	alloc().free(asid);
}


void Kernel::Pd::admit(Kernel::Cpu::Context * const c)
{
	c->protection_domain(asid);
	c->translation_table((addr_t)translation_table());
}
