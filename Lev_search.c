//Written by Matt Anderson. v1.4.3, 2016 through February 24, 2017.
#include <Python.h>
#if PY_MAJOR_VERSION >= 3
	#define PyString_FromString		PyUnicode_FromString
#endif

char module_docstring[] = "Search through a set of words to find words d distance away from a query word";
char init_wdset_docstring[] = "Add a Python list of strings to the wordset";
char lookup_docstring[] = "Lookup query word to find a subset of words d distance away or less";
char clr_wdset_docstring[] = "Clear word set and free memory";
char add_string_docstring[] = "Add a string to the wordset. Returns unique index.";
char rem_string_docstring[] = "Remove a string from the wordset";

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

struct WordSet {
	struct Btree* firstletter;
	int nwords;
	int nunique_words;
	struct WordSet* below;
};

struct WordSet* all_wordsets = NULL;
int nwordsets = 0;

static struct WordSet* new_WordSet() {
	struct WordSet* outp = (struct WordSet*)malloc(sizeof(struct WordSet));
	
	outp->firstletter = blank_Btree();
	outp->nwords = 0;
	outp->nunique_words = 0;
	outp->below = NULL;
	
	return outp;
}

struct WordMatch {
	struct WordMatch* left;
	struct WordMatch* right;
	char* myword;
	void* unique_id;
	unsigned char lev_dist;
};

//Standard constructor for WordMatch
static struct WordMatch* new_WordMatch(char* newword, void* unique_p, unsigned char dist) {
	struct WordMatch* p_wordmatch = (struct WordMatch*)malloc(sizeof(struct WordMatch));
	p_wordmatch->left = NULL;
	p_wordmatch->right = NULL;
	p_wordmatch->myword = newword;
	p_wordmatch->unique_id = unique_p;
	p_wordmatch->lev_dist = dist;
	
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
		if (toadd->lev_dist < p_node->lev_dist) {
			p_node->lev_dist = toadd->lev_dist;
		}
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

struct WordStats {
	int noccurrences;
	int idx;
};

static struct WordStats* new_WordStats() {
	struct WordStats* outp = (struct WordStats*)malloc(sizeof(struct WordStats));
	outp->noccurrences = 0;
	
