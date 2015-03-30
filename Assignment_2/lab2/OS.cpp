#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <algorithm>
#include <math.h>
using namespace std;

void displayprocesses();
void displaysummary();
ifstream myfile;
int num_of_proc;
int pid_g=0;
vector<int> randomnums;
int count_random=0;
int ofs=0;
int global_time=0;
int ioglobal_time=0;
int iototal_time=0;
bool io[50000]={0};


int myrandom(int burst) 
{ 
	int t = 1 + (randomnums[ofs] % burst); 
	ofs++;
	if(ofs==count_random)
		ofs=0;
	return t;
}

class process
{
public:
	int pid,begat,at,begtc,tc,cb,io,stat_prio,dyn_prio,ft,tt,it,cw,state,tempcb,tempio;
};
vector<process> pqueue;
vector<process> finqueue;

void setglobaltime()
{
	int min=pqueue[0].at;
	for(int i=1;i<pqueue.size();i++)
	{
		if(min > pqueue[i].at)
			min = pqueue[i].at;
	}
}



class summary
{
public:
int ft;
float cpuUt, ioUt, turnaroundavg, cpuwaitavg, throughput;
	void createsummary(summary &s)
	{

		//////////////////////////////// Calculating CPU utilization
		double cpuuttemp = 0.0;
		for(int i=0;i<finqueue.size();i++)
		{
			cpuuttemp += finqueue[i].begtc;
		}
		s.cpuUt = 100 * cpuuttemp/s.ft;

		//////////////////////////////// Calculating IO Utilization
		double tempp=0.0;
		for(int i=0;i<=s.ft;i++)
		{
			tempp+=(int) io[i];
		}
		s.ioUt = 100 * tempp/s.ft;

		////////////////////////////////  Calculating average turnaround time
		double avgtt=0.0;
		for(int i=0;i<finqueue.size();i++)
		{
			avgtt += finqueue[i].tt;
		}
		avgtt /= (finqueue.size());
		s.turnaroundavg = avgtt;

		////////////////////////////// Calculating avg cpu waiting time

		double avgcw =0.0;
		for(int i=0;i<finqueue.size();i++)
		{
			avgcw += finqueue[i].cw;
		}
		avgcw /= (finqueue.size());
		s.cpuwaitavg = avgcw;

		////////////////////////////// Calculating throughput

		double timeperprocess = s.ft / finqueue.size();
		double throughput = 100.00 / timeperprocess;
		s.throughput = throughput;
	}

}summary;


void sortOutput()
{
	for(int i =0;i<finqueue.size();i++)
	{
		for(int j =0;j<finqueue.size()-1;j++)
		{
			if(finqueue[j].pid > finqueue[j+1].pid )
			{
			process temp = finqueue[j];
			finqueue[j] = finqueue[j+1];
			finqueue[j+1] = temp;
			}
		}
	}
}

void printStackTrace()
{
	cout<<"-------"<<endl;
	for(int i=0;i<pqueue.size();i++)
		{
			cout << setfill('0') << setw(4) << pqueue[i].pid;
			cout<<": "<<pqueue[i].at<<" "<<pqueue[i].tc<<" "<<pqueue[i].cb<<" ";
			cout<<pqueue[i].io<<" "<<pqueue[i].stat_prio<<" | ";
			cout<<pqueue[i].ft<<" "<<pqueue[i].tt<<" "<<pqueue[i].it<<" "<<pqueue[i].cw<<" \n";
		}
	cout<<"-------"<<endl;
}


class scheduler
{
public:
	process current;

	void enqueue(process p)
	{
		pqueue.push_back(p);
	}

	void display()
	{
		displayprocesses();
		displaysummary();
	}

	virtual void run_scheduler(){
	}

	virtual void run_scheduler(int quantum){
	}

};

class fcfs : public scheduler
{
public:
	process dequeue()
	{
		int pos=0;
		process t=pqueue[0];
		for(int i=1;i<pqueue.size();i++)
		{
			if(t.at > pqueue[i].at)
				{
					pos=i;
					t=pqueue[i];
				}
		}
		pqueue.erase(pqueue.begin() + pos);
		return t;
	}

