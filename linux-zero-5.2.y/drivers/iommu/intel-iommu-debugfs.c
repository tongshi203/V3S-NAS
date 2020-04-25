// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright © 2018 Intel Corporation.
 *
 * Authors: Gayatri Kammela <gayatri.kammela@intel.com>
 *	    Sohil Mehta <sohil.mehta@intel.com>
 *	    Jacob Pan <jacob.jun.pan@linux.intel.com>
 */

#include <linux/debugfs.h>
#include <linux/dmar.h>
#include <linux/intel-iommu.h>
#include <linux/pci.h>

#include <asm/irq_remapping.h>

#include "intel-pasid.h"

struct tbl_walk {
	u16 bus;
	u16 devfn;
	u32 pasid;
	struct root_entry *rt_entry;
	struct context_entry *ctx_entry;
	struct pasid_entry *pasid_tbl_entry;
};

struct iommu_regset {
	int offset;
	const char *regs;
};

#define IOMMU_REGSET_ENTRY(_reg_)					\
	{ DMAR_##_reg_##_REG, __stringify(_reg_) }
static const struct iommu_regset iommu_regs[] = {
	IOMMU_REGSET_ENTRY(VER),
	IOMMU_REGSET_ENTRY(CAP),
	IOMMU_REGSET_ENTRY(ECAP),
	IOMMU_REGSET_ENTRY(GCMD),
	IOMMU_REGSET_ENTRY(GSTS),
	IOMMU_REGSET_ENTRY(RTADDR),
	IOMMU_REGSET_ENTRY(CCMD),
	IOMMU_REGSET_ENTRY(FSTS),
	IOMMU_REGSET_ENTRY(FECTL),
	IOMMU_REGSET_ENTRY(FEDATA),
	IOMMU_REGSET_ENTRY(FEADDR),
	IOMMU_REGSET_ENTRY(FEUADDR),
	IOMMU_REGSET_ENTRY(AFLOG),
	IOMMU_REGSET_ENTRY(PMEN),
	IOMMU_REGSET_ENTRY(PLMBASE),
	IOMMU_REGSET_ENTRY(PLMLIMIT),
	IOMMU_REGSET_ENTRY(PHMBASE),
	IOMMU_REGSET_ENTRY(PHMLIMIT),
	IOMMU_REGSET_ENTRY(IQH),
	IOMMU_REGSET_ENTRY(IQT),
	IOMMU_REGSET_ENTRY(IQA),
	IOMMU_REGSET_ENTRY(ICS),
	IOMMU_REGSET_ENTRY(IRTA),
	IOMMU_REGSET_ENTRY(PQH),
	IOMMU_REGSET_ENTRY(PQT),
	IOMMU_REGSET_ENTRY(PQA),
	IOMMU_REGSET_ENTRY(PRS),
	IOMMU_REGSET_ENTRY(PECTL),
	IOMMU_REGSET_ENTRY(PEDATA),
	IOMMU_REGSET_ENTRY(PEADDR),
	IOMMU_REGSET_ENTRY(PEUADDR),
	IOMMU_REGSET_ENTRY(MTRRCAP),
	IOMMU_REGSET_ENTRY(MTRRDEF),
	IOMMU_REGSET_ENTRY(MTRR_FIX64K_00000),
	IOMMU_REGSET_ENTRY(MTRR_FIX16K_80000),
	IOMMU_REGSET_ENTRY(MTRR_FIX16K_A0000),
	IOMMU_REGSET_ENTRY(MTRR_FIX4K_C0000),
	IOMMU_REGSET_ENTRY(MTRR_FIX4K_C8000),
	IOMMU_REGSET_ENTRY(MTRR_FIX4K_D0000),
	IOMMU_REGSET_ENTRY(MTRR_FIX4K_D8000),
	IOMMU_REGSET_ENTRY(MTRR_FIX4K_E0000),
	IOMMU_REGSET_ENTRY(MTRR_FIX4K_E8000),
	IOMMU_REGSET_ENTRY(MTRR_FIX4K_F0000),
	IOMMU_REGSET_ENTRY(MTRR_FIX4K_F8000),
	IOMMU_REGSET_ENTRY(MTRR_PHYSBASE0),
	IOMMU_REGSET_ENTRY(MTRR_PHYSMASK0),
	IOMMU_REGSET_ENTRY(MTRR_PHYSBASE1),
	IOMMU_REGSET_ENTRY(MTRR_PHYSMASK1),
	IOMMU_REGSET_ENTRY(MTRR_PHYSBASE2),
	IOMMU_REGSET_ENTRY(MTRR_PHYSMASK2),
	IOMMU_REGSET_ENTRY(MTRR_PHYSBASE3),
	IOMMU_REGSET_ENTRY(MTRR_PHYSMASK3),
	IOMMU_REGSET_ENTRY(MTRR_PHYSBASE4),
	IOMMU_REGSET_ENTRY(MTRR_PHYSMASK4),
	IOMMU_REGSET_ENTRY(MTRR_PHYSBASE5),
	IOMMU_REGSET_ENTRY(MTRR_PHYSMASK5),
	IOMMU_REGSET_ENTRY(MTRR_PHYSBASE6),
	IOMMU_REGSET_ENTRY(MTRR_PHYSMASK6),
	IOMMU_REGSET_ENTRY(MTRR_PHYSBASE7),
	IOMMU_REGSET_ENTRY(MTRR_PHYSMASK7),
	IOMMU_REGSET_ENTRY(MTRR_PHYSBASE8),
	IOMMU_REGSET_ENTRY(MTRR_PHYSMASK8),
	IOMMU_REGSET_ENTRY(MTRR_PHYSBASE9),
	IOMMU_REGSET_ENTRY(MTRR_PHYSMASK9),
	IOMMU_REGSET_ENTRY(VCCAP),
	IOMMU_REGSET_ENTRY(VCMD),
	IOMMU_REGSET_ENTRY(VCRSP),
};

static int iommu_regset_show(struct seq_file *m, void *unused)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;
	unsigned long flag;
	int i, ret = 0;
	u64 value;

	rcu_read_lock();
	for_each_active_iommu(iommu, drhd) {
		if (!drhd->reg_base_addr) {
			seq_puts(m, "IOMMU: Invalid base address\n");
			ret = -EINVAL;
			goto out;
		}

		seq_printf(m, "IOMMU: %s Register Base Address: %llx\n",
			   iommu->name, drhd->reg_base_addr);
		seq_puts(m, "Name\t\t\tOffset\t\tContents\n");
		/*
		 * Publish the contents of the 64-bit hardware registers
		 * by adding the offset to the pointer (virtual address).
		 */
		raw_spin_lock_irqsave(&iommu->register_lock, flag);
		for (i = 0 ; i < ARRAY_SIZE(iommu_regs); i++) {
			value = dmar_readq(iommu->reg + iommu_regs[i].offset);
			seq_printf(m, "%-16s\t0x%02x\t\t0x%016llx\n",
				   iommu_regs[i].regs, iommu_regs[i].offset,
				   value);
		}
		raw_spin_unlock_irqrestore(&iommu->register_lock, flag);
		seq_putc(m, '\n');
	}
out:
	rcu_read_unlock();

	return ret;
}
DEFINE_SHOW_ATTRIBUTE(iommu_regset);

