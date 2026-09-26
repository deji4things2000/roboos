/*
 * RILP Kernel Module - Deadline Enforcement via Priority Mapping
 *
 * Receives RILP message headers from userspace, reads the deadline_us
 * field, and sets the target thread's scheduling policy to SCHED_FIFO
 * with priority based on the deadline.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netlink.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <net/sock.h>

#define RILP_NETLINK_FAMILY 31
#define RILP_HEADER_SIZE 16

MODULE_LICENSE("GPL");
MODULE_AUTHOR("RoboOS");
MODULE_DESCRIPTION("RILP Kernel Module - Deadline to Priority Mapping");
MODULE_VERSION("0.1");

static struct {
    u32 messages_received;
    u32 deadlines_mapped;
    u32 errors;
    u32 emergency_stops;
} rilp_stats;

static struct sock *rilp_nl_sock = NULL;
static struct proc_dir_entry *rilp_proc_entry;

/* Priority mapping based on deadline_us */
static int rilp_deadline_to_priority(u32 deadline_us)
{
    if (deadline_us < 100)
        return 99;      /* Hard real-time */
    if (deadline_us < 1000)
        return 80;      /* Soft real-time */
    if (deadline_us < 10000)
        return 50;      /* Best-effort */
    return 0;           /* Non-RT (SCHED_OTHER) */
}

static void rilp_apply_scheduling(struct task_struct *task, u32 deadline_us)
{
    int priority;
    struct sched_param param = { .sched_priority = 0 };

    priority = rilp_deadline_to_priority(deadline_us);

    if (priority > 0) {
        param.sched_priority = priority;
        sched_setscheduler(task, SCHED_FIFO, &param);
        pr_debug("RILP: pid=%d deadline=%uus priority=%d SCHED_FIFO\n",
                 task->pid, deadline_us, priority);
    } else {
        sched_setscheduler(task, SCHED_OTHER, &param);
        pr_debug("RILP: pid=%d deadline=%uus SCHED_OTHER\n",
                 task->pid, deadline_us);
    }
    rilp_stats.deadlines_mapped++;
}

static void rilp_process_message(const u8 *data, size_t len)
{
    u32 deadline_us, target_pid;
    struct task_struct *task;
    u8 msg_type;

    if (len < RILP_HEADER_SIZE) {
        pr_warn("RILP: Message too short (%zu bytes)\n", len);
        rilp_stats.errors++;
        return;
    }

    msg_type = data[3];
    deadline_us = *(u32 *)(data + 8);
    target_pid = *(u32 *)(data + 12);
    rilp_stats.messages_received++;

    if (msg_type == 0xFF) {
        pr_alert("RILP: EMERGENCY STOP broadcast\n");
        rilp_stats.emergency_stops++;
        return;
    }

    rcu_read_lock();
    task = pid_task(find_vpid(target_pid), PIDTYPE_PID);
    if (task) {
        get_task_struct(task);
        rcu_read_unlock();
        rilp_apply_scheduling(task, deadline_us);
        put_task_struct(task);
    } else {
        rcu_read_unlock();
        pr_warn("RILP: No task for pid=%u\n", target_pid);
        rilp_stats.errors++;
    }
}

static void rilp_netlink_recv(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    u8 *data;
    u32 pid;

    nlh = (struct nlmsghdr *)skb->data;
    if (nlh->nlmsg_len < sizeof(struct nlmsghdr))
        return;

    data = (u8 *)nlmsg_data(nlh);
    if (nlh->nlmsg_len >= sizeof(u32)) {
        pid = *(u32 *)data;
        data += sizeof(u32);
        rilp_process_message(data,
            nlh->nlmsg_len - sizeof(struct nlmsghdr) - sizeof(u32));
    }
}

static struct netlink_kernel_cfg rilp_nl_cfg = {
    .input = rilp_netlink_recv,
};

static ssize_t rilp_proc_write(struct file *file, const char __user *buf,
                                size_t count, loff_t *pos)
{
    char kernel_buf[64];
    u32 deadline_us, target_pid;
    u8 rilp_header[RILP_HEADER_SIZE];

    if (count > sizeof(kernel_buf) - 1)
        count = sizeof(kernel_buf) - 1;

    if (copy_from_user(kernel_buf, buf, count))
        return -EFAULT;
    kernel_buf[count] = '\0';

    if (sscanf(kernel_buf, "%u %u", &deadline_us, &target_pid) != 2)
        return -EINVAL;

    memset(rilp_header, 0, RILP_HEADER_SIZE);
    rilp_header[0] = 1;
    rilp_header[1] = 2;
    rilp_header[2] = 1;
    rilp_header[3] = 0x04;
    *(u32 *)(rilp_header + 8) = deadline_us;
    *(u32 *)(rilp_header + 12) = target_pid;

    rilp_process_message(rilp_header, RILP_HEADER_SIZE);
    return count;
}

static const struct proc_ops rilp_proc_ops = {
    .proc_write = rilp_proc_write,
};

static int __init rilp_module_init(void)
{
    pr_info("========================================\n");
    pr_info("  RILP Kernel Module v0.1\n");
    pr_info("  Deadline-to-Priority Mapping\n");
    pr_info("========================================\n");

    memset(&rilp_stats, 0, sizeof(rilp_stats));

    rilp_nl_sock = netlink_kernel_create(&init_net,
                                         RILP_NETLINK_FAMILY,
                                         &rilp_nl_cfg);
    if (!rilp_nl_sock) {
        pr_err("RILP: Failed to create netlink socket\n");
        return -ENOMEM;
    }

    rilp_proc_entry = proc_create("rilp", 0222, NULL, &rilp_proc_ops);
    if (!rilp_proc_entry) {
        pr_err("RILP: Failed to create proc entry\n");
        netlink_kernel_release(rilp_nl_sock);
        return -ENOMEM;
    }

    pr_info("RILP: Netlink family %d\n", RILP_NETLINK_FAMILY);
    pr_info("RILP: Test: echo '<deadline_us> <pid>' > /proc/rilp\n");
    return 0;
}

static void __exit rilp_module_exit(void)
{
    proc_remove(rilp_proc_entry);
    netlink_kernel_release(rilp_nl_sock);
    pr_info("RILP: Module unloaded. msgs=%u mapped=%u errs=%u emerg=%u\n",
            rilp_stats.messages_received,
            rilp_stats.deadlines_mapped,
            rilp_stats.errors,
            rilp_stats.emergency_stops);
}

module_init(rilp_module_init);
module_exit(rilp_module_exit);
