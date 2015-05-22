Linux IRQ prober
==============================

This module is inspired from the [short.c](https://www.cs.fsu.edu/~baker/devices/lxr/http/source/ldd-examples/short/short.c) source code from the Linux Device Driver book.

It helps to find unhandled IRQ using the autoprobe IRQ kernel API, or request a specific IRQ.

#### How it works:
* If you want to use the autoprobe feature: `insmod irq-prober.ko`
* You can specify the delay in micro-seconds to use beetween the probe: `insmod irq-prober.ko delay=500`
* If you want to request a specific IRQ: `insmod irq-prober.ko irq=20`
* If this IRQ is already handle you can specify it as a share IRQ: `insmod irq-prober.ko irq=20 share=1`
* If your IRQ is driven by a GPIO: `insmod irq-prober.ko gpio=13`

Then use `dmesg` to retrieve the debug output:

#### Example:
```
# insmod irq-prober irq=19 share=1
# dmesg | grep irq-prober
irq-prober: IRQ 19 handle successfully
```
