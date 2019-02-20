#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <queue>

using namespace std;

//----------------------------structure definition--------------------------------
struct Dijkstra_Node{
	int id;
	unordered_map<int,int> neighbour; //key is the node ID, the value is the cost to each node.
}; 

struct forward_entry{                  //forward table sturcture
	int destination;
	int nexthop;
	int pathcost;	
}; 

struct message_entry{                 //output message structure
	int source;
	int destination;
	string message;
};

//----------------------------global parameters-----------------------------------
ofstream outfile;                     //write outputfile 
string topofile, messagefile, changefile;  
map<int,Dijkstra_Node*> topology_map;
queue<message_entry*> message_queue;

//----------------------------min function----------------------------------------
int min(int a, int b){
	if(a>b)
	return b;
	else
	return a;
}

//----------------------------print forward table---------------------------------
void Print_forwardtable(vector<forward_entry> &forwardtable){
	for(int i=0;i<forwardtable.size();i++){
		outfile <<forwardtable[i].destination;
		outfile <<" ";
		outfile <<forwardtable[i].nexthop;
		outfile <<" ";
		outfile <<forwardtable[i].pathcost;
		outfile <<"\n";
	}
}

//----------------------------get topology---------------------------------------
void get_topology(string filename){
	ifstream in(filename);
	int a,b,c;
	while(in >> a >> b >> c){
		if(topology_map.find(a)==topology_map.end()){
			Dijkstra_Node *source = new Dijkstra_Node;
			topology_map[a]= source;
			topology_map[a]->id=a;
		}
		if(topology_map.find(b)==topology_map.end()){
			Dijkstra_Node *destination = new Dijkstra_Node;
			topology_map[b]= destination;
			topology_map[b]->id=b;
		}
		topology_map[a]->neighbour[b]=c;         //set the cost between node a and node b
		topology_map[b]->neighbour[a]=c;
	}
	in.close();
}

//----------------------------Dijkstra algorithm-------------------------------------
void Dijkstra(int sourceid, vector<forward_entry> &forwardtable){
	 unordered_map<int, bool> Mark;
	 unordered_map<int, int> Distance;
	 unordered_map<int, int> P;      //the previous point number
	 Mark[sourceid]=1;
	 for( auto i = topology_map[sourceid]->neighbour.begin();i!=topology_map[sourceid]->neighbour.end();i++){
	 	Distance[i->first] = i->second;
	 }
	 if(Distance.size()==0){                   //no neighbours of this node
	 	for(auto i = topology_map.begin();i!= topology_map.end();i++){
	 		forward_entry *entry = new forward_entry;
	 		entry->pathcost=-999;              //unreachable
	 		entry->destination=i->first;
	 		if(i->first==sourceid){
	 			entry->pathcost=0;
	 			entry->nexthop=i->first;
			 }
			 else{
			 	entry->nexthop=-1;      //unreachable
			 }
			 forwardtable.push_back(*entry);
		 }
	 }
	 else{                                    // at least one neighbour
	 	while(Mark.size()<topology_map.size()){
	 		int min_id;
	 		int min_value=10000;
	 		for(auto i = Distance.begin();i!=Distance.end();i++){
	 			if(i->second<=min_value&&Mark.find(i->first)==Mark.end()){
	 				if(i->second<min_value){
	 					min_id=i->first;
	 					min_value=i->second;                   //choose the smallest cost node to the neighbours
					 }
					 else{
					 	min_id=min(min_id,i->first);           //choose the smallest id number
					 }
				 }
			 }
			if(min_value==10000){            //unreachable node
			  for(auto i = topology_map.begin();i!=topology_map.end();i++){
				if(Mark.find(i->first)==Mark.end()){
					Mark[i->first]=1;
					Distance[i->first]=-999;
					P[i->first]=-1;
				}
		      }
			}
			else{    
				Mark[min_id]=true;          //the node we choose has been the smallest cost route,so set the mark to true
				for(auto i = topology_map[min_id]->neighbour.begin(); i!=topology_map[min_id]->neighbour.end();i++){    //for the min cost node we choose, find its neighbours 
					if(Mark.find(i->first)==Mark.end()&&Distance.find(i->first)!=Distance.end()){    //  the  neighbour  has not been put into mark(not the shortest route) and are neighbours to previous nodes.
						if(Distance[i->first]>Distance[min_id]+i->second){                                    //previous distance is bigger than the distance to the new node plus the cost of one of its neighbour.
							Distance[i->first]=Distance[min_id]+i->second;
							P[i->first]=min_id;                                                             // the previsous node to it->first should be set min_id
						}
						else if(Distance[i->first]==Distance[min_id]+i->second){
							P[i->first]=min(min_id,P[i->first]);
						}
					}
					else if(Mark.find(i->first)==Mark.end()){
						Distance[i->first]=Distance[min_id]+i->second;
						P[i->first]=min_id;   
					}
				}
			}
		}
   }
   //turn path to forward table
   for(auto i = topology_map.begin(); i!=topology_map.end();i++){
       forward_entry *entry= new forward_entry;
       entry->destination=i->first;
       entry->pathcost=Distance[i->first];
       if(i->first==sourceid){
    	entry->pathcost=0;
    	entry->nexthop=sourceid;
	   }
	   else{
	   	    int previous_node=P[i->first];
	   	    if(previous_node=sourceid){
	   		   entry->nexthop=i->first;
		    }
	        else{
	        	while(previous_node!=sourceid){
	        		if(previous_node!=-1){
	        			if(P[previous_node]==sourceid){
	        				entry->nexthop=previous_node;
						}
						else{
							previous_node=P[previous_node];
						}
					}
					else{
						entry->nexthop=-1;
						break;
					}
				}
		    }
	   }
	   forwardtable.push_back(*entry);
   }
}

