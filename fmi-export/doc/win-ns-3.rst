Hints for building |ns3| on Windows
-----------------------------------

.. |ns3| replace:: ns-3

Cygwin
******

1. Run the `Cygwin installer <https://cygwin.com/install.html>`_

2. During installation, chose all required packages. For instance, for *Cygwin version 2.891 (32-bit)* the following packages are the minimum requirement:

    - cmake (version 3.6.2-1)
    - gcc-g++ (version 7.3.0-3)
    - git (version 2.17.0-1)
    - libboost-devel (version 1.66.0-1)
    - make (version 4.2.1-2)
    - mercurial (version 4.3.2-1)
    - python2-pip (version 9.0.1-1)
    - unzip (version 6.0-17)

3. In the *Cygwin Terminal*, set the compiler flags:

::

    $ export CXXFLAGS="-D_USE_MATH_DEFINES -D_BSD_SOURCE -include limits.h"

**IMPORTANT NOTE**: This command should always be run before ``./waf configure``.

4. In the *Cygwin Terminal*, follow the `standard installation instructions for Linux <https://www.nsnam.org/support/faq/setup/>`_

::

    $ hg clone http://code.nsnam.org/ns-3-allinone
    $ cd ns-3-allinone
    $ ./download.py
    $ ./build.py


Troubleshooting
===============

**Error:**

::

    child_info_fork::abort: address space needed by 'XYZ.dll' (0x790000) is already occupied
    error: [Errno 11] Resource temporarily unavailable

The shared library name, address values and specific error message can vary and it can occur with any process that tries to load a DLL, that is not re-baseable, into memory.
This is a well-known Cygwin issues, where the virtual address space being claimed is already occupied by another process.

**Solution:**

The issue is easily fixable and only requires that the DLL in question must be marked as re-baseable, which can be done using the ``peflags`` and ``rebase`` commands:

::

    $ /bin/peflags -d1 /path/to/XYZ.dll
    $ /bin/rebase /path/to/XYZ.dll

In order to mark a DLL as re-baseable, it must not be in use.
The following error message when using ``peflags`` or ``rebase`` is a good sign for a DLL being in use:

::

    /path/to/XYZ.dll: skipped because could not open

To fix, terminate all running Cygwin processes and start ``/usr/bin/dash`` (a minimalistic shell that does not depend on DLLs) from Windows desktop or command line, then try again.

The command ``peflags`` accepts wild cards and instead of ``rebase`` you can use ``rebaseall -v`` which will try to re-base every executable file that is marked as re-baseable.