	return outp;
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

static void* insert(struct Btree* btree, char tobeinserted) {
	void* nextletter;
	
	if (tobeinserted==0) {
		nextletter = (void*)new_WordStats();
	}
	else {
		nextletter = (void*)blank_Btree();
	}

	btree->root = rec_insert(btree->root,tobeinserted,nextletter);
	btree->numels += 1;
	
	if ((btree->possibleletters) != NULL) {
		free((void*)(btree->possibleletters));
		btree->possibleletters = NULL;
	}
	
	return nextletter;
}

static void* nextlett_lookup(struct Node* node, char lett) {
	if (node==NULL) return NULL;
	else if (lett == node->myletter) return node->nextletter;
	else if (lett < node->myletter) return nextlett_lookup(node->left,lett);
	else return nextlett_lookup(node->right,lett);
}

static int addword(struct WordSet* p_wordset, char* p_string, int wordlength) {
	struct Btree* curr_letter = p_wordset->firstletter;
	void* nextlettertree;
	int i;
	char isnew = 0;
	struct WordStats* p_wordstats;
	
	for (i = 0; i < wordlength; i++) {
		nextlettertree = nextlett_lookup(curr_letter->root,p_string[i]);
		if (nextlettertree==NULL) {
			isnew = 1;
			nextlettertree = insert(curr_letter,p_string[i]);
		}
		curr_letter = (struct Btree*)nextlettertree;
	}
	nextlettertree = nextlett_lookup(curr_letter->root,p_string[i]);
	if (nextlettertree==NULL) {
		isnew = 1;
		nextlettertree = insert(curr_letter,p_string[i]);
	}
	
	p_wordstats = (struct WordStats*)nextlettertree;
	p_wordstats->noccurrences += 1;
	if (isnew!=0) {
		p_wordstats->idx = p_wordset->nunique_words;
		p_wordset->nunique_words += 1;
	}
	
	return p_wordstats->idx;
}

static char* new_letterssofar(char* letterssofar, char newletter, int d_x) {
	int i;
	char* newletterssofar = (char *)malloc((d_x + 1) * sizeof(char));;
	
	for (i = 0; i < d_x; i++) {
		*(newletterssofar++) = *(letterssofar++);
	}
	*newletterssofar = newletter;
	
	return newletterssofar - d_x;
}

static char* add_letterssofar(char* letterssofar, char newletter, int d_x) {
	letterssofar = (char*)realloc((void*)letterssofar,(d_x+1)*sizeof(char));
	letterssofar[d_x] = newletter;
	return letterssofar;
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

static struct Node* replace_max(struct Node* node) {
	struct Node* maxnode;
	
	if (node->right == NULL) {
		return node;
	}
	else {
		maxnode = replace_max(node->right);
		if (maxnode==node->right) {
			node->right = maxnode->left;
		}
		return maxnode;
	}
}

static struct Node* delete_node(struct Node* node, char char_delete) {
	struct Node* replacement_node;
	
	if (char_delete == node->myletter) {
		if (node->left == NULL) {
			replacement_node = node->right;
		}
		else {
			replacement_node = replace_max(node->left);
			replacement_node->left = node->left;
			replacement_node->right = node->right;
		}
		free((void*)node);
		return replacement_node;
	}
	else if (char_delete < node->myletter) {
		node->left = delete_node(node->left,char_delete);
		return node;
	}
	else {
		node->right = delete_node(node->right,char_delete);
		return node;
	}
}

static void* delete_Btree(struct Btree* this_letter, int q_x, char* p_word, char* b_issafe) {
	void* nextletter = nextlett_lookup(this_letter->root,p_word[q_x]);
	void* wordstats;

	if (p_word[q_x] == 0) {		
		if (this_letter->numels == 1) {
			free((void*)this_letter->root);
			if (this_letter->possibleletters != NULL) {
				free((void*)(this_letter->possibleletters));
			}
		}
		else {
			this_letter->root = delete_node(this_letter->root,0);
			this_letter->numels -= 1;
			if (this_letter->possibleletters != NULL) {
				free((void*)(this_letter->possibleletters));
				this_letter->possibleletters = NULL;
			}
			*b_issafe = 0;
		}
		return nextletter;
	}
	else {
		wordstats = delete_Btree((struct Btree*)nextletter,q_x+1,p_word,b_issafe);
		if (*b_issafe != 0) {
			if (this_letter->numels == 1) {
				free(nextletter);
				free((void*)this_letter->root);
				if (this_letter->possibleletters != NULL) {
					free((void*)(this_letter->possibleletters));
				}
			}
			else {
				free(nextletter);
				this_letter->root = delete_node(this_letter->root,p_word[q_x]);
				this_letter->numels -= 1;
				if (this_letter->possibleletters != NULL) {
					free((void*)(this_letter->possibleletters));
					this_letter->possibleletters = NULL;
				}				
				*b_issafe = 0;
			}
		}
		return wordstats;
	}
}

static void compare_same(struct Btree* curr_letter, int d_x, unsigned char lev_dist, char* query_letter, char* letterssofar, struct WordMatch* wordlist) {
	char new_qletter = *query_letter;
	void* p_nextletter = nextlett_lookup(curr_letter->root,new_qletter);
	
	if (new_qletter==0) {
		if (p_nextletter != NULL) {
			letterssofar = add_letterssofar(letterssofar,0,d_x);
			wordlist->left = WordMatch_insert(wordlist->left,new_WordMatch(letterssofar,p_nextletter,lev_dist));
		}
		else {
			free((void*)letterssofar);
		}
	}
	else if (p_nextletter != NULL) {
		letterssofar = add_letterssofar(letterssofar,new_qletter,d_x);
		compare_same((struct Btree*)p_nextletter,d_x+1,lev_dist,query_letter+1,letterssofar,wordlist);
	}
	else {
		free((void*)letterssofar);
	}
}

void compare_right(struct Btree* curr_letter, int d_x, int q_x, int c_dist, int maxdist, char* query_word, int qwordlength, char* letterssofar, struct WordMatch* wordlist);
void compare_down(struct Btree* curr_letter, int d_x, int q_x, int c_dist, int maxdist, char* query_word, int qwordlength, char* letterssofar, struct WordMatch* wordlist);
void compare_letters(struct Btree* curr_letter, int d_x, int q_x, int c_dist, int maxdist, char* query_word, int qwordlength, char* letterssofar, struct WordMatch* wordlist);

extern void compare_right(struct Btree* curr_letter, int d_x, int q_x, int c_dist, int maxdist, char* query_word, int qwordlength, char* letterssofar, struct WordMatch* wordlist) {
	struct Node** p_possibleletters;
	struct Node* letternode;
	int n,i;
	struct Btree* p_nextletter;
	char new_nletter,new_qletter;
	char* nletterssofar;
		
	if (c_dist == maxdist) {
		compare_same(curr_letter,d_x,(unsigned char)c_dist,query_word+q_x,letterssofar,wordlist);
		return;
	}
	
	new_qletter = query_word[q_x];

	if (curr_letter->possibleletters == NULL) curr_letter->possibleletters = getpossibleletters(curr_letter);
	p_possibleletters = curr_letter->possibleletters;
	
	n = (curr_letter->numels) - 1;
	for (i = 0; i < n; i++) {
		letternode = *(p_possibleletters++);
		p_nextletter = (struct Btree*)(letternode->nextletter);
		new_nletter = letternode->myletter;
		if (new_nletter != 0) {
			if (new_nletter == new_qletter) {
				compare_letters(p_nextletter,d_x+1,q_x+1,c_dist,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_qletter,d_x),wordlist);
			}
			else if (new_qletter!=0) {
				compare_letters(p_nextletter,d_x+1,q_x+1,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			}
		}
		else if ((c_dist+qwordlength-q_x) <= maxdist) {
			nletterssofar = new_letterssofar(letterssofar,0,d_x);
			wordlist->left = WordMatch_insert(wordlist->left,new_WordMatch(nletterssofar,(void*)p_nextletter,(unsigned char)(c_dist+qwordlength-q_x)));
		}
	}
	letternode = *p_possibleletters;
	p_nextletter = (struct Btree*)(letternode->nextletter);
	new_nletter = letternode->myletter;
	if (new_nletter != 0) {
		if (new_nletter == new_qletter) {
			compare_letters(p_nextletter,d_x+1,q_x+1,c_dist,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_qletter,d_x),wordlist);
			compare_right(curr_letter,d_x,q_x+1,c_dist+1,maxdist,query_word,qwordlength,letterssofar,wordlist);
		}
		else if (new_qletter!=0) {
			compare_letters(p_nextletter,d_x+1,q_x+1,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			compare_right(curr_letter,d_x,q_x+1,c_dist+1,maxdist,query_word,qwordlength,letterssofar,wordlist);
		}
		else {
			free((void*)letterssofar);
		}
	}
	else if ((c_dist+qwordlength-q_x) <= maxdist) {
		wordlist->left = WordMatch_insert(wordlist->left,new_WordMatch(add_letterssofar(letterssofar,0,d_x),(void*)p_nextletter,(unsigned char)(c_dist+qwordlength-q_x)));
	}
	else {
		free((void*)letterssofar);
	}	
}

extern void compare_down(struct Btree* curr_letter, int d_x, int q_x, int c_dist, int maxdist, char* query_word, int qwordlength, char* letterssofar, struct WordMatch* wordlist) {
	struct Node** p_possibleletters;
	struct Node* letternode;
	int n,i;
	struct Btree* p_nextletter;
	char new_nletter,new_qletter;
	char* nletterssofar;
	
	if (c_dist == maxdist) {
		compare_same(curr_letter,d_x,(unsigned char)c_dist,query_word+q_x,letterssofar,wordlist);
		return;
	}
	new_qletter = query_word[q_x];

	if (curr_letter->possibleletters == NULL) curr_letter->possibleletters = getpossibleletters(curr_letter);
	p_possibleletters = curr_letter->possibleletters;
	
	n = (curr_letter->numels) - 1;
	for (i = 0; i < n; i++) {
		letternode = *(p_possibleletters++);
		p_nextletter = (struct Btree*)(letternode->nextletter);
		new_nletter = letternode->myletter;
		if (new_nletter != 0) {
			if (new_nletter == new_qletter) {
				compare_letters(p_nextletter,d_x+1,q_x+1,c_dist,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_qletter,d_x),wordlist);
				compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			}
			else if (new_qletter!=0) {
				compare_letters(p_nextletter,d_x+1,q_x+1,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
				compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			}
			else {
				compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			}
		}
		else if ((c_dist+qwordlength-q_x) <= maxdist) {
			nletterssofar = new_letterssofar(letterssofar,0,d_x);
			wordlist->left = WordMatch_insert(wordlist->left,new_WordMatch(nletterssofar,(void*)p_nextletter,(unsigned char)(c_dist+qwordlength-q_x)));
		}
	}
	letternode = *p_possibleletters;
	p_nextletter = (struct Btree*)(letternode->nextletter);
	new_nletter = letternode->myletter;
	if (new_nletter != 0) {
		if (new_nletter == new_qletter) {
			compare_letters(p_nextletter,d_x+1,q_x+1,c_dist,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_qletter,d_x),wordlist);
			compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,add_letterssofar(letterssofar,new_nletter,d_x),wordlist);
		}
		else if (new_qletter!=0) {
			compare_letters(p_nextletter,d_x+1,q_x+1,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,add_letterssofar(letterssofar,new_nletter,d_x),wordlist);
		}
		else {
			compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,add_letterssofar(letterssofar,new_nletter,d_x),wordlist);
		}
	}
	else if ((c_dist+qwordlength-q_x) <= maxdist) {
		wordlist->left = WordMatch_insert(wordlist->left,new_WordMatch(add_letterssofar(letterssofar,0,d_x),(void*)p_nextletter,(unsigned char)(c_dist+qwordlength-q_x)));
	}
	else {
		free((void*)letterssofar);
	}
}

