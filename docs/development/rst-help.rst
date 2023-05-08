:orphan:

==================================
!!! How to write reStructuredText?
==================================

First of all, some links to the documentation of the libraries which are used to generate this documentation:

* `Sphinx documentation - First steps <https://www.sphinx-doc.org/en/1.5/tutorial.html>`_

* `Breathe documentation <https://breathe.readthedocs.io/en/latest/>`_

Another helpful resource is the `Sphinx cheatsheet <https://matplotlib.org/sampledoc/cheatsheet.html>`_ for some basic text formatting.

And the `ReST sphinx Memo <https://rest-sphinx-memo.readthedocs.io/en/latest/ReST.html>`_ with basic rst syntax.

.. contents::


!!! The toctree-directive
-------------------------

* The toctree-directive is the most important thing, especially in the index.rst and looks like this::

    .. toctree::
       :maxdepth: 2

       intro
       Specified link name <tutorial>
       development/guidelines
       ...

* If there is just a word like 'intro' in the toctree-directive, Sphinx expects an intro.rst
  in the same folder and automatically creates a link and an ToC entry in the documentation.
  The link name will be the same as the main heading in the intro.rst file, written like this::

    ======================================================
    This is the first heading, which is also the link name
    ======================================================

* If you want to specify something else for the link name, look at the example line for 'tutorial'.
* If your \*.rst files are structured in subfolders, write the folder name in front end separate it with a '/'.

For more information about the toctree, you can read this:
`The TOC tree <https://www.sphinx-doc.org/en/1.5/markup/toctree.html#toctree-directive>`_


!!! Basic .rst Syntax
---------------------

* Commentaries::

    .. This is a comment

* The main heading::

    =====================
    This is a main header
    =====================

* Sub-headings are written like this and will be added to the Table of Contents::

    This is a header
    ----------------

* Subsub-headings are written like this and will be added to the Table of Contents::

    Sub heading
    ~~~~~~~~~~~

* To create an enumeration add a star in front of your text and be aware of indentations::

    * This creates a point enumeration
      after a line break, be aware of the correct indentation

* The 'Cross-Referencing' and 'how to add reference labels' is described here:
  `Inline markup <https://www.sphinx-doc.org/en/1.5/markup/inline.html>`_


!!! Breathe to Sphinx bridge
----------------------------

To add a doxygen class called 'namespace::Thingy' to the Sphinx documentation, you add to the \*.rst file the following piece of text::

   .. doxygenclass:: namespace::Thingy
      :members:

This works also for other things like structs or functions (doxygenstruct, doxygenfunction, ...).