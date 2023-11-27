#include<bits/stdc++.h>
#include <fstream> 
using namespace std;

vector<int> request_generator(int num, int spatial, int mem_blocks, int block_size, int num2)
{
    srand(time(0));
    int iterations = num / spatial;
    int random_offset;
    int random;

    vector<int> request;
    vector<int> spatial_locality;

    int counter = 0;
    for (int j = 0; j < iterations; j++)
    {
        int x;
        random = rand() % mem_blocks;
        // cout << random << "\n";
        for (int i = 0; i < spatial; i++)
        {
            random_offset = rand() % block_size;
            // cout << random_offset << " ";
            x = random * block_size + random_offset;
            spatial_locality.push_back(x);
        }
        // cout << "\n";
        counter++;
        if (num2 == counter)
        {
            random_shuffle(spatial_locality.begin(), spatial_locality.end());
            for (int p = 0; p < spatial_locality.size(); p++)
            {
                request.push_back(spatial_locality[p]);
            }
            spatial_locality.clear();
            counter = 0;
        }
    }
    return request;
}

void print(vector<int> &req)
{
    for (int i = 0; i < req.size(); i++)
    {
        cout << req[i] << '\n';
    }
}

int main(){
    int num, spatial, mem_blocks, block_size, num2;
    cin>>num>>spatial>>mem_blocks>>block_size>>num2;
    vector<int> vec=request_generator(num, spatial, mem_blocks, block_size, num2);
    ofstream filestream("input.txt");  
    string temp="";
    if (filestream.is_open())  
    {   
        // filestream << "Welcome to javaTpoint.\n";  
        // filestream << "C++ Tutorial.\n";  
        for(int i=0;i<vec.size();i++){
            temp=to_string(vec[i])+'\n';
            filestream<<temp;
        }
        filestream.close();  
    }  
    else cout <<"File opening is fail.";  

    return 0;
}