	void run_scheduler()
	{
		while(!pqueue.empty())
		{
			//printStackTrace();
			process t = dequeue();
			int tempcb = myrandom(t.cb);
			if(tempcb > t.tc)
				tempcb = t.tc;

			t.tc -= tempcb;

			if(global_time<t.at)
				t.at += tempcb;
			else
				t.at = global_time + tempcb;

			
			global_time = t.at;
			

			if(t.tc>0)
			{
				
				int tempio = myrandom(t.io);
				for(int k=0;k<tempio;k++)
				{
					io[k+t.at]=1;
				}	
				
				t.at +=  tempio;
				t.it +=  tempio;
				enqueue(t);
			}
			else
			{
				t.ft = t.at;
				summary.ft=t.ft;
				t.tt = t.ft - t.begat;
				t.cw = t.tt - t.begtc - t.it;

				finqueue.push_back(t);
			}
		}
		cout<<"FCFS"<<endl;
	}

};

////////////////////////////////////////////

class lcfs : public scheduler
{
public:
	process dequeue()
	{
		while(1)
		{
			int pos;
			process t;
			int max = -1;
			int min = 9999999;
			for(int i=pqueue.size()-1;i>=0;i--)
			{
				if(max < pqueue[i].at && pqueue[i].at <= global_time)
					{
						max = pqueue[i].at;
						pos=i;
						t=pqueue[i];
					}
				if(min > pqueue[i].at)
					min=pqueue[i].at;
			}

			if(max!=-1)
			{
				pqueue.erase(pqueue.begin() + pos);
				return t;
			}
			else
				global_time = min;
		}
	}

	void run_scheduler()
	{
		setglobaltime();
		while(!pqueue.empty())
		{
			//printStackTrace();
			process t = dequeue();
			int tempcb = myrandom(t.cb);
			if(tempcb > t.tc)
				tempcb = t.tc;

			t.tc -= tempcb;

			if(global_time<t.at)
				t.at += tempcb;
			else
				t.at = global_time + tempcb;

			
			global_time = t.at;
			

			if(t.tc>0)
			{
				
				int tempio = myrandom(t.io);
				for(int k=0;k<tempio;k++)
				{
					io[k+t.at]=1;
				}	
				
				t.at +=  tempio;
				t.it +=  tempio;
				enqueue(t);
			}
			else
			{
				t.ft = t.at;
				summary.ft=t.ft;
				t.tt = t.ft - t.begat;
				t.cw = t.tt - t.begtc - t.it;

				finqueue.push_back(t);
			}
		}
		cout<<"LCFS"<<endl;
	}

};
/////////////////////////////////////////////////////////////////////////// LCFS ENDS

class sjf : public scheduler
{
public:
	process dequeue()
	{
		while(1)
		{
			int pos=-1;
			process t;
			int mintc = 999999;
			int min = 999999;
			for(int i=0;i<pqueue.size();i++)
			{
				if(mintc > pqueue[i].tc && pqueue[i].at <= global_time)
					{
						mintc = pqueue[i].tc;
						pos=i;
						t=pqueue[i];
					}
				if(min > pqueue[i].at)
					{
						min=pqueue[i].at;
					}
			}

			if(pos!=-1)
			{
				pqueue.erase(pqueue.begin() + pos);
				return t;
			}
			else
			{
				global_time = min;
			}
		}
	}

