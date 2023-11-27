#include<bits/stdc++.h>
#include<fstream>
using namespace std;
ifstream in; ofstream out;
void work(){
    in.open("input.txt");
    out.open("output.txt");
}
const string cdn_ip_pref ="1.255.0"; 
#define sortarr(arr) sort(arr,arr+sizeof(arr)/sizeof(int))
#define all(s) s.begin(),s.end()
#define pb push_back
#define inputarr(arr,n) for(int index=0; index < n ; index++) cin>>arr[index];
#define mod  1000000007
#define mod2 998244353
#define MOD  1000000007 //Mint MOD
#define ll long long
#define lld long double
#define ull unsigned long long
#define deci(x, y)     fixed<<setprecision(y)<<x
#define PI             3.141592653589793238
#define mem0(x)        memset(x,0,sizeof x)
#define mem1(x)        memset(x,-1,sizeof x)
#define pr             pair<int,int>
#define prll           pair<ll,ll> 
#define vi             vector<int>
#define vll            vector<ll>
#define vvi            vector<vi>
#define vpr            vector<pr>
#define vprll          vector<prll>
#ifndef ONLINE_JUDGE
#define debug(x) cerr << #x <<' '; _print(x); cerr << endl;
#else
#define debug(x)
#endif
void _print(ll t) {cerr << t;}
void _print(int t) {cerr << t;}
void _print(string t) {cerr << t;}
void _print(char t) {cerr << t;}
void _print(lld t) {cerr << t;}
void _print(double t) {cerr << t;}
void _print(ull t) {cerr << t;}
template <class T, class V> void _print(pair <T, V> p);
template <class T> void _print(vector <T> v);
template <class T> void _print(set <T> v);
template <class T, class V> void _print(map <T, V> v);
template <class T> void _print(multiset <T> v);
template <class T, class V> void _print(pair <T, V> p) {cerr << '{'; _print(p.first); cerr << ','; _print(p.second); cerr << '}';}
template <class T> void _print(vector <T> v) {cerr << '['; for (T i : v) {_print(i); cerr << ' ';} cerr << ']';}
template <class T> void _print(set <T> v) {cerr << '['; for (T i : v) {_print(i); cerr << ' ';} cerr << ']';}
template <class T> void _print(multiset <T> v) {cerr << '['; for (T i : v) {_print(i); cerr << ' ';} cerr << ']';}
template <class T, class V> void _print(map <T, V> v) {cerr << '['; for (auto i : v) {_print(i); cerr << ' ';} cerr << ']';}



map<int,vector<int>> as_info;

struct Node{ 
    int nodeId,asn;
    map<int,int> routing_table; // dest next hop   
    vector<int> edges;
    map<int,pair<string,string>> interface_info;
    bool connected_to_server;
    map<int,vector<int>> ls_info;
    map<int,pair<int,int>> asst_nxthop;
    map<int,int> dist;
    Node(int nodeId ,int asn ,vector<int> edges, map<int,pair<string,string>> interface_info, int connected_to_server):
     nodeId(nodeId) ,asn(asn), interface_info(interface_info), edges(edges) ,connected_to_server(connected_to_server){
    }
};
vector<Node> nodes;

void get_input(){
    int no_of_nodes;
    in>>no_of_nodes;
    for(int node_id =0 ;node_id<no_of_nodes;++node_id){
        int no_of_edges,asn;
        in>>no_of_edges>>asn;
        bool connected_to_server;
        in>>connected_to_server;
        vector<int> edges;
        map<int,pair<string,string>> interface_info;
        for(int i=0;i<no_of_edges;i++){
            int a; in>>a;
            string ip1 , ip2;
            in>>ip1>>ip2;
            edges.push_back(a);
            interface_info[a] = {ip1,ip2};
        }
        Node new_node= Node(node_id,asn,edges,interface_info,connected_to_server);
        nodes.push_back(new_node);
        as_info[asn].push_back(node_id);
    }
    // for(auto x : as_info){
    //         out<<x.first<<"\n";
    //         for(auto y : x.second){
    //             out<<y<<" ";
    //         }
    //         out<<"\n";
    // }
}

string get_ip(string s){
    string ip = "";
    int l = s.size()-1;
    for(int i=l;i>=0;i--){
        if(s[i]==' ') break;
        ip = s[i]+ip;
    }
    return ip;
}

string get_ip_pref(string ip){
    int cnt  =0 ;
    string res="";
    for(auto x : ip){
        if(x == '.')cnt++;
        if(cnt == 3)break;
        res+=x;
    }
    return res;
}

