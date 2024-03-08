#include "ServerAllocationProblem.h"

namespace ServerAllocationProblem
{
	void FindnearestDatacenterID(ClientType *client, vector<DatacenterType *> datacenters)
	{
		client->nearestDatacenterID = datacenters.front()->id;
		for (auto dc : datacenters)
		{
			if (client->delayToDatacenter.at(dc->id) < client->delayToDatacenter.at(client->nearestDatacenterID))
			{
				client->nearestDatacenterID = dc->id;
			}
		}
	}

	void ResetEligibiltyCoverability(const vector<ClientType *> clients, const vector<DatacenterType *> datacenters)
	{
		for (auto c : clients)
		{
			c->eligibleDatacenters.clear();
		}

		for (auto d : datacenters)
		{
			d->coverableClients.clear();
		}
	}

	void ResetAssignment(const vector<ClientType *> clients, const vector<DatacenterType *> datacenters)
	{
		for (auto c : clients)
		{
			c->assignedDatacenterID = -1;
		}

		for (auto d : datacenters)
		{
			d->assignedClients.clear();
		}
	}

	// matchmaking for basic problem
	// result: the datacenter for hosting the G-server, and a list of clients to be involved
	// return true if found
	bool Matchmaking4BasicProblem(vector<DatacenterType *> allDatacenters,
								  vector<ClientType *> allClients,
								  int &GDatacenterID,
								  vector<ClientType *> &sessionClients,
								  double SESSION_SIZE,
								  double DELAY_BOUND_TO_G,
								  double DELAY_BOUND_TO_R)
	{
		ResetEligibiltyCoverability(allClients, allDatacenters);
		sessionClients.clear();

		random_shuffle(allDatacenters.begin(), allDatacenters.end());
		for (auto Gdc : allDatacenters)
		{
			random_shuffle(allClients.begin(), allClients.end());
			for (auto client : allClients)
			{
				if ((int)sessionClients.size() == SESSION_SIZE)
				{
					GDatacenterID = Gdc->id;
					return true;
				}
				for (auto dc : allDatacenters)
				{
					if ((client->delayToDatacenter[dc->id] + dc->delayToDatacenter[Gdc->id]) <= DELAY_BOUND_TO_G && client->delayToDatacenter[dc->id] <= DELAY_BOUND_TO_R)
					{
						client->eligibleDatacenters.push_back(dc);
						dc->coverableClients.push_back(client);
					}
				}
				if (!client->eligibleDatacenters.empty())
				{
					sessionClients.push_back(client);
				}
			}
			ResetEligibiltyCoverability(allClients, allDatacenters);
			sessionClients.clear();
		}

		return false;
	}

	// just to find if there are any eligible datacenters to open GS given the input (candidate datacenters, client group, delay bounds)
	// record all of them if found
	// for general problem
	void SearchEligibleGDatacenter(vector<DatacenterType *> allDatacenters,
								   vector<ClientType *> sessionClients,
								   vector<DatacenterType *> &eligibleGDatacenters,
								   double DELAY_BOUND_TO_G,
								   double DELAY_BOUND_TO_R)
	{
		eligibleGDatacenters.clear();

		for (auto GDatacenter : allDatacenters)
		{
			bool isValidGDatacenter = true;

			for (auto client : sessionClients)
			{
				int numEligibleDC = 0;
				for (auto dc : allDatacenters)
				{
					if ((client->delayToDatacenter[dc->id] + dc->delayToDatacenter[GDatacenter->id]) <= DELAY_BOUND_TO_G &&
						client->delayToDatacenter[dc->id] <= DELAY_BOUND_TO_R)
					{
						numEligibleDC++;
					}
				}
				if (numEligibleDC < 1)
				{
					isValidGDatacenter = false; // found a client that has no eligible R-server locations given this G-server location, so this G-server location is invalid
					break;
				}
			}

			if (isValidGDatacenter)
				eligibleGDatacenters.push_back(GDatacenter); // put it into the list
		}
	}

	// result: a list of datacenters that are eligible for hosting the G-server, and a list of clients to be involved
	// return true if found
	// for general problem
	bool Matchmaking4GeneralProblem(vector<DatacenterType *> allDatacenters,
									vector<ClientType *> allClients,
									vector<ClientType *> &sessionClients,
									vector<DatacenterType *> &eligibleGDatacenters,
									double SESSION_SIZE,
									double DELAY_BOUND_TO_G,
									double DELAY_BOUND_TO_R)
	{
		ResetEligibiltyCoverability(allClients, allDatacenters);
		sessionClients.clear();
		eligibleGDatacenters.clear();

		int initialGDatacenter; // just for satisfying MatchmakingBasicProblem's parameters
		if (Matchmaking4BasicProblem(allDatacenters, allClients, initialGDatacenter, sessionClients, SESSION_SIZE, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R))
		{
			SearchEligibleGDatacenter(allDatacenters, sessionClients, eligibleGDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R);
		}

		return (!sessionClients.empty() && !eligibleGDatacenters.empty());
	}

	// used inside each strategy function
	// for general problem
	void SimulationSetup4GeneralProblem(DatacenterType *GDatacenter, vector<ClientType *> sessionClients, vector<DatacenterType *> allDatacenters, double DELAY_BOUND_TO_G, double DELAY_BOUND_TO_R)
	{
		ResetEligibiltyCoverability(sessionClients, allDatacenters);

		for (auto client : sessionClients) // find eligible datacenters for each client and coverable clients for each dc
		{
			for (auto dc : allDatacenters)
			{
				if ((client->delayToDatacenter[dc->id] + dc->delayToDatacenter[GDatacenter->id]) <= DELAY_BOUND_TO_G && client->delayToDatacenter[dc->id] <= DELAY_BOUND_TO_R)
				{
					// client->eligibleDatacenterList.push_back(tuple<int, double, double, double, double>(dc->id, client->delayToDatacenter[dc->id], dc->priceServer, dc->priceBandwidth, dc->priceCombined)); // record eligible dc's id, delay, priceServer, priceBandwidth, priceCombined
					client->eligibleDatacenters.push_back(dc);

					// dc->coverableClientList.push_back(client->id);
					dc->coverableClients.push_back(client);
				}
			}
		}
	}

	// function to get the solution output
	// return <cost_total, cost_server, cost_bandwidth, capacity_wastage, average_delay>
	tuple<double, double, double, double, double> GetSolutionOutput(
		vector<DatacenterType *> allDatacenters,
		double serverCapacity,
		vector<ClientType *> sessionClients,
		int GDatacenterID)
	{
		double costServer = 0, costBandwidth = 0, numberServers = 0;
		for (auto dc : allDatacenters)
		{
			dc->openServerCount = ceil(double(dc->assignedClients.size()) / serverCapacity);
			costServer += dc->priceServer * dc->openServerCount;
			double totalChargedTrafficVolume = 0;
			for (auto client : dc->assignedClients)
			{
				totalChargedTrafficVolume += client->chargedTrafficVolume;
			}
			costBandwidth += dc->priceBandwidth * totalChargedTrafficVolume;
			numberServers += dc->openServerCount;
		}

		double totalDelay = 0;
		for (auto client : sessionClients)
		{
			totalDelay += client->delayToDatacenter[client->assignedDatacenterID] + allDatacenters.at(client->assignedDatacenterID)->delayToDatacenter[GDatacenterID];
		}

		return tuple<double, double, double, double, double>(
			costServer + costBandwidth,
			costServer,
			costBandwidth,
			(numberServers * serverCapacity - sessionClients.size()) / sessionClients.size(),
			totalDelay / sessionClients.size());
	}

	// return true if and only if all clients are assigned and each client is assigned to one dc
	bool CheckIfAllClientsExactlyAssigned(vector<ClientType *> sessionClients, vector<DatacenterType *> allDatacenters)
	{
		for (auto c : sessionClients)
		{
			bool validAssignment = false;
			for (auto edc : c->eligibleDatacenters)
			{
				if (c->assignedDatacenterID == edc->id)
				{
					validAssignment = true;
					break;
				}
			}
			if (!validAssignment)
				return false;
		}

		int totalAssignedClientCount = 0;
		for (auto d : allDatacenters)
		{
			// totalAssignedClientCount += (int)d->assignedClientList.size();
			totalAssignedClientCount += (int)d->assignedClients.size();
		}
		return (totalAssignedClientCount == (int)sessionClients.size());
	}

