#include<bits/stdc++.h>
// #include"debug.h"
using namespace std;

#define REQ_CLUSTER_DIFF 100

int main(){
    //server load analysis

    fstream file;
    file.open("server_load.txt", ios::in);

    vector<string> timestamps;
    string s;
    while (!file.eof()) {
        getline(file, s);
        if (!s.empty()) {
            timestamps.push_back(s);
        }
    }
    file.close();
    sort(timestamps.begin(), timestamps.end());

    auto difference = [&] (string a, string b) {
        int i = 0; 
        //assuming a.size() == b.size()
        while (i < (int)a.size() && a[i] == b[i]) i++;
        a = a.substr(i);
        b = b.substr(i);
        if (a.empty() || b.empty()) {
            return 0ll;
        }
        long long dif = abs(stoll(b) - stoll(a));
        return dif;
    };

    vector<int> req_cluster_sizes;
    for (int i = 0; i < (int)timestamps.size(); i++) {
        int ind = i;
        while (ind < (int)timestamps.size() && difference(timestamps[i], timestamps[ind]) <= REQ_CLUSTER_DIFF) {
        // while (ind < (int)timestamps.size() && ind - i < 10) {  
            ind++;
        }
        req_cluster_sizes.push_back(ind - i);
        i = ind - 1;
    }

    cout << "Maximum request cluster size: " << *max_element(req_cluster_sizes.begin(), req_cluster_sizes.end()) << '\n';
    cout << "The displayed number represents the maximum number of requests the server had to process at once simultaneously\n";

    //server load analysis end
    return 0;
}