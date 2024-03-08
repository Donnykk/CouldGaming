#pragma once

#include "Base.h"

namespace MatchmakingProblem
{	
	struct ClientType;
	struct DatacenterType;
	
	struct ClientType
	{
		int id;
		double chargedTrafficVolume;
		map<int, double> delayToDatacenter;		

		vector<DatacenterType*> eligibleDatacenters_G;
		vector<DatacenterType*> eligibleDatacenters_R;		
		map<int, vector<DatacenterType*>> eligibleDatacenters_R_indexed_by_G;
		map<int, vector<DatacenterType*>> eligibleDatacenters_G_indexed_by_R;
		DatacenterType* assignedDatacenter_G = nullptr;
		DatacenterType* assignedDatacenter_R = nullptr;

		double potential_cost_increase = 0;
		
		ClientType(int givenID)
		{
			this->id = givenID;
		}
	};
	bool ClientComparatorBy_Fewer_EligibleDatacenters_G(const ClientType* a, const ClientType* b);
	bool ClientComparatorBy_More_EligibleDatacenters_G(const ClientType* a, const ClientType* b);
	bool ClientComparatorByAssigned_R_ServerPrice(const ClientType* a, const ClientType* b);
	
	struct DatacenterType
	{
		int id; // id of this dc (fixed once initilized)
		double priceServer; // server price (per server per session duration that is supposed to be up to 1 hour)
		double priceBandwidth; // bandwidth price per unit traffic volume (per GB)
		map<int, double> delayToClient; // delay value mapped with client's id (fixed once initialized)	
		map<int, double> delayToDatacenter; // delay value mapped with dc's id (fixed once initialized)
		string name;
		
		vector<ClientType*> coverableClients_G;
		vector<ClientType*> coverableClients_R;
		vector<ClientType*> assignedClients_G;
		vector<ClientType*> assignedClients_R;
		
		DatacenterType(int givenID)
		{
			this->id = givenID;
		}
	};
	bool DatacenterComparatorByPrice(const DatacenterType a, const DatacenterType b);

	struct DatacenterPointerVectorCmp
	{
		bool operator()(const vector<DatacenterType*>& a, const vector<DatacenterType*>& b) const
		{
			if (a.size() > b.size()) return false;
			else if (a.size() == b.size())
			{
				if (a.empty()) return false;
				else
				{
					for (size_t i = 0; i < a.size(); i++)
					{
						if (i < (a.size() - 1))
						{
							if (a.at(i)->id > b.at(i)->id) return false;
							else if (a.at(i)->id < b.at(i)->id) return true;
						}
						else return (a.back()->id < b.back()->id);
					}
				}
			}
			else return true;
		}
	};
	
	struct SessionType
	{
		vector<ClientType*> sessionClients;
		int dc_g_id;
		set<int> dc_r_id_set;
	};

	struct PerformanceType
	{
		map<string, vector<double>> 
			sessionCountTable, 
			serverCostTable, 
			serverUtilizationTable, 
			R_G_colocation_ratio_table, 
			G_count_allSessions_table, 
			R_count_perSession_table;
	};
	
	struct Setting
	{
		bool regionControl; // pick clients in a controlled or pure random manner
		int clientCount; // the number of clients eligible for grouping
		int latencyThreshold; // one-way
		int sessionSize;
		int serverCapacity;
		int simulationCount; // the number of simulation runs to perform

		Setting(const bool regionControl_in, const int clientCount_in, const int latencyThreshold_in, const int sessionSize_in, const int serverCapacity_in, const int simulationCount_in)
		{
			regionControl = regionControl_in;
			clientCount = clientCount_in;
			latencyThreshold = latencyThreshold_in;
			sessionSize = sessionSize_in;
			serverCapacity = serverCapacity_in;
			simulationCount = simulationCount_in;
		}
	};

	class MatchmakingProblemBase
	{			
	public:
		string dataDirectory = ".\\Data\\"; // root path for input and output
		void Initialize();
	protected:
		vector<ClientType> globalClientList; // read from input
		vector<DatacenterType> globalDatacenterList; // read from input
		map<string, vector<int>> clientCluster;
		void ClientClustering();
	};

	class MaximumMatchingProblem : public MatchmakingProblemBase
	{
	public:		
		string outputDirectory = dataDirectory + "MaximumMatchingProblem\\";
		ofstream outFile;
		void Simulate(const string algToRun, const int clientCount = 100, const int latencyThreshold = 100, const int sessionSize = 10, const int simulationCount = 100);
		void CountConnectivity(const int latencyThreshold);
	private:
		vector<ClientType> candidateClients;
		vector<DatacenterType> candidateDatacenters;
		DatacenterType* GetClientNearestEligibleDC(ClientType & client);		
		void NearestAssignmentGrouping();
		void SimpleGreedyGrouping(const int sessionSize, const bool sorting = false);
		void LayeredGreedyGrouping(const int sessionSize);
	};

	class ParetoMatchingProblem : public MatchmakingProblemBase
	{
	public:			
		string outputDirectory = dataDirectory + "ParetoMatchingProblem\\";
		void Simulate(const Setting &);
	private:
		vector<ClientType> candidateClients; // copy of a subset of globalClientList
		vector<DatacenterType> candidateDatacenters; // copy of globalDatacenterList
		map<int, vector<SessionType>> sessionListPerG; // indexed by G
		
		double SearchEligibleDatacenters4Clients(const int latencyThreshold);
		vector<ClientType> GenerateCandidateClients(const int clientCount, const bool controlled);

		/*stage flags (need to be reset for each round)*/
		bool Assignment_G_Completed = false;
		bool Assignment_R_Completed = false;
		bool Session_Making_Completed = false;
		void ResetStageFlag();

		/*Assignment_Nearest*/
		void Assignment_Nearest(const int sessionSize);

		/*G_Assignment algorithms*/
		void G_Assignment_Simple(const int sessionSize, const int serverCapacity, const string sortingMode = "");
		void G_Assignment_Layered(const int sessionSize, const int serverCapacity, const string sortingMode = "");
		void Reset_G_Assignment();
		
		/*R_Assignment algorithms*/
		void R_Assignment_LSP();
		void R_Assignment_LCW(const int serverCapacity);
		void Reset_R_Assignment();

		/*assign each client in candidateClients to one dc_g and one dc_r*/
		void ClientAssignment(const int sessionSize, const int serverCapacity, const string algFirstStage, const string algSecondStage);
		
		/*make sessions*/
		void Session_Making_After_Assignment(const int sessionSize);

		/*compute final result based on the assignment and grouping results (sessionListPerG)*/
		void PerformanceMeasurement(PerformanceType & performanceMeasurement, const string solutionName, const int serverCapacity);		
	};
}