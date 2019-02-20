#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <queue>
#include <vector>
#include <map>
#include <unordered_map>
#include <time.h>

using namespace std;

//---------------------Node Structure----------------------
struct Node{
    int Rvalue;
    int random;
    int collision_count;
    int collision_count_temp;
    int transmission_count;   
};

//-------------------- Global Parameters-------------------
int N,L,M;
long int T;
vector <int> R;
map<int,Node*> node_map;

long int total_transmission=0;
long int total_collision=0;
long int idle_time=0;

//------------------------Read File------------------------
int readline(char flg, int value){
    switch (flg){
        case 'N': N=value; break;
        case 'L': L=value; break;
        case 'M': M=value; break;
        case 'T': T=value; break;
        case 'R': R.push_back(value); break;
        default: return 0;
    }
    return 1;
}

int readfile(string filename){
    ifstream file(filename.c_str());
    string line;
    if(file.is_open()){
        while(getline(file,line)){
            char flg;
            sscanf(line.c_str(), "%c", &flg);
            line = line.substr(line.find(" ") + 1);
            stringstream ss;
            ss.str("");
			ss.clear();
            ss << line;
            string temp;
            int value;
            while(getline(ss, temp, ' ')){
				stringstream(temp) >> value;
				if(!readline(flg, value))
				fprintf(stderr, "input file has invalid format\n");
			}
        }
        file.close();
    }
}

//---------------------Initialization----------------------
void set_node(){
    int i;
    for(i=0;i<N;i++){
        Node *nodei=new Node;
        nodei->collision_count=0;
        nodei->transmission_count=0;
        nodei->Rvalue=R[0];
        nodei->random=rand()%(R[0]+1);
        nodei->collision_count_temp=0;
        node_map[i]=nodei;
    }
}

//------------------------Count Down-------------------------
void countdown(){
    for(auto i=node_map.begin();i!=node_map.end();i++){
        i->second->random--;
    }
}

//-----------------------Change R Value-----------------------
void change_Rvalue(int node_num){
    int i;
    int j=-1;
    for(i=0;i<(R.size()-1);i++){
        if(node_map[node_num]->Rvalue==R[i]){
            j=i;
        }
    }
    if((j>=0)&&(R.size()>2)){
        node_map[node_num]->Rvalue=R[j+1];
    }
    else{
        node_map[node_num]->Rvalue*=2;
    }
}

//--------------------Change Random Value----------------------
void change_Random(int node_num){
     node_map[node_num]->random=rand()%(node_map[node_num]->Rvalue+1);
}

