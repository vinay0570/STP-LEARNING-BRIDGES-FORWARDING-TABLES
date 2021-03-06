#include "utils.h"

int main() {
	pva();
	int flag;
	int bridge_count;
	cin>>flag;
	cin>>bridge_count;

	 map<int, bridge> BRIDGES;
	 map<char, lan> LANS;

	// consuming newline character
	string str;
	getline(cin, str);

	for(int i=0;i<bridge_count; i++) {
		string str;
		getline(cin, str);
		istringstream ss(str); 
		int lan_count = -1;
		vector<char> connection;
		do { 
			string word; 
			ss >> word; 
			if( lan_count >= 0) {
				char l = word[0];
				connection.push_back(l);
				auto it = LANS.find(l);
				if(it == LANS.end()) {
					vector<int> v;
					v.push_back(i+1);
					LANS.insert({l, lan(1,l,v)});

				} else {
					int count = it->second.bridge_count;
					vector<int> v = it->second.lan_connect; 
					v.push_back(i+1);
					LANS.erase(it);
					LANS.insert({l, lan(count + 1, l, v)});

				}
			}
			lan_count++;
		} while (ss); 
		BRIDGES.insert({i+1, bridge(lan_count - 1, i + 1, connection)});
	}                   
	//removing null character lan
	LANS.erase(LANS.begin());

	int assumed_roots[bridge_count + 1];
	int dist_from_root[bridge_count + 1];
	int sender_bridge[bridge_count + 1];
	int GLOBAL_TIME = 0;

	map<int, vector<trace> > pro_traces;
	vector<trace> tr;

	// Initial configuration messages from each bridge

	for(int i=1;i<=bridge_count; i++) {
		assumed_roots[i] = i;
		dist_from_root[i] = 0;
		sender_bridge[i] = i;
		bridge b = BRIDGES[i];
		for(int j=0;j<b.lan_count; j++) {
			tr.push_back(trace('s', i, b.bridge_connect[j], message(i,0,i)));
		}
	}
	pro_traces[GLOBAL_TIME] = tr;

	// Implementing STP

	while(true) {
		auto it = pro_traces.end(); it--;
		vector<trace> tr = it->second;
		vector<trace> new_tr;
		for(int i=0; i<tr.size(); i++) {
			message m = tr[i].msg;
			int at = tr[i].occurs_at;
			lan local = LANS[tr[i].port];
			if(tr[i].status == 's') {
				for(int j=0; j< local.bridge_count; j++) {
					if(local.lan_connect[j] != at) {
						new_tr.push_back(trace('r', local.lan_connect[j], local.id, m));
					}
				}
			} else {
				if(m.assumed_root < assumed_roots[at] || (m.assumed_root == assumed_roots[at] && m.distance + 1 < dist_from_root[at]) ||
					(m.assumed_root == assumed_roots[at] && m.distance + 1 == dist_from_root[at] && m.sender < sender_bridge[at])) 
				{
					
					assumed_roots[at] = m.assumed_root;
					dist_from_root[at] = m.distance + 1;
					sender_bridge[at] = m.sender;
					for(int j=0;j<BRIDGES[at].lan_count; j++) {
						if(local.id != BRIDGES[at].bridge_connect[j])
						new_tr.push_back(trace('s', at, BRIDGES[at].bridge_connect[j], message(assumed_roots[at],dist_from_root[at],at)));
					}
				}
			}
		}
		

		GLOBAL_TIME++;
		bool all_set = true;
		for(int i=1; i<=bridge_count; i++) {
			if(assumed_roots[i] != 1) {
				all_set = false;
				break;
			}
		}

		pro_traces[GLOBAL_TIME] = new_tr;
		if(all_set) {
			if(flag == 1) {
				cout<<"SNAPSHOTS AT EACH BRIDGE AT EACH UNIT TIME:-"<<endl;
				cout<<"NOTE: r refers to received and s refers to send \n";
				for(auto it = pro_traces.begin(); it != pro_traces.end(); it++) {
					int time = it->first;
					vector<trace> v = it->second;
					for(int i=0; i < v.size() ; i++) {
						cout<<"at time "<<time<<" ";
						trace t = v[i];
						cout<<": at bridge "<<t.occurs_at<<" message(B"<<t.msg.assumed_root<<", "<<t.msg.distance<<", B"<<t.msg.sender<<") was "<<t.status<<" through port "<<t.port<<endl;
					}
				}
			}
			break;
		} 
	}

	cout<<endl;

	// Finding a root port for each bridge

	for(int i=1;i<=bridge_count; i++) {
		bridge b = BRIDGES[i];
		char port;
		int distance = INT_MAX;
		int bridge_num = INT_MAX;
		for(int j = 0; j< b.lan_count; j++) {
			lan local = LANS[b.bridge_connect[j]];
			int distance_lan = INT_MAX;
			int bridge_local = INT_MAX;
			for(int k = 0; k < local.bridge_count ; k++) {
				//seeing that lan is connected to other bridge or not
				if(i != local.lan_connect[k]) 
				{
					distance_lan = min(distance_lan, dist_from_root[local.lan_connect[k]]);
					bridge_local = min(bridge_local, local.lan_connect[k]);
					// if other bridge is connected to that lan 
					// then we calculate distance from the local.lan_connect[k] (current bridge) as a root and minimize it.
					// if distances same then we go with the minimum indices of the bridges i.e., Bridge ID here bridge_local
				}
			}
			if(distance_lan < distance) {
				distance = distance_lan;
				port = local.id;
			} else if (distance_lan == distance && bridge_local < bridge_num) {
				bridge_num = bridge_local;
				port = local.id;
			}
		}
		// cout<<port<<" as assumed root port at bridge "<<i<<endl;
		b.port_type[port] = 'R';
		BRIDGES[i] = b;
	}

	// finding a designated bridge for each lan (or) assiging a designated port on that bridge for that lan
	// cout<<endl;

	for(auto it = LANS.begin(); it != LANS.end() ; it++) {
		lan local = it->second;
		int distance = INT_MAX;
		int bridge_num;
		for(int k = 0; k < local.bridge_count ; k++) {
			if(dist_from_root[local.lan_connect[k]] < distance) {
				distance = dist_from_root[local.lan_connect[k]];
				bridge_num = local.lan_connect[k];
			}
		}
		bridge b = BRIDGES[bridge_num];
		// cout<<"for B"<<bridge_num<<" : "<<it->first<<" is assumed destination port"<<endl;
		b.port_type[it->first] = 'D';
		BRIDGES[bridge_num] = b;
	}

	//Update BRIDGES, LANS from root ports, designated ports(i.e designated bridges), null ports(i.e blocked ports)
	cout<<"final status of LANS based on their connecting bridges:\n";
	LANS.erase(LANS.begin(), LANS.end());
	for(int i=1; i<= bridge_count ; i++) {
		cout<<"B"<<i<<": ";
		bridge b = BRIDGES[i];  
		vector<char> v;
		for(int j = 0; j < b.lan_count ; j++) {
			auto it = b.port_type.find(b.bridge_connect[j]);
			if(it == b.port_type.end()) {
				cout<<b.bridge_connect[j]<<"-NP ";
			} else {
				char l = b.bridge_connect[j];
				auto it = LANS.find(l);
				if(it == LANS.end()) {
					vector<int> v;
					v.push_back(i);
					LANS.insert({l, lan(1,l,v)});
				} else {
					int count = it->second.bridge_count;
					vector<int> v = it->second.lan_connect; 
					v.push_back(i);
					LANS.erase(it);
					LANS.insert({l, lan(count + 1, l, v)});
				}
				if(b.port_type[b.bridge_connect[j]] == 'R') {
					cout<<b.bridge_connect[j]<<"-RP ";
				} else {
					cout<<b.bridge_connect[j]<<"-DP ";
				}
				v.push_back(b.bridge_connect[j]);
			}
		}
		cout<<endl;
		b.lan_count = v.size();
		b.bridge_connect = v;
		BRIDGES[i] = b;
	}

	cout<<endl;

	// Taking the data of hosts on each lan

	map<int, char> host_lan;

	for(auto it= LANS.begin();it != LANS.end(); it++) {
		string str;
		getline(cin, str);
		istringstream ss(str); 
		bool host_input = false;
		lan local = it->second;
		char l = local.id;
		do { 
			string word; 
			ss >> word; 
			if(host_input && word.length() >= 2) {
				int num = 0;
				for(int j = 1; j < word.length(); j++) {
					num = num * 10 + (word[j] - '0');
				}
				local.hosts.push_back(num);
				host_lan[num] = l;
			}
			host_input = true;
		} while (ss);  
		LANS[l] = local;
	}
	

	// Taking the hosts pair which is in communication

	int transmits;
	cin>>transmits;
	getline(cin, str);
	while(transmits > 0) 
	{
		string str;
		getline(cin, str);
		int sender = -1;
		int reciever = -1;
		istringstream ss(str);
		do 
		{ 
			string word; 
			ss >> word; 
			if(word.length() >= 2)
			{
				int num = 0;
				for(int j = 1; j < word.length(); j++)
					num = num * 10 + (word[j] - '0');
				if(sender > 0)
					reciever = num;
				 else
					sender = num;
			}
		} while (ss);  

		cout<<"Forwarding tables at each bridge after each transmission:\n";
		char from_lan = host_lan[sender], to_lan = host_lan[reciever];
		int at_lans[26] = {0};
		int initial_flag = 0;
		vector< pair<char, int> > to_bridges; 
		while(initial_flag == 0 || to_bridges.size() > 0) {
			if(initial_flag == 0) 
			{
				initial_flag = 1;
				for(int i=0; i < LANS[from_lan].bridge_count ; i++) {
					to_bridges.push_back({from_lan, LANS[from_lan].lan_connect[i]});
				}
			} 
			else 
			{
				vector< pair<char, int> > to_new_bridges;
				for(int i=0; i < to_bridges.size() ; i++) {
					from_lan = to_bridges[i].first;
					bridge b = BRIDGES[to_bridges[i].second];
					map<int, char> forward = b.table;
					auto it = forward.find(reciever);
					forward[sender] = from_lan;
					b.table = forward;
					BRIDGES[to_bridges[i].second] = b;
					if(it == forward.end()) {                      // if the reciever host is NOT in forwarding table
						for(int j = 0; j < b.lan_count ; j++)
						{
							lan local = LANS[b.bridge_connect[j]];
							if(local.id != from_lan) {
								for(int k = 0; k < local.bridge_count ; k++) 
								{
									if(b.id != local.lan_connect[k]) {
										to_new_bridges.push_back({local.id, local.lan_connect[k]});
									}
								}
							}
						}
					} else {                                   // if the reciever host is in forwarding table
						for(int j = 0; j < b.lan_count ; j++) {
							lan local = LANS[b.bridge_connect[j]];
							if(local.id == it->second && local.id != from_lan) {
								for(int k = 0; k < local.bridge_count ; k++) {
									if(b.id != local.lan_connect[k]) {
										to_new_bridges.push_back({local.id, local.lan_connect[k]});
									}
								}
								break;
							}
						}
					}
				}
				to_bridges = to_new_bridges;
			}
		}

		for(int i=1; i<=bridge_count; i++){
			cout<<"at B"<<i<<":		HOST ID | FORWARDING PORT\n";
			map<int, char> forward = BRIDGES[i].table;
			for(auto it = forward.begin(); it != forward.end() ; it++) {
				cout<<"			H"<<it->first<<"		|		"<<it->second<<"\n";
			}
		}
		cout<<endl;
		transmits--;
	}
}
