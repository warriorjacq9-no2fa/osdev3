#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <kernel/klog.h>
#include <kernel/kevent.h>
#include <kernel/kmalloc.h>
#include <mm.h>
#include <arch.h>

void kconsumer_char(kevent_input_t *evt) {
    putc(evt->ch.character);
}

#define TEST_NS "kmalloc_test"

// ----------------------
// Assertion macro
// ----------------------
#define KASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            kprintf(LOG_ERR, TEST_NS, "[FAIL] %s (%s:%d)\r\n", msg, __FILE__, __LINE__); \
            return; \
        } else { \
            kprintf(LOG_INFO, TEST_NS, "[PASS] %s\r\n", msg); \
        } \
    } while (0)

// ----------------------
// Helpers
// ----------------------
static void fill_pattern(uint8_t* ptr, size_t size, uint8_t pattern) {
    for (size_t i = 0; i < size; i++) {
        ptr[i] = pattern;
    }
}

static int verify_pattern(uint8_t* ptr, size_t size, uint8_t pattern) {
    for (size_t i = 0; i < size; i++) {
        if (ptr[i] != pattern) {
            return 0;
        }
    }
    return 1;
}

// ----------------------
// Basic allocation test
// ----------------------
static void test_basic_alloc_free() {
    kprintf(LOG_INFO, TEST_NS, "Running basic alloc/free test...\r\n");

    void* ptr = kmalloc(64);
    KASSERT(ptr != NULL, "kmalloc returned non-null");

    fill_pattern(ptr, 64, 0xAA);
    KASSERT(verify_pattern(ptr, 64, 0xAA), "memory writable");

    kfree(ptr);
}

// ----------------------
// Multiple allocations
// ----------------------
static void test_multiple_allocations() {
    kprintf(LOG_INFO, TEST_NS, "Running multiple allocation test...\r\n");

    void* ptrs[10];

    for (int i = 0; i < 10; i++) {
        ptrs[i] = kmalloc(32);
        KASSERT(ptrs[i] != NULL, "allocation succeeded");

        fill_pattern(ptrs[i], 32, (uint8_t)i);
    }

    for (int i = 0; i < 10; i++) {
        KASSERT(
            verify_pattern(ptrs[i], 32, (uint8_t)i),
            "memory integrity"
        );
    }

    for (int i = 0; i < 10; i++) {
        kfree(ptrs[i]);
    }
}

// ----------------------
// Fragmentation test
// ----------------------
static void test_fragmentation() {
    kprintf(LOG_INFO, TEST_NS, "Running fragmentation test...\r\n");

    void* a = kmalloc(64);
    void* b = kmalloc(64);
    void* c = kmalloc(64);

    KASSERT(a && b && c, "initial allocations");

    kfree(b); // create a hole

    void* d = kmalloc(32);
    KASSERT(d != NULL, "allocation in fragmented space");

    kfree(a);
    kfree(c);
    kfree(d);
}

// ----------------------
// Reuse freed memory
// ----------------------
static void test_reuse() {
    kprintf(LOG_INFO, TEST_NS, "Running reuse test...\r\n");

    void* a = kmalloc(128);
    KASSERT(a != NULL, "initial allocation");

    kfree(a);

    void* b = kmalloc(128);
    KASSERT(b != NULL, "reallocation works");

    // Optional check depending on allocator design
    if (a == b) {
        kprintf(LOG_INFO, TEST_NS, "[INFO] allocator reused same block\r\n");
    } else {
        kprintf(LOG_WARN, TEST_NS, "[WARN] allocator did not reuse same block\r\n");
    }

    kfree(b);
}

// ----------------------
// Edge cases
// ----------------------
static void test_edge_cases() {
    kprintf(LOG_INFO, TEST_NS, "Running edge case test...\r\n");

    void* zero = kmalloc(0);
    kprintf(LOG_WARN, TEST_NS, "kmalloc(0) returned %p\r\n", zero);

    void* ptr = kmalloc(64);
    KASSERT(ptr != NULL, "edge alloc works");

    kfree(ptr);

    // Only enable if safe in your allocator
    // kfree(ptr);

    kfree(NULL);
    kprintf(LOG_INFO, TEST_NS, "kfree(NULL) executed\r\n");
}

// ----------------------
// Stress test
// ----------------------
/*static void test_stress() {
    kprintf(LOG_INFO, TEST_NS, "Running stress test...\r\n");

    #define N 1000
    void* ptrs[N];

    for (int i = 0; i < N; i++) {
        size_t size = (i % 128) + 1;

        ptrs[i] = kmalloc(size);
        if (!ptrs[i]) {
            kprintf(LOG_WARN, TEST_NS, "allocation failed at index %d\r\n", i);
            break;
        }

        fill_pattern(ptrs[i], size, 0x5A);
    }

    for (int i = 0; i < N; i++) {
        if (ptrs[i]) {
            kfree(ptrs[i]);
        }
    }

    kprintf(LOG_INFO, TEST_NS, "stress test completed\r\n");
}*/

// ----------------------
// Entry point
// ----------------------
void kmalloc_test_suite() {
    kprintf(LOG_INFO, TEST_NS, "==== KMALLOC TEST SUITE START ====\r\n");

    test_basic_alloc_free();
    test_multiple_allocations();
    test_fragmentation();
    test_reuse();
    test_edge_cases();
    //test_stress();

    kprintf(LOG_INFO, TEST_NS, "==== KMALLOC TEST SUITE END ====\r\n");
}

void kmain() {
#ifndef __i386__
    mm_init();
#endif
    arch_init();
    kevent_init();
    kheap_init();
    kevent_consumer_t consumer = {
        .callback = kconsumer_char,
        .type = KEVENT_CHAR
    };
    if(kevent_register(consumer)) {
        kprintf(LOG_ERR, "kernel", "Failed to register consumer for char event\r\n");
        return;
    }
    kprintf(LOG_INFO, "kernel", "Hello world!\r\n");
    kmalloc_test_suite();
}