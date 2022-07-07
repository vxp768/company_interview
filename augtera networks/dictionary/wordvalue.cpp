#include<iostream>
#include<thread>
#include<fstream>
#include<vector>
#include<unordered_map>
#include<queue>

#include"wordvalue.hpp"

using namespace std;

vector<string> max_val_str;
int max_val=0;

//string file_path = "value_file_2.txt";
//string dictionary_file = "dictionary";

/*
    Add each string to Value Trie and store the value if node is end of
    valid string in valud file.
*/
void add_to_trie(TrieNode *root, string str, int val)
{
    TrieNode *node = root;
    for(int i=0; i<str.size(); i++){
        //assuming only lower case letters
        if(node->child[str[i]-'a'] == NULL) {
            node->child[str[i]-'a'] = new TrieNode();
        }
        node = node->child[str[i]-'a'];
    }
    node->isWord = true;
    node->value = val;
}


void parse_value_file(TrieNode *root, string value_file_path)
{
    string buf_str;
    ifstream infile(value_file_path);
    while(getline(infile, buf_str)) {
        string str_trie;
        int val=0;
        //extract string and integer value from each line
        for(int i=0; i<buf_str.size(); i++){
            if(isalpha(buf_str[i])){
                str_trie += buf_str[i];
            } else if (isdigit(buf_str[i])){
                val = (val * 10) + (buf_str[i]-'0');
            }   
        }
        add_to_trie(root, str_trie, val);
        //cout<<"str: "<<str_trie<<"val: "<<val<<endl;
    }
}

/*
    Loop till the prefix is valid in Value Trie.
    Return the index of char not found in Value Trie.
    If the first char in prefix is not found then ignore that char and return idx+1 
*/
int find_word_value_trie(TrieNode *root, string str, int idx, int& val)
{
    TrieNode *node = root;
    int tmp_val = 0;
    int i=idx;

    if(root->child[str[i]-'a']==NULL){
        return i+1;
    }
    for( ; i<str.size(); i++) {
        if(node->child[str[i]-'a']==NULL){
            val += tmp_val;
            return i;
        } else {
            node = node->child[str[i]-'a'];
            if(node->isWord){
                tmp_val = node->value;
            }
        }
    }
    val += tmp_val;
    return i;
}

void parse_dictionary_file(TrieNode *root, string dict_file_path)
{
    ifstream dict_file(dict_file_path);    
    string buf_str;
    while(getline(dict_file, buf_str)) {
        //cout<<buf_str<<endl;
        int i=0, val=0;
        while(i < buf_str.size()){
            //Reset to root of Value Trie after first valid prefix was found.
            i = find_word_value_trie(root, buf_str, i, val);
            //cout<<"i: "<<i<<"str: "<<"val: "<<val<<endl;
        }
        //cout<<"str: "<<buf_str<<" val: "<<val<<endl;
        if(val > max_val){
            max_val_str.clear(); //clear the array
            max_val_str.push_back(buf_str);
            max_val = val;
        } else if (val == max_val) {
            max_val_str.push_back(buf_str);
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc != 3){
        cout<<"Invalid arguments."<<endl;
        cout<<"Usage: <prog name> <dict file> <value file>"<<endl;
        return 0;
    }
    string dict_file_path = argv[1];
    string value_file_path = argv[2];

    TrieNode *root = new TrieNode();
    parse_value_file(root, value_file_path);
    parse_dictionary_file(root, dict_file_path);
    //print all max val words
    for(int i=0; i<max_val_str.size(); i++){
        cout<<max_val_str[i]<<" "<<max_val<<endl;
    }
    delete root;
    return 0;
}