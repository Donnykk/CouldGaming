#ifndef MAIN_H
#define MAIN_H

#include <bits/stdc++.h>
using namespace std;

class Server
{
private:
    int server_id;
    int workload;
    bool empty;
    bool available;

public:
    Server(int id, int workload);
    ~Server();
    int getServerId();
    int getWorkload();
    void setWorkload(int workload);
    bool isAvailable();
    void setAvailable(bool available);
    bool isEmpty();
    void setEmpty(bool empty);
};

class ServerCenter
{
private:
    int center_id;
    int server_num;
    int available_index;
    vector<Server> serverList;

public:
    ServerCenter(int id);
    ~ServerCenter();
    int getCenterId();
    int getServerNum();
    int getAvailableIndex();
    void setAvailableIndex(int index);
    vector<Server> getServerList();
};

class Part
{
private:
    int center_id;
    int server_id;
    int client_id;
    float proportion; // 切割占比

public:
    Part(int center_id, int server_id, int client_id);
    float getProportion();
    void setProportion(float p);
    int getCenterId();
    int getServerId();
    int getClientId();
};

class Client
{
private:
    int client_id;
    int workload;
    int data;
    vector<Part> screenDivider;

public:
    Client(int id, int workload, int data);
    ~Client();
    int getClientId();
    int getWorkload();
    int getData();
    vector<Part> getScreenDivider();
    void addPart(Part p);
};

const int M = 5; // number of server-centers
const int N = 3; // number of clients
const int ServerCost = 1000;
const int LantLimit = 200;
const int WorkloadLimit = 5000;
const int BandwidthLimit = 10;

vector<ServerCenter> serverCenterList;
vector<Client> clientList;

int cost[N][M] = {{50, 100, 200, 30, 50},
                  {150, 200, 50, 80, 300},
                  {100, 80, 500, 400, 100}};
int latency[N][M] = {{50, 100, 200, 30, 50},
                     {150, 200, 50, 80, 300},
                     {100, 80, 500, 400, 100}};
int bandwidth[N][M] = {{0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 0}};

void Init();
void Allocate1();
void Allocate2();
void PrintInfo();

#endif