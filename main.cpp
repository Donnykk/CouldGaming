#include <bits/stdc++.h>
#include "main.h"

using namespace std;

Server::Server(int id, int workload)
{
    this->server_id = id;
    this->workload = workload;
}
Server::~Server() {}
int Server::getServerId() { return this->server_id; }
int Server::getWorkload() { return this->workload; }
void Server::setWorkload(int workload) { this->workload = workload; }

ServerCenter::ServerCenter(int id)
{
    this->available_index = -1;
    this->center_id = id;
    // this->server_price = 100 * (rand() % 10) + 1000;
    this->server_price = 1000;
    cout << "Initialize ServerCenter " << this->center_id << "  Server Price:" << this->server_price << endl;
}
ServerCenter::~ServerCenter() {}
int ServerCenter::getCenterId() { return this->center_id; }
int ServerCenter::getAvailableIndex() { return this->available_index; }
void ServerCenter::setAvailableIndex(int index) { this->available_index = index; }
int ServerCenter::getServerPrice() { return this->server_price; }
vector<Server *> ServerCenter::getServerList() { return this->serverList; }
void ServerCenter::newServer(Server *s) { this->serverList.push_back(s); };

Client::Client(int id, int data)
{
    this->client_id = id;
    this->data = data;
    cout << "Initialize Client " << this->client_id << " , data: " << this->data << endl;
}
Client::~Client() {}
int Client::getClientId() { return this->client_id; }
int Client::getData() { return this->data; }
int Client::getWorkLoad() { return this->work_load; }
int Client::getAllocateCenterId() { return this->allocate_center_id; };
void Client::setAllocateCenterId(int allocate_center_id) { this->allocate_center_id = allocate_center_id; };
int Client::getAllocateServerId() { return this->allocate_server_id; };
void Client::setAllocateServerId(int allocate_server_id) { this->allocate_server_id = allocate_server_id; };

vector<vector<int>> ReadFile(const string input_file_name)
{
    vector<vector<int>> strings_read;
    const char delimiter = ',';
    fstream data_file(input_file_name, ios::in);
    if (data_file.is_open())
    {
        string one_line;
        int row_counter = 0;
        while (std::getline(data_file, one_line))
        {
            row_counter++;
            // 跳过首行
            if ((row_counter == 1))
                continue;
            stringstream one_line_stream(one_line);
            string one_string;
            vector<int> string_list;
            while (std::getline(one_line_stream, one_string, delimiter))
            {
                bool is_num = true;
                for (char c : one_string)
                {
                    if (c < '0' || c > '9')
                    {
                        is_num = false;
                        break;
                    }
                }
                if (!is_num)
                    continue;
                string_list.push_back(stoi(one_string));
            }
            strings_read.push_back(string_list);
        }
        data_file.close();

        if (strings_read.empty() || strings_read.front().empty())
        {
            std::cout << "ERROR: " << input_file_name << " is empty!\n";
            cin.get();
        }
    }
    else
    {
        std::cout << "ERROR: cannot open " << input_file_name << "!\n";
        cin.get();
    }
    return strings_read;
};

void Init()
{
    latency = ReadFile("F://Environment//CloudGaming//data//dc_to_pl_rtt.csv");
    cost = ReadFile("F://Environment//CloudGaming//data//dc_to_pl_rtt.csv");
    /*
    for (vector<int> v : latency)
        {
            for (int a : v)
                cout << a << " ";
            cout << endl;
        }
    */

    N = latency.size();
    M = latency[0].size();

    serverCenterList.clear();
    clientList.clear();

    cout << "ServerCenter number: " << M << " ......" << endl;
    for (int i = 0; i < M; i++)
    {
        ServerCenter serverCenter(i);
        serverCenterList.push_back(serverCenter);
    }
    cout << endl;

    cout << "Client number: " << N << " ......" << endl;
    for (int i = 0; i < N; i++)
    {
        int data = 500;
        Client client(i, data);
        clientList.push_back(client);
    }
    cout << endl;
}

