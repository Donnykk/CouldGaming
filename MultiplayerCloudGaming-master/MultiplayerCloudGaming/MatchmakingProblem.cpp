#include "MatchmakingProblem.h"

namespace MatchmakingProblem
{
	bool ClientComparatorBy_Fewer_EligibleDatacenters_G(const ClientType* a, const ClientType* b)
	{
		return (a->eligibleDatacenters_G.size() < b->eligibleDatacenters_G.size());
	}

	bool ClientComparatorBy_More_EligibleDatacenters_G(const ClientType * a, const ClientType * b)
	{
		return (a->eligibleDatacenters_G.size() > b->eligibleDatacenters_G.size());
	}

	bool ClientComparatorByAssigned_R_ServerPrice(const ClientType * a, const ClientType * b)
	{
		return (a->assignedDatacenter_R->priceServer < b->assignedDatacenter_R->priceServer);
	}

	bool DatacenterComparatorByPrice(const DatacenterType a, const DatacenterType b)
	{
		return (a.priceServer < b.priceServer);
	};

	void MatchmakingProblemBase::Initialize()
	{				
		auto startTime = clock();
		
		string ClientDatacenterLatencyFile = "ping_to_prefix_median_matrix.csv";
		string InterDatacenterLatencyFile = "ping_to_dc_median_matrix.csv";
		string BandwidthServerPricingFile = "pricing_bandwidth_server.csv";
		
		this->globalClientList.clear();
		this->globalDatacenterList.clear();

		/* temporary stuff */
		vector<vector<double>> ClientToDatacenterDelayMatrix;
		vector<vector<double>> InterDatacenterDelayMatrix;
		vector<string> DC_Name_List;
		vector<double> priceServerList;
		vector<double> priceBandwidthList;

		/* client-to-dc latency data */
		auto strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + ClientDatacenterLatencyFile, ',', true);
		if (strings_read.empty())
		{
			std::printf("ERROR: empty file!\n");
			cin.get();
			return;
		}
		for (auto row : strings_read)
		{
			vector<double> ClientToDatacenterDelayMatrixOneRow;
			for (size_t col = 1; col < row.size(); col++)
			{
				ClientToDatacenterDelayMatrixOneRow.push_back(stod(row.at(col)) / 2);
			}
			ClientToDatacenterDelayMatrix.push_back(ClientToDatacenterDelayMatrixOneRow);
		}
		const int totalClientCount = int(ClientToDatacenterDelayMatrix.size());
		const int totalDatacenterCount = int(ClientToDatacenterDelayMatrix.front().size());

