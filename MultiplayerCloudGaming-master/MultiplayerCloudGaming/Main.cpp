#include "ServerAllocationProblem.h" // must included for the simulation of ServerAllocationProblem
using namespace ServerAllocationProblem; // must included for the simulation of ServerAllocationProblem
int main(int argc, char *argv[])
{
	std::printf("ServerAllocationProblem simulation starts...\n");
	auto startTime = clock();
	
	// performing simulations with different settings
	// the function SimulateBasicProblem(...) is resposible for the basic problem of the MCG server allocation, which the G-Server is given and fixed
	// the function SimulateGeneralProblem(...) is responsible for the general problem of the MCG server allocation, which G-Server is not given but to be optimally chosen by the algortithm
	for (double sessionSize : { 10, 50 })
	{
		ServerAllocationProblem::SimulateBasicProblem(75, 50, sessionSize); // DELAY_BOUND_TO_G = 75, DELAY_BOUND_TO_R = 50, SESSION_SIZE = sessionSize, SESSION_COUNT = 1000 (by default if not specified)
		ServerAllocationProblem::SimulateBasicProblem(150, 100, sessionSize);
		ServerAllocationProblem::SimulateGeneralProblem(75, 50, sessionSize);
		ServerAllocationProblem::SimulateGeneralProblem(150, 100, sessionSize);
	}
	
	std::printf("ServerAllocationProblem simulation ended... ***elasped time: %.2f seconds***\n", std::difftime(clock(), startTime) / 1000);

	return 0;
}

/* To run the code below for MatchmakingProblem, you need to comment out the above code */
/*
#include "MatchmakingProblem.h" // must included for the simulation of MatchmakingProblem
using namespace MatchmakingProblem; // must included for the simulation of MatchmakingProblem
int main(int argc, char *argv[])
{
	std::printf("MatchmakingProblem simulation starts...\n");
	auto startTime = clock();

	// MaximumMatchingProblem
	auto simulator = MatchmakingProblem::MaximumMatchingProblem();	
	simulator.Initialize(); // initialize once
	_mkdir(simulator.outputDirectory.c_str());
	for (string algToRun : { "layered", "simple", "nearest" })
	{		
		for (int latencyThreshold : { 25, 50, 100 })
		{			
			for (int sessionSize : { 10 })
			{
				simulator.outFile = ofstream(simulator.outputDirectory + algToRun + "_" + std::to_string(latencyThreshold) + "_" + std::to_string(sessionSize) + ".csv");
				for (int clientCount = 50; clientCount <= 200; clientCount += 10) 
				{ simulator.Simulate(algToRun, clientCount, latencyThreshold, sessionSize, 1000); }
				simulator.outFile.close();
			}
		}
	}
	for (int latencyThreshold : { 25, 50, 100 })
	{
		simulator.outFile = ofstream(simulator.outputDirectory + "connectivityHistogram" + "_" + std::to_string(latencyThreshold) + ".csv");
		simulator.CountConnectivity(latencyThreshold);
		simulator.outFile.close();
	}

	// ParetoMatchingProblem	
	//auto simulator = ParetoMatchingProblem();
	//simulator.Initialize();
	//_mkdir(simulator.outputDirectory.c_str());
	//for (bool regionControl : { false })
	//{
	//	for (int clientCount : { 50, 100, 150, 200 })
	//	{
	//		for (int latencyThreshold : { 25, 50, 100 })
	//		{
	//			for (int sessionSize : { 10 })
	//			{
	//				for (int serverCapacity : { 4 })
	//				{
	//					cout << "\nSimulate() with setting: " << regionControl << "." << clientCount << "." << latencyThreshold << "." << sessionSize << "." << serverCapacity << "\n";
	//					simulator.Simulate(Setting(regionControl, clientCount, latencyThreshold, sessionSize, serverCapacity, 1000));
	//				}
	//			}
	//		}
	//	}
	//}

	std::printf("MatchmakingProblem simulation ended... ***elasped time: %.2f seconds***\n", std::difftime(clock(), startTime) / 1000);

	return 0;
}
*/