	void run_scheduler()
	{
		setglobaltime();
		while(!pqueue.empty())
		{
			process t = dequeue();
			int tempcb = myrandom(t.cb);
			if(tempcb > t.tc)
				tempcb = t.tc;

			t.tc -= tempcb;

			if(global_time<t.at)
				t.at += tempcb;
			else
				t.at = global_time + tempcb;

			
			global_time = t.at;
			

			if(t.tc>0)
			{
				
				int tempio = myrandom(t.io);
				for(int k=0;k<tempio;k++)
				{
					io[k+t.at]=1;
				}	
				
				t.at +=  tempio;
				t.it +=  tempio;
				enqueue(t);
			}
			else
			{
				t.ft = t.at;
				summary.ft=t.ft;
				t.tt = t.ft - t.begat;
				t.cw = t.tt - t.begtc - t.it;

				finqueue.push_back(t);
			}
		}
		cout<<"SJF"<<endl;
	}

};


////////////////////////////////////////////////////////////////////////// SJF Ends

class rr : public scheduler
{
public:

	void initialize_ready_queue(vector<process> &rqueue)
	{
		vector<int> deletepid;
		for(int i=0;i<pqueue.size();i++)
		{
			if(pqueue[i].at <= global_time)
			{ 
				rqueue.push_back(pqueue[i]);
				deletepid.push_back(pqueue[i].pid);
			}
		}

		for(int i=0;i<deletepid.size();i++)
		{
			for(int j=0;j<pqueue.size();j++)
			{
				if(deletepid[i]==pqueue[j].pid)
				{
					pqueue.erase(pqueue.begin() + j);
					break;
				}
			}
		}
			
		
	}

	process dequeue(vector<process> &rqueue)
	{
		/*process t = rqueue[0];
		if(global_time>t.at)
			t.at = global_time;
		rqueue.erase(rqueue.begin() + 0);
		return t;	
		*/
		process t = rqueue[0];
		int pos=0;;
		for(int i=0;i<rqueue.size();i++)
		{
			if(rqueue[i].at < t.at && rqueue[i].at <= global_time)
			{
				t = rqueue[i];
				pos = i;
			}
		}
		if(global_time > t.at)
			t.at = global_time;
		rqueue.erase(rqueue.begin() + pos);
		return t;

	}

	void run_scheduler(int quantum)
	{
		setglobaltime();
		vector<process> rqueue;
		initialize_ready_queue(rqueue);
		while(!pqueue.empty() || !rqueue.empty())
		{

			process t = dequeue(rqueue);

			if(t.tempcb==-100)
			{
				int tempcb = myrandom(t.cb);
				if(tempcb > t.tc)
					tempcb = t.tc;
				t.tempcb = tempcb;	
			}

			cout<<global_time<<" "<<t.pid<<" "<<"cb : "<<t.tempcb<<" "<<t.tc<<" "<<t.stat_prio-1<<endl;


			if(quantum <= t.tc && quantum <= t.tempcb)
			{
				t.tc -= quantum;
				t.tempcb -= quantum;
				global_time += quantum;
				t.at += quantum;
				if(t.tempcb==0)
				{
					if(t.tc == 0 )
					{
						t.ft = t.at;
						summary.ft=t.ft;
						t.tt = t.ft - t.begat;
						t.cw = t.tt - t.begtc - t.it;
						finqueue.push_back(t);
						if(rqueue.size()==0 && pqueue.size()!=0)
						setglobaltime();
					}
					else
					{
						int tempio = myrandom(t.io);
						cout<<"IO : "<<tempio<<endl;
						for(int k=0;k<tempio;k++)
						{
							io[k+t.at]=1;
						}
						t.at +=  tempio;
						t.it +=  tempio;
						t.tempcb = -100;
						enqueue(t);
						if(rqueue.size()==0 && pqueue.size()!=0)
						{
							setglobaltime();
						}
					}
				}
				else
				{	
					initialize_ready_queue(rqueue);
					rqueue.push_back(t);
				}
			}
			else if(quantum <= t.tc && quantum > t.tempcb)
			{
				t.tc -= t.tempcb;
				t.at += t.tempcb;
				global_time += t.tempcb;
				if(global_time>t.at) 
					t.at = global_time;
				
				int tempio = myrandom(t.io);
				cout<<"IO : "<<tempio<<endl;
				for(int k=0;k<tempio;k++)
				{
					io[k+t.at]=1;
				}
				t.at +=  tempio;
				t.it +=  tempio;
				t.tempcb = -100;
				enqueue(t);
				if(rqueue.size()==0 && pqueue.size()!=0)
					setglobaltime();
			}
			else if(quantum > t.tc && quantum > t.tempcb)
			{	
				global_time += t.tc;
				t.at += t.tc;
				t.tc = 0;
				t.ft = t.at;
				summary.ft=t.ft;
				t.tt = t.ft - t.begat;
				t.cw = t.tt - t.begtc - t.it;
				finqueue.push_back(t);
				if(rqueue.size()==0 && pqueue.size()!=0)
					setglobaltime();
			}

			if(rqueue.size()==0 && pqueue.size()!=0)
				setglobaltime();
			initialize_ready_queue(rqueue);

			if(rqueue.size()>0)
				cout<<rqueue.size()<<" "<<global_time<<" "<<rqueue[0].pid<<endl;

		}
		cout<<"RR "<<quantum<<endl;
	}

};