static inline void print_tbl_walk(struct seq_file *m)
{
	struct tbl_walk *tbl_wlk = m->private;

	seq_printf(m, "%02x:%02x.%x\t0x%016llx:0x%016llx\t0x%016llx:0x%016llx\t",
		   tbl_wlk->bus, PCI_SLOT(tbl_wlk->devfn),
		   PCI_FUNC(tbl_wlk->devfn), tbl_wlk->rt_entry->hi,
		   tbl_wlk->rt_entry->lo, tbl_wlk->ctx_entry->hi,
		   tbl_wlk->ctx_entry->lo);

	/*
	 * A legacy mode DMAR doesn't support PASID, hence default it to -1
	 * indicating that it's invalid. Also, default all PASID related fields
	 * to 0.
	 */
	if (!tbl_wlk->pasid_tbl_entry)
		seq_printf(m, "%-6d\t0x%016llx:0x%016llx:0x%016llx\n", -1,
			   (u64)0, (u64)0, (u64)0);
	else
		seq_printf(m, "%-6d\t0x%016llx:0x%016llx:0x%016llx\n",
			   tbl_wlk->pasid, tbl_wlk->pasid_tbl_entry->val[0],
			   tbl_wlk->pasid_tbl_entry->val[1],
			   tbl_wlk->pasid_tbl_entry->val[2]);
}

