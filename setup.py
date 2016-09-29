from setuptools import setup, Extension

setup(
name='Levenshtein_search',
url='https://github.com/mattandahalfew/Levenshtein_search',
version='1.1',
author='Matt Anderson',
author_email='manderson3606@gmail.com',
description='Search through a dictionary to find words up to d distance away from a query word',
packages=[],
ext_modules=[Extension('Levenshtein_search',['Lev_search1_1.c'])],
install_requires=[],
classifiers=[
	'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Scientific/Engineering',
        'Topic :: Scientific/Engineering :: Information Analysis'])