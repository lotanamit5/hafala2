#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/errno.h>

asmlinkage long sys_hello(void)
{
    printk("Hello, World!\n");
    return 0;
}

asmlinkage int sys_set_status(int status)
{
    if (status != 0 && status != 1)
    {
        printk("Error: status must be 0 or 1\n");
        return -EINVAL;
    }

    current->status = status;
    return 0;
}

asmlinkage int sys_get_status(void)
{
    return current->status;
}
asmlinkage int sys_register_process(void)
{
    printk("HW2: Registered %d\n", current->tgid);
    list_add_tail(&current->imp_entry, current->imp_list);
    return 0;
}

asmlinkage long sys_get_all_cs(void)
{
    struct list_head *ptr;
    struct task_struct *entry;
    long sum = 0;

    if (current->imp_list->next == current->imp_list)
    {
        printk("Error: no processes registered\n");
        return -ENODATA;
    }


    // debugging
    printk("HW2: getting all important cs processes\n");
    for (ptr = current->imp_list->next; ptr != current->imp_list; ptr = ptr->next) {
        entry = list_entry(ptr, struct task_struct, imp_entry);
        printk("HW2: process: %d, status: %d\n", entry->tgid, entry->status);
        if(entry->status == 1)
        {
            sum += entry->tgid;
        }
    }

    return sum;
}