	bool ClientComparatorByTrafficVolume(const ClientType *A, const ClientType *B)
	{
		return (A->chargedTrafficVolume < B->chargedTrafficVolume);
	}

	bool Initialize(string dataDirectory, vector<ClientType *> &allClients, vector<DatacenterType *> &allDatacenters)
	{
		/*datasets of CCR'13 and TCC'15 (used in our MM'16 paper)*/
		string ClientDatacenterLatencyFile = "dc_to_pl_rtt.csv";
		string InterDatacenterLatencyFile = "dc_to_dc_rtt.csv";
		string BandwidthServerPricingFile = "dc_pricing_bandwidth_server.csv";

		/*datasets of ours*/
		/*string ClientDatacenterLatencyFile = "ping_to_prefix_median_matrix.csv";
		string InterDatacenterLatencyFile = "ping_to_dc_median_matrix.csv";
		string BandwidthServerPricingFile = "pricing_bandwidth_server.csv";*/

		/* temporary stuff */
		vector<vector<double>> ClientToDatacenterDelayMatrix;
		vector<vector<double>> InterDatacenterDelayMatrix;
		vector<double> priceServerList;
		vector<double> priceBandwidthList;

		/* client-to-dc latency data */
		auto strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + ClientDatacenterLatencyFile, ',', true);
		if (strings_read.empty())
		{
			printf("ERROR: empty file!\n");
			cin.get();
			return false;
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

		/*dc-to-dc latency data*/
		strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + InterDatacenterLatencyFile, ',', true);
		if (strings_read.empty())
		{
			printf("ERROR: empty file!\n");
			cin.get();
			return false;
		}
		for (auto row : strings_read)
		{
			vector<double> InterDatacenterDelayMatrixOneRow;
			for (size_t col = 1; col < row.size(); col++)
			{
				InterDatacenterDelayMatrixOneRow.push_back(stod(row.at(col)) / 2);
			}
			InterDatacenterDelayMatrix.push_back(InterDatacenterDelayMatrixOneRow);
		}
		const int totalClientCount = int(ClientToDatacenterDelayMatrix.size());

