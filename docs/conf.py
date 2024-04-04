# SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.

import os
import sys
import sphinx_rtd_theme

# -- Project information -----------------------------------------------------

project = 'Vector SIL Kit'
copyright = '2022 Vector Informatik GmbH'
author = 'Vector Informatik GmbH'
version = '1.0.0'
# The full version, including alpha/beta/rc tags
release = version

###  The master toctree document. ###
# This is used mainly for the html_sidebars: globaltoc.html to show a
# static self-defined ToC at every page.
# ATTENTION: The main (entry) page is STILL the index.rst and therefore
# a layout.html template was added in _templates/ which extends 
# the navigation header line of the layout.html
master_doc = 'contents'

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [ 
    'sphinx_rtd_theme',
    #'sphinx.ext.autosectionlabel',
    "breathe" ]

# For automatic creation of labels per section
autosectionlabel_prefix_document = True
autosectionlabel_maxdepth = 10

# Additional flags to further configure Sphinx
numfig = True
numfig_secnum_depth = 1
numfig_format = {'figure': 'Figure %s'}

# Breathe Configuration
# The breathe project property is already set in the CMakeLists.txt
# breathe_projects = { "PROJECT_NAME": "PATH_TO_DOXYGEN_XML_FOLDER" }

breathe_default_project = "SilKit"

breathe_default_members = ('members', 'undoc-members')

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'
#html_theme = 'bizstyle'

html_sidebars = {
   '**': ['globaltoc.html', 'sourcelink.html', 'searchbox.html'],
   'using/windows': ['windowssidebar.html', 'searchbox.html'],
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_css_files = [
    'custom.css'
]
