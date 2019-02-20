#include "csma.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
#include <vector>
#include <unordered_map>
#include <time.h>

using namespace std;


//N is number of nodes, L is packet length, M is maximum collision count
int N, L, M;
long long int T;
vector<int> R;
unordered_map<int, Node*> nodes;

//used for output calculations
long int totalcollisions = 0;
long int totaltransmissions = 0;
long int channel_utilization = 0;
long int channel_idle = 0;
long int collision_time = 0;


//return 1 on success, 0 on failure
int readhelper(char wheretostore, int val){
	switch(wheretostore){
		case 'N':
			N = val;
			break;
		case 'L':
			L = val;
			break;
		case 'M':
			M = val;
			break;
		case 'T':
			T = val;
			break;
		case 'R':
			R.push_back(val);
			break;
		default:
			return 0;
	}
	return 1;
}

void readinput(string fname){
	ifstream infile(fname.c_str());
	string line;
	if(infile.is_open()){
		while(getline(infile, line)){
			char inputchar;
			sscanf(line.c_str(), "%c", &inputchar);
			line = line.substr(line.find(" ") + 1);
			stringstream ss;
			ss.str("");
			ss.clear();

			ss << line;
			string temp;
			int test;

			while(getline(ss, temp, ' ')){
				if(stringstream(temp) >>  test)
					test = stoi(temp); //i think this is redundant
					if(!readhelper(inputchar, test))
						fprintf(stderr, "input file has invalid format\n");
			}
		}
		infile.close();
	}
}

void create_nodes(){
	//srand (time(NULL));
	int i;
	for(i = 0; i < N; i++){
		if(nodes.find(i) == nodes.end()){
			Node *temp = new Node;
			temp->collision_count = 0;
			temp->backoff = rand()%(R[0]+1); //choose randomly between 0 and first R value initially
			temp->Rval = R[0];
			temp->transcount = 0;
			nodes[i] = temp;
		}
	}
}

/*
every node has its own backoff value calculated by randomly picking a number between 0 and R
in each iteration of the loop, nodes backoff values are decremented
but cant decrement until channel is checked, can use a global flag for this
once a node reaches 0, it can begin transmission by setting the channel flag
other nodes wont decrement in this iteration is the flag is set
once that node is done with its transmitting, it resets channel flag and gets new random backoff
next iteration both can countdown since flag not set
need to simulate collisions? and count how many packets are properly transmitted
	every time there is a collision need to do 2 things
	R is doubled for every collision unless next value is specified in input file
	1. increment global collision count by 1, this is the "total number of collisions"
	2. increment local collision count of each colliding node. at end of simulation
	the local collision counts are used to compute the variance in number of collisions across all nodes
need to be careful with orderering, dont decrement A if B is going to set flag in same iteration
*/
void decrement_all_backoffs(){
	for(auto it = nodes.begin(); it != nodes.end(); ++it){
		Node *gdbnode = it->second;
		int gdbnodekey = it->first;
		it->second->backoff--;
	}
}

//get new R value for node with key nodekey
void getnewR(int nodekey){
	bool found = false;
	int i;
	Node *gdbnode = nodes.find(nodekey)->second;
	for(i = 0; i < R.size(); i++){
		if(R[i] == nodes.find(nodekey)->second->Rval && R[i] != R.back()){
			found = true;
			break;
		}
	}
	//Node *gdbnode = nodes.find(nodekey)->second; //gdb cant look at nodes in map directly
	if(found)
		nodes.find(nodekey)->second->Rval = R[i+1]; //need to verify this works
	else{
		nodes.find(nodekey)->second->Rval *= 2; //double if no specified increase for R
	}
}

void getnewBackoff(int nodekey){
	Node *gdbnode = nodes.find(nodekey)->second;
	nodes.find(nodekey)->second->backoff = rand()%(nodes.find(nodekey)->second->Rval+1);
}


