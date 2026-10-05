Getting started
===============

Build the workspace with the colcon workflow.... To generate this
documentation locally, create a Python virtual environment, install the
documentation dependencies, and run:

.. code-block:: bash

   cd src/advrf-middleware
   python3 -m venv .venv
   . .venv/bin/activate
   python -m pip install -r docs/requirements.txt
   doxygen Doxyfile
   sphinx-build -b html docs/source docs/build/site

Open ``docs/build/site/index.html`` in a browser with ``xdg-open``.

Writing a page
--------------

Create a ``.rst`` file under ``docs/source`` and add its file stem to the
``toctree`` in ``index.rst``. Headings use a line of ``=`` characters below
the title.
