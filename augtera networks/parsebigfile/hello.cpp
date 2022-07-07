#if 0
#include<iostream>
#include<thread>
#include<fstream>
#include<vector>
#include<unordered_map>
#include<queue>
#include <cstdlib>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;
size_t read_buffer_size = 1024 * 1024; //1 MB

class WorkerThread {
    public:
        int worker_id;
        bool done;
        mutex work_mtx;
        queue<long> work_line;
        queue<char *> work_queue;
};

mutex print_mtx;
mutex freq_mtx;
mutex line_mtx;
unordered_map<string, long> word_freq_mp;
unordered_map<long, long> line_count_mp;

long num_lines=0;

void thread_process_each_line(WorkerThread *WT)
{
    int num_words=0;
    char *buf;
    long line;
    while(1){  
        num_words=0;

        WT->work_mtx.lock();

        if(WT->work_line.empty()){
            if (WT->done==false){
                WT->work_mtx.unlock();
                continue;
            } else {
                WT->work_mtx.unlock();
                break;
            }
        }
        //cout<<"Q size for thd:"<<WT->worker_id<<" is:"<<WT->work_line.size()<<endl;
        
        buf  = WT->work_queue.front();
        line = WT->work_line.front();
        WT->work_queue.pop();
        WT->work_line.pop();

        WT->work_mtx.unlock();
        /*print_mtx.lock();
        cout<<"TID: "<<WT->worker_id<<" Line: "<<line<<" string "<<buf;
        print_mtx.unlock();*/
        
        char *s = buf;
        char *tmp;
        
        tmp = strtok(s," ,'\n'");
        while(tmp != NULL) {
            num_words++;
            string tk(tmp);
            freq_mtx.lock();
            word_freq_mp[tk]++;
            freq_mtx.unlock();
            tmp = strtok(NULL," ,'\n'");
        }
        line_mtx.lock();
        //cout<<"DOne wit line "<<line<<endl;
        line_count_mp[line] = num_words;
        line_mtx.unlock();
        delete buf;
    }
}

int main(int argc, char *argv[])
{
    auto start = high_resolution_clock::now();
    int numCPU = thread::hardware_concurrency();
    //numCPU = 1;
    ofstream wordfreqfile("word_freq.txt");
    FILE * pFile;
    vector<thread> worker_thread;

    pFile = fopen ("data.txt","r");
    if (pFile==NULL) {
       cout<<"Error opening file"<<endl;
       return 0;
    }

    cout<<"Number of CPUs available "<<numCPU<<endl;
    //array of threads. Main thread will share with one of worker thread
    WorkerThread WT[numCPU]; 

    for(int i=0; i<numCPU; i++) {
        WT[i].worker_id = i;
        WT[i].done = false;
        worker_thread.push_back(thread(thread_process_each_line, &WT[i]));        
    }

    int round_robin_thread=0;
    char *buf = new char[1000]; //assuming max line size is 1000
    size_t size;
    while(fgets(buf, size, pFile) !=NULL) {
        
        //this_thread::sleep_for(std::chrono::milliseconds(10));
        WT[round_robin_thread].work_mtx.lock();
        WT[round_robin_thread].work_line.push(num_lines);
        WT[round_robin_thread].work_queue.push(buf);
        WT[round_robin_thread].work_mtx.unlock();

        round_robin_thread = (round_robin_thread+1) % numCPU;
        num_lines++;
        buf = new char[1000];
    }
    delete[] buf;
    
    /* Signal all thread that no more data present to process*/
    for(int j=0; j<numCPU; j++){
        WT[j].work_mtx.lock();
        WT[j].done = true;
        WT[j].work_mtx.unlock();
    }
    cout<<"Num of lines: "<<num_lines<<endl;
    
    /* Wait for all threads to finish */
    for(int i=0; i<worker_thread.size(); i++){
        worker_thread[i].join();
    }
   
    for(auto it=word_freq_mp.begin(); it!=word_freq_mp.end(); it++){
        wordfreqfile<<it->first<<" "<<it->second<<endl;
    }
    fclose(pFile);
    
    auto stop = high_resolution_clock::now();
    duration<double> run_time_diff = duration_cast<duration<double>>(stop - start);
    cout << "Time taken by function: "
         << run_time_diff.count() << " seconds" << endl;

    return 0;
}
#endif