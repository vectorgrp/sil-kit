:orphan:

==========================================
!!! Software Bill of Materials (SBOM)
==========================================

.. contents::
   :local:
   :depth: 2

SIL Kit ships an SPDX 2.3 software bill of materials at ``SilKit.spdx.json`` in the repository root.

Why it is maintained by hand
============================

Software composition scanners such as ``syft`` find nothing useful in this repository. There is no
package manifest for them to read: five of the six third-party components are git submodules that
record only a commit SHA, and rapidyaml is a vendored source amalgamation
(``ThirdParty/rapidyaml/rapidyaml.hpp``) with no version metadata a scanner recognises. The
amalgamation additionally bundles a second upstream project, c4core, which no scanner will ever
attribute.

The inventory is therefore declared explicitly in ``ThirdParty/third-party-components.json`` and
rendered into SPDX by ``SilKit/ci/generate_sbom.py``. A CI job keeps the declaration honest.

.. admonition:: Do not derive versions from git

   ``git describe`` and ``git submodule status`` report the nearest reachable tag, which is not the
   version that is actually pinned. At the time of writing they describe googletest as
   ``release-1.8.0-2986-g58d77fa8`` and spdlog as ``v1.2.1-2497-g48bcf39a``, while the commits in
   question are ``release-1.12.1`` and ``v1.15.2``. The ``version`` field in the metadata is
   authoritative and must be maintained by hand.

What the SBOM covers
====================

Only components that reach a released artifact. googletest is present in the metadata but excluded
from the document, because it is linked into the test executables only.

The document distinguishes *which* artifact each component ends up in, which is the part a scanner
could not reconstruct:

* ``SilKit`` — the distribution, described by the document.
* ``SilKit-library`` — the shared library. asio and fmt are compiled in as header-only libraries;
  spdlog and rapidyaml are linked in as static libraries.
* ``sil-kit-registry`` — the registry utility. It is the only artifact that carries oatpp, via the
  dashboard client. oatpp is *not* part of the SIL Kit library.

c4core is recorded as ``CONTAINS``-ed by rapidyaml rather than linked directly, reflecting that it
arrives inside the amalgamation.

Regenerating
============

The committed SBOM is generated from the default build configuration. After changing anything in
``ThirdParty/third-party-components.json``:

.. code-block:: powershell

    python3 SilKit/ci/generate_sbom.py

or, from a configured build tree:

.. code-block:: powershell

    cmake --build --preset debug --target silkit-sbom-update

Verify with the same check CI runs:

.. code-block:: powershell

    python3 SilKit/ci/generate_sbom.py --check

The check fails when the committed SBOM is stale, when a submodule was bumped without updating the
metadata, or when a component is missing from ``ThirdParty/LICENSES.rst``. It reads submodule
commits with ``git ls-tree``, so it does not require the submodules to be checked out.

Every build also writes an SBOM for its own configuration to
``<build dir>/sbom/SilKit-<version>.spdx.json``, via the ``silkit-sbom`` target. Unlike the
committed one, it records the build's git hash and reflects the options actually enabled — turning
off ``SILKIT_BUILD_DASHBOARD`` removes oatpp and the registry from the document. Set
``SILKIT_BUILD_SBOM=OFF`` to skip generation; it is also skipped automatically when no Python 3
interpreter is available.

Adding or updating a dependency
===============================

#. Update the submodule or the vendored copy as usual.
#. In ``ThirdParty/third-party-components.json``, set ``version`` and ``commit``. For submodules,
   read the commit with ``git ls-tree HEAD ThirdParty/<name>`` — not from ``git describe``. Take the
   version from the upstream tag or from the version macro in the sources.
#. Add the license text to ``ThirdParty/LICENSES.rst`` and ``docs/licenses/license.rst`` if the
   component is new, and make ``noticeName`` match the heading used there.
#. Set ``shipsIn`` to the artifacts that actually embed the component, and ``cmakeGuard`` to the
   CMake option that enables it, if any. A component that never ships gets an empty ``shipsIn``.
#. Regenerate and check, as above.

Reproducibility
===============

``SILKIT_BUILD_REPRODUCIBLE`` is on by default and the release pipeline sets ``SOURCE_DATE_EPOCH``,
so the document contains nothing that varies between two builds of the same sources: the
``documentNamespace`` is a UUIDv5 derived from the version and the build configuration rather than a
random UUID, and the creation timestamp honours ``SOURCE_DATE_EPOCH``.

For the same reason the committed ``SilKit.spdx.json`` does not record a commit hash — it would go
stale on every commit. It refers to the release tag instead, and ``--check`` compares content only,
reusing the recorded timestamp.

Validating
==========

.. code-block:: powershell

    pip install spdx-tools
    pyspdxtools -i SilKit.spdx.json