//--------------------Start Simulation-------------------------
void simulation(){
    srand(time(NULL));
    int t=0;
    // int flag==0;
    vector<int>transmitting_node;
    int current_transmitting_node;

    while(t<T){
        // cout<<t;
        // cout<<"\n";
        for(auto i=node_map.begin();i!=node_map.end();i++){
            if(i->second->random==0)
            transmitting_node.push_back(i->first); 
        }
        if(transmitting_node.size()==0){
            idle_time++;
            // cout<<"idle_time: "<<idle_time;
            // cout<<"\n";
            // cout<<"time\n"<<t<<"\n"<<"IDLE nodesize "<<transmitting_node.size()<<"\n\n";
            // cout<<"------------------------------------------------";
            // for(auto i=node_map.begin();i!=node_map.end();i++){
            //     cout<<i->second->random<<"\n";
            // }
            // cout<<"************************************************";
            countdown();
            // for(auto i=node_map.begin();i!=node_map.end();i++){
            //     cout<<i->second->random<<"\n";
            // }
            // cout<<"\n\n";
            t++;
            transmitting_node.clear();
            continue;
        }
        if(transmitting_node.size()==1){
            total_transmission++;
            current_transmitting_node=transmitting_node[0];
            node_map[current_transmitting_node]->transmission_count++;
            // node_map[current_transmitting_node]->Rvalue=R[0];
            // cout<<"time\n"<<t<<"\n"<<"Transmitting  "<<"nodesize "<<transmitting_node.size()<<"\n"<<current_transmitting_node<<"\n\n";
            // cout<<"total_transmission: "<<total_transmission<<"------------------------\n";
            // cout<<"current transmitting node:   "<<current_transmitting_node<<"\n\n";
            // cout<<"\n\n";
            // cout<<"random number: "<<node_map[current_transmitting_node]->random;
            node_map[current_transmitting_node]->random=rand()%
                                                (node_map[current_transmitting_node]->Rvalue+1);
            // cout<<"random number: "<<node_map[current_transmitting_node]->random;                                    
            t+=L;
            transmitting_node.clear();
            continue;
        }
        if(transmitting_node.size()>1){
            total_collision++;
            // cout<<"total_collision: "<<total_collision<<"\n";
            // cout<<"\n\n";
            // cout<<"\n\n"<<"time\n"<<t<<"\n"<<"Collision  "<<"nodesize "<<transmitting_node.size()<<"\n";
            t+=L;
            for(int j=0;j<transmitting_node.size();j++){
                current_transmitting_node=transmitting_node[j];
                // cout<<current_transmitting_node<<"\n";
                // cout<<"current transmitting node:   "<<current_transmitting_node<<"\n\n";
                node_map[current_transmitting_node]->collision_count++;
                node_map[current_transmitting_node]->collision_count_temp++;
                // cout<<"\n\n";
                // cout<<" collision_count: "<<node_map[current_transmitting_node]->collision_count;
                // cout<<" collision_count_temp: "<<node_map[current_transmitting_node]->collision_count_temp<<"\n";
                // cout<<"Current node  "<<current_transmitting_node<<"\n";
                // cout<<" R VALUE "<<node_map[current_transmitting_node]->Rvalue<<"\n";
                if(node_map[current_transmitting_node]->collision_count_temp>M){
                    node_map[current_transmitting_node]->collision_count_temp=0;
                    node_map[current_transmitting_node]->Rvalue=R[0];
                }
                else{
                    change_Rvalue(current_transmitting_node);
                }
                // cout<<" R VALUE "<<node_map[current_transmitting_node]->Rvalue<<"\n";
                // cout<<" Random Value "<<node_map[current_transmitting_node]->random<<"\n";
                change_Random(current_transmitting_node);
                // cout<<" Random Value "<<node_map[current_transmitting_node]->random<<"\n";
            }
            transmitting_node.clear();
            continue;
        }
    }
}

//----------------------Compute Variance----------------------
double trans_variance(){
    double sum=0.0;
    double mean=0.0;
    double variance=0.0;
    for(auto i=node_map.begin();i!=node_map.end();i++){
        sum+=i->second->transmission_count;
    }
    mean=sum/N;
    for(auto i=node_map.begin();i!=node_map.end();i++){
        variance+=(i->second->transmission_count-mean)*(i->second->transmission_count-mean);
    }
    return (variance/N);
}

double collision_variance(){
    double sum=0.0;
    double mean=0.0;
    double variance=0.0;
    for(auto i=node_map.begin();i!=node_map.end();i++){
        sum+=i->second->collision_count;
    }
    mean=sum/N;
    for(auto i=node_map.begin();i!=node_map.end();i++){
        variance+=(i->second->collision_count-mean)*(i->second->collision_count-mean);
    }
    return (variance/N);
}

//-----------------------Output File-------------------------
void printNodes(){
	ofstream nodefile;
	nodefile.open("nodes.txt");
	for(auto i = node_map.begin(); i!= node_map.end(); i++){
		nodefile << "Node key: " << i->first << "\n";
		nodefile << "Collision count: " << i->second->collision_count << "\n";
		nodefile << "Transmission count: " << i->second->transmission_count << "\n\n";
	}
	nodefile.close();
}

void output(){
    double transmission_var, collision_var;
    double channel_utilization, idle_fraction;
    transmission_var = trans_variance();
    collision_var = collision_variance();
    channel_utilization = (double) total_transmission*L/T;
    idle_fraction = (double) idle_time/T;

    ofstream outfile;
    outfile.open("output.txt");
    outfile << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
	outfile << "Channel idle fraction: " << idle_fraction * 100.0 << "%\n";
	outfile << "Total number of collisions: " << total_collision << "\n";
	outfile << "Variance in number of successful transmissions: " << transmission_var << "\n";
	outfile << "Variance in number of collisions: " << collision_var << "\n";
	outfile.close();
    printNodes();
}

