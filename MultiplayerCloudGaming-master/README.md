# Overview
Note: The "LatencyDataProcessing" project can be ignored.

Note: The "MultiplayerCloudGaming" project (the project name is the same with the entire solution name) was created using MS Visual Studio 2015. It is recommended to open and build the project using the same version of Visual Studio.

The "MultiplayerCloudGaming" project consists of two namespaces, namely MatchmakingProblem and ServerAllocationProblem, which are for simulating the Matchmaking Problem (connecting to our NetGames 2017 submission), and the Server Allocation Problem (connecting to our TMM submission), respectively.

In the "MultiplayerCloudGaming" project folder
- the "Main.cpp" is the entry point of the simulation program, i.e., main()
- the "Data" folder is the directory for both the input data and output data (there is a subfolder containing the result data for each specific problem, e.g., ./ServerAllocationProblemBasic/)
- all the input and output data files are in csv format
- the output (result) filename format for ServerAllocationProblem is: parameter_parameter_parameter_metricName.csv
- there are some python and matlab scripts inside the project directory, which are used to process the result and generate graphs for the paper
