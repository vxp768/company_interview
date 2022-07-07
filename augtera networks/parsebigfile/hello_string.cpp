#include<iostream>
#include<thread>
#include<fstream>
#include<vector>
#include<unordered_map>
#include<map>
#include<queue>
#include <cstdlib>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

mutex print_mtx;
mutex freq_mtx;
mutex line_mtx;
unordered_map<string, long> word_freq_mp;
map<long, long> line_count_mp; //will store lines in sorted order

long num_lines=0;

class WorkerThread {
    public:
        int worker_id;
        bool done;
        mutex work_mtx;
        queue<long> work_line;
        queue<string> work_queue;
        unordered_map<string, long> worker_freq_map;
        unordered_map<long, long> worker_line_count_mp;
};



void thread_process_each_line(WorkerThread *WT)
{
    int num_words=0;
    string str;
    long line;
    int idx = 0;
    while(!WT->work_line.empty()){  
        num_words=0;
        idx=0;

        //WT->work_mtx.lock();

        /*if(WT->work_line.empty()){
            if (WT->done==false){
                WT->work_mtx.unlock();
                continue;
            } else {
                WT->work_mtx.unlock();
                break;
            }
        }*/
        str  = WT->work_queue.front();
        line = WT->work_line.front();
        WT->work_queue.pop();
        WT->work_line.pop();

        //WT->work_mtx.unlock();
        //Get rid of initial spaces in a line
        while(idx<str.size() && isspace(str[idx])){
            idx++;
        }

        string tmp = "";
        while (idx < str.size()) {
            if(isspace(str[idx])){
                /*freq_mtx.lock();
                word_freq_mp[tmp]++;
                freq_mtx.unlock();*/
                WT->worker_freq_map[tmp]++;
                num_words++;
                while(idx < str.size() && isspace(str[idx])){
                    idx++;
                }
                tmp = "";
                continue;
            }
            tmp += str[idx];
            idx++;
        }
        if(tmp.size()>0){
            /*freq_mtx.lock();
            word_freq_mp[tmp]++;
            freq_mtx.unlock();*/
            num_words++; //last word
            WT->worker_freq_map[tmp]++;
        }
        /*line_mtx.lock();
        line_count_mp[line] = num_words;
        line_mtx.unlock();*/
        WT->worker_line_count_mp[line] = num_words;
    }
}

int main(int argc, char *argv[])
{
    /*if(argc != 2){
        cout<<"Invalid arguments."<<endl;
        cout<<"Usage: <prog name> <input file>"<<endl;
        return 0;
    }*/

    auto start = high_resolution_clock::now();
    int numCPU = thread::hardware_concurrency() - 1; //1 for main thread
    vector<thread> worker_thread;
    //numCPU = 1;
    ofstream wordfreqfile("word_freq.txt");
    ofstream linewordsfile("linewords.txt");
    ifstream in_file("inputfile");

    cout<<"Number of CPUs available "<<numCPU<<endl;
    //array of threads.
    WorkerThread WT[numCPU]; 

    
    int round_robin_thread=0;
    string buf_str;
     
    size_t size;
    while(getline(in_file, buf_str)) {

        //WT[round_robin_thread].work_mtx.lock();
        WT[round_robin_thread].work_line.push(num_lines);
        WT[round_robin_thread].work_queue.emplace(buf_str);
        //WT[round_robin_thread].work_mtx.unlock();

        round_robin_thread = (round_robin_thread+1) % numCPU;
        num_lines++;
        if(num_lines%1000000==0){
            cout<<"Processed num of lines: "<<num_lines<<endl;
        }
    }
    for(int i=0; i<numCPU; i++) {
        WT[i].worker_id = i;
        //WT[i].done = false;
        worker_thread.push_back(thread(thread_process_each_line, &WT[i]));        
    }

    /* Signal all thread that no more data present to process*/
    /*for(int j=0; j<numCPU; j++){
        WT[j].work_mtx.lock();
        WT[j].done = true;
        WT[j].work_mtx.unlock();
    }*/
    cout<<"Num of lines: "<<num_lines<<endl;
    
    /* Wait for all threads to finish */
    for(int i=0; i<worker_thread.size(); i++){
        //thread is joinable even if the thread execution is done
        worker_thread[i].join();
    }
    cout<<"All worker thread processing done"<<endl;
    
    for(int i=0; i<numCPU; i++){
        unordered_map<string, long> mp = WT[i].worker_freq_map;

        for(auto it=WT[i].worker_freq_map.begin(); it!= WT[i].worker_freq_map.end(); it++){
            word_freq_mp[it->first] += it->second;
        }
        /*for(auto it=WT[i].worker_line_count_mp.begin(); it!= WT[i].worker_line_count_mp.end(); it++){
            line_count_mp[it->first] = it->second;
        }*/
    }
    cout<<"Writing to word freq file"<<endl;
    for(auto it=word_freq_mp.begin(); it!=word_freq_mp.end(); it++){
        wordfreqfile<<it->first<<" "<<it->second<<endl;
    }
    cout<<"Writing to line word count file"<<endl;
    /*for(auto it=line_count_mp.begin(); it!=line_count_mp.end(); it++){
        linewordsfile<<it->first<<" "<<it->second<<"\n";
    }*/
    
    auto stop = high_resolution_clock::now();
    duration<double> run_time_diff = duration_cast<duration<double>>(stop - start);
    cout << "Time taken by function: "
         << run_time_diff.count() << " seconds" << endl;

    return 0;
}
