# Configuration file for the Sphinx documentation builder.

# Add projectairsim src/ folder to the Python path for sphinx to find it
import os
import sys

import sphinx_rtd_theme
sys.path.insert(0, os.path.abspath("../src"))

project = 'Project AirSim'
copyright = '2022, Microsoft'
author = 'Microsoft'

extensions = ['sphinx.ext.autodoc', 'sphinx.ext.napoleon']

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]