static void pasid_tbl_walk(struct seq_file *m, struct pasid_entry *tbl_entry,
			   u16 dir_idx)
{
	struct tbl_walk *tbl_wlk = m->private;
	u8 tbl_idx;

	for (tbl_idx = 0; tbl_idx < PASID_TBL_ENTRIES; tbl_idx++) {
		if (pasid_pte_is_present(tbl_entry)) {
			tbl_wlk->pasid_tbl_entry = tbl_entry;
			tbl_wlk->pasid = (dir_idx << PASID_PDE_SHIFT) + tbl_idx;
			print_tbl_walk(m);
		}

		tbl_entry++;
	}
}

static void pasid_dir_walk(struct seq_file *m, u64 pasid_dir_ptr,
			   u16 pasid_dir_size)
{
	struct pasid_dir_entry *dir_entry = phys_to_virt(pasid_dir_ptr);
	struct pasid_entry *pasid_tbl;
	u16 dir_idx;

	for (dir_idx = 0; dir_idx < pasid_dir_size; dir_idx++) {
		pasid_tbl = get_pasid_table_from_pde(dir_entry);
		if (pasid_tbl)
			pasid_tbl_walk(m, pasid_tbl, dir_idx);

		dir_entry++;
	}
}

static void ctx_tbl_walk(struct seq_file *m, struct intel_iommu *iommu, u16 bus)
{
	struct context_entry *context;
	u16 devfn, pasid_dir_size;
	u64 pasid_dir_ptr;

	for (devfn = 0; devfn < 256; devfn++) {
		struct tbl_walk tbl_wlk = {0};

		/*
		 * Scalable mode root entry points to upper scalable mode
		 * context table and lower scalable mode context table. Each
		 * scalable mode context table has 128 context entries where as
		 * legacy mode context table has 256 context entries. So in
		 * scalable mode, the context entries for former 128 devices are
		 * in the lower scalable mode context table, while the latter
		 * 128 devices are in the upper scalable mode context table.
		 * In scalable mode, when devfn > 127, iommu_context_addr()
		 * automatically refers to upper scalable mode context table and
		 * hence the caller doesn't have to worry about differences
		 * between scalable mode and non scalable mode.
		 */
		context = iommu_context_addr(iommu, bus, devfn, 0);
		if (!context)
			return;

		if (!context_present(context))
			continue;

		tbl_wlk.bus = bus;
		tbl_wlk.devfn = devfn;
		tbl_wlk.rt_entry = &iommu->root_entry[bus];
		tbl_wlk.ctx_entry = context;
		m->private = &tbl_wlk;

		if (pasid_supported(iommu) && is_pasid_enabled(context)) {
			pasid_dir_ptr = context->lo & VTD_PAGE_MASK;
			pasid_dir_size = get_pasid_dir_size(context);
			pasid_dir_walk(m, pasid_dir_ptr, pasid_dir_size);
			continue;
		}

		print_tbl_walk(m);
	}
}

static void root_tbl_walk(struct seq_file *m, struct intel_iommu *iommu)
{
	unsigned long flags;
	u16 bus;

	spin_lock_irqsave(&iommu->lock, flags);
	seq_printf(m, "IOMMU %s: Root Table Address: 0x%llx\n", iommu->name,
		   (u64)virt_to_phys(iommu->root_entry));
	seq_puts(m, "B.D.F\tRoot_entry\t\t\t\tContext_entry\t\t\t\tPASID\tPASID_table_entry\n");

	/*
	 * No need to check if the root entry is present or not because
	 * iommu_context_addr() performs the same check before returning
	 * context entry.
	 */
	for (bus = 0; bus < 256; bus++)
		ctx_tbl_walk(m, iommu, bus);

	spin_unlock_irqrestore(&iommu->lock, flags);
}

static int dmar_translation_struct_show(struct seq_file *m, void *unused)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;

	rcu_read_lock();
	for_each_active_iommu(iommu, drhd) {
		root_tbl_walk(m, iommu);
		seq_putc(m, '\n');
	}
	rcu_read_unlock();

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(dmar_translation_struct);

#ifdef CONFIG_IRQ_REMAP
static void ir_tbl_remap_entry_show(struct seq_file *m,
				    struct intel_iommu *iommu)
{
	struct irte *ri_entry;
	unsigned long flags;
	int idx;

