import Levenshtein_search

excerpt1 = ["We","went","to","the","fire","Mother","said","Is","he","cold","Versh","Nome","Versh","said","Take","his","overcoat","and","overshoes","off","Mother","said","How","many","times","do","I","have","to","tell","you","not","to","bring","him","into","the","house","with","his","overshoes","on"]
excerpt2 = ["Yessum","Versh","said","Hold","still","now","He","took","my","overshoes","off","and","unbuttoned","my","coat","Caddy","said","Wait","Versh","Cant","he","go","out","again","Mother","I","want","him","to","go","with","me","Youd","better","leave","him","here","Uncle","Maury","said","Hes","been","out","enough","today"]

first_wordset = Levenshtein_search.populate_wordset(-1,excerpt1)
print("Wordset index: " + str(first_wordset))

last_wordset = Levenshtein_search.populate_wordset(-1,excerpt2)
print("Wordset index: " + str(last_wordset))

q = "overcoat"
maxdist = 6
results1 = Levenshtein_search.lookup(first_wordset,q,maxdist);
results2 = Levenshtein_search.lookup(last_wordset,q,maxdist);

print("Query word: %s" % q)
for i in range(0,len(results1)):
	print("%s" % results1[i][0]+": "+ str(results1[i][1])+", "+str(results1[i][2]))

print("Query word: %s" % q)
for i in range(0,len(results2)):
	print("%s" % results2[i][0]+": "+ str(results2[i][1])+", "+str(results2[i][2]))
	
Levenshtein_search.clear_wordset(last_wordset)
print ("Python: removed wordset %i" % last_wordset)
Levenshtein_search.remove_string(first_wordset,q);

results3 = Levenshtein_search.lookup(first_wordset,q,maxdist);
print("Query word: %s" % q)
for i in range(0,len(results3)):
	print("%s" % results3[i][0]+": "+ str(results3[i][1])+", "+str(results3[i][2]))
	
Levenshtein_search.clear_wordset(first_wordset)

assert len(results3) == 1