void ospf(){
    for(auto sys : as_info){
        int sys_id = sys.first;
        vector<int> in_auto = sys.second;
        for(int i =0;i<in_auto.size();++i){
            queue<int> q;
            set<int> st;
            int node_id = in_auto[i];
            q.push(node_id);
            st.insert(node_id);
             for(auto y : nodes[node_id].edges)
                if(nodes[node_id].asn == nodes[y].asn)
                    nodes[node_id].ls_info[node_id].push_back(y);
                       
            while(!q.empty()){
                int frnt = q.front();
                q.pop();
                for(auto x : nodes[frnt].edges){
                    if( nodes[x].asn ==  sys_id && st.find(x) ==  st.end()){
                        st.insert(x);
                        q.push(x);
                        for(auto y : nodes[node_id].edges)
                          if(nodes[x].asn == nodes[y].asn)
                            nodes[x].ls_info[node_id].push_back(y);    
                    }
                }
            }
        }
    }
    auto dijkstra = [&](int s, map<int,int> &d, map<int,int> &p , map<int,vector<int>> &G){
         const int INF = 1e9;
         for(auto x : G)
            d[x.first] = INF; 
         d[s] = 0;
         p[s] = s ;
        set<pair<int, int>> q;
        q.insert({0, s});
        while (!q.empty()) {
            int v = q.begin()->second;
            q.erase(q.begin()); 
            for (auto vert : G[v]) {
                 int to = vert;
                 int len = 1;
                    if (d[v] + len < d[to]) {
                         q.erase({d[to], to});
                         d[to] = d[v] + len;
                         p[to] = v;
                         q.insert({d[to], to});
               }
             }
       }
    };
    for(auto sys : as_info){
        int sys_id = sys.first;
        vector<int> in_auto = sys.second;
        for(int i =0 ;i<in_auto.size();++i){
                map<int,int>  p,d;
                dijkstra(in_auto[i],d,p,nodes[in_auto[i]].ls_info);
                for(int  j =0 ;j<in_auto.size();++j){
                    nodes[in_auto[j]].routing_table[in_auto[i]] = p[in_auto[j]];
                    nodes[in_auto[j]].dist[in_auto[i]] = d[in_auto[j]];
                }
        }
    }
    for(int i =0 ;i<nodes.size();++i){
        int x = nodes[i].asn;
        for(int j =0 ;j<as_info[x].size();++j){
              int vert = as_info[x][j];
               vector<int> edg = nodes[vert].edges;
               for(int k =0 ;k<edg.size();++k){
                  if(nodes[edg[k]].asn != x){
                    if((nodes[i].asst_nxthop).find(nodes[edg[k]].asn) == nodes[i].asst_nxthop.end()){
                         nodes[i].asst_nxthop[nodes[edg[k]].asn] = {vert,edg[k]};
                    }
                    else{
                        int v1 = nodes[i].asst_nxthop[nodes[edg[k]].asn].first;
                        int v2 = vert;
                        if(nodes[i].dist[v1]>nodes[i].dist[v2])
                        nodes[i].asst_nxthop[nodes[edg[k]].asn] = {v2,edg[k]};
                    }
                  }
               }
        }
    }
    ofstream o;
    o.open("routing_info.txt");
    for(int node_id =0 ;node_id<nodes.size();++node_id){
        o<<"Routing Table Info for "<<node_id<<"\n";
        for(auto x : nodes[node_id].routing_table){
                o<<"Dest- "<<x.first<<"\tNextHop- "<<x.second<<" ip interface of current router- "<<nodes[node_id].interface_info[x.second].first<<" ip interface of next hop router- "<<nodes[node_id].interface_info[x.second].second<<"\n";
        }
        o<<"\n";
    }    
    o.close();
     o.open("asst_nexthop.txt");
    for(int node_id =0 ;node_id<nodes.size();++node_id){
        o<<"Asst next hop Info for "<<node_id<<"\n";
        for(auto x : nodes[node_id].asst_nxthop){
                o<<"Dest asn- "<<x.first<<"\tNextHop- {edge router,direct interface} : {"<<x.second.first<<" - "<<nodes[x.second.first].interface_info[x.second.second].first<<", "<<x.second.second<<" - "<<nodes[x.second.first].interface_info[x.second.second].second<<"}\n";
        }
        o<<"\n";
    } 
    o.close();
    o.open("ls.txt");
    for(int node_id =0 ;node_id<nodes.size();++node_id){
        o<<"Link State Info for "<<node_id<<"\n";
        for(auto x : nodes[node_id].ls_info){
               for(auto y : x.second){
                o<<x.first<<" "<<y<<"\n";
               }
        }
        o<<"\n";                           
    } 
    
}