//----------------------------get message------------------------------------- 
void get_msg(string filename){
	ifstream file(filename);
	string line,message;
	if(file.is_open()){
		while(getline(file,line)){
			int source;
			int destination;
			sscanf(line.c_str(),"%d %d %*s",&source,&destination);
			message = line.substr(line.find(" ")); //want message to start after second space so do twice
			message = message.substr(line.find(" ") + 1); //copy from position 4 to end of line to only get the message
			message_entry *msg = new message_entry;
			msg->source = source;
			msg->destination = destination;
			msg->message = message;
			message_queue.push(msg);
		}
	file.close();	
	}
}

//----------------------------send message------------------------------------- 
void send_msg(unordered_map<int,vector<forward_entry>> &forward_table){
	while(message_queue.size()>0){
		int source, destination;
		source=message_queue.front()->source;
		destination=message_queue.front()->destination;
//		message=message_queue.front()->message;
		outfile << "from " << source << " to " << destination;
		queue<int>path;
		path.push(source);
		int vectorindex=0;
		vector<forward_entry> current_table= forward_table[source];
		int isreachable=0;
		while(current_table[vectorindex].destination!=destination&&vectorindex<current_table.size()){
			vectorindex++;
		}
		if(current_table[vectorindex].destination==destination&&current_table[vectorindex].pathcost!=-1){
			isreachable=1;
		}
		if(isreachable){
			int nexthop = current_table[vectorindex].nexthop;
			int pathcost=current_table[vectorindex].pathcost;
			if(nexthop != destination){
				path.push(nexthop);
			}
			while(nexthop!=destination){
				current_table=forward_table[nexthop];
				nexthop=current_table[vectorindex].nexthop;
				if(nexthop!=destination){
					path.push(nexthop);
				}
			}
			outfile <<" cost " <<pathcost <<" hops ";
			while(path.size() > 0){
				outfile << path.front() << " ";
				path.pop();
			}
			outfile << "message" << message_queue.front()->message << "\n";
			message_queue.pop();
		}
		else{
			outfile << "cost infinite, hops unreachable, message" <<message_queue.front()->message << "\n";
			message_queue.pop();
		}
	}
	outfile << "\n";
}

//----------------------------do changes------------------------------------- 
void do_changes(string changefname, string msgfname){
	ifstream changefile(changefname);
	int source, destination, cost; 
	while(changefile >> source >> destination >> cost){
		//check for existing link b/w the nodes in file
		if(topology_map[source]->neighbour.find(destination) != topology_map[source]->neighbour.end()){
			//link costs can only be positive, never 0. -999 means remove link
			if(cost > 0){
				//update path cost
				topology_map[source]->neighbour[destination] = cost;
				//have to do for destinations neighbor too
				topology_map[destination]->neighbour[source] = cost;
			}
			//remove link if cost is -999
			else if(cost == -999){
				topology_map[source]->neighbour.erase(destination); //no longer neighbors
				topology_map[destination]->neighbour.erase(source);
			}
		}
		//no existing link exists, create one
		else{
			//if no link exists and cost value is 0 or -999 then ignore it
			if(cost > 0){
				topology_map[source]->neighbour[destination] = cost;
				topology_map[destination]->neighbour[source] = cost;
			}
		}
		//now need to get new routing tables
		unordered_map<int, vector<forward_entry>> forward_table;
		for(auto it = topology_map.begin(); it != topology_map.end(); ++it){
			vector<forward_entry> table;
			Dijkstra(it->first, table);
			Print_forwardtable(table);
			outfile << "\n";
			forward_table[it->first] = table;
		}
		get_msg(msgfname);
		send_msg(forward_table);
	}
	changefile.close();
}

//----------------------------main func------------------------------------- 
int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    outfile.open("output.txt");
    if (argc != 4) {
        printf("Usage: ./linkstate topofile messagefile changesfile\n");
        return -1;
    }
    topofile=argv[1];
    messagefile=argv[2];
    changefile=argv[3];
    get_topology(topofile);
    unordered_map<int, vector<forward_entry>> forward_table;
    for(auto it = topology_map.begin(); it != topology_map.end(); ++it){
		vector<forward_entry> table;
		Dijkstra(it->first, table);
		Print_forwardtable(table);
		outfile << "\n";
		forward_table[it->first] = table;
	}
	get_msg(messagefile);
	send_msg(forward_table);
	do_changes(changefile, messagefile);
	outfile.close();
}