///////////////////////////////////////////////////////////////////////// RR Ends

void readprocesses(string s)
{
	myfile.open(s);
	if (myfile.is_open())
 { 
 	string line;
 	int i=0;
 	while (std::getline(myfile, line))
    {
    	process p_temp;
        string temp;
        stringstream tokens(line);
        vector<string> tokens_stat;

        p_temp.pid = pid_g++;
        tokens>>temp;
        p_temp.at = atoi(temp.c_str());
        p_temp.begat = p_temp.at;
        tokens>>temp;
        p_temp.tc = atoi(temp.c_str());
        p_temp.begtc = p_temp.tc;
        tokens>>temp;
        p_temp.cb = atoi(temp.c_str());
        tokens>>temp;
        p_temp.io = atoi(temp.c_str());
        p_temp.stat_prio = myrandom(4);
        p_temp.state = 0;
        p_temp.dyn_prio = p_temp.stat_prio - 1;
        p_temp.tempcb = -100;
        pqueue.push_back(p_temp);
    }
    num_of_proc=i;
 }
 else
    cout << "Unable to open file"<<endl;

	myfile.close();
}


void readrandomfile(string s)
{
	myfile.open(s);
	string line;
	
	if (myfile.is_open())
 	{
 		getline(myfile,line);
 		int count_random = atoi(line.c_str());
 		while (getline(myfile, line))
    	{
    		randomnums.push_back(atoi(line.c_str()));
    	}
 	}
 	else
 		cout<<"Unable to open file"<<endl; 
 	myfile.close();

}



void displayprocesses()
{
	for(int i=0;i<finqueue.size();i++)
	{
		process p_temp = finqueue[i];
		cout <<setfill('0') << setw(4) << p_temp.pid<<":";
		cout<<"\t"<<p_temp.begat<<"\t"<<p_temp.begtc<<"\t"<<p_temp.cb<<"\t"<<p_temp.io<<"\t"<<p_temp.stat_prio<<" | "<<p_temp.ft<<"\t"<<p_temp.tt<<"\t"<<p_temp.it<<"\t"<<p_temp.cw<<endl;
	}
}

void displaysummary()
{	
	cout<<"SUM:\t"<<summary.ft<<"\t";
	summary.createsummary(summary);
	printf("%.2lf", summary.cpuUt);
	cout<<"\t";
	printf("%.2lf", summary.ioUt);
	cout<<"\t";
	printf("%.2lf", summary.turnaroundavg);
	cout<<"\t";
	printf("%.2lf", summary.cpuwaitavg);
	cout<<"\t";
	printf("%.3lf", summary.throughput);
	cout<<endl;
}


int main()
{
 string path1 = "input4";
 string path2 = "rfile.txt";
 char type = 'F';

 readrandomfile(path2);
 readprocesses(path1);

 //scheduler *s = new fcfs();
 scheduler *s = new rr();
 s->run_scheduler(2);
 
 sortOutput();
 s->display();
 //displayprocesses();
 
 return 0;
}
