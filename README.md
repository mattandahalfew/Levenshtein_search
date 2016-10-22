# Levenshtein_search
By Matt Anderson. 2016

Levenshtein_search is a Python module that stores any number of documents as Tries. It performs fuzzy searches for words in a document that are d distance away from a query word. Searches are designed to be used in conjunction with TF-IDF calculations. The term frequency (TF) is computed for each approximately matching word in the document, as well as the Levenshtein distance from your query word. The module was written in C and increases search speed by using graph search algorithms and minimizing the number of redundant comparisons.

#Usage
```python
import Levenshtein_search

excerpt1 = ["We","went","to","the","fire","Mother","said","Is","he","cold","Versh","Nome","Versh","said","Take","his","overcoat","and","overshoes","off","Mother","said","How","many","times","do","I","have","to","tell","you","not","to","bring","him","into","the","house","with","his","overshoes","on"]
excerpt2 = ["Yessum","Versh","said","Hold","still","now","He","took","my","overshoes","off","and","unbuttoned","my","coat","Caddy","said","Wait","Versh","Cant","he","go","out","again","Mother","I","want","him","to","go","with","me","Youd","better","leave","him","here","Uncle","Maury","said","Hes","been","out","enough","today"]

x = Levenshtein_search.populate_wordset(-1,excerpt1)
```
The module accepts documents as Python lists of strings. To create a new document and give it a set of words, use the populate_wordset(x,excerpt1) function where x is an integer representing the document's index. If you would like the new document's index to be assigned, x should equal -1 and the function will return the new document's index, starting with 0. If you would like to add words to a preexisting document, x should equal that document's index. In the example, excerpt1 is the Python list of strings.

