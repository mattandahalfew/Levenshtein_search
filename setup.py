from setuptools import setup, Extension

setup(
name='Levenshtein_search',
url='https://github.com/mattandahalfew/Levenshtein_search',
version='1.4.4',
author='Matt Anderson',
author_email='manderson3606@gmail.com',
description='Search through documents for approximately matching strings',
packages=[],
ext_modules=[Extension('Levenshtein_search',['Lev_search.c'])],
install_requires=[],
classifiers=[
	'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Scientific/Engineering',
        'Topic :: Scientific/Engineering :: Information Analysis'])