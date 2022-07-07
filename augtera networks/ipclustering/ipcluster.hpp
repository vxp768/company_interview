#ifndef __ipcluster__
#define __ipcluster__

using namespace std;

class TrieNode {
    public:
    TrieNode *child[2]; // 0 or 1 at each level
   // TrieNode *next_level[3];
    long pkt_count;
    bool valid_prefix;
    bool leaf_node;
    TrieNode() {
        for(int i=0; i<2; i++){
            child[i] = NULL;
        }
      //  for(int i=0; i<3; i++){
      //      next_level[i] = NULL;
     //   }
        pkt_count = 0;
        valid_prefix = false;
        leaf_node = false;
    }
};

void process_cluster_file(string cluster_file, TrieNode *root);
void get_ip_num_pkt(string str, TrieNode *root);
void update_trie_ip_pkt(TrieNode *root, string ip, int num_pkt);
void find_subnet_threshold(TrieNode *root, int thresh);

#endif //__ipcluster__