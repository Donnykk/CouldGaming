// header.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <list>
#include <string>
#include <algorithm>
#include <msclr/marshal_cppstd.h>

using namespace std;
using namespace System;
using namespace System::IO;
using namespace msclr::interop;

// clean the input dirty data file that contains dirty strings 
// create a cleaned file 
// return the cleaned file name
String^ CleanData(String^ dirtyFileName, String^ dirtyDataDescriptor)
{
	String^ cleanedFileName = dirtyFileName + "_cleaned";
	if (File::Exists(cleanedFileName)) {
		return cleanedFileName;
	}
	StreamWriter^ cleanedFile = gcnew StreamWriter(cleanedFileName);
	StreamReader^ dirtyFile = gcnew StreamReader(dirtyFileName);	
	String^ line;
	int dirtyLineCounter = 0;
	while (line = dirtyFile->ReadLine()) 
	{
		array<String^>^ lineStrList = line->Split(gcnew array<Char>{' '}, StringSplitOptions::RemoveEmptyEntries);
		bool isDirtyLine = false;
		for (int i = 0; i < lineStrList->Length; i++) 
		{
			if (lineStrList[i] == dirtyDataDescriptor)
			{
				isDirtyLine = true;
				dirtyLineCounter++;
				break;
			}
		}
		if (!isDirtyLine) {
			cleanedFile->WriteLine(line);
		}
	}
	cleanedFile->Close();
	Console::WriteLine("File created: \"{0}\"", cleanedFileName);

	return cleanedFileName;
}

// create a file that lists out the common PL nodes 
// return the list of common PL nodes
list<string> ExtractCommonPL(String^ first, String^ second)
{
	StreamReader^ firstFile = gcnew StreamReader(first);
	StreamReader^ secondFile = gcnew StreamReader(second);	
	list<string> firstList;
	list<string> secondList;
	String^ line;
	while (line = firstFile->ReadLine())
	{
		array<String^>^ lineStrList = line->Split(gcnew array<Char>{' '}, StringSplitOptions::RemoveEmptyEntries);
		String^ str = lineStrList[0]; // the pl's name
		firstList.push_back(marshal_as<string>(str));
	}
	while (line = secondFile->ReadLine())
	{
		array<String^>^ lineStrList = line->Split(gcnew array<Char>{' '}, StringSplitOptions::RemoveEmptyEntries);
		String^ str = lineStrList[0]; // the pl's name
		secondList.push_back(marshal_as<string>(str));
	}
	firstList.sort();
	secondList.sort();
	list<string> commonList(firstList.size());
	set_intersection(firstList.begin(), firstList.end(), secondList.begin(), secondList.end(), commonList.begin());

	String^ commonFileName = "pl_common_in_ec2_azure_gae";
	if (!File::Exists(commonFileName))
	{
		StreamWriter^ commonFile = gcnew StreamWriter(commonFileName);
		for (auto it = commonList.begin(); (it != commonList.end()) && (*it != ""); it++)
		{
			String^ str = marshal_as<String^>(*it);
			commonFile->WriteLine(str);
		}
		commonFile->Close();
		Console::WriteLine("File created: \"{0}\"", commonFileName);
	}

	return commonList;
}

// return true if a string is found in the input list of strings
bool IfElementInList(string e, list<string> &ls)
{
	for (auto it = ls.begin(); it != ls.end(); it++)
	{
		if ((e != "") && (e == *it)) {
			return true;
		}
	}

	return false;
}

// extract common data from input files
// create a file that contains all common data lines
// return the  file name
String^ CombineCommonData(String^ first, String^ second)
{
	if (!File::Exists(first) || !File::Exists(second))
	{
		Console::WriteLine("Warning: input files do not exist or are wrong");
		return "";
	}

	list<string> commonPLList = ExtractCommonPL(first, second);	
	StreamReader^ firstFile = gcnew StreamReader(first);
	StreamReader^ secondFile = gcnew StreamReader(second);
	list<string> firstList;
	list<string> secondList;
	String^ line;
	while (line = firstFile->ReadLine())
	{
		array<String^>^ lineStrList = line->Split(gcnew array<Char>{' '}, StringSplitOptions::RemoveEmptyEntries);		
		String^ str = lineStrList[0]; // the pl's name
		if (IfElementInList(marshal_as<string>(str), commonPLList)) {
			firstList.push_back(marshal_as<string>(line));
		}			
	}
	while (line = secondFile->ReadLine())
	{
		array<String^>^ lineStrList = line->Split(gcnew array<Char>{' '}, StringSplitOptions::RemoveEmptyEntries);
		String^ str = lineStrList[0]; // the pl's name
		if (IfElementInList(marshal_as<string>(str), commonPLList)) {
			secondList.push_back(marshal_as<string>(line));
		}
	}
	
	firstList.sort();
	secondList.sort();
	
	String^ firstExtractedFileName = first + "_extracted";
	String^ secondExtractedFileName = second + "_extracted";
	if (!File::Exists(firstExtractedFileName))
	{
		StreamWriter^ extractedFirstFile = gcnew StreamWriter(firstExtractedFileName);
		for (auto it : firstList)
		{
			String^ str = marshal_as<String^>(it);
			extractedFirstFile->WriteLine(str);
		}
		extractedFirstFile->Close();
		Console::WriteLine("File created: \"{0}\"", firstExtractedFileName);
	}
	if (!File::Exists(secondExtractedFileName))
	{
		StreamWriter^ extractedSecondFile = gcnew StreamWriter(secondExtractedFileName);
		for (auto it : secondList)
		{
			String^ str = marshal_as<String^>(it);
			extractedSecondFile->WriteLine(str);
		}
		extractedSecondFile->Close();
		Console::WriteLine("File created: \"{0}\"", secondExtractedFileName);
	}
	
	vector<string> firstVector (firstList.size());	
	copy(firstList.begin(), firstList.end(), firstVector.begin());
	vector<string> secondVector; 
	for (auto it : secondList)
	{
		String^ line = marshal_as<String^>(it);
		array<String^>^ lineStrList = line->Split(gcnew array<Char>{' '}, StringSplitOptions::RemoveEmptyEntries);
		String^ newLine = "";
		for (int j = 1; j < lineStrList->Length; j++) // get rid of the first column which refers to pl names
		{
			newLine += lineStrList[j] + " ";
		}
		secondVector.push_back(marshal_as<string>(newLine));
	}
	if (firstVector.size() != secondVector.size())
	{
		Console::WriteLine("The numbers of lines of the two extracted files don't match");
		return "";
	}
	String^ combinedDataFileName = first + "_combine_" + second;
	if (!File::Exists(combinedDataFileName))
	{
		StreamWriter^ combinedDataFile = gcnew StreamWriter(combinedDataFileName);
		for (int i = 0; i < (int)firstVector.size(); i++)
		{
			String^ combinedLine = marshal_as<String^>(firstVector.at(i) + " " + secondVector.at(i));
			combinedDataFile->WriteLine(combinedLine);
		}
		combinedDataFile->Close();
		Console::WriteLine("File created: \"{0}\"", combinedDataFileName);
	}

	return combinedDataFileName;
}

// return a processed combined file from two input raw files
String^ ProcessRawData(String^ firstRaw, String^ secondRaw)
{
	return CombineCommonData(CleanData(firstRaw, "*"), CleanData(secondRaw, "*"));
}

// return a processed file from one input raw file
String^ ProcessRawData(String^ firstRaw)
{
	return CleanData(firstRaw, "*");
}