	seq_puts(m, " Entry SrcID   DstID    Vct IRTE_high\t\tIRTE_low\n");

	raw_spin_lock_irqsave(&irq_2_ir_lock, flags);
	for (idx = 0; idx < INTR_REMAP_TABLE_ENTRIES; idx++) {
		ri_entry = &iommu->ir_table->base[idx];
		if (!ri_entry->present || ri_entry->p_pst)
			continue;

		seq_printf(m, " %-5d %02x:%02x.%01x %08x %02x  %016llx\t%016llx\n",
			   idx, PCI_BUS_NUM(ri_entry->sid),
			   PCI_SLOT(ri_entry->sid), PCI_FUNC(ri_entry->sid),
			   ri_entry->dest_id, ri_entry->vector,
			   ri_entry->high, ri_entry->low);
	}
	raw_spin_unlock_irqrestore(&irq_2_ir_lock, flags);
}

static void ir_tbl_posted_entry_show(struct seq_file *m,
				     struct intel_iommu *iommu)
{
	struct irte *pi_entry;
	unsigned long flags;
	int idx;

	seq_puts(m, " Entry SrcID   PDA_high PDA_low  Vct IRTE_high\t\tIRTE_low\n");

	raw_spin_lock_irqsave(&irq_2_ir_lock, flags);
	for (idx = 0; idx < INTR_REMAP_TABLE_ENTRIES; idx++) {
		pi_entry = &iommu->ir_table->base[idx];
		if (!pi_entry->present || !pi_entry->p_pst)
			continue;

		seq_printf(m, " %-5d %02x:%02x.%01x %08x %08x %02x  %016llx\t%016llx\n",
			   idx, PCI_BUS_NUM(pi_entry->sid),
			   PCI_SLOT(pi_entry->sid), PCI_FUNC(pi_entry->sid),
			   pi_entry->pda_h, pi_entry->pda_l << 6,
			   pi_entry->vector, pi_entry->high,
			   pi_entry->low);
	}
	raw_spin_unlock_irqrestore(&irq_2_ir_lock, flags);
}

/*
 * For active IOMMUs go through the Interrupt remapping
 * table and print valid entries in a table format for
 * Remapped and Posted Interrupts.
 */
static int ir_translation_struct_show(struct seq_file *m, void *unused)
{
	struct dmar_drhd_unit *drhd;
	struct intel_iommu *iommu;
	u64 irta;

	rcu_read_lock();
	for_each_active_iommu(iommu, drhd) {
		if (!ecap_ir_support(iommu->ecap))
			continue;

		seq_printf(m, "Remapped Interrupt supported on IOMMU: %s\n",
			   iommu->name);

		if (iommu->ir_table) {
			irta = virt_to_phys(iommu->ir_table->base);
			seq_printf(m, " IR table address:%llx\n", irta);
			ir_tbl_remap_entry_show(m, iommu);
		} else {
			seq_puts(m, "Interrupt Remapping is not enabled\n");
		}
		seq_putc(m, '\n');
	}

	seq_puts(m, "****\n\n");

	for_each_active_iommu(iommu, drhd) {
		if (!cap_pi_support(iommu->cap))
			continue;

		seq_printf(m, "Posted Interrupt supported on IOMMU: %s\n",
			   iommu->name);

		if (iommu->ir_table) {
			irta = virt_to_phys(iommu->ir_table->base);
			seq_printf(m, " IR table address:%llx\n", irta);
			ir_tbl_posted_entry_show(m, iommu);
		} else {
			seq_puts(m, "Interrupt Remapping is not enabled\n");
		}
		seq_putc(m, '\n');
	}
	rcu_read_unlock();

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(ir_translation_struct);
#endif

void __init intel_iommu_debugfs_init(void)
{
	struct dentry *intel_iommu_debug = debugfs_create_dir("intel",
						iommu_debugfs_dir);

	debugfs_create_file("iommu_regset", 0444, intel_iommu_debug, NULL,
			    &iommu_regset_fops);
	debugfs_create_file("dmar_translation_struct", 0444, intel_iommu_debug,
			    NULL, &dmar_translation_struct_fops);
#ifdef CONFIG_IRQ_REMAP
	debugfs_create_file("ir_translation_struct", 0444, intel_iommu_debug,
			    NULL, &ir_translation_struct_fops);
#endif
}