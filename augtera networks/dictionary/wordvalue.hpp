#ifndef __dictionary__
#define __dictionary__

//assuming only lower case alphabets
#define NUM_ALPHABETS 26

class TrieNode {
    public:
    class TrieNode* child[NUM_ALPHABETS];
    bool isWord;
    int value;
    TrieNode(){
        for(int i=0; i<NUM_ALPHABETS; i++){
            child[i] = NULL;
        }
        isWord = false;
        value = 0;
    }
};

#endif //__dictionary__