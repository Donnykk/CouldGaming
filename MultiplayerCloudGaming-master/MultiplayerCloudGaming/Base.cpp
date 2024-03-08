#include "Base.h"

vector<vector<string>> ReadDelimitedTextFileIntoVector(const string input_file_name, const char delimiter, const bool skip_first_row)
{
	vector<vector<string>> strings_read;

	fstream data_file(input_file_name);
	if (data_file.is_open())
	{
		string one_line;
		int row_counter = 0;
		while (std::getline(data_file, one_line))
		{
			row_counter++;
			if ((1 == row_counter) && skip_first_row) continue;
			stringstream one_line_stream(one_line);
			string one_string;
			vector<string> string_list;
			while (std::getline(one_line_stream, one_string, delimiter))
			{
				string_list.push_back(one_string);
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
}

int GenerateRandomIndex(const int size)
{
	if (size < 1) return -1;
	
	return (std::rand() % size);
}

double GetMeanValue(const vector<double> &v)
{
	if (v.empty()) return 0;

	double sum = 0;
	for (auto it : v)
	{
		sum += it;
	}

	return (sum / v.size());
}

double GetStdValue(const vector<double> &v)
{
	if (v.empty()) return 0;

	double mean = GetMeanValue(v);
	vector<double> temp;
	for (auto it : v)
	{
		temp.push_back((it - mean) * (it - mean));
	}

	return sqrt(GetMeanValue(temp));
}

double GetMinValue(const vector<double> &v)
{
	if (v.empty()) return 0;

	return *min_element(v.begin(), v.end());
}

double GetMaxValue(const vector<double> &v)
{
	if (v.empty()) return 0;

	return *max_element(v.begin(), v.end());
}

double GetPercentile(vector<double> v, const double p)
{
	std::sort(v.begin(), v.end());
	int rank = (int)ceil((p / 100) * v.size()) - 1;
	return v.at(rank);
}

double GetRatioOfGreaterThan(const vector<double>& v, const double x)
{
	double counter = 0;
	for (auto i : v)
	{
		if (i > x) counter++;
	}
	return counter / v.size();
}