#pragma once

#include "Base.h"

namespace ServerAllocationProblem
{
	struct ClientType;
	struct DatacenterType;

	struct ClientType
	{
		int id; // global id (fixed once initialized)	
		double chargedTrafficVolume;
		map<int, double> delayToDatacenter; // delay values mapped with dc's id (fixed once initialized)
		int nearestDatacenterID;
		vector<DatacenterType*> eligibleDatacenters;
		int assignedDatacenterID; // the id of the dc to which it is assigned	

		ClientType(int givenID)
		{
			this->id = givenID;			
		}
	};
	bool ClientComparatorByTrafficVolume(const ClientType * A, const ClientType * B);

	struct DatacenterType
	{
		int id; // id of this dc (fixed once initilized)
		double priceServer; // server price (per server per session duration that is supposed to be up to 1 hour)
		double priceBandwidth; // bandwidth price per unit traffic volume (per GB)
		map<int, double> delayToClient; // delay value mapped with client's id (fixed once initialized)	
		map<int, double> delayToDatacenter; // delay value mapped with dc's id (fixed once initialized)		
		vector<ClientType*> coverableClients; // alternative way to access its coverable clients		
		vector<ClientType*> assignedClients;
		double openServerCount;
		vector<ClientType*> unassignedCoverableClients;
		double amortizedCostPerClient;
		size_t addedClientCount;

		DatacenterType(int givenID)
		{
			this->id = givenID;			
		}
	};
	
	/*core functions*/
	bool Initialize(string, vector<ClientType*> &, vector<DatacenterType*> &);
	void SimulateBasicProblem(double DELAY_BOUND_TO_G, double DELAY_BOUND_TO_R, double SESSION_SIZE, double SESSION_COUNT = 1000);
	void SimulateGeneralProblem(double DELAY_BOUND_TO_G, double DELAY_BOUND_TO_R, double SESSION_SIZE, double SESSION_COUNT = 1000);
	bool Matchmaking4BasicProblem(vector<DatacenterType*>, vector<ClientType*>, int &, vector<ClientType*> &, double, double, double);
	void SearchEligibleGDatacenter(vector<DatacenterType*>, vector<ClientType*>, vector<DatacenterType*> &, double, double);
	bool Matchmaking4GeneralProblem(vector<DatacenterType*>, vector<ClientType*>, vector<ClientType*> &, vector<DatacenterType*> &, double, double, double);
	void SimulationSetup4GeneralProblem(DatacenterType*, vector<ClientType*>, vector<DatacenterType*>, double, double);

	/*untility functions*/
	void FindnearestDatacenterID(ClientType* client, vector<DatacenterType*> datacenters);
	void ResetEligibiltyCoverability(vector<ClientType*>, vector<DatacenterType*>);
	void ResetAssignment(vector<ClientType*>, vector<DatacenterType*> &);	
	tuple<double, double, double, double, double> GetSolutionOutput(vector<DatacenterType*>, double, vector<ClientType*>, int);
	bool CheckIfAllClientsExactlyAssigned(vector<ClientType*>, vector<DatacenterType*>);
	void WriteCostWastageDelayData(int, vector<double>, double, vector<vector<vector<tuple<double, double, double, double, double>>>>&, string, string);

	/*algorithm functions*/
	tuple<double, double, double, double, double> Alg_LB(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);
	tuple<double, double, double, double, double> Alg_LB(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double);
	tuple<double, double, double, double, double> Alg_RA(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);
	tuple<double, double, double, double, double> Alg_RA(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double);
	tuple<double, double, double, double, double> Alg_NA(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);
	tuple<double, double, double, double, double> Alg_NA(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double);
	tuple<double, double, double, double, double> Alg_LCP(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);	
	tuple<double, double, double, double, double> Alg_LSP(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double);
	tuple<double, double, double, double, double> Alg_LBP(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);
	tuple<double, double, double, double, double> Alg_LBP(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double);
	tuple<double, double, double, double, double> Alg_LSP(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);
	tuple<double, double, double, double, double> Alg_LCP(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double);
	tuple<double, double, double, double, double> Alg_LCW(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);
	tuple<double, double, double, double, double> Alg_LCW(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double);
	tuple<double, double, double, double, double> Alg_LAC(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);
	tuple<double, double, double, double, double> Alg_LAC(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double);	
	tuple<double, double, double, double, double> Alg_LAC_2(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);
	tuple<double, double, double, double, double> Alg_LAC_2(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double);
}