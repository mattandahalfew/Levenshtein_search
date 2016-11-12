import unittest
import Levenshtein_search

class LevenshteinTest(unittest.TestCase):
    def setUp(self):
        self.excerpt1 = ["We","went","to","the","fire","Mother","said","Is","he","cold","Versh","Nome","Versh","said","Take","his","overcoat","and","overshoes","off","Mother","said","How","many","times","do","I","have","to","tell","you","not","to","bring","him","into","the","house","with","his","overshoes","on"]
        self.excerpt2 = ["Yessum","Versh","said","Hold","still","now","He","took","my","overshoes","off","and","unbuttoned","my","coat","Caddy","said","Wait","Versh","Cant","he","go","out","again","Mother","I","want","him","to","go","with","me","Youd","better","leave","him","here","Uncle","Maury","said","Hes","been","out","enough","today"]

    def test_index_increment(self):
        first = Levenshtein_search.populate_wordset(-1, self.excerpt1)
        second = Levenshtein_search.populate_wordset(-1, self.excerpt2)
        print(first, second)
        
        assert first != second
		
    @staticmethod
    def getKey(item):
        return item[0]

    def test_query_overcoat(self):
        index = Levenshtein_search.populate_wordset(-1, self.excerpt1)
        results = Levenshtein_search.lookup(index, 'overcoat', 6)
        results = sorted(results, key=LevenshteinTest.getKey)
        assert results == sorted([['overcoat', 0, 0.023809523809523808],
                           ['went', 6, 0.023809523809523808],
                           ['cold', 6, 0.023809523809523808],
                           ['Versh', 6, 0.047619047619047616],
                           ['overshoes', 4, 0.047619047619047616],
                           ['not', 6, 0.023809523809523808]], key=LevenshteinTest.getKey)

        
        index = Levenshtein_search.populate_wordset(-1, self.excerpt2)
        results = Levenshtein_search.lookup(index, 'overcoat', 6)
        results = sorted(results, key=LevenshteinTest.getKey)
        assert results == sorted([['Versh', 6, 0.044444444444444446],
                           ['overshoes', 4, 0.022222222222222223],
                           ['coat', 4, 0.022222222222222223],
                           ['out', 6, 0.044444444444444446],
                           ['here', 6, 0.022222222222222223]], key=LevenshteinTest.getKey)

    def test_remove_doc(self):
        index = Levenshtein_search.populate_wordset(-1, self.excerpt1)
        Levenshtein_search.remove_string(index, 'overcoat')
        results = Levenshtein_search.lookup(index, 'overcoat', 6)
        results = sorted(results, key=LevenshteinTest.getKey)
        assert results == sorted([['went', 6, 0.024390243902439025],
                           ['cold', 6, 0.024390243902439025],
                           ['Versh', 6, 0.04878048780487805],
                           ['overshoes', 4, 0.04878048780487805],
                           ['not', 6, 0.024390243902439025]], key=LevenshteinTest.getKey)

    def test_clear(self):
        index = Levenshtein_search.populate_wordset(-1, self.excerpt1)
        Levenshtein_search.clear_wordset(index)        

