# Levenshtein_search
By Matt Anderson. 2016

Levenshtein_search is a Python module that stores any number of documents as Tries. It performs fuzzy searches for words in a document that are d distance away from a query word. Searches are designed to be used in conjunction with TF-IDF calculations. The term frequency (TF) is computed for each approximately matching word in the document, as well as the Levenshtein distance from your query word. The module was written in C and increases search speed by using graph search algorithms and minimizing the number of redundant comparisons.

#Usage
```python
import Levenshtein_search

excerpt1 = ["We","went","to","the","fire","Mother","said","Is","he","cold","Versh","Nome","Versh","said","Take","his","overcoat","and","overshoes","off","Mother","said","How","many","times","do","I","have","to","tell","you","not","to","bring","him","into","the","house","with","his","overshoes","on"]
excerpt2 = ["Yessum","Versh","said","Hold","still","now","He","took","my","overshoes","off","and","unbuttoned","my","coat","Caddy","said","Wait","Versh","Cant","he","go","out","again","Mother","I","want","him","to","go","with","me","Youd","better","leave","him","here","Uncle","Maury","said","Hes","been","out","enough","today"]

x = Levenshtein_search.populate_wordset(-1,excerpt1)
# x = 0
x = Levenshtein_search.populate_wordset(-1,excerpt2)
# x = 1
```
The module accepts documents as Python lists of strings. To create a new document and give it a set of words, use the populate_wordset(x,excerpt1) function where x is an integer representing the document's index. If you would like the new document's index to be assigned, x should equal -1 and the function will return the new document's index, starting with 0. If you would like to add words to a preexisting document, x should equal that document's index. In the example above, excerpt1 is the Python list of strings.

```python
q = "overcoat"
maxdist = 4
results1 = Levenshtein_search.lookup(0,q,maxdist);
results2 = Levenshtein_search.lookup(1,q,maxdist);
```

To search a document for your query word, use the lookup(x,q,maxdist) function where x is a non-negative integer representing the document's index, q is a string representing your query word, and maxdist is a non-negative integer representing the maximum allowable Levenshtein distance from your query word.

```python
Levenshtein_search.clear_wordset(1)
Levenshtein_search.clear_wordset(0)
```

To clear a document from memory, use the clear_wordset(x) function where x is a non-negative integer representing the document's index. After clearing a document, x, documents with indices greater than x are offset by -1, to take the place of the cleared document.

#Output
```python
print("Query word: %s" % q)
for i in range(0,len(results1)):
	print("%s" % results1[i][0]+": "+ str(results1[i][1])+", "+str(results1[i][2]))
	
print("Query word: %s" % q)
for i in range(0,len(results2)):
	print("%s" % results2[i][0]+": "+ str(results2[i][1])+", "+str(results2[i][2]))
```
The output of the lookup() function is a list of lists equal in length to the number of unique words returned by the search. For each unique word, the function retrieves the Levenshtein distance from your query word, and the term frequency of that word in the document.
```bash
Query word: overcoat
overcoat: 0, 0.0238095238095
overshoes: 4, 0.047619047619
Query word: overcoat
coat: 4, 0.0222222222222
overshoes: 4, 0.0222222222222
```