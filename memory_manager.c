#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/highmem.h>
#include <linux/pid.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/swap.h>
#include <linux/swapops.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aryan Shah");

// Module parameters
static int pid = 0; // Process ID monitor
static unsigned long long addr = 0; // Virtual address in process mem space
module_param(pid, int, 0);
module_param(addr, ullong, 0);
MODULE_PARM_DESC(pid, "Process ID");
MODULE_PARM_DESC(addr, "Virtual Address");

static int __init memory_manager_init(void)
{
    struct task_struct *proc_task; // Holds process
    struct mm_struct *memory_space; // Process memory management structure
    pgd_t *pgd; // Global Directory
    p4d_t *p4d; // 4th Directory
    pud_t *pud; // Upper Directory
    pmd_t *pmd; // Middle Directory
    pte_t *pte; // Table Entry

    printk(KERN_INFO "[DEBUG] Module loaded, attempting to reslove address...\n");

    // Retrieve the task_struct for the given PID
    proc_task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!proc_task) {
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d] not found.\n", pid);
        return -EINVAL;
    }

    // Get the memory management structure for the process
    memory_space = proc_task->mm;
    if (!memory_space) {
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d] has no memory mapped.\n", pid);
        return -EINVAL;
    }

    // Start walking the page table

    pgd = pgd_offset(memory_space, addr); // Retrieve
    if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd))) {
        //printk(KERN_INFO "[DEBUG] Invalid detected\n");
        goto handle_invalid_page;
    }

    p4d = p4d_offset(pgd, addr); // Retrieve
    if (p4d_none(*p4d) || unlikely(p4d_bad(*p4d))) {
        //printk(KERN_INFO "[DEBUG] Invalid detected\n");
        goto handle_invalid_page;
    }

    pud = pud_offset(p4d, addr); // Retrieve
    if (pud_none(*pud) || unlikely(pud_bad(*pud))) {
        printk(KERN_INFO "[DEBUG] Invalid detected\n");
        goto handle_invalid_page;
    }

    pmd = pmd_offset(pud, addr); // Retrieve
    if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd))) {
        //printk(KERN_INFO "[DEBUG] Invalid detected\n");
        goto handle_invalid_page;
    }

    pte = pte_offset_kernel(pmd, addr); // Retrieve
    if (!pte || pte_none(*pte)) {
        printk(KERN_INFO "[DEBUG] PTE entry is null/invalid\n");
        goto handle_invalid_page;
    }

    // Check if the page is present in memory
    if (pte_present(*pte)) {
        unsigned long pfn = pte_pfn(*pte); // Get page frame number
        unsigned long physical_addr = (pfn << PAGE_SHIFT) | (addr & ~PAGE_MASK);
        printk(KERN_INFO "[CSE330-Memory-Manager] PID [%d]: virtual address [0x%llx] physical address [0x%lx] swap identifier [NA]\n",
               pid, addr, physical_addr);
    } else {
        // Check if swapped
        if (!pte_none(*pte)) {
            swp_entry_t swap_entry = pte_to_swp_entry(*pte); // Convert PTE to swap entry
            printk(KERN_INFO "[CSE330-Memory-Manager] PID [%d]: virtual address [0x%llx] physical address [NA] swap identifier [0x%lx]\n",
                   pid, addr, swap_entry.val);
            rcu_read_unlock();
            return 0;
        }
    }

    rcu_read_unlock();
    return 0;

handle_invalid_page:
    // Handle if address is invalid
    printk(KERN_INFO "[CSE330-Memory-Manager] PID [%d]: virtual address [0x%llx] physical address [NA] swap identifier [NA]\n",
           pid, addr);
    rcu_read_unlock();
    return -EFAULT;
}

static void __exit memory_manager_exit(void)
{
    printk(KERN_INFO "[DEBUG] Module unloaded successfully.\n");
}

module_init(memory_manager_init);
module_exit(memory_manager_exit);

