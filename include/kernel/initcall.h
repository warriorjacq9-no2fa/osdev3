#ifndef INITCALL_H
#define INITCALL_H

typedef void (*initcall_t)(void);

#define __initcall_low __attribute((used, section(".initcall.low")))
#define __initcall_0 __attribute((used, section(".initcall.0")))
#define __initcall_1 __attribute((used, section(".initcall.1")))
#define __initcall_2 __attribute((used, section(".initcall.2")))
#define __initcall_3 __attribute((used, section(".initcall.3")))

#endif