		/*dc-to-dc latency data*/
		strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + InterDatacenterLatencyFile, ',', true);
		if (strings_read.empty())
		{
			std::printf("ERROR: empty file!\n");
			cin.get();
			return;
		}
		for (auto row : strings_read)
		{
			DC_Name_List.push_back(row.at(0));
			vector<double> InterDatacenterDelayMatrixOneRow;
			for (size_t col = 1; col < row.size(); col++)
			{
				InterDatacenterDelayMatrixOneRow.push_back(stod(row.at(col)) / 2);
			}
			InterDatacenterDelayMatrix.push_back(InterDatacenterDelayMatrixOneRow);
		}
		/*detect the asymmetry of dc-to-dc latency*/
		/*for (int i = 0; i < InterDatacenterDelayMatrix.size(); i++)
		{
			for (int j = 0; j < InterDatacenterDelayMatrix.size(); j++)
			{
				if (InterDatacenterDelayMatrix.at(i).at(j) != InterDatacenterDelayMatrix.at(j).at(i))
				{
					std::printf("\n*** %f != %f -> using the maximum to make them symmetric***\n", InterDatacenterDelayMatrix.at(i).at(j), InterDatacenterDelayMatrix.at(j).at(i));
					InterDatacenterDelayMatrix.at(i).at(j) = std::max(InterDatacenterDelayMatrix.at(i).at(j), InterDatacenterDelayMatrix.at(j).at(i));
				}
			}
		}*/

		/* bandwidth and server price data */
		strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + BandwidthServerPricingFile, ',', true);
		if (strings_read.empty())
		{
			std::printf("ERROR: empty file!\n");
			cin.get();
			return;
		}
		for (auto row : strings_read)
		{
			priceBandwidthList.push_back(stod(row.at(1)));
			priceServerList.push_back(stod(row.at(3))); // 2: g2.8xlarge, 3: g2.2xlarge
		}		

		/* creating clients */
		for (int i = 0; i < totalClientCount; i++)
		{
			ClientType client(i);
			client.chargedTrafficVolume = 2;
			for (int j = 0; j < totalDatacenterCount; j++)
			{
				client.delayToDatacenter[j] = ClientToDatacenterDelayMatrix.at(i).at(j);
			}
			globalClientList.push_back(client);
		}
		std::printf("%d clients loaded\n", int(globalClientList.size()));		

		/* create datacenters */
		for (int i = 0; i < totalDatacenterCount; i++)
		{
			DatacenterType dc(i);
			dc.priceServer = priceServerList.at(i);
			dc.priceBandwidth = priceBandwidthList.at(i);
			dc.name = DC_Name_List.at(i);
			for (auto client : globalClientList)
			{
				dc.delayToClient[client.id] = client.delayToDatacenter[dc.id];
			}
			for (int j = 0; j < totalDatacenterCount; j++)
			{
				dc.delayToDatacenter[j] = InterDatacenterDelayMatrix.at(i).at(j);
			}
			globalDatacenterList.push_back(dc);
		}
		std::printf("%d datacenters loaded\n", int(globalDatacenterList.size()));

		/*create client clusters*/
		ClientClustering();

		std::printf("\n***Initialize(): %.2f seconds***\n", std::difftime(clock(),startTime) / 1000);
	}

	void MatchmakingProblemBase::ClientClustering()
	{
		for (int c = 0; c < globalClientList.size(); c++)
		{			
			/*find nearestDC*/
			int nearestDC = 0;
			for (int d = 1; d < globalDatacenterList.size(); d++)
			{
				if (globalClientList.at(c).delayToDatacenter.at(d) < globalClientList.at(c).delayToDatacenter.at(nearestDC))
				{
					nearestDC = d;
				}
			}

			/*determine region*/
			auto pos = globalDatacenterList.at(nearestDC).name.find_first_of("-");
			auto region = globalDatacenterList.at(nearestDC).name.substr(pos + 1, 2); // e.g. extract "ap" from "ec2-ap-northeast-1"

			/*add to cluster based on region*/
			clientCluster[region].push_back(c);
		}
		cout << clientCluster.size() << " client clusters are created\n";
		for (auto & cluster : clientCluster)
		{
			cout << cluster.first << ": " << cluster.second.size() << " clients\n";
			if (cluster.second.empty())
			{
				std::printf("\n***ERROR: some cluster has no clients***\n");
				cin.get();
				return;
			}
		}		
	}	
	

	void MaximumMatchingProblem::NearestAssignmentGrouping()
	{	
		/*reset assignedClients_G*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients_G.clear(); }
		/*reset assignedDatacenter_G*/
		for (auto & client : candidateClients) { client.assignedDatacenter_G = nullptr; }

		/*determine each dc's assignedClients_G*/
		for (auto & client : candidateClients)
		{			
			if (!client.eligibleDatacenters_G.empty()) client.assignedDatacenter_G = GetClientNearestEligibleDC(client);
			if (client.assignedDatacenter_G != nullptr) { client.assignedDatacenter_G->assignedClients_G.push_back(&client); }
		}
	}

	void MaximumMatchingProblem::SimpleGreedyGrouping(const int sessionSize, const bool sorting)
	{
		/*reset assignedClients_G*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients_G.clear(); }
		/*reset assignedDatacenter_G*/
		for (auto & client : candidateClients) { client.assignedDatacenter_G = nullptr; }

		/*sort coverableClients_G for each datacenter*/
		if (sorting)
		{
			for (auto & dc : candidateDatacenters)
			{
				std::sort(dc.coverableClients_G.begin(), dc.coverableClients_G.end(), ClientComparatorBy_Fewer_EligibleDatacenters_G);
			}
		}

		/*simple greedy*/
		while (true)
		{
			/*pick the maxDC*/
			auto maxDC = &(candidateDatacenters.front());
			int maxRank = 0;
			for (auto & client : maxDC->coverableClients_G) 
			{ 
				if (nullptr == client->assignedDatacenter_G) { maxRank++; } 
			}
			for (auto & dc : candidateDatacenters)
			{
				int thisRank = 0;
				for (auto & client : dc.coverableClients_G) 
				{ 
					if (nullptr == client->assignedDatacenter_G) { thisRank++; }
				}

				if (thisRank > maxRank)
				{
					maxRank = thisRank;
					maxDC = &dc; 
				}
			}

			/*determine how many unassigned clients to assign in this round*/
			double unassignedClients_GInMaxDC = (double)maxRank;
			int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_GInMaxDC / sessionSize) * sessionSize);			
			if (0 == clientsToBeGroupedInMaxDC) { break; }

			/*group (assign) clients in the maxDC*/		
			for (auto client : maxDC->coverableClients_G)
			{				
				/*group (assign) one not-yet-grouped client*/
				if (clientsToBeGroupedInMaxDC > 0)
				{
					if (nullptr == client->assignedDatacenter_G)
					{
						client->assignedDatacenter_G = maxDC;
						maxDC->assignedClients_G.push_back(client);
						clientsToBeGroupedInMaxDC--;						
					}
				}
				else break;
			}
		}
	}

	void MaximumMatchingProblem::LayeredGreedyGrouping(const int sessionSize)
	{
		/*reset assignedClients_G*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients_G.clear(); }
		/*reset assignedDatacenter_G*/
		for (auto & client : candidateClients) { client.assignedDatacenter_G = nullptr; }

		/*sort coverableClients_G for each dc*/
		for (auto & dc : candidateDatacenters)
		{
			map<vector<DatacenterType*>, vector<ClientType*>, DatacenterPointerVectorCmp> clientSectors;
			for (auto & c : dc.coverableClients_G) { clientSectors[c->eligibleDatacenters_G].push_back(c); }

			/*cout << clientSectors.size() << " sectors\n";
			cout << "original coverableClients_G: { ";
			for (auto & c : dc.coverableClients_G) { cout << c->id << " "; }
			cout << "}\n";*/

			dc.coverableClients_G.clear();
			for (auto & sector : clientSectors) 
			{ 					
				for (auto & c : sector.second) 
				{ 
					dc.coverableClients_G.push_back(c);

					/*cout << c->id << ": { ";
					for (auto & it : c->eligibleDatacenters_G) { cout << it->id << " "; }
					cout << "}\n";*/
				}
			}

			/*cout << "sorted coverableClients_G: { ";
			for (auto & c : dc.coverableClients_G) { cout << c->id << " "; }
			cout << "}\n";
			cin.get();*/
		}

		/*layered greedy*/
		for (size_t layerIndex = 1; layerIndex <= candidateClients.size(); layerIndex++)
		{
			while (true)
			{
				/*pick the maxDC*/
				auto maxDC = &(candidateDatacenters.front());
				int maxRank = 0;
				for (auto & client : maxDC->coverableClients_G)
				{
					if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G.size() <= layerIndex) { maxRank++; }
				}
				for (auto & dc : candidateDatacenters)
				{
					int thisRank = 0;
					for (auto & client : dc.coverableClients_G)
					{
						if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G.size() <= layerIndex) { thisRank++; }
					}

					if (thisRank > maxRank)
					{ 						
						maxRank = thisRank;
						maxDC = &dc;						
					}
				}

				/*determine how many unassigned clients to assign in this round*/
				double unassignedClients_GInMaxDC = (double)maxRank;
				int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_GInMaxDC / sessionSize) * sessionSize);
				if (0 == clientsToBeGroupedInMaxDC) { break; }

				/*group (assign) clients in the maxDC*/
				for (auto & client : maxDC->coverableClients_G)
				{					
					if (clientsToBeGroupedInMaxDC > 0)
					{
						if (nullptr == client->assignedDatacenter_G)
						{
							client->assignedDatacenter_G = maxDC;
							maxDC->assignedClients_G.push_back(client);
							clientsToBeGroupedInMaxDC--;
						}
					}
					else break;
				}
			}
		}
	}

	DatacenterType* MaximumMatchingProblem::GetClientNearestEligibleDC(ClientType & client)
	{
		if (client.eligibleDatacenters_G.empty())
			return nullptr;

		auto nearest = client.eligibleDatacenters_G.front();
		for (auto & edc : client.eligibleDatacenters_G)
		{
			if (client.delayToDatacenter.at(edc->id) < client.delayToDatacenter.at(nearest->id))
			{
				nearest = edc;
			}
		}
		return nearest;
	}
	
	void MaximumMatchingProblem::Simulate(const string algToRun, const int clientCount, const int latencyThreshold, const int sessionSize, const int simulationCount)
	{
		/*fixing the random seed such that every run of the Simulate() will have the same random candidateClients in each round*/
		srand(0);

		/*handle bad parameters*/
		if (clientCount < sessionSize)
		{
			std::printf("\n***ERROR: clientCount < sessionSize***\n");
			cin.get();
			return;
		}

		/*stuff to record performance*/
		vector<double> eligibleRate;
		vector<double> groupedRate;
		vector<double> groupingTime;

		/*run simulation round by round*/
		for (int round = 1; round <= simulationCount; round++)
		{
			/*generate candidateDatacenters (copy from globalDatacenterList)*/
			candidateDatacenters = globalDatacenterList;

			/*generate eligible candidateClients (copy from globalClientList)*/
			candidateClients.clear();
			while (candidateClients.size() < clientCount)
			{
				auto oneClient = globalClientList.at(GenerateRandomIndex(globalClientList.size()));
				for (auto & dc : candidateDatacenters)
				{
					if (oneClient.delayToDatacenter.at(dc.id) <= latencyThreshold)
					{
						candidateClients.push_back(oneClient);
						break;
					}
				}
			}

			/*update eligibleDatacenter_G and coverableClient_G*/
			for (auto & client : candidateClients)
			{
				for (auto & dc : candidateDatacenters)
				{
					if (client.delayToDatacenter.at(dc.id) <= latencyThreshold)
					{
						client.eligibleDatacenters_G.push_back(&dc);
						dc.coverableClients_G.push_back(&client);
					}
				}
			}

			/*all candidate clients are eligible clients*/
			eligibleRate.push_back(1.0);

			/*run grouping algorithm*/
			auto timeStart = clock();
			if ("nearest" == algToRun) NearestAssignmentGrouping();
			else if ("simple" == algToRun) SimpleGreedyGrouping(sessionSize);
			else if ("layered" == algToRun) LayeredGreedyGrouping(sessionSize);
			else std::printf("invalid algoritm name!\n");
			groupingTime.push_back((double)difftime(clock(), timeStart));

			/*get the result of this round*/
			double totalGroupedClients = 0;
			for (auto & dc : candidateDatacenters) { totalGroupedClients += std::floor((double)dc.assignedClients_G.size() / sessionSize) * sessionSize; }
			groupedRate.push_back(totalGroupedClients / clientCount);
		}

		/*dump to disk*/
		outFile << clientCount << "," << GetMeanValue(eligibleRate) << "," << GetMeanValue(groupedRate) << "," << GetMeanValue(groupingTime) << "\n";
		std::printf("%d,%.2f,%.2f,%.2f\n", clientCount, GetMeanValue(eligibleRate), GetMeanValue(groupedRate), GetMeanValue(groupingTime));
	}

	void MaximumMatchingProblem::CountConnectivity(const int latencyThreshold)
	{		
		auto globalDatacenterList_copy = globalDatacenterList; // to avoid modifying the original one
		auto globalClientList_copy = globalClientList; // to avoid modifying the original one
		
		map<int, int> eligibleDcCountHistogram;
		for (int eligibleDc_G_count = 0; eligibleDc_G_count <= globalDatacenterList_copy.size(); eligibleDc_G_count++)
		{
			eligibleDcCountHistogram[eligibleDc_G_count] = 0; // initialize
		}

		for (auto & client : globalClientList_copy)
		{
			for (auto & dc : globalDatacenterList_copy)
			{
				if (client.delayToDatacenter.at(dc.id) <= latencyThreshold)
				{
					client.eligibleDatacenters_G.push_back(&dc);
				}
			}

			eligibleDcCountHistogram.at((int)client.eligibleDatacenters_G.size())++;
			//cout << (int)client.eligibleDatacenters_G.size() << "," << eligibleDcCountHistogram.at((int)client.eligibleDatacenters_G.size()) << "\n";
		}

		for (auto & it : eligibleDcCountHistogram)
		{
			outFile << it.first << "," << it.second << "\n";
		}
	}

	double ParetoMatchingProblem::SearchEligibleDatacenters4Clients(const int latencyThreshold)
	{					
		if (candidateDatacenters.empty())
		{
			std::printf("\n***ERROR: candidateDatacenters is empty***\n");
			cin.get();
			return 0;
		}
		
		for (auto & client : globalClientList)
		{			
			/*remember to reset*/
			client.eligibleDatacenters_G.clear();
			client.eligibleDatacenters_R.clear();
			client.eligibleDatacenters_R_indexed_by_G.clear();
			client.eligibleDatacenters_G_indexed_by_R.clear();
			
			for (auto & dc_g : candidateDatacenters)
			{
				vector<DatacenterType*> eligibleDatacenters_R_indexed_by_G;
				for (auto & dc_r : candidateDatacenters)
				{
					if ((client.delayToDatacenter.at(dc_r.id) + dc_r.delayToDatacenter.at(dc_g.id)) < latencyThreshold)
					{
						client.eligibleDatacenters_R.push_back(&dc_r);
						eligibleDatacenters_R_indexed_by_G.push_back(&dc_r);
					}
				}
				if (!eligibleDatacenters_R_indexed_by_G.empty())
				{
					client.eligibleDatacenters_G.push_back(&dc_g);
					client.eligibleDatacenters_R_indexed_by_G[dc_g.id] = eligibleDatacenters_R_indexed_by_G;

					for (auto & dc_r_by_g : client.eligibleDatacenters_R_indexed_by_G.at(dc_g.id))
					{
						client.eligibleDatacenters_G_indexed_by_R[dc_r_by_g->id].push_back(&dc_g);
					}
				}
			}				
		}

		/*dump eligible client distribution to file*/
		auto dataFile = ofstream(outputDirectory + std::to_string(latencyThreshold) + ".eligibleClientDist" + ".csv");
		map<int, int> eligibleClientDist;
		for (const auto & d : globalDatacenterList)
		{
			eligibleClientDist[d.id] = 0;
		}
		for (const auto & c : globalClientList)
		{
			for (const auto & d : globalDatacenterList)
			{				
				for (const auto & d_r : c.eligibleDatacenters_R)
				{
					if (d.id == d_r->id) 
					{
						eligibleClientDist.at(d.id)++;
						break;
					}
				}
			}
		}
		for (const auto & it : eligibleClientDist)
		{
			dataFile << it.first << "," << it.second << "\n";
		}
		dataFile.close();		

		/*eligible client rate*/
		double totalEligibleClientCount = 0;
		for (const auto & client : globalClientList)
		{
			if (!client.eligibleDatacenters_G.empty()) { totalEligibleClientCount++; }
		}
		double eligibleClientRate = totalEligibleClientCount / globalClientList.size();
		dataFile = ofstream(outputDirectory + std::to_string(latencyThreshold) + ".eligibleClientRate" + ".csv");
		dataFile << eligibleClientRate;
		dataFile.close();

		return eligibleClientRate;
	}

	/*each generated client is guaranteed to have at least one eligible G datacenter*/
	vector<ClientType> ParetoMatchingProblem::GenerateCandidateClients(const int clientCount, const bool regionControl)
	{
		vector<ClientType> candidateClientList;

		if (regionControl)
		{
			while (candidateClientList.size() < clientCount)
			{
				for (auto & cluster : clientCluster)
				{
					if (candidateClientList.size() < clientCount)
					{
						while (true)
						{
							auto clientIndex = GenerateRandomIndex((int)cluster.second.size());
							auto oneClient = globalClientList.at(cluster.second.at(clientIndex));
							if (!oneClient.eligibleDatacenters_G.empty())
							{
								candidateClientList.push_back(oneClient);
								break;
							}
						}
					}					
				}
			}
		}
		else
		{
			while (candidateClientList.size() < clientCount)
			{
				auto clientIndex = GenerateRandomIndex((int)globalClientList.size());
				auto oneClient = globalClientList.at(clientIndex);
				if (!oneClient.eligibleDatacenters_G.empty()) 
				{ 
					candidateClientList.push_back(oneClient); 
				}
			}
		}

		/*double-check*/
		for (auto & client : candidateClientList)
		{
			if (client.eligibleDatacenters_G.empty())
			{
				std::printf("\n***ERROR: eligibleDatacenters_G is empty***\n");
				cin.get();
			}

			if (client.eligibleDatacenters_R.empty()) 
			{ 
				std::printf("\n***ERROR: eligibleDatacenters_R is empty***\n");
				cin.get();
			}

			if (client.eligibleDatacenters_R_indexed_by_G.empty())
			{
				std::printf("\n***ERROR: eligibleDatacenters_R_indexed_by_G is empty***\n");
				cin.get();
			}
			else
			{
				for (auto & dc_g : client.eligibleDatacenters_G)
				{
					if (client.eligibleDatacenters_R_indexed_by_G.at(dc_g->id).empty())
					{
						std::printf("\n***ERROR: eligibleDatacenters_R_indexed_by_G.at(%d) is empty***\n", dc_g->id);
						cin.get();
					}
				}
			}

			if (client.eligibleDatacenters_G_indexed_by_R.empty())
			{
				std::printf("\n***ERROR: eligibleDatacenters_G_indexed_by_R is empty***\n");
				cin.get();
			}
			else
			{
				for (auto & dc_r : client.eligibleDatacenters_R)
				{
					if (client.eligibleDatacenters_G_indexed_by_R.at(dc_r->id).empty())
					{
						std::printf("\n***ERROR: eligibleDatacenters_G_indexed_by_R.at(%d) is empty***\n", dc_r->id);
						cin.get();
					}
				}
			}
		}

		/*just in case*/
		if (candidateClientList.size() != clientCount)
		{
			std::printf("\n***ERROR: candidateClientList.size() != clientCount***\n");
			cin.get();
			return vector<ClientType>();
		}

		return candidateClientList;
	}

	void ParetoMatchingProblem::ResetStageFlag()
	{
		Assignment_G_Completed = false;
		Assignment_R_Completed = false;
		Session_Making_Completed = false;
	}

	void ParetoMatchingProblem::Assignment_Nearest(const int sessionSize)
	{
		/*ensure not yet assigned*/
		if (Assignment_G_Completed || Assignment_R_Completed)
		{
			std::printf("\n***ERROR: already assigned to G***\n");
			cin.get();
			return;
		}

		/*reset*/
		Reset_G_Assignment();
		Reset_R_Assignment();

		/*G and R assignment for each client*/
		for (auto & client : candidateClients)
		{
			// using eligibleDatacenters_G for G assignment
			client.assignedDatacenter_G = client.eligibleDatacenters_G.front();
			for (const auto & dc_g : client.eligibleDatacenters_G)
			{
				if (globalClientList.at(client.id).delayToDatacenter.at(dc_g->id) < globalClientList.at(client.id).delayToDatacenter.at(client.assignedDatacenter_G->id))
				{
					client.assignedDatacenter_G = dc_g; // choose the nearer one
				}
			}
			client.assignedDatacenter_G->assignedClients_G.push_back(&client);

			// R is the same as G
			client.assignedDatacenter_R = client.assignedDatacenter_G;
			client.assignedDatacenter_R->assignedClients_R.push_back(&client);
		}

		/*select clients that can be finally grouped according to the sessionSize, and update all the related stuff*/
		for (auto & dc : candidateDatacenters)
		{
			// only the first m clients can be grouped to this dc
			int m = int(std::floor((double)dc.assignedClients_G.size() / sessionSize) * sessionSize);

			// reset the R and G of each client from m to the last
			for (int i = m; i < dc.assignedClients_G.size(); i++)
			{
				dc.assignedClients_G.at(i)->assignedDatacenter_G = nullptr;
				dc.assignedClients_R.at(i)->assignedDatacenter_R = nullptr;
			}

			// now assignedClients_G and assignedClients_R only contain the first m clients
			dc.assignedClients_G.assign(dc.assignedClients_G.begin(), dc.assignedClients_G.begin() + m);
			dc.assignedClients_R.assign(dc.assignedClients_R.begin(), dc.assignedClients_R.begin() + m);
		}

		/*marked*/
		Assignment_G_Completed = true;
		Assignment_R_Completed = true;
	}	
	
	/*G_Assignment_Simple does not guarrantee that each client will be assigned to an G*/
	void ParetoMatchingProblem::G_Assignment_Simple(const int sessionSize, const int serverCapacity, const string sortingMode)
	{			
		/*ensure not yet assigned*/
		if (Assignment_G_Completed)
		{
			std::printf("\n***ERROR: already assigned to G***\n");
			cin.get();
			return;
		}
		
		Reset_G_Assignment();
		
		/*search coverableClients_G*/
		for (auto & dc_g : candidateDatacenters)
		{			
			dc_g.coverableClients_G.clear();
			if (Assignment_R_Completed)
			{
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).begin(), client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).end(), &dc_g) != client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).end())
					{
						dc_g.coverableClients_G.push_back(&client);
					}
				}
						
				// sorting
				if (sortingMode == "PriceAscending") std::sort(dc_g.coverableClients_G.begin(), dc_g.coverableClients_G.end(), ClientComparatorByAssigned_R_ServerPrice);
				else if (sortingMode == "LayerAscending") std::sort(dc_g.coverableClients_G.begin(), dc_g.coverableClients_G.end(), ClientComparatorBy_Fewer_EligibleDatacenters_G);
				else if (sortingMode == "LayerDescending") std::sort(dc_g.coverableClients_G.begin(), dc_g.coverableClients_G.end(), ClientComparatorBy_More_EligibleDatacenters_G);				
			}
			else
			{
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_G.begin(), client.eligibleDatacenters_G.end(), &dc_g) != client.eligibleDatacenters_G.end())
					{
						dc_g.coverableClients_G.push_back(&client);
					}
				}

				// sorting
				if (sortingMode == "LayerAscending") std::sort(dc_g.coverableClients_G.begin(), dc_g.coverableClients_G.end(), ClientComparatorBy_Fewer_EligibleDatacenters_G);
				else if (sortingMode == "LayerDescending") std::sort(dc_g.coverableClients_G.begin(), dc_g.coverableClients_G.end(), ClientComparatorBy_More_EligibleDatacenters_G);
			}			
		}
			
		if (Assignment_R_Completed && sortingMode == "CostIncrease")
		{
			/*stuff for calculating potential_cost_increase for each client*/
			map<int, int> assignedClientCount_R;
			for (auto & dc_r : candidateDatacenters) { assignedClientCount_R[dc_r.id] = 0; }

			/*G-Assignment*/			
			while (true)
			{
				/*pick the maxDC*/
				auto maxDC = &(candidateDatacenters.front());
				int highestRank = 0;
				for (auto & client : maxDC->coverableClients_G)
				{
					if (nullptr == client->assignedDatacenter_G) { highestRank++; }
				}
				for (auto & dc : candidateDatacenters)
				{
					int thisRank = 0;
					for (auto & client : dc.coverableClients_G)
					{
						if (nullptr == client->assignedDatacenter_G) { thisRank++; }
					}
					if (thisRank > highestRank)
					{
						highestRank = thisRank;
						maxDC = &dc;
					}
				}

				/*determine how many unassigned clients to assign in this round*/
				double unassignedClients_G_InMaxDC = (double)highestRank;
				int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_G_InMaxDC / sessionSize) * sessionSize);
				if (0 == clientsToBeGroupedInMaxDC) { break; }

				/*group the unassigned client that incurs the minimal cost increase at every iteration*/
				while (clientsToBeGroupedInMaxDC != 0)
				{					
					/*compute the potential_cost_increase for each client*/
					for (auto & client : maxDC->coverableClients_G)
					{
						if (nullptr == client->assignedDatacenter_G)
						{
							if (0 == assignedClientCount_R.at(client->assignedDatacenter_R->id))
							{
								client->potential_cost_increase = client->assignedDatacenter_R->priceServer;
							}
							else
							{
								if (assignedClientCount_R.at(client->assignedDatacenter_R->id) % serverCapacity == 0)
									client->potential_cost_increase = client->assignedDatacenter_R->priceServer;
								else
									client->potential_cost_increase = 0;
							}
						}
					}
					
					/*pick the client with minimal potential_cost_increase*/
					auto client_to_be_grouped = maxDC->coverableClients_G.front();
					for (auto & client : maxDC->coverableClients_G) // initialize the client_to_be_grouped by the first unassigned client
					{
						if (nullptr == client->assignedDatacenter_G)
						{
							client_to_be_grouped = client;
							break;
						}
					}
					for (auto & client : maxDC->coverableClients_G) // pick the unassigned client with the minimal potential cost increase
					{
						if (nullptr == client->assignedDatacenter_G) // critical
						{
							if (client->potential_cost_increase < client_to_be_grouped->potential_cost_increase)
							{
								client_to_be_grouped = client;
							}
						}
					}

					/*assign client_to_be_grouped to maxDC*/
					client_to_be_grouped->assignedDatacenter_G = maxDC;
					client_to_be_grouped->assignedDatacenter_G->assignedClients_G.push_back(client_to_be_grouped);
					clientsToBeGroupedInMaxDC--;

					/*increase assignedClientCount_R*/
					assignedClientCount_R.at(client_to_be_grouped->assignedDatacenter_R->id)++;
				}
			}
		}
		else
		{
			/*G-Assignment*/
			while (true)
			{
				/*pick the maxDC*/
				auto maxDC = &(candidateDatacenters.front());
				int highestRank = 0;
				for (auto & client : maxDC->coverableClients_G)
				{
					if (nullptr == client->assignedDatacenter_G) { highestRank++; }
				}
				for (auto & dc : candidateDatacenters)
				{
					int thisRank = 0;
					for (auto & client : dc.coverableClients_G)
					{
						if (nullptr == client->assignedDatacenter_G) { thisRank++; }
					}
					if (thisRank > highestRank)
					{
						highestRank = thisRank;
						maxDC = &dc;
					}
				}

				/*determine how many unassigned clients to assign in this round*/
				double unassignedClients_G_InMaxDC = (double)highestRank;
				int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_G_InMaxDC / sessionSize) * sessionSize);
				if (0 == clientsToBeGroupedInMaxDC) { break; }

				/*group (assign) clients in the maxDC*/
				for (auto & client : maxDC->coverableClients_G)
				{
					/*group (assign) one not-yet-grouped client*/
					if (clientsToBeGroupedInMaxDC > 0)
					{
						if (nullptr == client->assignedDatacenter_G)
						{
							client->assignedDatacenter_G = maxDC;
							client->assignedDatacenter_G->assignedClients_G.push_back(client);
							clientsToBeGroupedInMaxDC--;
						}
					}
					else break;
				}
			}
		}		

		/*marked*/
		Assignment_G_Completed = true;
	}

	/*G_Assignment_Layered does not guarrantee that each client will be assigned to an G*/
	void ParetoMatchingProblem::G_Assignment_Layered(const int sessionSize, const int serverCapacity, const string sortingMode)
	{	
		/*ensure not yet assigned*/
		if (Assignment_G_Completed)
		{
			std::printf("\n***ERROR: already assigned to G***\n");
			cin.get();
			return;
		}
		
		Reset_G_Assignment();	
		
		if (Assignment_R_Completed)
		{
			/*search coverableClients_G*/
			for (auto & dc_g : candidateDatacenters)
			{				
				dc_g.coverableClients_G.clear();
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).begin(), client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).end(), &dc_g) != client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).end())
					{
						dc_g.coverableClients_G.push_back(&client);
					}
				}

				/*sort coverableClients_G*/
				map<vector<DatacenterType*>, vector<ClientType*>, DatacenterPointerVectorCmp> clientSectors;
				for (auto & client : dc_g.coverableClients_G)
				{
					clientSectors[client->eligibleDatacenters_G_indexed_by_R.at(client->assignedDatacenter_R->id)].push_back(client);
				}
				dc_g.coverableClients_G.clear();
				for (auto & sector : clientSectors)
				{
					// extra_sorting_by_R_server_price (only if Assignment_R_Completed)
					if (sortingMode == "PriceAscending") std::sort(sector.second.begin(), sector.second.end(), ClientComparatorByAssigned_R_ServerPrice);

					for (auto & client : sector.second)
					{
						dc_g.coverableClients_G.push_back(client);
					}
				}
			}

			if (sortingMode == "CostIncrease")
			{
				/*stuff for calculating potential_cost_increase for each client*/
				map<int, int> assignedClientCount_R;
				for (auto & dc_r : candidateDatacenters) { assignedClientCount_R[dc_r.id] = 0; }

				/*G-Assignment*/
				for (size_t layerIndex = 1; layerIndex <= candidateClients.size(); layerIndex++)
				{
					while (true)
					{
						/*pick the maxDC*/
						auto maxDC = &(candidateDatacenters.front());
						int maxRank = 0;
						for (auto & client : maxDC->coverableClients_G)
						{
							if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G_indexed_by_R.at(client->assignedDatacenter_R->id).size() <= layerIndex) { maxRank++; }
						}
						for (auto & dc : candidateDatacenters)
						{
							int thisRank = 0;
							for (auto & client : dc.coverableClients_G)
							{
								if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G_indexed_by_R.at(client->assignedDatacenter_R->id).size() <= layerIndex) { thisRank++; }
							}

							if (thisRank > maxRank)
							{
								maxRank = thisRank;
								maxDC = &dc;
							}
						}

						/*determine how many unassigned clients to assign in this round*/
						double unassignedClients_G_InMaxDC = (double)maxRank;
						int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_G_InMaxDC / sessionSize) * sessionSize);
						if (0 == clientsToBeGroupedInMaxDC) { break; }

						/*group the unassigned client that incurs the minimal cost increase at every iteration*/
						while (clientsToBeGroupedInMaxDC != 0)
						{
							/*compute the potential_cost_increase for each client*/
							for (auto & client : maxDC->coverableClients_G)
							{
								if (nullptr == client->assignedDatacenter_G)
								{
									if (0 == assignedClientCount_R.at(client->assignedDatacenter_R->id))
									{
										client->potential_cost_increase = client->assignedDatacenter_R->priceServer;
									}
									else
									{
										if (assignedClientCount_R.at(client->assignedDatacenter_R->id) % serverCapacity == 0)
											client->potential_cost_increase = client->assignedDatacenter_R->priceServer;
										else
											client->potential_cost_increase = 0;
									}
								}
							}

							/*pick the client with minimal potential_cost_increase*/
							ClientType * client_to_be_grouped = maxDC->coverableClients_G.front();
							for (auto & client : maxDC->coverableClients_G) // initialize the client_to_be_grouped by the first unassigned client
							{
								if (nullptr == client->assignedDatacenter_G)
								{
									client_to_be_grouped = client;
									break;
								}
							}
							for (auto & client : maxDC->coverableClients_G) // pick the client with the minimal potential cost increase
							{
								if (nullptr == client->assignedDatacenter_G) // critical
								{
									if (client->potential_cost_increase < client_to_be_grouped->potential_cost_increase)
									{
										client_to_be_grouped = client;
									}
								}
							}

							/*assign client_to_be_grouped to maxDC*/
							client_to_be_grouped->assignedDatacenter_G = maxDC;
							client_to_be_grouped->assignedDatacenter_G->assignedClients_G.push_back(client_to_be_grouped);
							clientsToBeGroupedInMaxDC--;

							/*increase assignedClientCount_R*/
							assignedClientCount_R.at(client_to_be_grouped->assignedDatacenter_R->id)++;
						}
					}
				}
			}
			else 
			{
				/*G-Assignment*/
				for (size_t layerIndex = 1; layerIndex <= candidateClients.size(); layerIndex++)
				{
					while (true)
					{
						/*pick the maxDC*/
						auto maxDC = &(candidateDatacenters.front());
						int maxRank = 0;
						for (auto & client : maxDC->coverableClients_G)
						{
							if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G_indexed_by_R.at(client->assignedDatacenter_R->id).size() <= layerIndex) { maxRank++; }
						}
						for (auto & dc : candidateDatacenters)
						{
							int thisRank = 0;
							for (auto & client : dc.coverableClients_G)
							{
								if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G_indexed_by_R.at(client->assignedDatacenter_R->id).size() <= layerIndex) { thisRank++; }
							}

							if (thisRank > maxRank)
							{
								maxRank = thisRank;
								maxDC = &dc;
							}
						}

						/*determine how many unassigned clients to assign in this round*/
						double unassignedClients_G_InMaxDC = (double)maxRank;
						int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_G_InMaxDC / sessionSize) * sessionSize);
						if (0 == clientsToBeGroupedInMaxDC) { break; }

						/*group (assign) clients in the maxDC*/
						for (auto & client : maxDC->coverableClients_G)
						{
							if (clientsToBeGroupedInMaxDC > 0)
							{
								if (nullptr == client->assignedDatacenter_G)
								{
									client->assignedDatacenter_G = maxDC;
									client->assignedDatacenter_G->assignedClients_G.push_back(client);
									clientsToBeGroupedInMaxDC--;
								}
							}
							else break;
						}
					}
				}
			}			
		}
		else
		{
			/*search coverableClients_G*/
			for (auto & dc_g : candidateDatacenters)
			{
				dc_g.coverableClients_G.clear();
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_G.begin(), client.eligibleDatacenters_G.end(), &dc_g) != client.eligibleDatacenters_G.end())
					{
						dc_g.coverableClients_G.push_back(&client);
					}
				}

				/*sort coverableClients_G*/
				map<vector<DatacenterType*>, vector<ClientType*>, DatacenterPointerVectorCmp> clientSectors;
				for (auto & c : dc_g.coverableClients_G)
				{
					clientSectors[c->eligibleDatacenters_G].push_back(c);
				}
				dc_g.coverableClients_G.clear();
				for (auto & sector : clientSectors)
				{
					for (auto & c : sector.second)
					{
						dc_g.coverableClients_G.push_back(c);
					}
				}
			}

			/*G-Assignment*/
			for (size_t layerIndex = 1; layerIndex <= candidateClients.size(); layerIndex++)
			{
				while (true)
				{
					/*pick the maxDC*/
					auto maxDC = &(candidateDatacenters.front());
					int maxRank = 0;
					for (auto & client : maxDC->coverableClients_G)
					{
						if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G.size() <= layerIndex) { maxRank++; }
					}
					for (auto & dc : candidateDatacenters)
					{
						int thisRank = 0;
						for (auto & client : dc.coverableClients_G)
						{
							if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G.size() <= layerIndex) { thisRank++; }
						}

						if (thisRank > maxRank)
						{
							maxRank = thisRank;
							maxDC = &dc;
						}
					}

					/*determine how many unassigned clients to assign in this round*/
					double unassignedClients_G_InMaxDC = (double)maxRank;
					int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_G_InMaxDC / sessionSize) * sessionSize);
					if (0 == clientsToBeGroupedInMaxDC) { break; }

					/*group (assign) clients in the maxDC*/
					for (auto & client : maxDC->coverableClients_G)
					{
						if (clientsToBeGroupedInMaxDC > 0)
						{
							if (nullptr == client->assignedDatacenter_G)
							{
								client->assignedDatacenter_G = maxDC;
								client->assignedDatacenter_G->assignedClients_G.push_back(client);
								clientsToBeGroupedInMaxDC--;
							}
						}
						else break;
					}
				}
			}
		}

		/*marked*/
		Assignment_G_Completed = true;
	}

	void ParetoMatchingProblem::Reset_G_Assignment()
	{
		for (auto & client : candidateClients)
		{
			client.assignedDatacenter_G = nullptr;			
		}
		for (auto & dc : candidateDatacenters)
		{
			dc.coverableClients_G.clear();			
			dc.assignedClients_G.clear();			
		}
	}	

	void ParetoMatchingProblem::R_Assignment_LSP()
	{
		/*ensure not yet assigned*/
		if (Assignment_R_Completed)
		{
			std::printf("\n***ERROR: already assigned to R***\n");
			cin.get();
			return;
		}
		
		Reset_R_Assignment();
		
		if (Assignment_G_Completed)
		{
			for (auto & client : candidateClients)
			{
				if (nullptr != client.assignedDatacenter_G)
				{
					client.assignedDatacenter_R = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).front();
					for (auto & dc_r : client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id))
					{
						if (dc_r->priceServer < client.assignedDatacenter_R->priceServer)
						{
							client.assignedDatacenter_R = dc_r;
						}
					}
					client.assignedDatacenter_R->assignedClients_R.push_back(&client);
				}
			}			
		}
		else
		{
			for (auto & client : candidateClients)
			{
				client.assignedDatacenter_R = client.eligibleDatacenters_R.front();
				for (auto & dc_r : client.eligibleDatacenters_R)
				{
					if (dc_r->priceServer < client.assignedDatacenter_R->priceServer)
					{
						client.assignedDatacenter_R = dc_r;
					}
				}
				client.assignedDatacenter_R->assignedClients_R.push_back(&client);
			}
		}

		/*marked*/
		Assignment_R_Completed = true;
	}

	void ParetoMatchingProblem::R_Assignment_LCW(const int serverCapacity)
	{
		/*ensure not yet assigned*/
		if (Assignment_R_Completed)
		{
			std::printf("\n***ERROR: already assigned to R***\n");
			cin.get();
			return;
		}
		
		Reset_R_Assignment();
		
		/*search coverableClients_R*/
		for (auto & dc_r : candidateDatacenters)
		{			
			dc_r.coverableClients_R.clear();
			if (Assignment_G_Completed)
			{
				for (auto & client : candidateClients)
				{
					if (nullptr != client.assignedDatacenter_G)
					{
						if (std::find(client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).begin(), client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).end(), &dc_r) != client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).end())
						{
							dc_r.coverableClients_R.push_back(&client);
						}
					}
				}				
			}
			else
			{
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_R.begin(), client.eligibleDatacenters_R.end(), &dc_r) != client.eligibleDatacenters_R.end())
					{
						dc_r.coverableClients_R.push_back(&client);
					}
				}
			}
		}

		/*R-Assignment*/
		while (true)
		{				
			/*compute projectedWastage*/
			vector<int> projectedWastage(candidateDatacenters.size(), serverCapacity);
			for (int i = 0; i < candidateDatacenters.size(); i++)
			{											
				int numberOfNewClientsToAssign = 0;
				for (auto & client : candidateDatacenters.at(i).coverableClients_R)
				{
					if (nullptr == client->assignedDatacenter_R)
						numberOfNewClientsToAssign++;
				}	

				if (numberOfNewClientsToAssign > 0)
				{
					if (0 == (numberOfNewClientsToAssign % serverCapacity)) { projectedWastage.at(i) = 0; }
					else { projectedWastage.at(i) = serverCapacity - (numberOfNewClientsToAssign % serverCapacity); }
				}
				else { projectedWastage.at(i) = serverCapacity; }
			}

			/*find the index of the dc with the minimum projectedWastage*/
			int index_assignedDatacenter_R = 0;
			for (int i = 0; i < candidateDatacenters.size(); i++)
			{
				if (projectedWastage.at(i) < projectedWastage.at(index_assignedDatacenter_R)) { index_assignedDatacenter_R = i; }
			}

			/*assign unassigned clients to index_assignedDatacenter_R*/
			int numOfNewlyAssignedClients = 0;
			for (auto & client : candidateDatacenters.at(index_assignedDatacenter_R).coverableClients_R)
			{
				if (nullptr == client->assignedDatacenter_R)
				{
					client->assignedDatacenter_R = &(candidateDatacenters.at(index_assignedDatacenter_R));
					client->assignedDatacenter_R->assignedClients_R.push_back(client);
					numOfNewlyAssignedClients++;
				}
			}

			/*terminate*/
			if (numOfNewlyAssignedClients < 1) { break; }
		}
		
		/*marked*/
		Assignment_R_Completed = true;
	}

	void ParetoMatchingProblem::Reset_R_Assignment()
	{
		for (auto & client : candidateClients)
		{			
			client.assignedDatacenter_R = nullptr;
		}
		for (auto & dc : candidateDatacenters)
		{			
			dc.coverableClients_R.clear();			
			dc.assignedClients_R.clear();
		}
	}
	
	void ParetoMatchingProblem::ClientAssignment(const int sessionSize, const int serverCapacity, const string algFirstStage, const string algSecondStage)
	{
		if ("Assignment_Nearest" == (algFirstStage + algSecondStage)) // when using nearest assignment, G and R are simply collocated, thus there is only one stage
		{
			Assignment_Nearest(sessionSize);
		}
		else
		{
			/*detect invalid algorithm combination*/
			if ("G_Assignment_Simple" == algFirstStage || "G_Assignment_Layered" == algFirstStage)
			{
				if ("G_Assignment_Simple" == algSecondStage || "G_Assignment_Layered" == algSecondStage)
				{
					std::printf("\n***ERROR: invalid assignment algorithm combination***\n");
					cin.get();
					return;
				}
			}
			else
			{
				if ("R_Assignment_LSP" == algSecondStage || "R_Assignment_LCW" == algSecondStage)
				{
					std::printf("\n***ERROR: invalid assignment algorithm combination***\n");
					cin.get();
					return;
				}
			}

			/*first stage*/
			if ("G_Assignment_Simple" == algFirstStage) G_Assignment_Simple(sessionSize, serverCapacity);
			else if ("G_Assignment_Simple_PriceAscending" == algFirstStage) G_Assignment_Simple(sessionSize, serverCapacity, "PriceAscending");
			else if ("G_Assignment_Simple_LayerAscending" == algFirstStage) G_Assignment_Simple(sessionSize, serverCapacity, "LayerAscending");
			else if ("G_Assignment_Simple_LayerDescending" == algFirstStage) G_Assignment_Simple(sessionSize, serverCapacity, "LayerDescending");
			else if ("G_Assignment_Simple_CostIncrease" == algFirstStage) G_Assignment_Simple(sessionSize, serverCapacity, "CostIncrease");
			else if ("G_Assignment_Layered" == algFirstStage) G_Assignment_Layered(sessionSize, serverCapacity);
			else if ("G_Assignment_Layered_PriceAscending" == algFirstStage) G_Assignment_Layered(sessionSize, serverCapacity, "PriceAscending");
			else if ("G_Assignment_Layered_CostIncrease" == algFirstStage) G_Assignment_Layered(sessionSize, serverCapacity, "CostIncrease");
			else if ("R_Assignment_LSP" == algFirstStage) R_Assignment_LSP();
			else if ("R_Assignment_LCW" == algFirstStage) R_Assignment_LCW(serverCapacity);

			/*second stage*/
			if ("G_Assignment_Simple" == algSecondStage) G_Assignment_Simple(sessionSize, serverCapacity);
			else if ("G_Assignment_Simple_PriceAscending" == algSecondStage) G_Assignment_Simple(sessionSize, serverCapacity, "PriceAscending");
			else if ("G_Assignment_Simple_LayerAscending" == algSecondStage) G_Assignment_Simple(sessionSize, serverCapacity, "LayerAscending");
			else if ("G_Assignment_Simple_LayerDescending" == algSecondStage) G_Assignment_Simple(sessionSize, serverCapacity, "LayerDescending");
			else if ("G_Assignment_Simple_CostIncrease" == algSecondStage) G_Assignment_Simple(sessionSize, serverCapacity, "CostIncrease");
			else if ("G_Assignment_Layered" == algSecondStage) G_Assignment_Layered(sessionSize, serverCapacity);
			else if ("G_Assignment_Layered_PriceAscending" == algSecondStage) G_Assignment_Layered(sessionSize, serverCapacity, "PriceAscending");
			else if ("G_Assignment_Layered_CostIncrease" == algSecondStage) G_Assignment_Layered(sessionSize, serverCapacity, "CostIncrease");
			else if ("R_Assignment_LSP" == algSecondStage) R_Assignment_LSP();
			else if ("R_Assignment_LCW" == algSecondStage) R_Assignment_LCW(serverCapacity);
		}
	}
	
	/*a.k.a. Grouping*/
	void ParetoMatchingProblem::Session_Making_After_Assignment(const int sessionSize)
	{
		/*ensure ClientAssignment() is completed*/
		if (!Assignment_G_Completed || !Assignment_R_Completed)
		{
			std::printf("\n***ERROR: cannot run Session_Making_After_Assignment() as ClientAssignment() not yet completed***\n");
			cin.get();
			return;
		}

		/*ensure not yet grouped*/
		if (Session_Making_Completed)
		{
			std::printf("\n***ERROR: already grouped***\n");
			cin.get();
			return;
		}

		/*initialize (reset sessionListPerG)*/
		for (auto & dc_g : candidateDatacenters)
		{
			vector<SessionType> emptySessionList;
			sessionListPerG[dc_g.id] = emptySessionList;
		}

		/*grouping*/
		for (auto & dc_g : candidateDatacenters)
		{			
			SessionType oneSession;
			for (auto & client : candidateClients)
			{				
				if ((nullptr != client.assignedDatacenter_G) && (dc_g.id == client.assignedDatacenter_G->id))
				{
					oneSession.sessionClients.push_back(&client);
					if (oneSession.sessionClients.size() == sessionSize)
					{
						sessionListPerG.at(dc_g.id).push_back(oneSession);
						oneSession.sessionClients.clear(); // reset for next session
					}
				}				
			}
		}

		/*marked*/
		Session_Making_Completed = true;
	}	

	void ParetoMatchingProblem::PerformanceMeasurement(PerformanceType & performanceMeasurement, const string solutionName, const int serverCapacity)
	{
		/*ensure G_Assignment, R_Assignment, and Grouping completed*/
		if (!Assignment_G_Completed || !Assignment_R_Completed || !Session_Making_Completed)
		{
			std::printf("\n***ERROR: Assignment_G_Completed, Assignment_R_Completed, or Session_Making_Completed is false***\n");
			cin.get();
			return;
		}
		
		/*stuff to record*/
		double 
			sessionCount = 0, 
			serverCost = 0, 
			serverUtilization = 0, 
			R_G_colocation_ratio = 0, 
			G_count_allSessions = 0, 
			R_count_perSession = 0;

		/*supporting stuff*/
		double groupedClientCount = 0, serverCount = 0, colocatedSessionCount = 0;
		map<int, double> assignedClientCount_R;		
		for (auto & dc_r : candidateDatacenters) { assignedClientCount_R[dc_r.id] = 0; }		
		vector<double> R_count_list;
		set<int> dc_g_id_set;
		
		/*measurement*/
		for (auto & sessionList : sessionListPerG)
		{
			for (auto & session : sessionList.second)
			{
				sessionCount++;
				for (auto & client : session.sessionClients)
				{
					assignedClientCount_R.at(client->assignedDatacenter_R->id)++;
					groupedClientCount++;

					session.dc_g_id = client->assignedDatacenter_G->id;
					session.dc_r_id_set.insert(client->assignedDatacenter_R->id);					
				}
				if (1 == session.dc_r_id_set.size() && (session.dc_r_id_set.find(session.dc_g_id) != session.dc_r_id_set.end()))
				{
					colocatedSessionCount++;
				}
				R_count_list.push_back((double)session.dc_r_id_set.size());
				dc_g_id_set.insert(session.dc_g_id);
			}
		}
		for (auto & dc_r : candidateDatacenters)
		{
			double serverCount_R = std::ceil(assignedClientCount_R.at(dc_r.id) / serverCapacity);			
			serverCount += serverCount_R;
			serverCost += (serverCount_R * dc_r.priceServer);
		}
		serverUtilization = groupedClientCount / (serverCount * serverCapacity);
		R_G_colocation_ratio = colocatedSessionCount / sessionCount;
		G_count_allSessions = (double)dc_g_id_set.size();
		R_count_perSession = GetMeanValue(R_count_list);
		
		/*record*/
		performanceMeasurement.sessionCountTable[solutionName].push_back(sessionCount);
		performanceMeasurement.serverCostTable[solutionName].push_back(serverCost);
		performanceMeasurement.serverUtilizationTable[solutionName].push_back(serverUtilization);
		performanceMeasurement.R_G_colocation_ratio_table[solutionName].push_back(R_G_colocation_ratio);
		performanceMeasurement.G_count_allSessions_table[solutionName].push_back(G_count_allSessions);
		performanceMeasurement.R_count_perSession_table[solutionName].push_back(R_count_perSession);
	}

	void ParetoMatchingProblem::Simulate(const Setting & sim_setting)
	{	
		auto startTime = clock();
		
		/*fixing the random seed such that every run of the Simulate() will have the same random candidateClients in each round*/
		srand(0);
		
		/*handle bad parameters*/
		if (sim_setting.clientCount < sim_setting.sessionSize)
		{
			std::printf("\n***ERROR: clientCount < sessionSize***\n");
			cin.get();
			return;
		}

		/*consider all datacenters as candidateDatacenters*/
		candidateDatacenters = globalDatacenterList;

		/*search eligible datacenters from candidateDatacenters for every client in globalClientList*/
		double eligibleClientRate = SearchEligibleDatacenters4Clients(sim_setting.latencyThreshold);
		if (eligibleClientRate * globalClientList.size() < sim_setting.clientCount)
		{
			std::printf("\n***ERROR: totalEligibleClientCount < clientCount***\n");
			cin.get();
			return;
		}

		/*staff to record performance*/
		auto performanceMeasurement = PerformanceType();

		/*run after SearchEligibleDatacenters4Clients() because we only consider eligible clients*/
		vector<vector<ClientType>> candidateClients4AllRounds;		
		while (candidateClients4AllRounds.size() < sim_setting.simulationCount)
		{
			candidateClients4AllRounds.push_back(GenerateCandidateClients(sim_setting.clientCount, sim_setting.regionControl));
		}

		/*run simulation round by round (each round corresponds to a set of randomly selected candidateClients)*/
		vector<pair<string, string>> algorithmCombinations;

		algorithmCombinations.push_back({ "Assignment_Nearest", "" });

		algorithmCombinations.push_back({ "G_Assignment_Simple", "R_Assignment_LSP" });
		algorithmCombinations.push_back({ "G_Assignment_Layered", "R_Assignment_LSP" });

		algorithmCombinations.push_back({ "R_Assignment_LSP", "G_Assignment_Simple_PriceAscending" });
		algorithmCombinations.push_back({ "R_Assignment_LSP", "G_Assignment_Layered_PriceAscending" });

		for (const auto & clients4OneRound : candidateClients4AllRounds)
		{			
			candidateClients = clients4OneRound;
			
			for (const auto & one_combination : algorithmCombinations)
			{
				auto algFirstStage = one_combination.first;
				auto algSecondStage = one_combination.second;				

				/*perform assignment and make sessions*/
				ResetStageFlag();
				ClientAssignment(sim_setting.sessionSize, sim_setting.serverCapacity, algFirstStage, algSecondStage);
				Session_Making_After_Assignment(sim_setting.sessionSize);

				/*measure and record performance*/
				auto solutionName = algFirstStage + "." + algSecondStage;
				PerformanceMeasurement(performanceMeasurement, solutionName, sim_setting.serverCapacity);
			}
		}

		/*dump recorded data to disk files*/
		string settingPerTest = outputDirectory + std::to_string(sim_setting.regionControl) + "." + std::to_string(sim_setting.clientCount) + "." + std::to_string(sim_setting.latencyThreshold) + "." + std::to_string(sim_setting.sessionSize) + "." + std::to_string(sim_setting.serverCapacity);
		auto dataFile = ofstream(settingPerTest + ".csv");				
		dataFile << "solutionName,sessionCount,serverCost,serverUtilization,R_G_colocation_ratio,G_count_allSessions,R_count_perSession\n"; // header line
		for (auto & it : performanceMeasurement.sessionCountTable)
		{
			dataFile << it.first << "," 
				<< GetMeanValue(performanceMeasurement.sessionCountTable.at(it.first)) << ","
				<< GetMeanValue(performanceMeasurement.serverCostTable.at(it.first)) / GetMeanValue(performanceMeasurement.sessionCountTable.at(it.first)) << ","
				<< GetMeanValue(performanceMeasurement.serverUtilizationTable.at(it.first)) << ","
				<< GetMeanValue(performanceMeasurement.R_G_colocation_ratio_table.at(it.first)) << ","				
				<< GetMeanValue(performanceMeasurement.G_count_allSessions_table.at(it.first)) << ","
				<< GetMeanValue(performanceMeasurement.R_count_perSession_table.at(it.first)) << "\n";
		}
		dataFile.close();

		/*dump session_count for every run*/
		/*dataFile = ofstream(settingPerTest + ".sessionCount" + ".csv");
		for (auto & it : performanceMeasurement.sessionCountTable)
		{
			dataFile << it.first << ",";
			for (auto & oneCount : it.second)
			{
				dataFile << oneCount << ",";
			}
			dataFile << "\n";
		}
		dataFile.close();*/

		std::printf("\n***Simulate(): %.2f seconds***\n", std::difftime(clock(), startTime) / 1000);
	}
}