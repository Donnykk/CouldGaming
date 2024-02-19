#include <bits/stdc++.h>
#include "main.h"

using namespace std;

Server::Server(int id, int workload)
{
    this->server_id = id;
    this->workload = workload;
    this->available = true;
    this->empty = true;
}
Server::~Server() {}
int Server::getServerId() { return this->server_id; }
int Server::getWorkload() { return this->workload; }
void Server::setWorkload(int workload) { this->workload = workload; }
bool Server::isAvailable() { return this->available; }
void Server::setAvailable(bool available) { this->available = available; }
bool Server::isEmpty() { return this->empty; }
void Server::setEmpty(bool empty) { this->empty = empty; }

ServerCenter::ServerCenter(int id)
{
    this->available_index = 0;
    this->center_id = id;
    this->server_num = rand() % 9 + 2;
    cout << "Initialize ServerCenter " << this->center_id << ", Server number: " << this->server_num << " ." << endl;
    for (int i = 0; i < this->server_num; i++)
    {
        int workload = rand() % 2000 + 1000;
        Server s(i + 1, workload);
        this->serverList.push_back(s);
        cout << "Initialize Server " << i + 1 << " , workload: " << workload << endl;
    }
    cout << endl;
}
ServerCenter::~ServerCenter() {}
int ServerCenter::getCenterId() { return this->center_id; }
int ServerCenter::getServerNum() { return this->server_num; }
int ServerCenter::getAvailableIndex() { return this->available_index; }
void ServerCenter::setAvailableIndex(int index) { this->available_index = index; }
vector<Server> ServerCenter::getServerList() { return this->serverList; }

Client::Client(int id, int workload, int data)
{
    this->client_id = id;
    this->workload = workload;
    this->data = data;
    cout << "Initialize Client " << this->client_id << " , workload: " << this->workload << endl;
}
Client::~Client() {}
int Client::getClientId() { return this->client_id; }
int Client::getWorkload() { return this->workload; }
int Client::getData() { return this->data; }
void Client::addPart(Part p) { this->screenDivider.push_back(p); }
vector<Part> Client::getScreenDivider() { return this->screenDivider; }

Part::Part(int center_id, int server_id, int client_id)
{
    this->center_id = center_id;
    this->server_id = server_id;
    this->client_id = client_id;
}
float Part::getProportion() { return this->proportion; }
void Part::setProportion(float p) { this->proportion = p; }
int Part::getCenterId() { return this->center_id; }
int Part::getServerId() { return this->server_id; }
int Part::getClientId() { return this->client_id; }

void Init()
{
    cout << "ServerCenter number: " << M << " ......" << endl;
    for (int i = 0; i < M; i++)
    {
        ServerCenter serverCenter(i + 1);
        serverCenterList.push_back(serverCenter);
    }
    cout << endl;

    cout << "Client number: " << N << " ......" << endl;
    for (int i = 0; i < N; i++)
    {
        int workload = rand() % 1000 + 500;
        int data = rand() % 100 + 50;
        Client client(i + 1, workload, data);
        clientList.push_back(client);
    }
    cout << endl;

    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            bandwidth[i][j] = 0;
}

void Allocate1()
{
    // 直接分配
    for (Client &client : clientList)
    {
        int id = client.getClientId();
        int workload_ = client.getWorkload();
        bool allocated = false;
        for (int i = 0; i < M; i++)
        {
            if (allocated)
                break;
            // 延迟及带宽约束
            if (latency[i][id] >= LantLimit || bandwidth[i][id] >= BandwidthLimit)
                continue;
            while (workload_)
            {
                int server_id = serverCenterList[i].getAvailableIndex();
                Server server = serverCenterList[i].getServerList()[server_id];
                if (workload_ <= server.getWorkload())
                {
                    // 服务器有足够工作负载，分配完成
                    Part p(i + 1, server.getServerId(), id);
                    float prop = workload_ / client.getWorkload();
                    p.setProportion(prop);
                    client.addPart(p);
                    server.setEmpty(false);
                    if (workload_ == server.getWorkload())
                    {
                        server.setAvailable(false);
                        serverCenterList[i].setAvailableIndex(server_id++);
                    }
                    server.setWorkload(server.getWorkload() - workload_);
                    workload_ = 0;
                    // 带宽
                    bandwidth[id][i] += client.getData() * prop;
                    // cout << "Client " << id << " allocated" << endl;
                    allocated = true;
                }
                else
                {
                    // 服务器工作负载不够，客户端切分
                    Part p(i + 1, server.getServerId(), id);
                    float prop = server.getWorkload() / client.getWorkload();
                    p.setProportion(prop);
                    client.addPart(p);
                    server.setEmpty(false);
                    server.setAvailable(false);
                    server.setWorkload(0);
                    workload_ -= server.getWorkload();
                    bandwidth[id][i] += client.getData() * prop;
                }
            }
        }
    }
}

bool cmp(const pair<int, int> &a, const pair<int, int> &b)
{
    return a.second < b.second;
}

