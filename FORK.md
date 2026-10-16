# CG100 maintenance fork

This repository preserves the history of [upstream gint](https://git.planet-casio.com/Lephenixnoir/gint).
The fork starts at `463f180ffb71ccdcef05c928ad19f2a480b14bc1` on upstream's `dev` branch.
Upstream's authorship and licensing statements in README.md remain applicable.

The initial compatibility commits migrate CGDoom's existing patch:

- Exact OS 02.00.0000 and 02.20.0000 selection and the 2.20 fixed-address call table.
- The `GINT_CGDOOM_OS_COMPAT` marker and API-query symbol used by CGDoom.
- LCD controller 0x32/0x52 handling during world switches.
- Exclusion of the fx-9860G-only relocation stage on CG models.

The fork additionally makes linker values explicit at the compiler boundary and
adds tests of actual SuperH startup code. See [the audit](docs/linker-symbol-audit.md)
and [the startup test instructions](tests/startup/README.md).

Keep general compiler/startup fixes separate from model-specific compatibility
changes so they can be reviewed and proposed upstream independently. No changes
are submitted to upstream automatically. Preserve the upstream remote when
cloning for development:

```sh
git remote add upstream https://git.planet-casio.com/Lephenixnoir/gint.git
```

Consumers should pin a full commit, not follow the moving `main` branch. When
building a `git archive` export, pass both `GINT_GIT_HASH` (the commit's first
seven hexadecimal digits) and `GINT_GIT_VERSION` (a version beginning with
`2.11.0`, followed by the fork identity) to CMake. This preserves usable version
metadata without embedding an outdated upstream hash in every exported build.

The previously patched CG100 startup was tested on hardware. The broader
linker hardening is covered by the target simulator and build checks; it still
needs a new calculator smoke test. The simulator does not model MMU, cache,
interrupt, display, or OS behavior.
