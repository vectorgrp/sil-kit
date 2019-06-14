How to write reStructuredText?
==============================

First of all, some links to the documentation of the libraries which are used to generate this documentation:

* `Sphinx documentation - First steps <https://www.sphinx-doc.org/en/1.5/tutorial.html>`_

* `Breathe documentation <https://breathe.readthedocs.io/en/latest/>`_


Basic rst - functionalities
---------------------------

* The toctree-directive is the most important thing, especially in the index.rst and looks like this::

    .. toctree::
       :maxdepth: 2

       intro
       tutorial
       ...

For more information, you can read this: 
`The TOC tree <https://www.sphinx-doc.org/en/1.5/markup/toctree.html#toctree-directive>`_

* If there is just a word like 'intro' in the toctree-directive, Sphinx expects a intro.rst
  in the same folder and automatically creates a link and an ToC entry in the documentation.
  The link label will be the same as the first heading in the intro.rst file, written like this::

    This is the first heading, which is also the link label
    =======================================================

* Commentaries are written like this::

    ..This is a comment

* Headings are written like this and will be added to the Table of Contents::

    This is a header
    ----------------

* Subheadings are written like this and will be added to the Table of Contents as subheading::

    Sub heading
    ~~~~~~~~~~~

* To create an enumeration add a star in front of your text and be aware of intendations::

    * This creates a point enumeration
      after a line break, be aware of the correct intendation

* The 'Cross-Referencing' and 'how to add reference labels' is described pretty well here:
  `Inline markup <https://www.sphinx-doc.org/en/1.5/markup/inline.html>`_


Breathe to Sphinx bridge
------------------------

To add a doxygen class called 'namespace::Thingy' to the Sphinx documentation, you add to the *.rst file the following piece of text::

   .. doxygenclass:: namespace::Thingy
      :members:

This works also for other things like structs or functions (doxygenstruct, doxygenfunction, ...).