void Allocate2()
{
    // 最低组合价
    for (Client &c : clientList)
    {
        int id = c.getClientId();
        int workload_ = c.getWorkload();
        // int data_ = c.getData();
        //  哈希表按value排序
        unordered_map<int, int> spend;
        for (ServerCenter sc : serverCenterList)
        {
            if (latency[sc.getCenterId()][id] >= LantLimit || bandwidth[sc.getCenterId()][id] >= BandwidthLimit)
                continue;
            spend[sc.getCenterId()] = cost[c.getClientId() - 1][sc.getCenterId() - 1];
        }
        vector<pair<int, int>> vec;
        for (auto x : spend)
            vec.push_back(x);
        sort(vec.begin(), vec.end(), cmp);
        // for (auto x : vec)
        //     cout << x.first << " " << x.second << endl;
        int index = 0;
        bool inEmpty = true;
        while (workload_)
        {
            if (!inEmpty)
            {
                // 开启新服务器
                int server_id = serverCenterList[vec[0].first - 1].getAvailableIndex();
                Server server = serverCenterList[vec[0].first - 1].getServerList()[server_id];
                if (workload_ <= server.getWorkload())
                {
                    // 服务器有足够工作负载，分配完成
                    Part p(vec[0].first, server.getServerId(), id);
                    float prop = workload_ / c.getWorkload();
                    p.setProportion(prop);
                    c.addPart(p);
                    server.setEmpty(false);
                    if (workload_ == server.getWorkload())
                    {
                        server.setAvailable(false);
                        serverCenterList[vec[0].first].setAvailableIndex(server_id++);
                    }
                    server.setWorkload(server.getWorkload() - workload_);
                    workload_ = 0;
                    bandwidth[id][vec[0].first] += c.getData() * prop;
                }
                else
                {
                    Part p(vec[0].first, server.getServerId(), id);
                    float prop = server.getWorkload() / c.getWorkload();
                    p.setProportion(prop);
                    c.addPart(p);
                    server.setEmpty(false);
                    server.setAvailable(false);
                    serverCenterList[vec[0].first].setAvailableIndex(server_id++);
                    server.setWorkload(0);
                    workload_ -= server.getWorkload();
                    bandwidth[id][vec[0].first] += c.getData() * prop;
                }
            }
            else
            {
                // TODO
                while (true)
                {
                    if (index == int(vec.size()))
                    {
                        inEmpty = false;
                        break;
                    }
                    int center_id = vec[index].first - 1;
                    int server_id = serverCenterList[center_id].getAvailableIndex();
                    Server server = serverCenterList[center_id].getServerList()[server_id];
                    if (server.isEmpty())
                        index++;
                    else
                        break;
                }
                if (!inEmpty)
                    continue;
                int server_id = serverCenterList[vec[index].first].getAvailableIndex();
                Server server = serverCenterList[vec[index].first].getServerList()[server_id];
                if (workload_ <= server.getWorkload())
                {
                    Part p(vec[index].first, server.getServerId(), id);
                    float prop = workload_ / c.getWorkload();
                    p.setProportion(prop);
                    c.addPart(p);
                    if (workload_ == server.getWorkload())
                    {
                        server.setAvailable(false);
                        serverCenterList[vec[index].first].setAvailableIndex(server_id++);
                    }
                    server.setWorkload(server.getWorkload() - workload_);
                    workload_ = 0;
                    bandwidth[id][vec[index].first] += c.getData() * prop;
                }
                else
                {
                    Part p(vec[index].first, server.getServerId(), id);
                    float prop = server.getWorkload() / c.getWorkload();
                    p.setProportion(prop);
                    c.addPart(p);
                    server.setAvailable(false);
                    serverCenterList[vec[index].first].setAvailableIndex(server_id++);
                    server.setWorkload(0);
                    workload_ -= server.getWorkload();
                    bandwidth[id][vec[index].first] += c.getData() * prop;
                }
            }
        }
    }
}

void PrintInfo()
{
    int Cost = 0;
    for (Client c : clientList)
    {
        cout << "Client " << c.getClientId()
             << ", workload: " << c.getWorkload()
             << ", data: " << c.getData() << endl;
        int trans_cost = 0;
        int index = 0;
        vector<Part> v = c.getScreenDivider();
        for (Part p : v)
        {
            int cid = p.getCenterId();
            int sid = p.getServerId();
            float pro = p.getProportion();
            trans_cost += cost[c.getClientId()][cid] * c.getData() * pro;
            cout << "Screen Part " << ++index
                 << ", allocated to ServerCenter " << cid
                 << ", Server " << sid
                 << ", Proportion: " << pro << endl;
        }
        Cost += trans_cost;
        cout << "TransCost: " << trans_cost << endl;
    }
    for (ServerCenter sc : serverCenterList)
        for (Server s : sc.getServerList())
            if (!s.isEmpty())
                Cost += ServerCost;
    cout << "Total Cost : " << Cost << endl
         << endl;
}

int main()
{
    srand(time(0));
    Init();
    Allocate2();
    PrintInfo();
    return 0;
}