		/* bandwidth and server price data */
		strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + BandwidthServerPricingFile, ',', true);
		if (strings_read.empty())
		{
			printf("ERROR: empty file!\n");
			cin.get();
			return false;
		}
		for (auto row : strings_read)
		{
			priceBandwidthList.push_back(stod(row.at(1)));
			priceServerList.push_back(stod(row.at(2))); // 2: g2.8xlarge, 3: g2.2xlarge
		}
		const int totalDatacenterCount = int(ClientToDatacenterDelayMatrix.front().size());

		/* create clients */
		allClients.clear();
		for (int i = 0; i < totalClientCount; i++)
		{
			ClientType *client = new ClientType(i);

			client->assignedDatacenterID = -1;
			client->chargedTrafficVolume = 1; // could be revised somewhere before simulation for considering heterogeneity

			for (int j = 0; j < totalDatacenterCount; j++)
			{
				client->delayToDatacenter[j] = ClientToDatacenterDelayMatrix.at(i).at(j);
			}

			allClients.push_back(client);
		}
		// printf("%d clients created according to the input latency data file\n", int(allClients.size()));

		/* create datacenters */
		allDatacenters.clear();
		for (int i = 0; i < totalDatacenterCount; i++)
		{
			DatacenterType *dc = new DatacenterType(i);

			dc->priceServer = priceServerList.at(i);
			dc->priceBandwidth = priceBandwidthList.at(i);

			for (auto client : allClients)
			{
				dc->delayToClient[client->id] = client->delayToDatacenter[dc->id];
			}
			for (int j = 0; j < totalDatacenterCount; j++)
			{
				dc->delayToDatacenter[j] = InterDatacenterDelayMatrix.at(i).at(j);
			}

			allDatacenters.push_back(dc);
		}
		// printf("%d datacenters created according to the input latency data file\n", int(allDatacenters.size()));

		/*find the nearest dc for each client*/
		for (auto client : allClients)
		{
			FindnearestDatacenterID(client, allDatacenters);
		}

		return true;
	}

	void WriteCostWastageDelayData(int STRATEGY_COUNT, vector<double> SERVER_CAPACITY_LIST, double SESSION_COUNT,
								   vector<vector<vector<tuple<double, double, double, double, double>>>> &outcomeAtAllSessions,
								   string outputDirectory, string experimentSettings)
	{
		// record total cost to files
		vector<vector<vector<double>>> costTotalStrategyCapacitySession;
		for (int i = 0; i < STRATEGY_COUNT; i++) // columns
		{
			vector<vector<double>> costTotalCapacitySession;
			for (size_t j = 0; j < SERVER_CAPACITY_LIST.size(); j++) // rows
			{
				vector<double> costTotalSession;
				for (int k = 0; k < SESSION_COUNT; k++) // entries
				{
					// costTotalSession.push_back(get<0>(outcomeAtAllSessions.at(k).at(j).at(i))); // for each session k of this capacity of this strategy (raw data)
					costTotalSession.push_back(get<0>(outcomeAtAllSessions.at(k).at(j).at(i)) / get<0>(outcomeAtAllSessions.at(k).at(j).front())); // normalized by the LB
				}
				costTotalCapacitySession.push_back(costTotalSession); // for each capacity j of this strategy
			}
			costTotalStrategyCapacitySession.push_back(costTotalCapacitySession); // for each strategy i
		}

		ofstream costTotalMeanFile(outputDirectory + experimentSettings + "_" + "costTotalMean.csv");
		ofstream costTotalStdFile(outputDirectory + experimentSettings + "_" + "costTotalStd.csv");
		for (size_t j = 0; j < SERVER_CAPACITY_LIST.size(); j++)
		{
			for (int i = 0; i < STRATEGY_COUNT; i++)
			{
				costTotalMeanFile << GetMeanValue(costTotalStrategyCapacitySession.at(i).at(j)) << ",";
				costTotalStdFile << GetStdValue(costTotalStrategyCapacitySession.at(i).at(j)) << ",";
			}
			costTotalMeanFile << "\n";
			costTotalStdFile << "\n";
		}
		costTotalMeanFile.close();
		costTotalStdFile.close();

		// record capacity wastage ratio
		vector<vector<vector<double>>> capacityWastageStrategyCapacitySession;
		for (int i = 0; i < STRATEGY_COUNT; i++)
		{
			vector<vector<double>> capacityWastageCapacitySession;
			for (size_t j = 0; j < SERVER_CAPACITY_LIST.size(); j++)
			{
				vector<double> capacityWastageSession;
				for (int k = 0; k < SESSION_COUNT; k++)
				{
					capacityWastageSession.push_back(get<3>(outcomeAtAllSessions.at(k).at(j).at(i))); // for each session k of this capacity of this strategy
				}
				capacityWastageCapacitySession.push_back(capacityWastageSession); // for each capacity j of this strategy
			}
			capacityWastageStrategyCapacitySession.push_back(capacityWastageCapacitySession); // for each strategy i
		}

		ofstream capacityWastageMeanFile(outputDirectory + experimentSettings + "_" + "capacityWastageMean.csv");
		ofstream capacityWastageStdFile(outputDirectory + experimentSettings + "_" + "capacityWastageStd.csv");
		for (size_t j = 0; j < SERVER_CAPACITY_LIST.size(); j++)
		{
			for (int i = 0; i < STRATEGY_COUNT; i++)
			{
				capacityWastageMeanFile << GetMeanValue(capacityWastageStrategyCapacitySession.at(i).at(j)) << ",";
				capacityWastageStdFile << GetStdValue(capacityWastageStrategyCapacitySession.at(i).at(j)) << ",";
			}
			capacityWastageMeanFile << "\n";
			capacityWastageStdFile << "\n";
		}
		capacityWastageMeanFile.close();
		capacityWastageStdFile.close();

		// record average delay (client to G-server)
		vector<vector<vector<double>>> averageDelayStrategyCapacitySession;
		for (int i = 0; i < STRATEGY_COUNT; i++)
		{
			vector<vector<double>> averageDelayCapacitySession;
			for (size_t j = 0; j < SERVER_CAPACITY_LIST.size(); j++)
			{
				vector<double> averageDelaySession;
				for (int k = 0; k < SESSION_COUNT; k++)
				{
					averageDelaySession.push_back(get<4>(outcomeAtAllSessions.at(k).at(j).at(i))); // for each session k of this capacity of this strategy
				}
				averageDelayCapacitySession.push_back(averageDelaySession); // for each capacity j of this strategy
			}
			averageDelayStrategyCapacitySession.push_back(averageDelayCapacitySession); // for each strategy i
		}

		ofstream averageDelayMeanFile(outputDirectory + experimentSettings + "_" + "averageDelayMean.csv");
		ofstream averageDelayStdFile(outputDirectory + experimentSettings + "_" + "averageDelayStd.csv");
		for (size_t j = 0; j < SERVER_CAPACITY_LIST.size(); j++)
		{
			for (int i = 0; i < STRATEGY_COUNT; i++)
			{
				averageDelayMeanFile << GetMeanValue(averageDelayStrategyCapacitySession.at(i).at(j)) << +",";
				averageDelayStdFile << GetStdValue(averageDelayStrategyCapacitySession.at(i).at(j)) << +",";
			}
			averageDelayMeanFile << "\n";
			averageDelayStdFile << "\n";
		}
		averageDelayMeanFile.close();
		averageDelayStdFile.close();
	}

	void SimulateBasicProblem(double DELAY_BOUND_TO_G, double DELAY_BOUND_TO_R, double SESSION_SIZE, double SESSION_COUNT)
	{
		// srand((unsigned)time(NULL)); // using current time as the seed for random_shuffle() and rand(), otherwise, they will generate the same sequence of random numbers in every run

		auto t0 = clock(); // start time

		string dataDirectory = ".\\Data\\";
		string outputDirectory = dataDirectory + "ServerAllocationProblemBasic\\";
		_mkdir(outputDirectory.c_str());
		vector<ClientType *> allClients;
		vector<DatacenterType *> allDatacenters;
		if (!Initialize(dataDirectory, allClients, allDatacenters))
		{
			printf("ERROR: simulation initialization failed!\n");
			cin.get();
			return;
		}

		// data structures for storing results
		vector<vector<vector<tuple<double, double, double, double, double>>>> outcomeAtAllSessions;
		vector<vector<vector<double>>> computationAtAllSessions;
		vector<int> eligibleRDatacenterCount;
		vector<int> GDatacenterIDAtAllSessions;
		vector<double> matchmakingTimeAtAllSessions;

		vector<int> sessionCompositionAtAllSessions;
		map<int, int> sessionCompositionStatisticsAtAllSessions;
		for (int i = 0; i < allDatacenters.size(); i++)
		{
			sessionCompositionStatisticsAtAllSessions[i] = 0;
		}

		vector<double> SERVER_CAPACITY_LIST = {2, 4, 6, 8};
		int STRATEGY_COUNT = 8;

		for (int sessionID = 1; sessionID <= SESSION_COUNT; sessionID++)
		{
			vector<ClientType *> sessionClients;
			int GDatacenterID;
			auto matchmakingStartTime = clock();
			bool isFeasibleSession = Matchmaking4BasicProblem(allDatacenters, allClients, GDatacenterID, sessionClients, SESSION_SIZE, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R);
			for (auto client : sessionClients)
			{
				client->chargedTrafficVolume = 1 + (rand() % 5);
			} // each client may have a trafficVolume in the range [1, 5]
			matchmakingTimeAtAllSessions.push_back(difftime(clock(), matchmakingStartTime));
			if (!isFeasibleSession)
			{
				printf("--------------------------------------------------------------------\n");
				printf("ERROR: infeasible session\n");
				printf("total elapsed time: %d seconds\n", (int)(difftime(clock(), t0) / 1000));
				cin.get();
				return;
			}
			GDatacenterIDAtAllSessions.push_back(GDatacenterID);
			for (auto client : sessionClients)
			{
				eligibleRDatacenterCount.push_back((int)client->eligibleDatacenters.size());
			}

			set<int> sessionComposition;
			for (auto client : sessionClients)
			{
				sessionComposition.insert(client->nearestDatacenterID);
				sessionCompositionStatisticsAtAllSessions.at(client->nearestDatacenterID)++;
			}
			sessionCompositionAtAllSessions.push_back((int)sessionComposition.size());

			printf("------------------------------------------------------------------------\n");
			printf("delay bounds: (%d, %d) session size: %d session: %d\n", (int)DELAY_BOUND_TO_G, (int)DELAY_BOUND_TO_R, (int)SESSION_SIZE, sessionID);
			printf("executing strategies\n");

			vector<vector<tuple<double, double, double, double, double>>> outcomeAtOneSession; // per session
			vector<vector<double>> computationAtOneSession;									   // per session

			for (auto serverCapacity : SERVER_CAPACITY_LIST)
			{
				vector<tuple<double, double, double, double, double>> outcomeAtOneCapacity; // per capacity
				vector<double> computationAtOneCapacity;									// per capacity

				for (int strategyID = 1; strategyID <= STRATEGY_COUNT; strategyID++)
				{
					tuple<double, double, double, double, double> outcome;
					auto timePoint = clock();
					switch (strategyID)
					{
					case 1:
						outcome = Alg_LB(sessionClients, allDatacenters, serverCapacity, GDatacenterID);
						break;
					case 2:
						outcome = Alg_RA(sessionClients, allDatacenters, serverCapacity, GDatacenterID);
						break;
					case 3:
						outcome = Alg_NA(sessionClients, allDatacenters, serverCapacity, GDatacenterID);
						break;
					case 4:
						outcome = Alg_LSP(sessionClients, allDatacenters, serverCapacity, GDatacenterID);
						break;
					case 5:
						outcome = Alg_LBP(sessionClients, allDatacenters, serverCapacity, GDatacenterID);
						break;
					case 6:
						outcome = Alg_LCP(sessionClients, allDatacenters, serverCapacity, GDatacenterID);
						break;
					case 7:
						outcome = Alg_LCW(sessionClients, allDatacenters, serverCapacity, GDatacenterID);
						break;
					case 8:
						outcome = Alg_LAC_2(sessionClients, allDatacenters, serverCapacity, GDatacenterID);
						break;
					default:
						outcome = tuple<double, double, double, double, double>(0, 0, 0, 0, 0);
						break;
					}
					computationAtOneCapacity.push_back((double)(clock() - timePoint)); // record computation time per strategy
					outcomeAtOneCapacity.push_back(outcome);						   // record outcome per strategy

					if (!CheckIfAllClientsExactlyAssigned(sessionClients, allDatacenters))
					{
						printf("Something wrong with client-to-datacenter assignment!\n");
						cin.get();
						return;
					}

					cout << "*";
				} // end of strategy loop

				outcomeAtOneSession.push_back(outcomeAtOneCapacity);		 // record outcome per capacity
				computationAtOneSession.push_back(computationAtOneCapacity); // record computation time per capacity

				cout << endl;
			} // end of capacity loop

			outcomeAtAllSessions.push_back(outcomeAtOneSession);		 // per session size
			computationAtAllSessions.push_back(computationAtOneSession); // per session size

			printf("end of executing strategies\n");
		}

		/*******************************************************************************************************/

		string experimentSettings = std::to_string((int)DELAY_BOUND_TO_G) + "_" + std::to_string((int)DELAY_BOUND_TO_R) + "_" + std::to_string((int)SESSION_SIZE);

		// record cost, wastage and delay
		WriteCostWastageDelayData(STRATEGY_COUNT, SERVER_CAPACITY_LIST, SESSION_COUNT, outcomeAtAllSessions, outputDirectory, experimentSettings);

		// record eligible RDatacenter count
		ofstream outFile(outputDirectory + experimentSettings + "_" + "eligibleRDatacenterCount.csv");
		for (auto it : eligibleRDatacenterCount)
		{
			outFile << it << ",";
		}
		outFile.close();

		// record session composition
		/*outFile = ofstream(outputDirectory + experimentSettings + "_" + "sessionComposition.csv");
		for (auto it : sessionCompositionAtAllSessions)
		{
			outFile << it << ",";
		}
		outFile.close();
		outFile = ofstream(outputDirectory + experimentSettings + "_" + "sessionCompositionStatistics.csv");
		for (auto it : sessionCompositionStatisticsAtAllSessions)
		{
			outFile << it.second << ",";
		}
		outFile.close();*/

		// record GDatancenter
		/*outFile = ofstream(outputDirectory + experimentSettings + "_" + "GDatacenterID.csv");
		for (auto it : GDatacenterIDAtAllSessions)
		{
			outFile << it << ",";
		}
		outFile.close();*/

		// record matchmaking's time
		/*outFile = ofstream(outputDirectory + experimentSettings + "_" + "matchmakingTime");
		for (auto it : matchmakingTimeAtAllSessions)
		{
			outFile << it << ",";
		}
		outFile.close();*/

		// record computation time
		vector<vector<vector<double>>> computationStrategyCapacitySession;
		for (int i = 0; i < STRATEGY_COUNT; i++)
		{
			vector<vector<double>> computationCapacitySession;
			for (size_t j = 0; j < SERVER_CAPACITY_LIST.size(); j++)
			{
				vector<double> computationSession;
				for (int k = 0; k < SESSION_COUNT; k++)
				{
					computationSession.push_back(computationAtAllSessions.at(k).at(j).at(i)); // for each session k of this capacity of this strategy
				}
				computationCapacitySession.push_back(computationSession); // for each capacity j of this strategy
			}
			computationStrategyCapacitySession.push_back(computationCapacitySession); // for each strategy i
		}
		ofstream computationMeanFile(outputDirectory + experimentSettings + "_" + "computationMean.csv");
		ofstream computationStdFile(outputDirectory + experimentSettings + "_" + "computationStd.csv");
		for (size_t j = 0; j < SERVER_CAPACITY_LIST.size(); j++)
		{
			for (int i = 0; i < STRATEGY_COUNT; i++)
			{
				computationMeanFile << GetMeanValue(computationStrategyCapacitySession.at(i).at(j)) << ",";
				computationStdFile << GetStdValue(computationStrategyCapacitySession.at(i).at(j)) << ",";
			}
			computationMeanFile << "\n";
			computationStdFile << "\n";
		}
		computationMeanFile.close();
		computationStdFile.close();

		/*******************************************************************************************************/

		printf("------------------------------------------------------------------------\n");
		printf("total elapsed time: %d seconds\n", (int)(difftime(clock(), t0) / 1000)); // elapsed time of the process
		// cin.get();
		return;
	}

	void SimulateGeneralProblem(double DELAY_BOUND_TO_G, double DELAY_BOUND_TO_R, double SESSION_SIZE, double SESSION_COUNT)
	{
		// srand((unsigned)time(NULL)); // using current time as the seed for random_shuffle() and rand(), otherwise, they will generate the same sequence of random numbers in every run

		auto t0 = clock(); // start time

		string dataDirectory = ".\\Data\\";
		string outputDirectory = dataDirectory + "ServerAllocationProblemGeneral\\";
		_mkdir(outputDirectory.c_str());
		vector<ClientType *> allClients;
		vector<DatacenterType *> allDatacenters;
		if (!Initialize(dataDirectory, allClients, allDatacenters))
		{
			printf("ERROR: simulation initialization failed!\n");
			cin.get();
			return;
		}

		// data structures for storing results
		vector<vector<vector<tuple<double, double, double, double, double>>>> outcomeAtAllSessions;
		vector<vector<vector<double>>> computationAtAllSessions;
		vector<vector<vector<int>>> finalGDatacenterAtAllSessions;

		vector<int> eligibleGDatacenterCountAtAllSessions;
		vector<double> matchmakingTimeAtAllSessions;

		map<int, double> serverCountPerDC4LCP;
		map<int, double> serverCountPerDC4LCW;
		map<int, double> serverCountPerDC4LAC;
		for (auto dc : allDatacenters)
		{
			serverCountPerDC4LCP[dc->id] = 0;
			serverCountPerDC4LCW[dc->id] = 0;
			serverCountPerDC4LAC[dc->id] = 0;
		}

		vector<double> SERVER_CAPACITY_LIST = {2, 4, 6, 8};
		int STRATEGY_COUNT = 8;

		for (int sessionID = 1; sessionID <= SESSION_COUNT; sessionID++)
		{
			vector<ClientType *> sessionClients;
			vector<DatacenterType *> eligibleGDatacenters;

			auto matchmakingStartTime = clock();
			bool isFeasibleSession = Matchmaking4GeneralProblem(allDatacenters, allClients, sessionClients, eligibleGDatacenters, SESSION_SIZE, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R);
			for (auto client : sessionClients)
			{
				client->chargedTrafficVolume = double(1 + (rand() % 5));
			} // each client may have a trafficVolume in the range [1, 5]
			matchmakingTimeAtAllSessions.push_back(difftime(clock(), matchmakingStartTime));
			if (!isFeasibleSession)
			{
				printf("------------------------------------------------------------------------\n");
				printf("ERROR: infeasible session\n");
				printf("total elapsed time: %d seconds\n", (int)(difftime(clock(), t0) / 1000));
				cin.get();
				return;
			}

			printf("------------------------------------------------------------------------\n");
			printf("delay bounds: (%d, %d)   session size: %d   session: %d\n", (int)DELAY_BOUND_TO_G, (int)DELAY_BOUND_TO_R, (int)SESSION_SIZE, sessionID);
			printf("start of one session\n");

			vector<vector<tuple<double, double, double, double, double>>> outcomeAtOneSession; // per session
			vector<vector<double>> computationAtOneSession;									   // per session
			vector<vector<int>> finalGDatacenterAtOneSession;								   // per session

			for (auto serverCapacity : SERVER_CAPACITY_LIST)
			{
				vector<tuple<double, double, double, double, double>> outcomeAtOneCapacity; // per capacity
				vector<double> computationAtOneCapacity;									// per capacity
				vector<int> finalGDatacenterAtOneCapacity;									// per capacity

				for (int strategyID = 1; strategyID <= STRATEGY_COUNT; strategyID++)
				{
					tuple<double, double, double, double, double> outcome;
					int finalGDatacenter;
					auto timePoint = clock();
					switch (strategyID)
					{
					case 1:
						outcome = Alg_LB(eligibleGDatacenters, finalGDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, serverCapacity);
						break;
					case 2:
						outcome = Alg_RA(eligibleGDatacenters, finalGDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, serverCapacity);
						break;
					case 3:
						outcome = Alg_NA(eligibleGDatacenters, finalGDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, serverCapacity);
						break;
					case 4:
						outcome = Alg_LSP(eligibleGDatacenters, finalGDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, serverCapacity);
						break;
					case 5:
						outcome = Alg_LBP(eligibleGDatacenters, finalGDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, serverCapacity);
						break;
					case 6:
						outcome = Alg_LCP(eligibleGDatacenters, finalGDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, serverCapacity);
						/*if (8 == serverCapacity)
						{
							for (auto dc : allDatacenters)
							{
								serverCountPerDC4LCP[dc->id] += dc->openServerCount;
							}
						}*/
						break;
					case 7:
						outcome = Alg_LCW(eligibleGDatacenters, finalGDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, serverCapacity);
						/*if (8 == serverCapacity)
						{
							for (auto dc : allDatacenters)
							{
								serverCountPerDC4LCW[dc->id] += dc->openServerCount;
							}
						}*/
						break;
					case 8:
						outcome = Alg_LAC_2(eligibleGDatacenters, finalGDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, serverCapacity);
						/*if (8 == serverCapacity)
						{
							for (auto dc : allDatacenters)
							{
								serverCountPerDC4LAC[dc->id] += dc->openServerCount;
							}
						}*/
						break;
					default:
						outcome = tuple<double, double, double, double, double>(0, 0, 0, 0, 0);
						break;
					}
					computationAtOneCapacity.push_back((double)(clock() - timePoint)); // per strategy
					outcomeAtOneCapacity.push_back(outcome);						   // per strategy
					finalGDatacenterAtOneCapacity.push_back(finalGDatacenter);		   // per strategy

					if (!CheckIfAllClientsExactlyAssigned(sessionClients, allDatacenters))
					{
						printf("Something wrong with the assignment!\n");
						cin.get();
						return;
					}

					cout << "*";
				} // end of strategy loop

				outcomeAtOneSession.push_back(outcomeAtOneCapacity);				   // per capacity
				computationAtOneSession.push_back(computationAtOneCapacity);		   // per capacity
				finalGDatacenterAtOneSession.push_back(finalGDatacenterAtOneCapacity); // per capacity

				cout << endl;
			} // end of capacity loop

			outcomeAtAllSessions.push_back(outcomeAtOneSession);				   // per session
			computationAtAllSessions.push_back(computationAtOneSession);		   // per session
			finalGDatacenterAtAllSessions.push_back(finalGDatacenterAtOneSession); // per session

			eligibleGDatacenterCountAtAllSessions.push_back((int)eligibleGDatacenters.size()); // per session

			printf("end of one session\n");
		} // end of session iteration

		/*******************************************************************************************************/

		string experimentSettings = std::to_string((int)DELAY_BOUND_TO_G) + "_" + std::to_string((int)DELAY_BOUND_TO_R) + "_" + std::to_string((int)SESSION_SIZE);

		// record cost, wastage and delay
		WriteCostWastageDelayData(STRATEGY_COUNT, SERVER_CAPACITY_LIST, SESSION_COUNT, outcomeAtAllSessions, outputDirectory, experimentSettings);

		// record server count at each datacenter in all sessions
		ofstream outfile(outputDirectory + experimentSettings + "_" + "serverCountPerDC.csv");
		for (auto dc : allDatacenters)
		{
			outfile << serverCountPerDC4LCP[dc->id] << ",";
		}
		outfile << "\n";
		for (auto dc : allDatacenters)
		{
			outfile << serverCountPerDC4LCW[dc->id] << ",";
		}
		outfile << "\n";
		for (auto dc : allDatacenters)
		{
			outfile << serverCountPerDC4LAC[dc->id] << ",";
		}
		outfile.close();

		// record final G datacenter
		/*ofstream = outfile(outputDirectory + experimentSettings + "_" + "finalGDatacenterID.csv");
		for (size_t i = 0; i < finalGDatacenterAtAllSessions.size(); i++)
		{
			for (size_t j = 0; j < finalGDatacenterAtAllSessions.at(i).size(); j++)
			{
				for (size_t k = 0; k < finalGDatacenterAtAllSessions.at(i).at(j).size(); k++)
				{
					outfile << finalGDatacenterAtAllSessions.at(i).at(j).at(k) << ",";
				}
				outfile << "\n";
			}
		}
		outfile.close();*/

		// record eligible GDatacenter count
		outfile = ofstream(outputDirectory + experimentSettings + "_" + "eligibleGDatacenterCount.csv");
		for (auto it : eligibleGDatacenterCountAtAllSessions)
		{
			outfile << it << ",";
		}
		outfile.close();

		// record computation time
		vector<vector<vector<double>>> computationStrategyCapacitySession;
		for (int i = 0; i < STRATEGY_COUNT; i++)
		{
			vector<vector<double>> computationCapacitySession;
			for (size_t j = 0; j < SERVER_CAPACITY_LIST.size(); j++)
			{
				vector<double> computationSession;
				for (int k = 0; k < SESSION_COUNT; k++)
				{
					computationSession.push_back(computationAtAllSessions.at(k).at(j).at(i)); // for each session k of this capacity of this strategy
				}
				computationCapacitySession.push_back(computationSession); // for each capacity j of this strategy
			}
			computationStrategyCapacitySession.push_back(computationCapacitySession); // for each strategy i
		}
		ofstream computationMeanFile(outputDirectory + experimentSettings + "_" + "computationMean.csv");
		ofstream computationStdFile(outputDirectory + experimentSettings + "_" + "computationStd.csv");
		for (size_t j = 0; j < SERVER_CAPACITY_LIST.size(); j++)
		{
			for (int i = 0; i < STRATEGY_COUNT; i++)
			{
				computationMeanFile << GetMeanValue(computationStrategyCapacitySession.at(i).at(j)) << ",";
				computationStdFile << GetStdValue(computationStrategyCapacitySession.at(i).at(j)) << ",";
			}
			computationMeanFile << "\n";
			computationStdFile << "\n";
		}
		computationMeanFile.close();
		computationStdFile.close();

		/*******************************************************************************************************/

		printf("------------------------------------------------------------------------\n");
		printf("total elapsed time: %d seconds\n", (int)(difftime(clock(), t0) / 1000)); // elapsed time of the process
		// cin.get();
		return;
	}

	// Lower-Bound (LB)
	// for basic problem
	tuple<double, double, double, double, double> Alg_LB(
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double serverCapacity,
		int GDatacenterID)
	{
		ResetAssignment(sessionClients, allDatacenters);

		for (auto client : sessionClients)
		{
			client->assignedDatacenterID = client->eligibleDatacenters.front()->id;
			for (auto edc : client->eligibleDatacenters)
			{
				double priceCombinedPrevious = allDatacenters.at(client->assignedDatacenterID)->priceServer / serverCapacity + allDatacenters.at(client->assignedDatacenterID)->priceBandwidth * client->chargedTrafficVolume;
				double priceCombinedCurrent = edc->priceServer / serverCapacity + edc->priceBandwidth * client->chargedTrafficVolume;
				if (priceCombinedCurrent < priceCombinedPrevious)
				{
					client->assignedDatacenterID = edc->id;
				}
				else if (priceCombinedCurrent == priceCombinedPrevious)
				{
					if (client->delayToDatacenter.at(edc->id) < client->delayToDatacenter.at(client->assignedDatacenterID))
					{
						client->assignedDatacenterID = edc->id;
					}
				}
			}

			allDatacenters.at(client->assignedDatacenterID)->assignedClients.push_back(client);
		}

		double costServer = 0, costBandwidth = 0, numberServers = 0;
		for (auto dc : allDatacenters)
		{
			dc->openServerCount = double(dc->assignedClients.size()) / serverCapacity;
			numberServers += dc->openServerCount;
			costServer += dc->priceServer * dc->openServerCount;
			double totalChargedTrafficVolume = 0;
			for (auto client : dc->assignedClients)
			{
				totalChargedTrafficVolume += client->chargedTrafficVolume;
			}
			costBandwidth += dc->priceBandwidth * totalChargedTrafficVolume;
		}

		double totalDelay = 0;
		for (auto client : sessionClients)
		{
			totalDelay += client->delayToDatacenter[client->assignedDatacenterID] + allDatacenters.at(client->assignedDatacenterID)->delayToDatacenter[GDatacenterID];
		}

		return tuple<double, double, double, double, double>(
			costServer + costBandwidth,
			costServer,
			costBandwidth,
			0,
			totalDelay / sessionClients.size());
	}

	// Lower-Bound (LB)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LB(
		vector<DatacenterType *> eligibleGDatacenters,
		int &finalGDatacenter,
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double DELAY_BOUND_TO_G,
		double DELAY_BOUND_TO_R,
		double serverCapacity)
	{
		tuple<double, double, double, double, double> finalOutcome;
		double totalCost = INT_MAX;
		int tempFinalGDatacenter = eligibleGDatacenters.front()->id;

		for (auto GDatacenter : eligibleGDatacenters)
		{
			SimulationSetup4GeneralProblem(GDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R); // initilization
			auto tempOutcome = Alg_LB(sessionClients, allDatacenters, serverCapacity, GDatacenter->id);
			double tempTotalCost = get<0>(tempOutcome);

			if (tempTotalCost < totalCost) // choose the smaller cost
			{
				totalCost = tempTotalCost;
				finalOutcome = tempOutcome;
				tempFinalGDatacenter = GDatacenter->id;
			}
		}

		finalGDatacenter = tempFinalGDatacenter;
		return finalOutcome;
	}

	// Random-Assignment
	// for basic problem
	tuple<double, double, double, double, double> Alg_RA(
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double serverCapacity,
		int GDatacenterID)
	{
		ResetAssignment(sessionClients, allDatacenters);

		for (auto client : sessionClients) // choose a dc for each client
		{
			client->assignedDatacenterID = client->eligibleDatacenters.at(GenerateRandomIndex((int)client->eligibleDatacenters.size()))->id;
			allDatacenters.at(client->assignedDatacenterID)->assignedClients.push_back(client);
		}

		return GetSolutionOutput(allDatacenters, serverCapacity, sessionClients, GDatacenterID);
	}

	// Random-Assignment
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_RA(
		vector<DatacenterType *> eligibleGDatacenters,
		int &finalGDatacenter,
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double DELAY_BOUND_TO_G,
		double DELAY_BOUND_TO_R,
		double serverCapacity)
	{
		tuple<double, double, double, double, double> finalOutcome;
		double totalCost = INT_MAX;
		int tempFinalGDatacenter = eligibleGDatacenters.front()->id;

		for (auto GDatacenter : eligibleGDatacenters)
		{
			SimulationSetup4GeneralProblem(GDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R); // initilization
			auto tempOutcome = Alg_RA(sessionClients, allDatacenters, serverCapacity, GDatacenter->id);
			double tempTotalCost = get<0>(tempOutcome);

			if (tempTotalCost < totalCost) // choose the smaller cost
			{
				totalCost = tempTotalCost;
				finalOutcome = tempOutcome;
				tempFinalGDatacenter = GDatacenter->id;
			}
		}

		finalGDatacenter = tempFinalGDatacenter;
		return finalOutcome;
	}

	// Nearest-Assignment
	// for basic problem
	tuple<double, double, double, double, double> Alg_NA(
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double serverCapacity,
		int GDatacenterID)
	{
		ResetAssignment(sessionClients, allDatacenters);

		for (auto client : sessionClients)
		{
			client->assignedDatacenterID = client->eligibleDatacenters.front()->id;
			for (auto edc : client->eligibleDatacenters)
			{
				if (client->delayToDatacenter[edc->id] < client->delayToDatacenter[client->assignedDatacenterID])
				{
					client->assignedDatacenterID = edc->id;
				}
			}

			allDatacenters.at(client->assignedDatacenterID)->assignedClients.push_back(client);
		}

		return GetSolutionOutput(allDatacenters, serverCapacity, sessionClients, GDatacenterID);
	}

	// Nearest-Assignment
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_NA(
		vector<DatacenterType *> eligibleGDatacenters,
		int &finalGDatacenter,
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double DELAY_BOUND_TO_G,
		double DELAY_BOUND_TO_R,
		double serverCapacity)
	{
		tuple<double, double, double, double, double> finalOutcome;
		double totalCost = INT_MAX;
		int tempFinalGDatacenter = eligibleGDatacenters.front()->id;

		for (auto GDatacenter : eligibleGDatacenters)
		{
			SimulationSetup4GeneralProblem(GDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R); // initilization
			auto tempOutcome = Alg_NA(sessionClients, allDatacenters, serverCapacity, GDatacenter->id);
			double tempTotalCost = get<0>(tempOutcome);

			if (tempTotalCost < totalCost) // choose the smaller cost
			{
				totalCost = tempTotalCost;
				finalOutcome = tempOutcome;
				tempFinalGDatacenter = GDatacenter->id;
			}
		}

		finalGDatacenter = tempFinalGDatacenter;
		return finalOutcome;
	}

	// Lowest-Server-Price-Datacenter-Assignment (LSP)
	// for basic problem
	tuple<double, double, double, double, double> Alg_LSP(
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double serverCapacity,
		int GDatacenterID)
	{
		ResetAssignment(sessionClients, allDatacenters);

		for (auto client : sessionClients)
		{
			client->assignedDatacenterID = client->eligibleDatacenters.front()->id;
			for (auto edc : client->eligibleDatacenters)
			{
				if (edc->priceServer < allDatacenters.at(client->assignedDatacenterID)->priceServer)
				{
					client->assignedDatacenterID = edc->id;
				}
				else if (edc->priceServer == allDatacenters.at(client->assignedDatacenterID)->priceServer)
				{
					if (client->delayToDatacenter.at(edc->id) < client->delayToDatacenter.at(client->assignedDatacenterID))
					{
						client->assignedDatacenterID = edc->id;
					}
				}
			}

			allDatacenters.at(client->assignedDatacenterID)->assignedClients.push_back(client);
		}

		return GetSolutionOutput(allDatacenters, serverCapacity, sessionClients, GDatacenterID);
	}

	// Lowest-Bandwidth-Price-Datacenter-Assignment (LBP)
	// for basic problem
	tuple<double, double, double, double, double> Alg_LBP(
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double serverCapacity,
		int GDatacenterID)
	{
		ResetAssignment(sessionClients, allDatacenters);

		auto clock_begin = clock();
		for (auto client : sessionClients)
		{
			client->assignedDatacenterID = client->eligibleDatacenters.front()->id;
			for (auto edc : client->eligibleDatacenters)
			{
				if (edc->priceBandwidth < allDatacenters.at(client->assignedDatacenterID)->priceBandwidth)
				{
					client->assignedDatacenterID = edc->id;
				}
				else if (edc->priceBandwidth == allDatacenters.at(client->assignedDatacenterID)->priceBandwidth)
				{
					if (client->delayToDatacenter.at(edc->id) < client->delayToDatacenter.at(client->assignedDatacenterID))
					{
						client->assignedDatacenterID = edc->id;
					}
				}
			}

			allDatacenters.at(client->assignedDatacenterID)->assignedClients.push_back(client);
		}

		return GetSolutionOutput(allDatacenters, serverCapacity, sessionClients, GDatacenterID);
	}

	// Lowest-Combined-Price-Datacenter-Assignment (LCP)
	// for basic problem
	tuple<double, double, double, double, double> Alg_LCP(
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double serverCapacity,
		int GDatacenterID)
	{
		ResetAssignment(sessionClients, allDatacenters);

		for (auto client : sessionClients)
		{
			client->assignedDatacenterID = client->eligibleDatacenters.front()->id;
			for (auto edc : client->eligibleDatacenters)
			{
				double priceCombinedPrevious = allDatacenters.at(client->assignedDatacenterID)->priceServer / serverCapacity + allDatacenters.at(client->assignedDatacenterID)->priceBandwidth * client->chargedTrafficVolume;
				double priceCombinedCurrent = edc->priceServer / serverCapacity + edc->priceBandwidth * client->chargedTrafficVolume;
				if (priceCombinedCurrent < priceCombinedPrevious)
				{
					client->assignedDatacenterID = edc->id;
				}
				else if (priceCombinedCurrent == priceCombinedPrevious)
				{
					if (client->delayToDatacenter.at(edc->id) < client->delayToDatacenter.at(client->assignedDatacenterID))
					{
						client->assignedDatacenterID = edc->id;
					}
				}
			}

			allDatacenters.at(client->assignedDatacenterID)->assignedClients.push_back(client);
		}

		return GetSolutionOutput(allDatacenters, serverCapacity, sessionClients, GDatacenterID);
	}

	// Lowest-Server-Price-Datacenter-Assignment (LSP)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LSP(
		vector<DatacenterType *> eligibleGDatacenters,
		int &finalGDatacenter,
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double DELAY_BOUND_TO_G,
		double DELAY_BOUND_TO_R,
		double serverCapacity)
	{
		tuple<double, double, double, double, double> finalOutcome;

		double totalCost = INT_MAX;
		int tempFinalGDatacenter = eligibleGDatacenters.front()->id;

		map<int, double> finalServerCountPerDC;

		for (auto GDatacenter : eligibleGDatacenters)
		{
			SimulationSetup4GeneralProblem(GDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R);
			auto tempOutcome = Alg_LSP(sessionClients, allDatacenters, serverCapacity, GDatacenter->id);
			double tempTotalCost = get<0>(tempOutcome);

			map<int, double> tempServerCountPerDC;
			for (auto dc : allDatacenters)
			{
				tempServerCountPerDC[dc->id] = dc->openServerCount;
			}

			if (tempTotalCost < totalCost) // choose the smaller cost
			{
				totalCost = tempTotalCost;

				finalOutcome = tempOutcome;

				tempFinalGDatacenter = GDatacenter->id;

				finalServerCountPerDC = tempServerCountPerDC;
			}
		}

		finalGDatacenter = tempFinalGDatacenter;

		for (auto dc : allDatacenters)
		{
			dc->openServerCount = finalServerCountPerDC[dc->id];
		}

		return finalOutcome;
	}

	// Lowest-Bandwidth-Price-Datacenter-Assignment (LBP)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LBP(
		vector<DatacenterType *> eligibleGDatacenters,
		int &finalGDatacenter,
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double DELAY_BOUND_TO_G,
		double DELAY_BOUND_TO_R,
		double serverCapacity)
	{
		tuple<double, double, double, double, double> finalOutcome;

		double totalCost = INT_MAX;
		int tempFinalGDatacenter = eligibleGDatacenters.front()->id;

		for (auto GDatacenter : eligibleGDatacenters)
		{
			SimulationSetup4GeneralProblem(GDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R);
			auto tempOutcome = Alg_LBP(sessionClients, allDatacenters, serverCapacity, GDatacenter->id);
			double tempTotalCost = get<0>(tempOutcome);

			if (tempTotalCost < totalCost) // choose the smaller cost
			{
				totalCost = tempTotalCost;
				finalOutcome = tempOutcome;
				tempFinalGDatacenter = GDatacenter->id;
			}
		}

		finalGDatacenter = tempFinalGDatacenter;
		return finalOutcome;
	}

	// Lowest-Combined-Price-Datacenter-Assignment (LCP)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LCP(
		vector<DatacenterType *> eligibleGDatacenters,
		int &finalGDatacenter,
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double DELAY_BOUND_TO_G,
		double DELAY_BOUND_TO_R,
		double serverCapacity)
	{
		tuple<double, double, double, double, double> finalOutcome;
		double totalCost = INT_MAX;
		int tempFinalGDatacenter = eligibleGDatacenters.front()->id;

		for (auto GDatacenter : eligibleGDatacenters)
		{
			SimulationSetup4GeneralProblem(GDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R);
			auto tempOutcome = Alg_LCP(sessionClients, allDatacenters, serverCapacity, GDatacenter->id);
			double tempTotalCost = get<0>(tempOutcome);

			if (tempTotalCost < totalCost) // choose the smaller cost
			{
				totalCost = tempTotalCost;
				finalOutcome = tempOutcome;
				tempFinalGDatacenter = GDatacenter->id;
			}
		}

		finalGDatacenter = tempFinalGDatacenter;
		return finalOutcome;
	}

	// Lowest-Capacity-Wastage-Assignment (LCW)
	// if server capacity < 2, reduce to LCP
	// for basic problem
	tuple<double, double, double, double, double> Alg_LCW(
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double serverCapacity,
		int GDatacenterID)
	{
		if (serverCapacity < 2)
			return Alg_LCP(sessionClients, allDatacenters, serverCapacity, GDatacenterID);

		ResetAssignment(sessionClients, allDatacenters);

		while (true)
		{
			vector<DatacenterType *> candidateDatacenters;
			candidateDatacenters.clear();
			for (auto dc : allDatacenters)
			{
				dc->unassignedCoverableClients.clear(); // reset for new iteration

				for (auto client : dc->coverableClients)
				{
					if (client->assignedDatacenterID < 0)
					{
						dc->unassignedCoverableClients.push_back(client);
					}
				}

				if (!dc->unassignedCoverableClients.empty())
				{
					candidateDatacenters.push_back(dc);
				}
			}
			if (candidateDatacenters.empty()) // indicating that no more datacenters that have unassigned coverable clients
			{
				break; // terminate
			}

			DatacenterType *nextDC = candidateDatacenters.front(); // initialization
			for (auto dc : candidateDatacenters)
			{
				double utilization_dc; // compute utilization for the current dc
				int unassignedClientCount_dc = (int)dc->unassignedCoverableClients.size();
				if (unassignedClientCount_dc % (int)serverCapacity == 0)
					utilization_dc = 1;
				else
					utilization_dc = (double)(unassignedClientCount_dc % (int)serverCapacity) / serverCapacity;

				double utilization_nextDC; // compute utilization for the previously selected dc (the one with highest utilization so far)
				int unassignedClientCount_nextDC = (int)nextDC->unassignedCoverableClients.size();
				if (unassignedClientCount_nextDC % (int)serverCapacity == 0)
					utilization_nextDC = 1;
				else
					utilization_nextDC = (double)(unassignedClientCount_nextDC % (int)serverCapacity) / serverCapacity;

				// choose the one with higher projected utilization
				// if two utilizations tie, select the one with lower server price
				if (utilization_dc > utilization_nextDC || (utilization_dc == utilization_nextDC && dc->priceServer < nextDC->priceServer))
				{
					nextDC = dc;
				}
			}

			for (auto client : nextDC->unassignedCoverableClients) // client-to-datacenter assignment
			{
				client->assignedDatacenterID = nextDC->id;
				nextDC->assignedClients.push_back(client);
			}
		}

		return GetSolutionOutput(allDatacenters, serverCapacity, sessionClients, GDatacenterID);
	}

	// Lowest-Capacity-Wastage-Assignment (LCW)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LCW(
		vector<DatacenterType *> eligibleGDatacenters,
		int &finalGDatacenter,
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double DELAY_BOUND_TO_G,
		double DELAY_BOUND_TO_R,
		double serverCapacity)
	{
		tuple<double, double, double, double, double> finalOutcome;
		double totalCost = INT_MAX;
		int tempFinalGDatacenter = eligibleGDatacenters.front()->id;

		map<int, double> finalServerCountPerDC;

		for (auto GDatacenter : eligibleGDatacenters)
		{
			SimulationSetup4GeneralProblem(GDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R);
			auto tempOutcome = Alg_LCW(sessionClients, allDatacenters, serverCapacity, GDatacenter->id);
			double tempTotalCost = get<0>(tempOutcome);

			map<int, double> tempServerCountPerDC;
			for (auto dc : allDatacenters)
			{
				tempServerCountPerDC[dc->id] = dc->openServerCount;
			}

			if (tempTotalCost < totalCost) // choose the smaller cost
			{
				totalCost = tempTotalCost;
				finalOutcome = tempOutcome;
				tempFinalGDatacenter = GDatacenter->id;
				finalServerCountPerDC = tempServerCountPerDC;
			}
		}

		finalGDatacenter = tempFinalGDatacenter;

		for (auto dc : allDatacenters)
		{
			dc->openServerCount = finalServerCountPerDC[dc->id];
		}

		return finalOutcome;
	}

	tuple<double, double, double, double, double> Alg_LAC(
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double serverCapacity,
		int GDatacenterID)
	{
		if (serverCapacity < 2)
			return Alg_LCP(sessionClients, allDatacenters, serverCapacity, GDatacenterID);

		ResetAssignment(sessionClients, allDatacenters);

		for (auto dc : allDatacenters)
		{
			sort(dc->coverableClients.begin(), dc->coverableClients.end(), ClientComparatorByTrafficVolume);
		}

		while (true)
		{
			// prepare for this iteration
			vector<DatacenterType *> candidateDatacenters;
			for (auto dc : allDatacenters)
			{
				dc->unassignedCoverableClients.clear();
				for (auto client : dc->coverableClients)
				{
					if (client->assignedDatacenterID < 0)
						dc->unassignedCoverableClients.push_back(client);
				}

				if (!dc->unassignedCoverableClients.empty())
				{
					candidateDatacenters.push_back(dc);
				}
			}

			// no unassigned clients at all, so terminate the loop
			if (candidateDatacenters.empty())
			{
				break;
			}

			// compute the amortized cost per client if opening a server in each cdc
			for (auto cdc : candidateDatacenters)
			{
				const size_t addedClientCount = (cdc->unassignedCoverableClients.size() <= serverCapacity) ? cdc->unassignedCoverableClients.size() : (size_t)serverCapacity;
				if (0 == addedClientCount)
				{
					cdc->amortizedCostPerClient = INT_MAX;
				} // double-check
				else
				{
					double totalBandwidthCostOfAddedClients = 0;
					for (size_t i = 0; i < addedClientCount; i++)
					{
						totalBandwidthCostOfAddedClients += cdc->priceBandwidth * cdc->unassignedCoverableClients.at(i)->chargedTrafficVolume;
					}

					cdc->amortizedCostPerClient = (cdc->priceServer + totalBandwidthCostOfAddedClients) / addedClientCount;
				}
			}

			// assign at most (serverCapacity) unassignedCoverable clients to the dc with the lowest amortizedCostPerClient
			auto nextDC = candidateDatacenters.front();
			for (auto cdc : candidateDatacenters)
			{
				if (cdc->amortizedCostPerClient < nextDC->amortizedCostPerClient)
				{
					nextDC = cdc;
				}
			}
			const size_t numberOfClientsToAssign = (nextDC->unassignedCoverableClients.size() <= serverCapacity) ? nextDC->unassignedCoverableClients.size() : (size_t)serverCapacity;
			for (size_t i = 0; i < numberOfClientsToAssign; i++)
			{
				nextDC->unassignedCoverableClients.at(i)->assignedDatacenterID = nextDC->id;
				nextDC->assignedClients.push_back(nextDC->unassignedCoverableClients.at(i));
			}
		}

		return GetSolutionOutput(allDatacenters, serverCapacity, sessionClients, GDatacenterID);
	}

	tuple<double, double, double, double, double> Alg_LAC(
		vector<DatacenterType *> eligibleGDatacenters,
		int &finalGDatacenter,
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double DELAY_BOUND_TO_G,
		double DELAY_BOUND_TO_R,
		double serverCapacity)
	{
		tuple<double, double, double, double, double> finalOutcome;
		double totalCost = INT_MAX;
		int tempFinalGDatacenter = eligibleGDatacenters.front()->id;

		map<int, double> finalServerCountPerDC;

		for (auto GDatacenter : eligibleGDatacenters)
		{
			SimulationSetup4GeneralProblem(GDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R);
			auto tempOutcome = Alg_LAC(sessionClients, allDatacenters, serverCapacity, GDatacenter->id);
			double tempTotalCost = get<0>(tempOutcome);

			map<int, double> tempServerCountPerDC;
			for (auto dc : allDatacenters)
			{
				tempServerCountPerDC[dc->id] = dc->openServerCount;
			}

			if (tempTotalCost < totalCost) // choose the smaller cost
			{
				totalCost = tempTotalCost;
				finalOutcome = tempOutcome;
				tempFinalGDatacenter = GDatacenter->id;
				finalServerCountPerDC = tempServerCountPerDC;
			}
		}

		finalGDatacenter = tempFinalGDatacenter;

		for (auto dc : allDatacenters)
		{
			dc->openServerCount = finalServerCountPerDC[dc->id];
		}

		return finalOutcome;
	}

	tuple<double, double, double, double, double> Alg_LAC_2(
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double serverCapacity,
		int GDatacenterID)
	{
		if (serverCapacity < 2)
			return Alg_LCP(sessionClients, allDatacenters, serverCapacity, GDatacenterID);

		ResetAssignment(sessionClients, allDatacenters);

		for (auto dc : allDatacenters)
		{
			sort(dc->coverableClients.begin(), dc->coverableClients.end(), ClientComparatorByTrafficVolume);
		}

		for (auto dc : allDatacenters)
		{
			dc->openServerCount = 0;
		}

		// at each iteration, a new server will be opened at a specific DC
		while (true)
		{
			// prepare for this iteration
			vector<DatacenterType *> candidateDatacenters;
			for (auto dc : allDatacenters)
			{
				dc->unassignedCoverableClients.clear();
				for (auto client : dc->coverableClients)
				{
					if (client->assignedDatacenterID < 0)
						dc->unassignedCoverableClients.push_back(client);
				}

				if (!dc->unassignedCoverableClients.empty())
				{
					candidateDatacenters.push_back(dc);
				}
			}

			// no unassigned clients at all, so terminate the loop
			if (candidateDatacenters.empty())
			{
				break;
			}

			// compute the minimum amortized cost per client for each cdc if opening a new server in it
			for (auto cdc : candidateDatacenters)
			{
				const size_t maxAddedClientCount = (cdc->unassignedCoverableClients.size() <= serverCapacity) ? cdc->unassignedCoverableClients.size() : (size_t)serverCapacity;
				if (0 == maxAddedClientCount)
				{
					cdc->amortizedCostPerClient = INT_MAX;
				} // double-check
				else
				{
					// compute the best addedClientCount and the corresponding amortized cost
					cdc->amortizedCostPerClient = INT_MAX;
					cdc->addedClientCount = maxAddedClientCount;
					for (size_t addedClientCountTemp = 1; addedClientCountTemp <= maxAddedClientCount; addedClientCountTemp++)
					{
						double totalBandwidthCostOfAddedClients = 0;
						for (size_t i = 0; i < addedClientCountTemp; i++)
						{
							totalBandwidthCostOfAddedClients += cdc->priceBandwidth * cdc->unassignedCoverableClients.at(i)->chargedTrafficVolume;
						}
						double temp = (cdc->priceServer + totalBandwidthCostOfAddedClients) / addedClientCountTemp;
						if (temp < cdc->amortizedCostPerClient)
						{
							cdc->amortizedCostPerClient = temp;
							cdc->addedClientCount = addedClientCountTemp;
						}
					}
				}
			}

			// choose the cdc with the lowest minimum amortizedCostPerClient
			auto nextDC = candidateDatacenters.front();
			for (auto cdc : candidateDatacenters)
			{
				if (cdc->amortizedCostPerClient < nextDC->amortizedCostPerClient)
				{
					nextDC = cdc;
				}
			}

			// assign addedClientCount clients
			for (size_t i = 0; i < nextDC->addedClientCount; i++)
			{
				nextDC->unassignedCoverableClients.at(i)->assignedDatacenterID = nextDC->id;
				nextDC->assignedClients.push_back(nextDC->unassignedCoverableClients.at(i));
			}

			// open a new server at nextDC
			nextDC->openServerCount++;
		}

		// post-processing
		for (auto dc : allDatacenters)
		{
			while ((serverCapacity * dc->openServerCount - dc->assignedClients.size()) >= serverCapacity)
			{
				printf("\npost-processing\n");
				dc->openServerCount--;
			}
		}

		// output
		double costServer = 0, costBandwidth = 0, numberServers = 0;
		for (auto dc : allDatacenters)
		{
			costServer += dc->priceServer * dc->openServerCount;
			double totalChargedTrafficVolume = 0;
			for (auto client : dc->assignedClients)
			{
				totalChargedTrafficVolume += client->chargedTrafficVolume;
			}
			costBandwidth += dc->priceBandwidth * totalChargedTrafficVolume;
			numberServers += dc->openServerCount;
		}

		double totalDelay = 0;
		for (auto client : sessionClients)
		{
			totalDelay += client->delayToDatacenter[client->assignedDatacenterID] + allDatacenters.at(client->assignedDatacenterID)->delayToDatacenter[GDatacenterID];
		}

		return tuple<double, double, double, double, double>(
			costServer + costBandwidth,
			costServer,
			costBandwidth,
			(numberServers * serverCapacity - sessionClients.size()) / sessionClients.size(),
			totalDelay / sessionClients.size());
	}

	tuple<double, double, double, double, double> Alg_LAC_2(
		vector<DatacenterType *> eligibleGDatacenters,
		int &finalGDatacenter,
		const vector<ClientType *> &sessionClients,
		const vector<DatacenterType *> &allDatacenters,
		double DELAY_BOUND_TO_G,
		double DELAY_BOUND_TO_R,
		double serverCapacity)
	{
		tuple<double, double, double, double, double> finalOutcome;
		double totalCost = INT_MAX;
		int tempFinalGDatacenter = eligibleGDatacenters.front()->id;

		map<int, double> finalServerCountPerDC;

		for (auto GDatacenter : eligibleGDatacenters)
		{
			SimulationSetup4GeneralProblem(GDatacenter, sessionClients, allDatacenters, DELAY_BOUND_TO_G, DELAY_BOUND_TO_R);
			auto tempOutcome = Alg_LAC_2(sessionClients, allDatacenters, serverCapacity, GDatacenter->id);
			double tempTotalCost = get<0>(tempOutcome);

			map<int, double> tempServerCountPerDC;
			for (auto dc : allDatacenters)
			{
				tempServerCountPerDC[dc->id] = dc->openServerCount;
			}

			if (tempTotalCost < totalCost) // choose the smaller cost
			{
				totalCost = tempTotalCost;
				finalOutcome = tempOutcome;
				tempFinalGDatacenter = GDatacenter->id;
				finalServerCountPerDC = tempServerCountPerDC;
			}
		}

		finalGDatacenter = tempFinalGDatacenter;

		for (auto dc : allDatacenters)
		{
			dc->openServerCount = finalServerCountPerDC[dc->id];
		}

		return finalOutcome;
	}
}