# Levenshtein_search
By Matt Anderson. 2016

Search through a dictionary of words to find a set of words up to d distance away from your query word. This module is case-sensitive.

Install using: pip install Levenshtein-search

#Here is an example of use:

import Levenshtein_search

examplewords = []

examplewords.append("foxes")

examplewords.append("fixes")

examplewords.append("faxes")

examplewords.append("fonts")

examplewords.append("first")

Levenshtein_search.populate_dictionary(pythonstrings)

results = Levenshtein_search.lookup("foxes",1) #First argument is a string. Second argument is a non-negative integer representing the max distance from the query word.

Levenshtein_search.clear_dictionary() #Removes the dictionary from memory when finished
