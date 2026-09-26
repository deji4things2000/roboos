# RILP Kernel Module

Enforces the `deadline_us` field from userspace RILP messages by mapping
it to Linux scheduling policies.

## Priority Mapping

| Deadline (μs) | Priority | Policy     |
|---------------|----------|------------|
| < 100         | 99       | SCHED_FIFO |
| < 1000        | 80       | SCHED_FIFO |
| < 10000       | 50       | SCHED_FIFO |
| >= 10000      | 0        | SCHED_OTHER|

## Interfaces

- **Procfs**: `echo "<deadline_us> <pid>" > /proc/rilp`
- **Netlink**: Family 31

## Building

Must be built against the target kernel headers:

```bash
make -C /lib/modules/$(uname -r)/build M=$(PWD) modules