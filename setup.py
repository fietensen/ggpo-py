from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

__version__ = '0.0.1'
__desc__ = 'Python bindings to GGPO'

ext_modules = [
    Pybind11Extension('ggpo_py',
                      ['src/main.cpp'],
                      define_macros=[('GGPOPY_VERSION', __version__), ('GGPOPY_DESC', __desc__)]
                      ),
]

setup(
    name='ggpo_py',
    version=__version__,
    author='Fiete Minge',
    url='https://github.com/fietensen/ggpo-py',
    description=__desc__,
    long_description='',
    ext_modules=ext_modules,
    cmdclass={'build_ext': build_ext},
    zip_safe=False,
    python_requires='>=3.9',
)
