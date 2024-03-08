#include "header.h"

int main(array<String^> ^argv)
{
	ProcessRawData(".\\Data\\pl_to_ec2_azure_rtt", ".\\Data\\pl_to_gae_rtt");

	system("pause");
	return 0;
}