void runsimulation(){
	srand (time(NULL));
	bool channel_used = false; //when a node is transmitting, this flag will be set to its id
	vector<int> collisions;
	int cur_transmitting_node;
	Node *gdbnode; //used to view current node being accessed in gdb
	int gdbnodekey;

	for(long long int i = 0; i < T; i++){
		//check if node is going to set flag
		for(auto it = nodes.begin(); it != nodes.end(); ++it){
			gdbnode = it->second;
			gdbnodekey = it->first;
			if(it->second->backoff == 0)
				collisions.push_back(it->first); //check each nodes backoff value, push its key onto collision vector if its 0
		}
		//if there are multiple nodes in collision vector then we have a collision
		//if only 1 node in vector then we have a transmission
		//if no nodes in vector then we can decrement all nodes backoff values

		switch(collisions.size()){

			//decrement all backoff values
			case 0:
				decrement_all_backoffs();
				channel_idle++;
				break;

			//there is a transmission
			case 1:

				//edge case where don't get to fully transmit so dont count transaction but still need to 
				//increase transmission time
				 if(L > T - i){
				 	channel_utilization += T - i;
				 	i+= T - i;
				 	break;
				 }


				//set channel
				channel_used = true;
				cur_transmitting_node = collisions[0];
				//for gdb tracking
				gdbnode = nodes.find(cur_transmitting_node)->second;
				gdbnodekey = nodes.find(cur_transmitting_node)->first;

				nodes.find(cur_transmitting_node)->second->transcount++;
				i += L - 1; //transmit the full packet, need the -1 since i gets incremented in for loop
				nodes.find(cur_transmitting_node)->second->Rval = R[0];
				getnewBackoff(cur_transmitting_node);
				nodes.find(cur_transmitting_node)->second->collision_count = 0;
				cur_transmitting_node = -1;
				channel_used = false;
				totaltransmissions++;
				channel_utilization += L; //used for full transmission
				break;

			//collision case since size > 1
			default:

				//edge case at end of clock time
				 if(L > T - i){
				 	collision_time += T - i;
				 	i += T - i;
				 	break; //not sure if i want this
				 }


				//increment local node collision counts
				for(int j = 0; j < collisions.size(); j++){
					//these are so i can check correct node in gdb
					gdbnode = nodes.find(collisions[j])->second;
					gdbnodekey = nodes.find(collisions[j])->first;

					nodes.find(collisions[j])->second->collision_count++;
					//check if this should be > or >=
					if(nodes.find(collisions[j])->second->collision_count >= M){
						nodes.find(collisions[j])->second->Rval = R[0];
						nodes.find(collisions[j])->second->collision_count = 0;
					}
					else{
						//need to increment R (double if not specified)
						getnewR(collisions[j]);
					}
					
					getnewBackoff(collisions[j]);
				}
				//increment global collision count
				totalcollisions++;
				collision_time += L; //think this is right?
				i += L - 1; //packet transmission wasted
				break;

		}
		collisions.clear(); //gets renewed every time anyways
	}
	//printf("Total transmissions: %ld\n", totaltransmissions);
	//printf("Total collisions: %ld\n", totalcollisions);
}

//need variance in number of successful transmissions across all nodes
//and variance in number of collisions across all nodes
double calc_variance(vector<int> &data){
	double sum = 0.0;
	double mean;
	for(int i = 0; i < data.size(); i++){
		sum += (double) data[i];
	}
	mean = sum / (double) data.size();
	//printf("Mean is: %f\n", mean);
	double temp = 0;
	for(int i = 0; i < data.size(); i++){
		temp += (mean - (double) data[i]) * (mean - (double) data[i]);
	}
	return temp / (double) data.size();
}

void test_variance(){
	ifstream infile;
	vector<int> data;
	infile.open("transmission.txt");
	int temp;
	while(infile >> temp){
		data.push_back(temp);
	}
	double tranvar = calc_variance(data);
	printf("Variance of numbers in transmissions.txt is: %f\n", tranvar);
	infile.close();
}

void printNodes(){
	ofstream nodefile;
	nodefile.open("nodes.txt");
	for(auto it = nodes.begin(); it != nodes.end(); ++it){
		nodefile << "Node key: " << it->first << "\n";
		nodefile << "Collision count: " << it->second->collision_count << "\n";
		nodefile << "Transmission count: " << it->second->transcount << "\n\n";
	}
	nodefile.close();
}

void printData(vector<int> &data, string fname){
	ofstream datafile;
	datafile.open(fname);
	for(int i = 0; i < data.size(); i++){
		datafile << data[i] << "\n";
	}
	datafile.close();
}

void getResults(){

	//calculate variances
	double transmission_var, collision_var;
	vector<int> trandata, coll_data;
	for(auto it = nodes.begin(); it != nodes.end(); ++it){
		trandata.push_back(it->second->transcount);
		coll_data.push_back(it->second->collision_count);
	}
	transmission_var = calc_variance(trandata);
	collision_var = calc_variance(coll_data);


	ofstream outfile;
	outfile.open("output.txt");
	//printf("Channel Used: %ld\n", channel_utilization);
	//printf("Channel Idle: %ld\n", channel_idle);
	//printf("Channel collision time: %ld\n", collision_time);
	double collis_perc = ((double) collision_time / (double) T)  * 100.0;
	//printf("Channel collision fraction: %f\n", collis_perc);
	outfile << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";
	outfile << "Channel idle fraction: " << ((double) channel_idle / (double) T) * 100.0 << "%\n";
	outfile << "Total number of collisions: " << totalcollisions << "\n";
	outfile << "Variance in number of successful transmissions: " << transmission_var << "\n";
	outfile << "Variance in number of collisions: " << collision_var << "\n";
	outfile.close();

	//debugging functions to check calculations
	printNodes();
	//printData(trandata, "transmissions.txt");
	//printData(coll_data, "collisions.txt");
}

