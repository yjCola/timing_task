#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <map>

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

using namespace std;

void splitString(const string &str, const string &split, vector<string> &vcSubStr)
{
	set<char> stSplit;
	for(unsigned i = 0; i < split.length(); ++i) 
	{
		stSplit.insert(split[i]);
	}

	string strTmp;
	for(unsigned i = 0; i < str.length(); ++i) 
	{
		if(stSplit.count(str[i])) 
		{
			vcSubStr.push_back(strTmp);
			strTmp.clear();
		}
		else 
		{
			strTmp.insert(strTmp.end(), str[i]);
		}
	}

	if(!strTmp.empty()) 
	{
		vcSubStr.push_back(strTmp);
	}
}

void loopExecute(const string &mark, const string &command, int time_interval)
{
	cout << "begin to loop execute mark: " << mark << " command: " << command << " time_interval: " << time_interval << endl;
	while(true) 
	{
		FILE *fp_exe;
		fp_exe = popen(command.c_str(), "r");

		char buff[1024];
		while(fgets(buff, 1024, fp_exe) != NULL) 
		{
			cerr << buff << endl;
		}

		pclose(fp_exe);
		usleep(time_interval);
	}
}

pid_t generateMission(const string &mark, const string &command, int time_interval)
{
	pid_t pid;
	if((pid = fork()) < 0) 
	{
		cerr << "fork for mark: " << mark << " failed! pid: " << pid << endl;
	}
	else if(pid == 0) 
	{
		cout << "child created succ! mark: " << mark
			<< " command: " << command << " time_interval: " << time_interval << endl;

		loopExecute(mark, command, time_interval);
	}
	else 
	{
		cout << "fork for mark: " << mark << " succ! pid: " << pid << endl;
	}

	return pid;
}

int main(int argc, char **argv)
{
	int ret = 0;

	//Save parent_pid to a file
	ofstream fout("./parent_pid", ofstream::out);
	pid_t parent_pid = getpid();
	fout << parent_pid << endl;

	//Parse task fconfig
	map<string, string> mpMark2Command;
	map<string, int> mpMark2MSeconds;
	map<pid_t, string> mpPid2Mark;

	ifstream fin("./task.conf", ifstream::in);
	string line;
	while(getline(fin, line)) 
	{
		if(!line.empty() && line[0] == '#')
		{
			//It`s an annotation
			continue;
		}
		
		//mark str;executable command;timing interval(us)
		vector<string> vcSubStr;
		splitString(line, ";", vcSubStr);

		if(vcSubStr.size() != 3) 
		{
			cerr << "invalid conf: " << line << endl;
		}
		else 
		{
			mpMark2Command.insert(pair<string, string>(vcSubStr[0], vcSubStr[1]));
			mpMark2MSeconds.insert(pair<string, int>(vcSubStr[0], atoi(vcSubStr[2].c_str())));
		}
	}

	//Generate all missions
	typeof(mpMark2Command.begin()) c_it = mpMark2Command.begin();
	for(; c_it != mpMark2Command.end(); ++c_it) 
	{
		const string &mark = c_it->first;
		const string &command = c_it->second;
		int time_interval = mpMark2MSeconds[mark];

		if(time_interval <= 0) 
		{
			cerr << "invalid time_interval: " << time_interval << " for mark: " << mark << endl;
			continue;
		}

		pid_t pid = generateMission(mark, command, time_interval);
		if(pid > 0)
		{
			mpPid2Mark[pid] = mark;
		}
	}

	//Reload aborted mission
	while(true) 
	{
		int status;
		pid_t pid;
		pid = wait(&status);

		const string &mark = mpPid2Mark[pid];
		const string &command = mpMark2Command[mark];
		int time_interval = mpMark2MSeconds[mark];

		pid_t new_pid = generateMission(mark, command, time_interval);
		if(new_pid > 0)
		{
			mpPid2Mark[pid] = mark;
		}
	}

	return ret;
}
