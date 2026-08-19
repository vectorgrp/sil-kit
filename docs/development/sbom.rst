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

The documentation is no better served: the HTML that ships in a release embeds stylesheets, scripts
and webfonts from the Sphinx toolchain, which is pinned in a requirements file rather than described
by any package manifest inside the release.

The inventory is therefore declared explicitly in ``ThirdParty/third-party-components.json`` and
rendered into SPDX by ``SilKit/ci/generate_sbom.py``. A CI job keeps the declaration honest: it
verifies the submodule pins against the tree and the documentation pins against
``SilKit/ci/docker/docs_requirements.txt``.

.. admonition:: Do not derive versions from git

   ``git describe`` and ``git submodule status`` report the nearest reachable tag, which is not the
   version that is actually pinned. At the time of writing they describe googletest as
   ``release-1.8.0-2986-g58d77fa8`` and spdlog as ``v1.2.1-2497-g48bcf39a``, while the commits in
   question are ``release-1.12.1`` and ``v1.15.2``. The ``version`` field in the metadata is
   authoritative and must be maintained by hand.

What the SBOM covers
====================

The canonical SBOM describes a **full release** — everything a user receives, not just the compiled
binaries. It is organised around the four release artifacts, and the relationship to each records
*how* the component gets there, which is the part a scanner could not reconstruct:

``SilKit-library``
    The shared library. asio and fmt are compiled in as header-only libraries, spdlog and rapidyaml
    are linked in as static libraries. All four are ``STATIC_LINK``.

``sil-kit-registry``
    The registry utility. It is the only binary that carries oatpp, via the dashboard client. oatpp
    is *not* part of the SIL Kit library.

``SilKit-Documentation``
    The generated HTML. Sphinx and sphinx-rtd-theme ``CONTAINS`` — they copy stylesheets, scripts
    and webfonts into the output — as does jQuery, which ships verbatim as ``_static/jquery.js``.
    breathe and myst-parser are ``BUILD_TOOL_OF``: they are needed to produce the documentation but
    none of their own code ends up in it.

``SilKit-Source``
    The source distribution. ``install(DIRECTORY ThirdParty/ ...)`` copies the whole tree, so every
    third-party component is redistributed as source here — **including googletest**, which is never
    linked into any released binary. This is why googletest is in the SBOM at all; with
    ``SILKIT_INSTALL_SOURCE=OFF`` it correctly disappears.

Components bundled inside another component are recorded as ``CONTAINS``-ed by their container
rather than attached to an artifact: c4core inside the rapidyaml amalgamation, and Font Awesome,
Lato and Roboto Slab inside sphinx-rtd-theme.

.. admonition:: What is not in the SBOM

   GitHub Actions and other CI definitions. They are build infrastructure that produces nothing a
   user receives, and the GitHub workflows do not build releases at all. Build *provenance* is a
   separate concern from a bill of materials.

   Doxygen, although it is required to build the documentation. Unlike the Python packages it is not
   pinned by this repository, so any version recorded here would describe one machine rather than
   the release.

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
committed one, it records the build's git hash and reflects the options actually enabled:
``SILKIT_BUILD_UTILITIES``, ``SILKIT_BUILD_DOCS``, ``SILKIT_INSTALL_SOURCE`` and
``SILKIT_BUILD_DASHBOARD`` each add or remove an artifact and everything that reaches only that
artifact. An ordinary ``debug`` build therefore describes two artifacts and six components, while
the canonical release SBOM describes four and fifteen.

Set ``SILKIT_BUILD_SBOM=OFF`` to skip generation; it is also skipped automatically when no Python 3
interpreter is available.

Adding or updating a dependency
===============================

#. Update the submodule, the vendored copy, or the pin in
   ``SilKit/ci/docker/docs_requirements.txt`` as usual.
#. In ``ThirdParty/third-party-components.json``, set ``version`` and ``commit``. For submodules,
   read the commit with ``git ls-tree HEAD ThirdParty/<name>`` — not from ``git describe``. Take the
   version from the upstream tag or from the version macro in the sources. For a component with
   ``pinnedIn``, the version must match the requirements file exactly; the check enforces this.
#. Add the license text to ``ThirdParty/LICENSES.rst`` and ``docs/licenses/license.rst`` if the
   component is new, and make ``noticeName`` match the heading used there. Components with
   ``noticeName: null`` are exempt from that check.
#. Fill in ``partOf``: one entry per release artifact the component reaches, with the relationship
   to use — ``STATIC_LINK`` for code linked into a binary, ``CONTAINS`` for files shipped verbatim,
   ``BUILD_TOOL_OF`` for a tool that produces an artifact without shipping its own code. Add a
   per-entry ``guard`` when a CMake option decides whether the component reaches that artifact, as
   oatpp does. A component bundled inside another one gets ``containedBy`` and an empty ``partOf``.
#. Regenerate and check, as above.

Anything vendored under ``ThirdParty/`` needs a ``SilKit-Source`` entry, because the source
distribution ships the whole directory regardless of what the component is used for.

Known gap
=========

``ThirdParty/LICENSES.rst`` and ``docs/licenses/license.rst`` cover only the C++ components. The
assets that the documentation build ships — the Sphinx and sphinx-rtd-theme static files, jQuery,
Font Awesome, Lato and Roboto Slab — are recorded in the SBOM but have no license text in either
notice file. Those components therefore carry ``noticeName: null`` and are exempt from the notice
check. Adding their texts, and generating both ``.rst`` files from the metadata so that the three
lists cannot diverge, is outstanding work.

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