struct as_node{ 
    int id;
    vector<int> edges;
    int has_cdn=0;  
    map<string,string> as_path;
};
vector<as_node> as;
void create_as_graph(){
    auto lst = as_info.end();
    lst--;
    int no_of_as = (*lst).first+1;
    as.resize(no_of_as);

    for(auto sys : as_info){
        int sys_id = sys.first;
        vector<int> in_auto = sys.second;
        as[sys_id].id = sys_id;
        for(int i =0 ;i<in_auto.size();++i){
            int vert = in_auto[i];
            as[sys_id].has_cdn|= nodes[vert].connected_to_server;
            vector<int> edg = nodes[vert].edges;
                for(int j =0 ;j< edg.size();++j){
                    if(nodes[edg[j]].asn != sys_id && find(as[sys_id].edges.begin(),as[sys_id].edges.end(),nodes[edg[j]].asn) == as[sys_id].edges.end()){
                        as[sys_id].edges.push_back(nodes[edg[j]].asn);
                    }
                }                        
        }
    }
}
void broadcast(){
    for(int i =0;i<as.size();++i){
        int as_id = i;
         queue<pair<int,string>> q;
         set<int> st;
        if(as[as_id].has_cdn)
        {
          q.push({as_id,cdn_ip_pref});
          st.insert(as_id);          
          while(!q.empty()){
                int frnt = q.front().first;
                string path_till_now = q.front().second;
                q.pop();
                string ip = get_ip(path_till_now);
                if(as[frnt].as_path.find(ip) == as[frnt].as_path.end())
                as[frnt].as_path[ip] = path_till_now;
                else{
                    if(as[frnt].as_path[ip].size()>path_till_now.size()){
                        as[frnt].as_path[ip] = path_till_now;
                    }
                }
                for(auto x : as[frnt].edges){
                    if(st.find(x) ==  st.end()){
                        st.insert(x);
                        string now = "AS"+to_string(frnt)+" "+path_till_now;
                        q.push({x,now});
                    }
                }
            }
        }
            st.clear();
            q.push({as_id,get_ip_pref((*nodes[as_info[as_id][0]].interface_info.begin()).second.first)});
            st.insert(as_id);          
            while(!q.empty()){
                int frnt = q.front().first;
                string path_till_now = q.front().second;
                q.pop();
                string ip = get_ip(path_till_now);
                 if(as[frnt].as_path.find(ip) == as[frnt].as_path.end())
                 as[frnt].as_path[ip] = path_till_now;
                else{
                    if(as[frnt].as_path[ip].size()>path_till_now.size()){
                        as[frnt].as_path[ip] = path_till_now;
                    }
                }
                for(auto x : as[frnt].edges){
                     if(st.find(x) ==  st.end()){
                         st.insert(x);
                         string now = "AS"+to_string(frnt)+" "+path_till_now;
                         q.push({x,now});
                     }
                }
            }
        
        }  
}  
int main(){
    work();
    get_input();
    ospf();
    create_as_graph();
    broadcast();

    cout<<"Enter Query\n";
    int id;
    string ip ="1.255.0.0";
    cout<<"Enter id of current Node\n";
    cin>>id; 
    cout<<"Outputting the path to the nearest cdn server\n";
    int cur_as = nodes[id].asn;
    cout<<"AS PATH :: "<<as[cur_as].as_path[get_ip_pref(ip)]<<"\n";
    if(as[cur_as].as_path[get_ip_pref(ip)] == get_ip_pref(ip)){
        cout<<"In the same AS\n";
        cout<<"NEXT HOP ROUTER\n";
        set<pair<int,int>> st;
        for(auto x  : as_info[cur_as]){
            if(nodes[x].connected_to_server){
                    st.insert({nodes[id].dist[x],x});
            }
        }
        int desti = (*st.begin()).second;
        cout<<"FINAL GOTO DEST "<<desti<<"| TAKE NEXT HOP "<<nodes[id].interface_info[nodes[id].routing_table[desti]].first;
        return 0;
    }
    int nex_as = as[cur_as].as_path[get_ip_pref(ip)][2] - 48;   
    cout<<"NEXT HOP\n";
    cout<<"GOTO EDGE ROUTER "<<nodes[id].asst_nxthop[nex_as].first<<"\n";
    int edg_router =nodes[id].asst_nxthop[nex_as].first;
    cout<<"THEN GO FROM INTERFACE "<<nodes[edg_router].interface_info[nodes[id].asst_nxthop[nex_as].second].first<<" TO INTERFACE "<<nodes[edg_router].interface_info[nodes[id].asst_nxthop[nex_as].second].second;
    
}
