#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<bitset>
#include "ipcluster.hpp"


long total_num_packets=0;
vector<pair<vector<string>, long>> subnet_prefix;

void update_trie_ip_pkt(TrieNode *root, string ip, int num_pkt)
{
    TrieNode *node = root;
    vector<int> ip_level(4,0);
    int pos;
    total_num_packets += num_pkt;
    for(int i=0; i<ip_level.size()-1; i++){
        pos=ip.find(".");
        ip_level[i]  = stoi(ip.substr(0, pos));
        ip = ip.substr(pos+1);
    }
    ip_level[3]  = stoi(ip);
    //cout<<"L1: "<<ip_level[0]<<" L2: "<<ip_level[1]<<" L3: "<<ip_level[2]<<" L4: "<<ip_level[3]<<endl;
    
    string ip_str;
    for(int i=0; i<ip_level.size(); i++){
        ip_str = bitset<8>(ip_level[i]).to_string();
        if(ip_level[i]==0){
            ip_str="0";
        } else {
            int idx=0;
            while(ip_str[idx] == '0')
                idx++;
            ip_str = ip_str.substr(idx);
        }
        //cout<<"add bit ip string: "<<ip_str<<endl;
        for(int j=0; j<ip_str.size(); j++){
            if(node->child[ip_str[j]-'0'] == NULL){
                node->child[ip_str[j]-'0'] = new TrieNode;
            }
            node->pkt_count += num_pkt; //total count of packets till this bit in prefix
            node = node->child[ip_str[j]-'0'];
        }
        if(i<3){
            node->valid_prefix = true;
        } else {
            node->leaf_node = true;
        }
    }
}
void get_ip_num_pkt(string str, TrieNode *root)
{
    string ip;
    int num_pkt;
    int idx=0;
    while(isspace(str[idx])){
        idx++;
    }
    str = str.substr(idx);
    int pos = str.find(" ");
    ip = str.substr(idx, pos-idx+1);
    num_pkt = stoi(str.substr(pos+1));
    cout<<"ip: "<<ip<<" pkt: "<<num_pkt<<endl;
    update_trie_ip_pkt(root, ip, num_pkt);
}

void process_cluster_file(string cluster_file, TrieNode *root)
{
    ifstream file(cluster_file);    
    string buf_str;
    while(getline(file, buf_str)) {
        //cout<<buf_str<<endl;
        get_ip_num_pkt(buf_str, root);
    }
}

int find_subnet_threshold(TrieNode *root, long threshold_pkt, string str, vector<string> ip_pfix_arr)
{
    string prefix_so_far = str;
    if(root==NULL){
        return 0;
    }
    if(root->pkt_count < threshold_pkt){
        return -1;
    }
    if (root->valid_prefix){
        //store prefix so far and reset str
        ip_pfix_arr.push_back(prefix_so_far);
        prefix_so_far = "";
    }
    if (find_subnet_threshold(root->child[0], threshold_pkt, prefix_so_far+"0", ip_pfix_arr) == -1){
        //store the prefix found so far and pkt count
        ip_pfix_arr.push_back(prefix_so_far);
        subnet_prefix.push_back({ip_pfix_arr, root->pkt_count});
    }
    if (find_subnet_threshold(root->child[1], threshold_pkt, prefix_so_far+"1", ip_pfix_arr) == -1){
        //store the prefix found so far and pkt count
        ip_pfix_arr.push_back(prefix_so_far);
        subnet_prefix.push_back({ip_pfix_arr, root->pkt_count});
    }
    return 0;
}

void display_prefix_pktcount()
{
    for(int i=0; i<subnet_prefix.size(); i++){
        pair<vector<string>, long> p = subnet_prefix[i];
        vector<string> ip_str = p.first;
        vector<int> ip;
        for(int j=0; j<ip_str.size(); j++) {
            cout<<ip_str[j]<<" ";
        }
        cout<<" pkt: "<<p.second<<endl;
    }
}

int main(int argc, char *argv[])
{
    /*if(argc != 3){
        cout<<"Invalid arguments."<<endl;
        cout<<"Usage: <prog name> <cluster file> <threshold value>"<<endl;
        return 0;
    }
    string cluster_file = argv[1];
    int threshold = atoi(argv[2]);*/
    TrieNode root;
    string cluster_file = "config-clustering-small.txt";
    int T = 25;
    vector<string> ip_pfix_arr;
    process_cluster_file(cluster_file, &root);
    
    long threshold_pkt = total_num_packets * ((float)T/100);
    root.pkt_count = total_num_packets;
    cout<<"Total Packets: "<<total_num_packets<<" threshold: "<<threshold_pkt<<endl;
    find_subnet_threshold(&root, threshold_pkt, "", ip_pfix_arr);
    cout<<"Dsiplay: "<<subnet_prefix.size()<<endl;
    display_prefix_pktcount();
    return 0;
}