Configuration and Build Strategy                       01-Sep-2000, ahu
--------------------------------

This is *not* an installation manual. See README and INSTALL for that.

This document describes the main guidelines used for the configuration
and build system of this software package and summarizes the steps for
the involved processes.


1. Overview and Rationale
   ----------------------

The `autoconf' package is used to gather system configuration data
(see `info autoconf'). System configuration data is kept in `config.mk'
and `config.h'. These files are included from Makefiles and source
files, respectively.

System configuration is done from the top level of the directory tree.
Usually, `./configure' is enough to create all system configuration
files and Makefiles. In turn, `make distclean' issued from the top
level directory will remove all files created by the configuration and
build processes (while `make distclean' issued in a subdirectory will
only remove files created by the build process but no configuration
files).

Makefiles should follow GNU Makefile conventions (see `info make').

Most targets in the top level Makefile simply call `make' in each
subdirectory and recursively perform a given task. This ensures that
Makefiles in the subdirectories are sufficently independent. It must
be possible to use all standard targets in all subdirectories to
independently work in a subdirectory with consistent targets.

Using an existing, proven and well documented configuration system
saves us the effort of re-inventing the wheel. It allows us to draw
from a wealth of experience in this area. Building on carefully chosen
existing standards and conventions should not only save us from some
common pitfalls but also help people familiarize with our development
environment quickly.


2. Configuration
   -------------

Preparing a software package for distribution (see section 2.1) is
very much a one time job. Maintenance of an already prepared package
is less challenging (see section 2.2) and only needs to be performed
from time to time.

The strength of using the `autoconf' process is that configuring and
building a package, once it is prepared, is very simple (see section
2.3). In most cases, a `./configure; make' in the top level directory
should be enough to configure and build the entire package.

2.1 The steps to prepare a software package for distribution
    --------------------------------------------------------

    1. Run `autoscan' in top level directory
       Input:  source files
       Output: `configure.scan'
    2. Manually examine and fix `configure.scan'
    3. Rename `configure.scan' to `configure.in'
    4. Manually create `acconfig.h' for additional AC_DEFINE'd symbols
    5. Run `autoheader'
       Input:  `configure.in' `acconfig.h'
       Output: `config.h.in'
    6. Run `autoconf'
       Input:  `configure.in' `acconfig.h'
       Output: `configure'
    7. Manually create `Makefile.in' template

    Here is a complete diagram showing this process (from `info autoconf'):

       source files --> [autoscan*] --> [configure.scan] --> configure.in

       configure.in --.   .------> autoconf* -----> configure
                      +---+
       [aclocal.m4] --+   `---.
       [acsite.m4] ---'       |
                              +--> [autoheader*] -> [config.h.in]
       [acconfig.h] ----.     |
                        +-----'
       [config.h.top] --+
       [config.h.bot] --'

       Makefile.in -------------------------------> Makefile.in

2.2 Maintenance of a prepared package
    ---------------------------------

Only the most frequently used maintenance processes are described
here.

2.2.1 `Makefile' changes
      ------------------

`Makefile's are generated from the respective `Makefile.in' template
by the `configure' script. Therefore, if a change to a `Makefile' is
needed, it must be done in the `Makefile.in' template and the
`configure' script must be run to re-generate the `Makefile'.

2.2.2 `configure.in' and `acconfig.h' updates
      ---------------------------------------

Such updates may be required to add a new feature or test to the
package. Also, from time to time, e.g., after substantial source code
changes or an upgrade of the `autoconf' package, it may be useful to
re-run `autoscan' and compare the resulting `configure.scan' with the
existing `configure.in' and check for new macros and tests suggested
in `configure.scan'. Usually, you can then easily update `configure.in'
manually.

After applying the changes, `configure' and `config.h.in' need to be
re-built using `autoconf' and `autoheader', respectively (see section
2.1, steps 5. and 6.).


2.3 The steps to configuring and building a software package
    --------------------------------------------------------

    1. Run `./configure' in top level directory
       Input:  `Makefile.in' `config.h.in'
       Output: `Makefile' `config.mk' `config.h'
    2. Run `make' in the top level directory
       Input:  `Makefile' `config.mk' `config.h' source code
       Output: Executables, Libraries, etc.

    The diagram showing this process (from `info autoconf') looks
    like this:

                              .-------------> config.cache
       configure* ------------+-------------> config.log
                              |
       [config.h.in] -.       v            .-> [config.h] -.
                      +--> config.status* -+               +--> make*
       Makefile.in ---'                    `-> Makefile ---'


3. Makefile Conventions
   --------------------

Makefiles should follow GNU Makefile conventions (see `info make').

3.1 Standard Makefile Targets
    -------------------------

`all'
    Compile the entire program. Should be the default target.

`check'
    Perform self-tests (if any).

`install'
    Compile the program and copy executables, libraries, etc., to the
    file names where they should reside for actual use.

`uninstall'
    Delete all the installed files created by `install'.

`clean'
    Delete all files that are normally created by building the program.

`mostlyclean'
    Delete all files that are normally created by building the program,
    except executables, libraries, etc.

`distclean'
    In the top level directory, delete all files that are created by
    configuring or building the program.
    In a subdirectory, same as `clean'.

`maintainer-clean'
    Delete almost everything from the current directory that can be
    reconstructed with this Makefile.

If in doubt about the standard targets and for more details, see GNU
Makefile conventions in `info make'.