void resetRerun(){
	R.clear();
	nodes.clear();
}

void printGraphData()
{
	int i;
	const string plota = "plota.txt";
	const string plotb = "plotb.txt";
	const string plotc = "plotc.txt";

	const string plotd1 = "plotd1.txt";
	const string plotd2 = "plotd2.txt";
	const string plotd3 = "plotd3.txt";
	const string plotd4 = "plotd4.txt";
	const string plotd5 = "plotd5.txt";

	const string plote1 = "plote1.txt";
	const string plote2 = "plote2.txt";
	const string plote3 = "plote3.txt";
	const string plote4 = "plote4.txt";
	const string plote5 = "plote5.txt";

	ofstream f1(plota);
	ofstream f2(plotb);
	ofstream f3(plotc);

	ofstream f4(plotd1);
	ofstream f5(plotd2);
	ofstream f6(plotd3);
	ofstream f7(plotd4);
	ofstream f8(plotd5);

	ofstream f9(plote1);
	ofstream f10(plote2);
	ofstream f11(plote3);
	ofstream f12(plote4);
	ofstream f13(plote5);

	//resetRerun();

	//calculate f1, f2, f3
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 20;
		R.push_back(8);
		R.push_back(16);
		R.push_back(32);
		R.push_back(64);
		R.push_back(128);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f1 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";
		f2 << "Channel idle fraction: " << ((double) channel_idle / (double) T) * 100.0 << "%\n";
		f3 << "Total number of collisions: " << totalcollisions << "\n";
	}

	//resetRerun();
	//calculate f4
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 20;
		R.push_back(1);
		R.push_back(2);
		R.push_back(4);
		R.push_back(8);
		R.push_back(16);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f4 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";
	}

	//resetRerun();
	//calculate f5
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 20;
		R.push_back(2);
		R.push_back(4);
		R.push_back(8);
		R.push_back(16);
		R.push_back(32);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f5 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";
	}

	//resetRerun();
	//calculate f6
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 20;
		R.push_back(4);
		R.push_back(8);
		R.push_back(16);
		R.push_back(32);
		R.push_back(64);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f6 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";
	}

	//resetRerun();
	//calculate f7
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 20;
		R.push_back(8);
		R.push_back(16);
		R.push_back(32);
		R.push_back(64);
		R.push_back(128);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f7 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";
	}

	//resetRerun();
	//calculate f8
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 20;
		R.push_back(16);
		R.push_back(32);
		R.push_back(64);
		R.push_back(128);
		R.push_back(256);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f8 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";
	}

	//resetRerun();

	//calculate f9
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 20;
		R.push_back(8);
		R.push_back(16);
		R.push_back(32);
		R.push_back(64);
		R.push_back(128);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f9 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";
		
	}

	//resetRerun();
	//calculate f10
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 40;
		R.push_back(8);
		R.push_back(16);
		R.push_back(32);
		R.push_back(64);
		R.push_back(128);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f10 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";
	}

	//resetRerun();
	//calculate f11
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 60;
		R.push_back(8);
		R.push_back(16);
		R.push_back(32);
		R.push_back(64);
		R.push_back(128);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f11 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";	
	}
	//resetRerun();
	//calculate f12
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 80;
		R.push_back(8);
		R.push_back(16);
		R.push_back(32);
		R.push_back(64);
		R.push_back(128);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f12 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";	
	}
	//resetRerun();
	//calculate f13
	for(i = 5; i <= 500; i+=5)
	{
		resetRerun();
		N = i;
		L = 100;
		R.push_back(8);
		R.push_back(16);
		R.push_back(32);
		R.push_back(64);
		R.push_back(128);
		M = 6;
		T = 50000;
		//reset the values!
		channel_utilization = 0;
		channel_idle = 0;
		totalcollisions = 0;
		create_nodes();
		runsimulation();
		f13 << "Channel utilization: " << ((double) channel_utilization / (double) T) * 100.0 << "%\n";	
	}
	resetRerun();
	



	f1.close();
	f2.close();
	f3.close();

	f4.close();
	f5.close();
	f6.close();
	f7.close();
	f8.close();

	f9.close();
	f10.close();
	f11.close();
	f12.close();
	f13.close();

	return;
}

int main(int argc, char** argv){
	
	string inputfname;

	if(argc != 2){
		fprintf(stderr, "usage: %s inputfile\n", argv[0]);
		exit(1);
	}
	inputfname = argv[1];
	srand (time(NULL));
	readinput(inputfname);
	create_nodes();
	runsimulation();
	getResults();
	printGraphData();
}
