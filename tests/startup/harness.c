/* Execute the real kernel/start.c; replace only its hardware/OS boundaries.
   libc's real SH setjmp/longjmp exercises the startup -> exit -> teardown path. */
#include <gint/hardware.h>
#include <gint/drivers.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int start(int, uint32_t *);
void _Exit(int);
uint32_t gint[HW_KEYS];
__attribute__((used, externally_visible)) int __cpucap; /* fxlibc memory primitives: generic SH instructions. */
volatile uint32_t data_buffer[10], bss_buffer[10], il_buffer[10];
volatile uint32_t xy_buffer[10], mapped_buffer[10], fixup_buffer[5];
uint32_t const source_words[8] = {11, 22, 33, 44, 55, 66, 77, 88};
static uint32_t loadinfo[] = {1, 1, 0};
static volatile unsigned init_calls, quit_calls, ctor_order, dtor_order;
static volatile unsigned clear_calls;
static uint32_t *seen_loadinfo;
static unsigned const sentinel = 0xa5a5a5a5;

void hw_detect(void)
{
    gint[HWMPU] = TEST_SH3 ? 1 : 2;
    gint[HWCALC] = GINT_OS_CG ? HWCALC_FXCG100 : HWCALC_FX9860G_SH4;
}
void *mmu_uram(void) { return (void *)&mapped_buffer[1]; }
void gint_osmenu_native(void) { __builtin_trap(); }
void kinit(uint32_t *info) { init_calls++; seen_loadinfo = info; }
void kquit(void) { quit_calls++; }
void fixture_ctor1(void) { ctor_order = ctor_order * 10 + 1; }
void fixture_ctor2(void) { ctor_order = ctor_order * 10 + 2; }
void fixture_dtor1(void) { dtor_order = dtor_order * 10 + 1; }
void fixture_dtor2(void) { dtor_order = dtor_order * 10 + 2; }
void exit(int status) { _Exit(status); }

void *memset(void *p, int value, size_t size)
{
    /* On-chip device addresses cannot be accessed by this flat-RAM simulator. */
    if((uintptr_t)p == 0xe5200000 || (uintptr_t)p == 0xe500e000) {
        if(value != 0 || (size != 4096 && size != 16384)) __builtin_trap();
        clear_calls++;
        return p;
    }
    unsigned char *q = p;
    for(size_t i = 0; i < size; i++) q[i] = (unsigned char)value;
    return p;
}

int main(void)
{
    unsigned expected = TABLE_COUNT == 2 ? 12 : TABLE_COUNT;
    return init_calls == 1 && ctor_order == expected ? 23 : 24;
}

static int check(volatile uint32_t *buffer, unsigned words, int clear)
{
    if(buffer[0] != sentinel || buffer[9] != sentinel) return 0;
    for(unsigned i = 0; i < 8; i++) {
        uint32_t expected = i < words ? (clear ? 0 : source_words[i]) : sentinel;
        if(buffer[i+1] != expected) return 0;
    }
    return 1;
}

int test_main(void)
{
    volatile uint32_t *buffers[] = {data_buffer, bss_buffer, il_buffer,
                                   xy_buffer, mapped_buffer};
    for(unsigned b = 0; b < 5; b++)
        for(unsigned i = 0; i < 10; i++) buffers[b][i] = sentinel;
    for(unsigned i = 0; i < 5; i++) fixup_buffer[i] = i * 100;
    int load_type = BAD_LOAD == 1 ? 0 : 0x4d504d30;
    uint32_t *info = BAD_LOAD == 2 ? (void *)((uintptr_t)loadinfo + 1) : loadinfo;
    int status = start(load_type, info);
    if(status != 23 || quit_calls != 1) return 1;
    if(seen_loadinfo != (BAD_LOAD ? NULL : loadinfo)) return 2;
    if(!check(data_buffer, GINT_OS_CP ? 0 : SECTION_WORDS, 0)) return 3;
    if(!check(bss_buffer, GINT_OS_CP ? 0 : SECTION_WORDS, 1)) return 4;
    if(!check(il_buffer, (TEST_SH3 || GINT_OS_CP) ? 0 : SECTION_WORDS, 0)) return 5;
    if(!check(xy_buffer, (TEST_SH3 || GINT_OS_CP) ? 0 : XY_WORDS, 0)) return 6;
    if(!check(mapped_buffer, GINT_OS_FX ? SECTION_WORDS : 0, 0)) return 7;
    for(unsigned i = 0; i < 5; i++) {
        uint32_t expected = i * 100;
        if(GINT_OS_FX && i > 0 && i <= RELOC_WORDS)
            expected += (uintptr_t)&mapped_buffer[1];
        if(fixup_buffer[i] != expected) return 8;
    }
    if(dtor_order != (TABLE_COUNT == 2 ? 12u : TABLE_COUNT)) return 9;
    if(clear_calls != ((TEST_SH3 || GINT_OS_CP) ? 0u : 2u)) return 10;
    if(gint_driver_count() != DRIVER_COUNT) return 11;
    return 0;
}
