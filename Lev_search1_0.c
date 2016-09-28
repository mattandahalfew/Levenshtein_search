//Written by Matt Anderson. 2016.
#include <Python.h>
#if PY_MAJOR_VERSION >= 3
	#define PyString_FromString		PyUnicode_FromString
#endif

char module_docstring[] = "Search through a dictionary to find words d distance away from a query word";
char pop_dict_docstring[] = "Add a Python list of strings to the dictionary";
char lookup_docstring[] = "Lookup query word to find word set d distance away or less";
char clr_dict_docstring[] = "Clear dictionary and free memory";

struct Node {
	char myletter;
	struct Node* left;
	struct Node* right;
	void* nextletter;
};

struct Btree {
	struct Node* root;
	int numels;
	struct Node** possibleletters;
};

static struct Btree* blank_Btree() {
	struct Btree* outp = (struct Btree*)malloc(sizeof(struct Btree));
	
	outp->root = NULL;
	outp->numels = 0;
	outp->possibleletters = NULL;
	
	return outp;
}

struct WordMatch {
	struct WordMatch* left;
	struct WordMatch* right;
	char* myword;
	char* unique_id;
};

//Standard constructor for WordMatch
static struct WordMatch* new_WordMatch(char* newword, char* unique_p) {
	struct WordMatch* p_wordmatch = (struct WordMatch*)malloc(sizeof(struct WordMatch));
	p_wordmatch->left = NULL;
	p_wordmatch->right = NULL;
	p_wordmatch->myword = newword;
	p_wordmatch->unique_id = unique_p;
	
	return p_wordmatch;
}

static struct WordMatch* WordMatch_insert(struct WordMatch* p_node, struct WordMatch* toadd) {
	if (p_node == NULL) {
		p_node = toadd;
	}
	else if (toadd->unique_id < p_node->unique_id) {
		p_node->left = WordMatch_insert(p_node->left,toadd);
	}
	else if (toadd->unique_id > p_node->unique_id) {
		p_node->right = WordMatch_insert(p_node->right,toadd);
	}
	else {
		free((void*)(toadd->myword));
		free((void*)toadd);
	}
	
	return p_node;
}

struct WordLList {
	struct WordLList* below;
	int length;
	struct WordMatch* myword;
};

static struct WordLList* new_WordLList(struct WordMatch* p_wordmatch) {
	struct WordLList* outp = (struct WordLList*)malloc(sizeof(struct WordLList));
	outp->length = 0;
	outp->myword = p_wordmatch;
	outp->below = NULL;
	
	return outp;
}

static void WordLList_recadd(struct WordLList* p_node, struct WordLList* toadd) {
	if (p_node->below == NULL) {
		p_node->below = toadd;
	}
	else {
		WordLList_recadd(p_node->below,toadd);
	}
}

static struct Node* newnode(char lett, void* nextlett) {
	struct Node* outputnode = (struct Node*)malloc(sizeof(struct Node));

	outputnode->myletter = lett;
	outputnode->left = NULL;
	outputnode->right = NULL;
	outputnode->nextletter = nextlett;

	return outputnode;
}

static struct Node* rec_insert(struct Node* node, char tobeinserted, void* nextletter) {
	if (node == NULL) {
		node = newnode(tobeinserted,nextletter);
	}
	else if (tobeinserted < node->myletter) {
		node->left = rec_insert(node->left,tobeinserted,nextletter);
	}
	else {
		node->right = rec_insert(node->right,tobeinserted,nextletter);
	}

	return node;
}

static struct Btree* insert(struct Btree* btree, char tobeinserted) {
	struct Btree* nextletter_casted = blank_Btree();

	btree->root = rec_insert(btree->root,tobeinserted,(void*)nextletter_casted);
	btree->numels += 1;
	
	if ((btree->possibleletters) != NULL) {
		free((void*)(btree->possibleletters));
		btree->possibleletters = NULL;
	}
	
	return nextletter_casted;
}

struct Btree* dictionary = NULL;

static struct Btree* nextlett_lookup(struct Node* node, char lett) {
	if (node==NULL) return NULL;
	else if (lett == node->myletter) return (struct Btree*)(node->nextletter);
	else if (lett < node->myletter) return nextlett_lookup(node->left,lett);
	else return nextlett_lookup(node->right,lett);
}

static void addword(char* p_string, int wordlength) {
	struct Btree* curr_letter = dictionary;
	struct Btree* nextlettertree;
	int i;
	
	wordlength++; //Because we are also adding the null character at the end
	
	for (i = 0; i < wordlength; i++) {
		nextlettertree = nextlett_lookup(curr_letter->root,p_string[i]);
		if (nextlettertree==NULL) {
			nextlettertree = insert(curr_letter,p_string[i]);
		}
		curr_letter = nextlettertree;
	}
}

