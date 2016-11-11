# Levenshtein_search
By Matt Anderson. 2016

Levenshtein_search is a Python module that stores any number of documents as Tries. It performs fuzzy searches for words in a document that are d distance away from a query word. Searches can also be used in conjunction with TF-IDF calculations. The term frequency (TF) is computed for each approximately matching word in the document, as well as the Levenshtein distance from your query word. The module was written in C and increases search speed by using graph search algorithms and minimizing the number of redundant comparisons.

#Usage
```python
import Levenshtein_search

excerpt1 = ["We","went","to","the","fire","Mother","said","Is","he","cold","Versh","Nome","Versh","said","Take","his","overcoat","and","overshoes","off","Mother","said","How","many","times","do","I","have","to","tell","you","not","to","bring","him","into","the","house","with","his","overshoes","on"]
excerpt2 = ["Yessum","Versh","said","Hold","still","now","He","took","my","overshoes","off","and","unbuttoned","my","coat","Caddy","said","Wait","Versh","Cant","he","go","out","again","Mother","I","want","him","to","go","with","me","Youd","better","leave","him","here","Uncle","Maury","said","Hes","been","out","enough","today"]

first_wordset = Levenshtein_search.populate_wordset(-1,excerpt1)
# first_wordset = 0
last_wordset = Levenshtein_search.populate_wordset(-1,excerpt2)
# last_wordset = 1
```
The module accepts documents as Python lists of strings. To create a new document and give it a set of words, use the populate_wordset(x,excerpt1) function where x is an integer representing the document's index. If you would like the new document's index to be assigned, x should equal -1 and the function will return the new document's index, starting with 0. If you would like to add words to a preexisting document, x should equal that document's index. In the example above, excerpt1 is the Python list of strings.

```python
q = "overcoat"
maxdist = 4
results1 = Levenshtein_search.lookup(first_wordset,q,maxdist)
results2 = Levenshtein_search.lookup(last_wordset,q,maxdist)
```

To search a document for your query word, use the lookup(x,q,maxdist) function where x is a non-negative integer representing the document's index, q is a string representing your query word, and maxdist is a non-negative integer representing the maximum allowable Levenshtein distance from your query word.

```python
idx = Levenshtein_search.add_string(first_wordset,"coat")
rm_idx = Levenshtein_search.remove_string(first_wordset,"coat")
# idx==rm_idx
Levenshtein_search.remove_string(first_wordset,q)

results3 = Levenshtein_search.lookup(first_wordset,q,maxdist)
```
You can add or remove words from the document on an individual basis, using add_string and remove_string functions. The first argument is the document's index and the second is the word to add or remove.

```python
Levenshtein_search.clear_wordset(last_wordset)
```

To clear a document from memory, use the clear_wordset(x) function where x is a non-negative integer representing the document's index. After clearing a document, x, documents retain their previous index (this is different from v1.3).

#Output
```python
print("Query word: %s" % q)
for i in range(0,len(results1)):
	print("%s" % results1[i][0]+": "+ str(results1[i][1])+", "+str(results1[i][2]))
	
print("Query word: %s" % q)
for i in range(0,len(results2)):
	print("%s" % results2[i][0]+": "+ str(results2[i][1])+", "+str(results2[i][2]))
	
print("Query word: %s" % q)
for i in range(0,len(results3)):
	print("%s" % results3[i][0]+": "+ str(results3[i][1])+", "+str(results3[i][2]))
```
The output of the lookup() function is a list of lists equal in length to the number of unique words returned by the search. For each unique word, the function retrieves the Levenshtein distance from your query word, and the term frequency of that word in the document.
```bash
Query word: overcoat
overcoat: 0, 0.0238095238095
overshoes: 4, 0.047619047619
Query word: overcoat
coat: 4, 0.0222222222222
overshoes: 4, 0.0222222222222
Query word: overcoat
overshoes: 4, 0.0487804878049
```
If there is any word in the results that is the same as your query word, it is guaranteed to be the first item of the result list. All other words are in no particular order.

#Installation
pip install Levenshtein-search