#!/usr/bin/env python3
"""Compile and run actual gint startup with SH instructions, linker symbols and LTO.

Needs GCC 16+ for SuperH, fxlibc headers/library, and sh-elf-run (GDB's SH
simulator). No calculator or firmware. Missing tools are errors, never skips.
"""
import argparse
import contextlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent


def run(args, **kwargs):
    result = subprocess.run([str(a) for a in args], capture_output=True,
                            text=True, timeout=20, **kwargs)
    if result.returncode:
        raise RuntimeError(f"Command failed ({result.returncode}): {args}\n"
                           f"{result.stdout}{result.stderr}")
    return result.stdout


def linker_script(words, reloc, table, drivers, xy, zero_tables):
    ctor = ''.join(f'LONG(_fixture_ctor{i+1});' for i in range(table))
    dtor = ''.join(f'LONG(_fixture_dtor{i+1});' for i in range(table))
    tables = (f'_bctors = .; {ctor} _ectors = .; '
              f'_bdtors = .; {dtor} _edtors = .;')
    if zero_tables:
        tables = '_bctors = 0; _ectors = 0; _bdtors = 0; _edtors = 0;'
    # gint_driver_t is 48 bytes in the target ABI; a compile-time assertion
    # below checks that this fixture keeps matching the actual public struct.
    return f'''
OUTPUT_FORMAT("elf32-sh")
OUTPUT_ARCH(sh)
ENTRY(_test_entry)
EXTERN(_fixture_ctor1 _fixture_ctor2 _fixture_dtor1 _fixture_dtor2)
SECTIONS {{
 . = 0x1000;
 .text : {{ *(.text.test_entry) *(.text .text.*) }}
 .rodata : {{ *(.rodata .rodata.*) }}
 . = ALIGN(4);
 .tables : {{ {tables}
   _gint_drivers = .; . += {drivers} * 48; _gint_drivers_end = .;
 }}
 .data : {{ *(.data .data.*) }}
 .bss : {{ *(.bss .bss.* .gint.bss) *(COMMON) }}
 /DISCARD/ : {{ *(.eh_frame .comment) }}
 _brom = 0; _srom = 0;
 _ldata = _source_words; _rdata = _data_buffer + 4; _sdata = {words} * 4;
 _rbss = _bss_buffer + 4; _sbss = {words} * 4;
 _lilram = _source_words; _rilram = _il_buffer + 4; _silram = {words} * 4;
 _lxyram = _source_words; _rxyram = _xy_buffer + 4; _sxyram = {xy} * 4;
 _lgmapped = _source_words; _sgmapped = {words} * 4;
 _lreloc = {('_fixup_buffer + 4' if reloc else '0')}; _sreloc = {reloc} * 4;
}}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cc', default=os.environ.get('SH_CC', 'sh-elf-gcc'))
    parser.add_argument('--sim', default=os.environ.get('SH_SIM', 'sh-elf-run'))
    parser.add_argument('--startup-source', type=Path, default=ROOT/'src/kernel/start.c',
                        help='Override the startup file for regression sensitivity checks')
    parser.add_argument('--output', type=Path, help='Keep generated fixture and last ELF for inspection')
    parser.add_argument('--quick', action='store_true', help='Only -Os with LTO')
    args = parser.parse_args()
    for tool in (args.cc, args.sim):
        if not shutil.which(tool):
            raise SystemExit(f'Missing required tool: {tool}')
    version = run([args.cc, '-dumpfullversion']).strip()
    if int(version.split('.')[0]) < 16:
        raise SystemExit(f'Requires GCC 16 or newer; found {version}')
    modes = [('-Os', True)] if args.quick else [(o, l) for o in ('-Os', '-O2') for l in (False, True)]
    cases = [(0, 0, 0, 0, 0, False, 0), (0, 0, 0, 0, 0, True, 0),
             (4, 1, 1, 1, 0, False, 0), (8, 3, 2, 2, 8, False, 0),
             (4, 0, 0, 1, 4, False, 1), (4, 0, 0, 1, 4, False, 2)]
    if args.output: args.output.mkdir(parents=True, exist_ok=True)
    context = contextlib.nullcontext(args.output) if args.output else tempfile.TemporaryDirectory(prefix='gint-startup-')
    with context as temp:
        out = Path(temp)
        (out/'include/gint').mkdir(parents=True, exist_ok=True)
        config = (ROOT/'include/gint/config.h.in').read_text()
        config = config.replace('@GINT_GIT_HASH@', '0000000').replace('@GINT_GIT_VERSION@', 'test')
        config = re.sub(r'^#cmakedefine.*$', '', config, flags=re.M)
        (out/'include/gint/config.h').write_text(config)
        harness = out/'harness.c'
        harness.write_text((HERE/'harness.c').read_text() + '\n_Static_assert(sizeof(gint_driver_t) == 48, "driver ABI changed");\n')
        total = 0
        for platform, sh3 in [('FXCG50', 0), ('FX9860G', 0), ('FX9860G', 1), ('FXCP', 0)]:
            for opt, lto in modes:
                for words, reloc, table, drivers, xy, zero_tables, bad in cases:
                    case = f'{platform}-sh3{sh3}-{opt}-lto{lto}-case{cases.index((words,reloc,table,drivers,xy,zero_tables,bad))}'
                    script = out/'test.ld'
                    script.write_text(linker_script(words, reloc, table, drivers, xy, zero_tables))
                    flags = ['-m4-nofpu', '-mb', opt, '-ffreestanding', '-fno-builtin',
                             '-fstrict-volatile-bitfields', '-ffunction-sections', '-fdata-sections',
                             '-Wall', '-Wextra', '-Werror', '-D'+platform,
                             '-I'+str(ROOT/'include'), '-I'+str(out/'include'),
                             '-I'+str(ROOT/'src/kernel')]
                    if lto: flags.append('-flto')
                    for key, val in [('SECTION_WORDS', words), ('XY_WORDS', xy), ('RELOC_WORDS', reloc),
                                     ('TABLE_COUNT', table), ('DRIVER_COUNT', drivers),
                                     ('TEST_SH3', sh3), ('BAD_LOAD', bad)]:
                        flags.append(f'-D{key}={val}')
                    try:
                        run([args.cc, *flags, '-nostdlib', args.startup_source,
                             harness, HERE/'entry.S', '-Wl,-T,'+str(script),
                             '-Wl,--gc-sections,--no-warn-rwx-segments', '-o', out/'test.elf', '-lc', '-lgcc'])
                        run([args.sim, '--memory-size=16M', out/'test.elf'])
                    except (RuntimeError, subprocess.TimeoutExpired) as error:
                        raise SystemExit(f'FAIL {case}: {error}')
                    total += 1
        print(f'PASS: {total} actual-startup executions with GCC {version}; FX/CG/CP, empty/populated sections, constructors, exit, and LTO.')


if __name__ == '__main__':
    main()