void Allocate1()
{
    // LCP
    for (Client &c : clientList)
    {
        int cl_id = c.getClientId();
        auto cmp = [cl_id](ServerCenter *x, ServerCenter *y)
        { return cost[cl_id][x->getCenterId()] < cost[cl_id][y->getCenterId()]; };

        // priority_queue<ServerCenter, vector<ServerCenter>, decltype(cmp)> D(cmp);
        vector<ServerCenter *> D;
        for (ServerCenter &sc : serverCenterList)
        {
            int sc_id = sc.getCenterId();
            if (latency[cl_id][sc_id] < LatLimit)
            {
                D.push_back(&sc);
            }
        }
        sort(D.begin(), D.end(), cmp);
        for (int i = 0; i < int(D.size()); i++)
        {
            ServerCenter *sc = D[i];
            if (sc->getAvailableIndex() != -1)
            {
                Server *s = sc->getServerList()[sc->getAvailableIndex()];
                if (s->getWorkload() >= c.getData())
                {
                    // 有可用服务器
                    s->setWorkload(s->getWorkload() - c.getData());
                    c.setAllocateCenterId(sc->getCenterId());
                    c.setAllocateServerId(s->getServerId());
                    if (s->getWorkload() == 0)
                        sc->setAvailableIndex(-1);
                    cout << "Client " << c.getClientId() << " allocated to center " << sc->getCenterId() << " server " << s->getServerId()
                         << " Remain workload:" << s->getWorkload() << endl;
                    break;
                }
            }
            // 无其他服务器可分配
            bool gotosc = false;
            ServerCenter *next = D[i + 1];
            for (int j = i + 1; j <= int(D.size()); j++)
            {
                if (j == int(D.size()))
                {
                    gotosc = true;
                    break;
                }
                next = D[j];
                if (next->getAvailableIndex() > -1)
                {
                    if (next->getServerList()[next->getAvailableIndex()]->getWorkload() >= c.getData())
                    {
                        break;
                    }
                }
            }
            int cost_dif = cost[cl_id][next->getCenterId()] - cost[cl_id][sc->getCenterId()];
            int data = c.getData();
            int pr_diff = sc->getServerPrice() * float(c.getData() / WorkloadLimit);
            if (pr_diff <= data * cost_dif || gotosc)
            {
                // 当前数据中心开启新服务器
                int s_id = sc->getServerList().size();
                Server *s = new Server(s_id, WorkloadLimit - data);
                sc->setAvailableIndex(s_id);
                sc->newServer(s);
                c.setAllocateCenterId(sc->getCenterId());
                c.setAllocateServerId(s->getServerId());
                cout << "Client " << c.getClientId() << " allocated to center " << sc->getCenterId() << " server " << s->getServerId()
                     << " Remain workload:" << s->getWorkload() << endl;
                break;
            }
            sc = next;
            continue;
        }
    }
}

void Allocate2()
{
    // LL
    for (Client &c : clientList)
    {
        int cl_id = c.getClientId();
        int data = c.getData();
        auto cmp = [cl_id](ServerCenter *x, ServerCenter *y)
        { return latency[cl_id][x->getCenterId()] < latency[cl_id][y->getCenterId()]; };
        vector<ServerCenter *> D;
        for (ServerCenter &sc : serverCenterList)
        {
            int sc_id = sc.getCenterId();
            if (latency[cl_id][sc_id] < LatLimit)
            {
                D.push_back(&sc);
            }
        }
        if (D.empty())
        {
            cout << "No available center for client " << c.getClientId() << endl;
            continue;
        }
        sort(D.begin(), D.end(), cmp);
        ServerCenter *sc = D[0];
        if (sc->getAvailableIndex() >= 0)
        {
            Server *s = sc->getServerList()[sc->getAvailableIndex()];
            if (s->getWorkload() >= c.getData())
            {
                // 有可用服务器
                s->setWorkload(s->getWorkload() - c.getData());
                c.setAllocateCenterId(sc->getCenterId());
                c.setAllocateServerId(s->getServerId());
                if (s->getWorkload() == 0)
                    sc->setAvailableIndex(-1);
                cout << "Client " << c.getClientId() << " allocated to center " << sc->getCenterId() << " server " << s->getServerId()
                     << " Remain workload:" << s->getWorkload() << endl;
                continue;
            }
        }
        int s_id = sc->getServerList().size();
        Server *s = new Server(s_id, WorkloadLimit - data);
        sc->setAvailableIndex(s_id);
        sc->newServer(s);
        c.setAllocateCenterId(sc->getCenterId());
        c.setAllocateServerId(s->getServerId());
        cout << "Client " << c.getClientId() << " allocated to center " << c.getAllocateCenterId() << " server " << c.getAllocateServerId()
             << " Remain workload:" << s->getWorkload() << endl;
    }
}

void PrintInfo()
{
    int total_cost = 0;
    int total_latency = 0;
    for (Client &c : clientList)
    {
        int cl_id = c.getClientId();
        int sc_id = c.getAllocateCenterId();
        total_latency += latency[cl_id][sc_id];
        total_cost += cost[cl_id][sc_id] * c.getData();
    }
    for (ServerCenter &sc : serverCenterList)
    {
        int num = sc.getServerList().size();
        total_cost += num * sc.getServerPrice();
    }
    float average_cost = (float)total_cost / (float)clientList.size();
    float average_latency = (float)total_latency / (float)clientList.size();
    cout << "Average cost: " << average_cost << endl
         << "Average latency: " << average_latency << endl;
}

int main()
{
    srand(time(0));
    Init();
    Allocate1();
    PrintInfo();
    Init();
    Allocate2();
    PrintInfo();
    return 0;
}