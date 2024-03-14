#ifndef MAIN_H
#define MAIN_H

#include <bits/stdc++.h>
using namespace std;

class Server
{
private:
    int server_id;
    int workload;

public:
    Server(int id, int workload);
    ~Server();
    int getServerId();
    int getWorkload();
    void setWorkload(int workload);
};

class ServerCenter
{
private:
    int center_id;
    int server_num;
    int available_index;
    int server_price;
    vector<Server *> serverList;

public:
    ServerCenter(int id);
    ~ServerCenter();
    int getCenterId();
    int getAvailableIndex();
    void setAvailableIndex(int index);
    int getServerPrice();
    vector<Server *> getServerList();
    void newServer(Server *s);
};

class Client
{
private:
    int client_id;
    int data;
    int work_load;
    int allocate_center_id;
    int allocate_server_id;

public:
    Client(int id, int data);
    ~Client();
    int getClientId();
    int getData();
    int getWorkLoad();
    int getAllocateCenterId();
    void setAllocateCenterId(int allocate_center_id);
    int getAllocateServerId();
    void setAllocateServerId(int allocate_server_id);
};

int M; // number of server-centers
int N; // number of clients

const int LatLimit = 400;
const int WorkloadLimit = 5000;

vector<ServerCenter> serverCenterList;
vector<Client> clientList;

vector<vector<int>> cost;
vector<vector<int>> latency;

vector<vector<int>> ReadFile(const string input_file_name);
void Allocate1();
void Allocate2();
void PrintInfo();

#endif