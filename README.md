# Levenshtein_search
By Matt Anderson. 2016

Levenshtein search is a Python module that will add words to a dictionary, then search the dictionary for words that are d distance away from a query word. This module was written in C and was designed to increase search speed by minimizing the number of redundant comparisons.

#How does it work?
The query word and words stored in the dictionary are case-sensitive. The dictionary is stored as a kind of tree data structure called a trie. The keys to the data structure are strings of letters that comprise words. Words are terminated with a null character and there are no restrictions to their length.

When searching the dictionary for a query word, the search algorithm is a depth first search (DFS). DFS first compares corresponding characters from the dictionary and query word, by moving along the query word recursively. Then it compares alternative characters from the dictionary if the maximum Levenshtein distance has not been exceeded or the end of the words has not been reached. However, since this algorithm finds every possible word in the dictionary that is within the specified Levenshtein distance, the order of search is of little consequence. Cumulative Levenshtein distance is passed on or added to at each recursion, so a search will terminate when it is futile.

Results returned by the algorithm are guaranteed to be unique. During the search, arriving at the same word by more than one path is detected by searching a binary search tree (BST) for all newly-added words. Unique words are added to the BST. After searching all paths that are within Levenshtein distance, the BST is traversed and these words are added to a python list.

#Installation:

pip install Levenshtein-search

#Use example:

import Levenshtein_search

examplewords = []

examplewords.append("foxes")

examplewords.append("fixes")

examplewords.append("faxes")

examplewords.append("fonts")

examplewords.append("first") #Populate the dictionary with a Python list of strings

Levenshtein_search.populate_dictionary(examplewords)

results = Levenshtein_search.lookup("foxes",1) #First argument is the query word. Second argument is a non-negative integer representing the max distance from the query word.

Levenshtein_search.clear_dictionary() #Removes the dictionary from memory when finished