static char* new_letterssofar(char* letterssofar, char newletter, int d_x) {
	int i;
	char* newletterssofar;
	if (newletter==0) newletterssofar = (char *)malloc(d_x * sizeof(char));
	else newletterssofar = (char *)malloc((d_x + 1) * sizeof(char));
	
	for (i = 0; i < d_x; i++) {
		*(newletterssofar++) = *(letterssofar++);
	}
	if (newletter!=0) *newletterssofar = newletter;
	
	return newletterssofar - d_x;
}

static void rec_getletters(struct Node** letterarray, int* p_idxletter, struct Node* node) {
	if (node != NULL) {
		rec_getletters(letterarray,p_idxletter,node->left);
		*(letterarray+(*p_idxletter)) = node;
		*p_idxletter += 1;
		rec_getletters(letterarray,p_idxletter,node->right);
	}
}

static struct Node** getpossibleletters(struct Btree* btree) {
	struct Node** possibleletters = (struct Node**)malloc((btree->numels) * sizeof(struct Node*));
	int idxletter = 0;
	
	rec_getletters(possibleletters,&idxletter,btree->root);
	
	return possibleletters;
}

static void compare_letters(struct Btree* curr_letter, int d_x, int q_x, int c_dist, int maxdist, char* query_word, int qwordlength, char* letterssofar, struct WordMatch* wordlist) {
	char new_qletter = query_word[q_x];
	if (q_x >= qwordlength) {
		new_qletter = 0;
	}

	if (curr_letter->possibleletters == NULL) curr_letter->possibleletters = getpossibleletters(curr_letter);
	struct Node** p_possibleletters = curr_letter->possibleletters;
	struct Node* letternode;
	int n = (curr_letter->numels) - 1;
	int i;
	struct Btree* p_nextletter;
	char new_nletter;
	char* nletterssofar;
	
	for (i = 0; i < n; i++) {
		letternode = *(p_possibleletters++);
		p_nextletter = (struct Btree*)(letternode->nextletter);
		new_nletter = letternode->myletter;
		if (new_nletter != 0) {
			if (new_nletter == new_qletter) {
				compare_letters(p_nextletter,d_x+1,q_x+1,c_dist,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_qletter,d_x),wordlist);
				compare_letters(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
				compare_letters(curr_letter,d_x,q_x+1,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,0,d_x),wordlist);
			}
			else if (c_dist < maxdist) {
				if (new_qletter!=0) {
					compare_letters(p_nextletter,d_x+1,q_x+1,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
					compare_letters(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
					compare_letters(curr_letter,d_x,q_x+1,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,0,d_x),wordlist);
				}
				else {
					compare_letters(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
				}
			}
		}
		else if ((c_dist+qwordlength-(q_x+1)) < maxdist) {
			nletterssofar = new_letterssofar(letterssofar,0,d_x+1);
			nletterssofar[d_x] = 0;
			wordlist->left = WordMatch_insert(wordlist->left,new_WordMatch(nletterssofar,&(letternode->myletter)));
		}
	}
	letternode = *(p_possibleletters++);
	p_nextletter = (struct Btree*)(letternode->nextletter);
	new_nletter = letternode->myletter;
	if (new_nletter != 0) {
		if (new_nletter == new_qletter) {
			compare_letters(p_nextletter,d_x+1,q_x+1,c_dist,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_qletter,d_x),wordlist);
			compare_letters(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			compare_letters(curr_letter,d_x,q_x+1,c_dist+1,maxdist,query_word,qwordlength,letterssofar,wordlist);
		}
		else if (c_dist < maxdist) {
			if (new_qletter!=0) {
				compare_letters(p_nextletter,d_x+1,q_x+1,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
				compare_letters(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
				compare_letters(curr_letter,d_x,q_x+1,c_dist+1,maxdist,query_word,qwordlength,letterssofar,wordlist);
			}
			else {
				compare_letters(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			}
		}
		else {
			free((void*)letterssofar);
		}
	}
	else if ((c_dist+qwordlength-(q_x+1)) < maxdist) {
		nletterssofar = new_letterssofar(letterssofar,0,d_x+1);
		free((void*)letterssofar);
		nletterssofar[d_x] = 0;
		wordlist->left = WordMatch_insert(wordlist->left,new_WordMatch(nletterssofar,&(letternode->myletter)));
	}
	else {
		free((void*)letterssofar);
	}	

}

static void traverse_wordlist(struct WordMatch* node, struct WordLList* dest) {
	if (node != NULL) {
		traverse_wordlist(node->left,dest);
		dest->length += 1;
		WordLList_recadd(dest,new_WordLList(node));
		traverse_wordlist(node->right,dest);
	}
}

static struct WordLList* gen_wordllist(struct WordMatch* p_wordlist) {
	struct WordLList* outp = new_WordLList(NULL);
	
	traverse_wordlist(p_wordlist->left,outp);
	
	return outp;	
}

static struct WordLList* generate_wordlist(char* query_word, int maxdist) {
	struct WordMatch* p_wordlist = new_WordMatch(NULL,NULL);
	int wordlength = 0;
	struct WordLList* p_wordllist;

	while (query_word[wordlength]!=0) {
		wordlength++;
	}
	
	compare_letters(dictionary,0,0,0,maxdist,query_word,wordlength,NULL,p_wordlist);
	p_wordllist = gen_wordllist(p_wordlist);
	free((void*)p_wordlist);
	
	return p_wordllist;
}

static void rec_clear(struct Node* node) {
	struct Btree* p_btree;
	if (node != NULL) {
		p_btree = (struct Btree*)(node->nextletter);
		rec_clear(node->left);
		rec_clear(node->right);
		if (p_btree != NULL) {
			rec_clear(p_btree->root);
			if (p_btree->possibleletters != NULL) {
				free((void*)(p_btree->possibleletters));
			}
			free((void*)p_btree);
		}
		printf("%c",node->myletter);
		free((void*)node);
	}
}

static PyObject* llist2pylist(struct WordLList* llist) {
	int len = llist->length;
	PyObject *list,*pystring;
	int i;
	struct WordMatch* p_wordmatch;
	struct WordLList *p_llist,*temp_llist;
	
	list = PyList_New(len);
	p_llist = llist->below;
	free((void*)llist);
	for (i = 0; i < len; i++) {
		p_wordmatch = p_llist->myword;
		pystring = PyString_FromString(p_wordmatch->myword);
		PyList_SetItem(list, i, pystring);
		temp_llist = p_llist;
		p_llist = p_llist->below;
		
		free((void*)p_wordmatch->myword);
		free((void*)p_wordmatch);
		free((void*)temp_llist);
	}
	
	return list;	
}

static PyObject* clear_dictionary(PyObject *self, PyObject *args) {
	printf("Deleting letters...\n");
	rec_clear(dictionary->root);
	dictionary = NULL;
	Py_RETURN_NONE;
}

static PyObject* lookup(PyObject *self, PyObject *args)
{
	PyObject *pystring,*pyoutlist;
	char* mystring;
	struct WordLList* p_wordllist;
	int maxdist;
	
	if (!PyArg_ParseTuple(args, "si", &pystring, &maxdist))
		return NULL;

	mystring = (char*)pystring;
	
	printf("Lookup word: %s\n",mystring);

	p_wordllist = generate_wordlist(mystring,maxdist);
	
	pyoutlist = llist2pylist(p_wordllist);
	
	return pyoutlist;
}

static PyObject *populate_dictionary(PyObject *self, PyObject *args)
{
	PyObject *pystrings, *listobj;
#if PY_MAJOR_VERSION >= 3
	PyObject *listobj2;
#endif
	char *mystring;
	Py_ssize_t l, i;
	int wordlength;

	if (!PyArg_ParseTuple(args, "O", &pystrings)) {
		Py_RETURN_NONE;
	}
	l = PyList_Size(pystrings);
	
	if (l > 0) {
		dictionary = blank_Btree();
	}

	for (i = 0; i < l; i++) {
		listobj = PyList_GetItem(pystrings,i);
	#if PY_MAJOR_VERSION >= 3
		listobj2 = PyUnicode_AsASCIIString(listobj);
		wordlength = (int)PyBytes_Size(listobj2);
		mystring = PyBytes_AsString(listobj2);
	#else
		wordlength = (int)PyString_Size(listobj);
		mystring = PyString_AsString(listobj);
	#endif
		printf("Adding: %s\n",mystring);
		addword(mystring,wordlength);
	}
		
	return listobj;
}

static PyMethodDef module_methods[] = {
	{"populate_dictionary", populate_dictionary, METH_VARARGS, pop_dict_docstring},
	{"lookup", lookup, METH_VARARGS, lookup_docstring},
	{"clear_dictionary", clear_dictionary, METH_VARARGS,clr_dict_docstring},
		{NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3
static PyModuleDef module_def = {
	PyModuleDef_HEAD_INIT,
	"Levenshtein_search",
	module_docstring,
	-1,
	module_methods,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC PyInit_Levenshtein_search()
{
	PyObject *m = PyModule_Create(&module_def);
}
#else
PyMODINIT_FUNC initLevenshtein_search(void)
{
	PyObject *m = Py_InitModule3("Levenshtein_search", module_methods, module_docstring);
}
#endif