extern void compare_letters(struct Btree* curr_letter, int d_x, int q_x, int c_dist, int maxdist, char* query_word, int qwordlength, char* letterssofar, struct WordMatch* wordlist) {
	struct Node** p_possibleletters;
	struct Node* letternode;
	int n,i;
	struct Btree* p_nextletter;
	char new_nletter,new_qletter;
	char* nletterssofar;
		
	if (c_dist == maxdist) {
		compare_same(curr_letter,d_x,(unsigned char)c_dist,query_word+q_x,letterssofar,wordlist);
		return;
	}
	
	new_qletter = query_word[q_x];

	if (curr_letter->possibleletters == NULL) curr_letter->possibleletters = getpossibleletters(curr_letter);
	p_possibleletters = curr_letter->possibleletters;
	
	n = (curr_letter->numels) - 1;
	for (i = 0; i < n; i++) {
		letternode = *(p_possibleletters++);
		p_nextletter = (struct Btree*)(letternode->nextletter);
		new_nletter = letternode->myletter;
		if (new_nletter != 0) {
			if (new_nletter == new_qletter) {
				compare_letters(p_nextletter,d_x+1,q_x+1,c_dist,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_qletter,d_x),wordlist);
				compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			}
			else if (new_qletter!=0) {
				compare_letters(p_nextletter,d_x+1,q_x+1,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
				compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			}
			else {
				compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			}
		}
		else if ((c_dist+qwordlength-q_x) <= maxdist) {
			nletterssofar = new_letterssofar(letterssofar,0,d_x);
			wordlist->left = WordMatch_insert(wordlist->left,new_WordMatch(nletterssofar,(void*)p_nextletter,(unsigned char)(c_dist+qwordlength-q_x)));
		}
	}
	letternode = *p_possibleletters;
	p_nextletter = (struct Btree*)(letternode->nextletter);
	new_nletter = letternode->myletter;
	if (new_nletter != 0) {
		if (new_nletter == new_qletter) {
			compare_letters(p_nextletter,d_x+1,q_x+1,c_dist,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_qletter,d_x),wordlist);
			compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			compare_right(curr_letter,d_x,q_x+1,c_dist+1,maxdist,query_word,qwordlength,letterssofar,wordlist);
		}
		else if (new_qletter!=0) {
			compare_letters(p_nextletter,d_x+1,q_x+1,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,new_letterssofar(letterssofar,new_nletter,d_x),wordlist);
			compare_right(curr_letter,d_x,q_x+1,c_dist+1,maxdist,query_word,qwordlength,letterssofar,wordlist);
		}
		else {
			compare_down(p_nextletter,d_x+1,q_x,c_dist+1,maxdist,query_word,qwordlength,add_letterssofar(letterssofar,new_nletter,d_x),wordlist);
		}
	}
	else if ((c_dist+qwordlength-q_x) <= maxdist) {
		wordlist->left = WordMatch_insert(wordlist->left,new_WordMatch(add_letterssofar(letterssofar,0,d_x),(void*)p_nextletter,(unsigned char)(c_dist+qwordlength-q_x)));
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

static struct WordLList* generate_wordlist(struct WordSet* p_wordset, char* query_word, int maxdist) {
	struct WordMatch* p_wordlist = new_WordMatch(NULL,NULL,255);
	int wordlength = 0;
	struct WordLList* p_wordllist;

	if ((p_wordset->nwords) > 0) {
		while (query_word[wordlength]!=0) {
			wordlength++;
		}
		
		compare_letters(p_wordset->firstletter,0,0,0,maxdist,query_word,wordlength,NULL,p_wordlist);
		p_wordllist = gen_wordllist(p_wordlist);
	}
	else {
		p_wordllist = new_WordLList(p_wordlist);
	}
	free((void*)p_wordlist);
	
	return p_wordllist;
}

static void rec_clear(struct Node* node) {
	struct Btree* p_btree;
	void* p_nextletter;
	if (node != NULL) {
		p_nextletter = node->nextletter;
		rec_clear(node->left);
		rec_clear(node->right);
		if (p_nextletter != NULL) {
			if (node->myletter != 0) {
				p_btree = (struct Btree*)p_nextletter;
				rec_clear(p_btree->root);
				if (p_btree->possibleletters != NULL) {
					free((void*)(p_btree->possibleletters));
				}
			}
			free(p_nextletter);
		}
		//printf("%c",node->myletter);
		free((void*)node);
	}
}

static char b_samestring(char* a, char* b) {
	char outp = 1;
	
	while ((outp!=0) && ((*a!=0) || (*b!=0))) {
		if (*a==*b) {
			a++;
			b++;
		}
		else {
			outp = 0;
		}
	}
	
	return outp;
}

static void wordinfront(struct WordLList* llist, char* frontword) {
	int i;
	int len = llist->length;
	struct WordLList* temp_llist;
	struct WordLList* p_llist = llist;
	
	for (i = 0; i < len; i++) {
		temp_llist = p_llist->below;
		if (b_samestring(temp_llist->myword->myword,frontword)!=0) {
			p_llist->below = temp_llist->below;
			temp_llist->below = llist->below;
			llist->below = temp_llist;
			i = len; //To exit the loop
		}
		else {
			p_llist = temp_llist;
		}
	}	
}

//The term-frequency is computed in this function
static PyObject* llist2pylist(struct WordLList* llist, int totalwords, char* frontword) {
	int len = llist->length;
	PyObject *list,*wf_pair;
	int i;
	struct WordMatch* p_wordmatch;
	struct WordLList *p_llist,*temp_llist;
	struct WordStats* p_word_freq;
	double n = (double)totalwords;
	double f;
	
	wordinfront(llist,frontword);
	
	list = PyList_New(len);
	p_llist = llist->below;
	free((void*)llist);
	for (i = 0; i < len; i++) {
		p_wordmatch = p_llist->myword;

		wf_pair = PyList_New(3);
		PyList_SetItem(wf_pair,0,PyString_FromString(p_wordmatch->myword));
		PyList_SetItem(wf_pair,1,Py_BuildValue("b", p_wordmatch->lev_dist));
		
		p_word_freq = (struct WordStats*)(p_wordmatch->unique_id);
		f = ((double)(p_word_freq->noccurrences)) / n;
		PyList_SetItem(wf_pair,2,Py_BuildValue("d", f));

		PyList_SetItem(list, i, wf_pair);
		temp_llist = p_llist;
		p_llist = p_llist->below;
		
		free((void*)p_wordmatch->myword);
		free((void*)p_wordmatch);
		free((void*)temp_llist);
	}
	
	return list;	
}

//Returns null if x >= the number of WordSets. Otherwise returns the appropriate WordSet.
static struct WordSet* get_xwordset(struct WordSet* ws, int x) {
	if ((x<1) || (ws==NULL)) {
		return ws;
	}
	else {
		return get_xwordset(ws->below,x-1);
	}
}

static struct WordSet* get_pwordset(int idx_ws) {
	struct WordSet* p_wordset;
	
	if ((all_wordsets==NULL) || (idx_ws<0) || (idx_ws>=nwordsets)) {
		p_wordset = NULL;
	}
	else {
		p_wordset = get_xwordset(all_wordsets,idx_ws);
	}
	
	return p_wordset;
}

static PyObject* clear_wordset(PyObject *self, PyObject *args) {
	struct WordSet *p_wordset;
	struct Btree* p_firstletter;
	int idx_ws;
	
	if (!PyArg_ParseTuple(args, "i", &idx_ws))
		Py_RETURN_NONE;
	//printf("Deleting letters...\n");

	p_wordset = get_pwordset(idx_ws);

	if (p_wordset!=NULL) {
		p_firstletter = p_wordset->firstletter;
		rec_clear(p_firstletter->root);
		p_firstletter->root = NULL;
		if (p_firstletter->possibleletters != NULL) {
			free((void*)(p_firstletter->possibleletters));
		}
		p_firstletter->possibleletters = NULL;
		p_firstletter->numels = 0;
		p_wordset->nwords = -1;
	}
	
	Py_RETURN_NONE;
}

static int get_length(char* p_string) {
	int l = 0;
	
	while (*p_string != 0) {
		p_string++;
		l++;
	}
	
	return l;
}

static PyObject* add_string(PyObject *self, PyObject *args) {
	PyObject *pystring;
	char* mystring;
	int wordindex = -1;
	int idx_ws = 0;
	struct WordSet* p_wordset;
	
	if (!PyArg_ParseTuple(args, "is", &idx_ws, &pystring))
		Py_RETURN_NONE;
	
	p_wordset = get_pwordset(idx_ws);	
	if (p_wordset!=NULL) {
		mystring = (char*)pystring;
		wordindex = addword(p_wordset,mystring,get_length(mystring));
		p_wordset->nwords += 1;
	}
		
	return Py_BuildValue("i", wordindex);
}

static PyObject* remove_string(PyObject *self, PyObject *args) {
	PyObject *pystring;
	char* mystring;
	int w_idx = 0;
	struct WordSet* p_wordset;
	struct WordLList *p_wordllist,*temp_wordllist;
	struct WordStats* p_wordstats;
	char b_issafe = 1;
	
	if (!PyArg_ParseTuple(args, "is", &w_idx, &pystring))
		Py_RETURN_NONE;
	
	p_wordset = get_pwordset(w_idx);
	if (p_wordset != NULL) {
		mystring = (char*)pystring;
		p_wordllist = generate_wordlist(p_wordset,mystring,0);
		if (p_wordllist->length == 0) {
			printf("String to be removed was not found in the dictionary\n");
			free((void*)p_wordllist);
			w_idx = -1;
		}
		else {
			//printf("Word found in dictionary\n");
			temp_wordllist = p_wordllist;
			p_wordllist = p_wordllist->below;
			free((void*)temp_wordllist);
			free((void*)(p_wordllist->myword->myword));
			free((void*)(p_wordllist->myword));
			free((void*)p_wordllist);		
			//printf("Deleting word\n");
			p_wordstats = (struct WordStats*)delete_Btree(p_wordset->firstletter,0,mystring,&b_issafe);
			p_wordset->nwords -= p_wordstats->noccurrences;
			w_idx = p_wordstats->idx;
			free((void*)p_wordstats);
		}
	}
	else {
		w_idx = -1;
	}

	return Py_BuildValue("i", w_idx);
}

static PyObject* lookup(PyObject *self, PyObject *args)
{
	PyObject *pystring;
	char* mystring;
	struct WordLList* p_wordllist;
	int maxdist,idx_ws;
	struct WordSet* p_wordset;
	
	if (!PyArg_ParseTuple(args, "isi", &idx_ws, &pystring, &maxdist))
		Py_RETURN_NONE;
	
	p_wordset = get_pwordset(idx_ws);	
	if (p_wordset!=NULL) {
		mystring = (char*)pystring;
		//printf("Lookup word: %s\n",mystring);
		p_wordllist = generate_wordlist(p_wordset,mystring,maxdist);
		//printf("Number of words in doc: %i\n",p_wordset->nwords);		
		return llist2pylist(p_wordllist,p_wordset->nwords,mystring);
	}
	else {
		Py_RETURN_NONE;
	}
}

static struct WordSet* get_blankwordset(struct WordSet* ws, int x) {
	if (ws->below == NULL) {
		ws->below = new_WordSet();
		ws->below->nwords = x+1;
		return ws->below;
	}
	else if (ws->nwords == -1) {
		ws->nwords = x;
		return ws;
	}
	else {
		return get_blankwordset(ws->below,x+1);
	}
}

static PyObject *populate_wordset(PyObject *self, PyObject *args)
{
	PyObject *pystrings, *listobj;
#if PY_MAJOR_VERSION >= 3
	PyObject *listobj2;
#endif
	char *mystring;
	Py_ssize_t l, i;
	int idx,idx_ws;
	struct WordSet* p_wordset;

	if (!PyArg_ParseTuple(args, "iO", &idx_ws, &pystrings)) {
		return Py_BuildValue("i", -1);
	}
	l = PyList_Size(pystrings);
	
	//printf("all_wordsets address: %p\n",all_wordsets);
	if (all_wordsets==NULL) {
		all_wordsets = new_WordSet();
		p_wordset = all_wordsets;
		nwordsets = 1;
		idx_ws = 0;
	}
	else if ((idx_ws < 0) || (idx_ws >= nwordsets)) {
		p_wordset = get_blankwordset(all_wordsets,0);
		idx_ws = p_wordset->nwords;
		p_wordset->nwords = 0;
		if (idx_ws >= nwordsets) {
			nwordsets = idx_ws + 1;
		}
	}
	else {
		p_wordset = get_xwordset(all_wordsets,idx_ws);
	}

	for (i = 0; i < l; i++) {
		listobj = PyList_GetItem(pystrings,i);
	#if PY_MAJOR_VERSION >= 3
		listobj2 = PyUnicode_AsASCIIString(listobj);
		idx = (int)PyBytes_Size(listobj2);
		mystring = PyBytes_AsString(listobj2);
	#else
		idx = (int)PyString_Size(listobj);
		mystring = PyString_AsString(listobj);
	#endif
		//printf("Adding: %s\n",mystring);
		idx = addword(p_wordset,mystring,idx);
	}
	p_wordset->nwords += (int)l;
		
	return Py_BuildValue("i", idx_ws);
}

static PyMethodDef module_methods[] = {
	{"populate_wordset", populate_wordset, METH_VARARGS, init_wdset_docstring},
	{"lookup", lookup, METH_VARARGS, lookup_docstring},
	{"clear_wordset", clear_wordset, METH_VARARGS,clr_wdset_docstring},
	{"add_string", add_string, METH_VARARGS,add_string_docstring},
	{"remove_string", remove_string, METH_VARARGS,rem_string_docstring},
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