//------------------------Plot--------------------------
void plotabc(){
    ofstream fa("plota.txt");    //Channel utilization
    ofstream fb("plotb.txt");    //Channel idle fraction
    ofstream fc("plotc.txt");    //Total Collision
    double channel_utilization, idle_fraction;
    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=20;
        R.push_back(8);
        R.push_back(16);
        R.push_back(32);
        R.push_back(64);
        R.push_back(128);
        R.push_back(256);
        R.push_back(512);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fa << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
	    fb << "Channel idle fraction: " << idle_fraction * 100.0 << "%\n";
	    fc << "Total number of collisions: " << total_collision << "\n";
    }
    fa.close();
	fb.close();
	fc.close();
}

void plotd(){
    ofstream fd1("plotd1.txt");    //Channel utilization
    ofstream fd2("plotd2.txt");    //Channel utilization
    ofstream fd3("plotd3.txt");    //Channel utilization
    ofstream fd4("plotd4.txt");    //Channel utilization
    ofstream fd5("plotd5.txt");    //Channel utilization
    double channel_utilization, idle_fraction;
    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=20;
        R.push_back(1);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fd1 << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
    }

    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=20;
        R.push_back(2);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fd2 << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
    }

    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=20;
        R.push_back(4);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fd3 << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
    }

    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=20;
        R.push_back(8);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fd4 << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
    }

    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=20;
        R.push_back(16);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fd5 << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
    }
    fd1.close();
	fd2.close();
	fd3.close();
	fd4.close();
	fd5.close();
}

void plote(){
    ofstream fe1("plote1.txt");    //Channel utilization
    ofstream fe2("plote2.txt");    //Channel utilization
    ofstream fe3("plote3.txt");    //Channel utilization
    ofstream fe4("plote4.txt");    //Channel utilization
    ofstream fe5("plote5.txt");    //Channel utilization
    double channel_utilization, idle_fraction;
    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=20;
        R.push_back(8);
        R.push_back(16);
        R.push_back(32);
        R.push_back(64);
        R.push_back(128);
        R.push_back(256);
        R.push_back(512);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fe1 << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
    }

    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=40;
        R.push_back(8);
        R.push_back(16);
        R.push_back(32);
        R.push_back(64);
        R.push_back(128);
        R.push_back(256);
        R.push_back(512);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fe2 << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
    }

    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=60;
        R.push_back(8);
        R.push_back(16);
        R.push_back(32);
        R.push_back(64);
        R.push_back(128);
        R.push_back(256);
        R.push_back(512);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fe3 << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
    }

    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=80;
        R.push_back(8);
        R.push_back(16);
        R.push_back(32);
        R.push_back(64);
        R.push_back(128);
        R.push_back(256);
        R.push_back(512);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fe4 << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
    }

    for (int i=5;i<500;i+=5){
        R.clear();
	    node_map.erase(node_map.begin(),node_map.end());
        N=i;
        L=100;
        R.push_back(8);
        R.push_back(16);
        R.push_back(32);
        R.push_back(64);
        R.push_back(128);
        R.push_back(256);
        R.push_back(512);
        M=6;
        T=50000;
        total_transmission=0;
        total_collision=0;
        idle_time=0;
        set_node();
        simulation();
        channel_utilization = (double) total_transmission*L/T;
        idle_fraction = (double) idle_time/T;
        fe5 << "Channel utilization: " <<  channel_utilization * 100.0 << "%\n";
    }
    fe1.close();
	fe2.close();
	fe3.close();
	fe4.close();
	fe5.close();
}

void plot(){
    plotabc();
    plotd();
    plote();
}

//---------------------Main function--------------------
int main(int argc, char** argv){
	string inputfname;
	if(argc != 2){
		fprintf(stderr, "usage: %s inputfile\n", argv[0]);
		exit(1);
	}
	inputfname = argv[1];
	srand (time(NULL));
	readfile(inputfname);
	set_node();
    // cout<<"R size "<<R.size()<<"\n";
    // cout<<"R value "<<R[0]<<"\n";
    // cout<<"R value "<<R[1]<<"\n";
    // cout<<"R value "<<R[2]<<"\n";
    // cout<<"R value "<<R[3]<<"\n";
    // cout<<"N size "<<node_map.size()<<"\n";
	simulation();
	output();
	plot();
}

