# Configuration file for the Sphinx documentation builder.

# Add projectairsim src/ folder to the Python path for sphinx to find it
import os
import sys
sys.path.insert(0, os.path.abspath("../src"))

project = 'Project AirSim'
copyright = '2022, Microsoft'
author = 'Microsoft'

extensions = ['sphinx.ext.autodoc', 'sphinx.ext.napoleon']

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'haiku'
html_static_path = ['_static']
