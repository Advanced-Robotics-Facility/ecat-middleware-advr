"""Sphinx configuration for the ADVRF Middleware documentation site."""

project = "ADVRF Middleware"
author = "Advanced Robotics Facility"

extensions = ["breathe"]

templates_path = ["_templates"]
exclude_patterns = ["_build"]

html_theme = "sphinx_rtd_theme"
html_static_path = ["_static"]
html_title = "ADVRF Middleware"

# Doxygen writes this directory before Sphinx is invoked. Breathe reads it to
# render C++ and IDL declarations inside the Sphinx pages.
breathe_projects = {"advrf_middleware": "../build/xml"}
breathe_default_project = "advrf_middleware"
