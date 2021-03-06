#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
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
int quantum = 0;
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
	bool is_active;
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
	if(global_time < min)	
		global_time = min;
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

		double throughput = 100.00 * finqueue.size() / s.ft;
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
			t.state = 1;
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
		return t;	*/

		process t = rqueue[0];
		int pos=0;;
		for(int i=0;i<rqueue.size();i++)
		{
			if(rqueue[i].at <= t.at && rqueue[i].at <= global_time)
			{
				if(rqueue[i].at != t.at || (rqueue[i].at==t.at && rqueue[i].pid < t.pid && t.tempcb!=-100))
				{
					t = rqueue[i];
					pos = i;
				}
			}
		}
		
		if(global_time > t.at)
			t.at = global_time;

		rqueue.erase(rqueue.begin() + pos);
		return t;
	}

	void run_scheduler()
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

			//cout<<global_time<<" "<<t.pid<<" "<<"cb : "<<t.tempcb<<" "<<t.tc<<" "<<t.stat_prio-1<<endl;


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
						//cout<<"IO : "<<tempio<<endl;
						for(int k=0;k<tempio;k++)
						{
							io[k+t.at]=1;
						}
						t.at +=  tempio;
						t.it +=  tempio;
						t.tempcb = -100;
						initialize_ready_queue(rqueue);
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
				//cout<<"IO : "<<tempio<<endl;
				for(int k=0;k<tempio;k++)
				{
					io[k+t.at]=1;
				}
				t.at +=  tempio;
				t.it +=  tempio;
				t.tempcb = -100;
				initialize_ready_queue(rqueue);
				enqueue(t);
				if(rqueue.size()==0 && pqueue.size()!=0)
					setglobaltime();
			}

			else if(quantum > t.tc && quantum > t.tempcb)
			{	
				if(t.tempcb >= t.tc)
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
				else
				{
					t.tc -= t.tempcb;
					t.at += t.tempcb;
					global_time += t.tempcb;
					if(global_time>t.at) 
						t.at = global_time;
					
					int tempio = myrandom(t.io);
					//cout<<"IO : "<<tempio<<endl;
					for(int k=0;k<tempio;k++)
					{
						io[k+t.at]=1;
					}
					t.at +=  tempio;
					t.it +=  tempio;
					t.tempcb = -100;
					initialize_ready_queue(rqueue);
					enqueue(t);
					if(rqueue.size()==0 && pqueue.size()!=0)
						setglobaltime();

				}
			}

			if(rqueue.size()==0 && pqueue.size()!=0)
				setglobaltime();
			initialize_ready_queue(rqueue);


			/*if(rqueue.size()>0)
				cout<<rqueue.size()<<" "<<global_time<<" "<<rqueue[0].pid<<endl;*/

		}
		cout<<"RR "<<quantum<<endl;
	}

};



///////////////////////////////////////////////////////////////////////// RR Ends &&  prio starts


class prio : public scheduler
{
public:

	void initialize_ready_queue(vector<process> &rqueue)
	{
		vector<int> deletepid;
		for(int i=0;i<pqueue.size();i++)
		{
			if(pqueue[i].at <= global_time)
			{ 
				pqueue[i].is_active = 1;
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
		process t = rqueue[0];
		int pos=0;
		for(int i=0;i<rqueue.size();i++)
		{
			if(rqueue[i].dyn_prio >= t.dyn_prio && rqueue[i].at <= global_time && rqueue[i].is_active == 1)
			{
				if(rqueue[i].dyn_prio != t.dyn_prio || (rqueue[i].dyn_prio==t.dyn_prio && rqueue[i].at < t.at))
				{
					t = rqueue[i];
					pos = i;
				}
			}
		}
		
		if(global_time > t.at)
			t.at = global_time;

		rqueue.erase(rqueue.begin() + pos);
		return t;
	}

	void run_scheduler()
	{
		setglobaltime();
		vector<process> aqueue;
		vector<process> equeue;

		initialize_ready_queue(aqueue);

		while(!pqueue.empty() || !aqueue.empty() || !equeue.empty())
		{
			process t = dequeue(aqueue);

			if(t.tempcb==-100)
			{
				int tempcb = myrandom(t.cb);
				if(tempcb > t.tc)
					tempcb = t.tc;
				t.tempcb = tempcb;	
			}

//			cout<<global_time<<" "<<t.pid<<" "<<"cb : "<<t.tempcb<<" "<<t.tc<<" "<<t.dyn_prio<<endl;

			if(quantum <= t.tc && quantum <= t.tempcb)
			{
				t.tc -= quantum;
				t.tempcb -= quantum;
				global_time += quantum;
				t.at += quantum;

				t.dyn_prio -= 1;

				if(t.tempcb==0)
				{
					if(t.tc == 0 )
					{
						t.ft = t.at;
						summary.ft=t.ft;
						t.tt = t.ft - t.begat;
						t.cw = t.tt - t.begtc - t.it;
						finqueue.push_back(t);
						if(aqueue.size()==0 && pqueue.size()!=0 && equeue.size()==0)
							setglobaltime();
					}
					else
					{
						int tempio = myrandom(t.io);
//						cout<<"IO : "<<tempio<<endl;
						for(int k=0;k<tempio;k++)
						{
							io[k+t.at]=1;
						}
						t.at +=  tempio;
						t.it +=  tempio;

						t.dyn_prio = t.stat_prio - 1;
						
						t.tempcb = -100;
						initialize_ready_queue(aqueue);
						enqueue(t);
						if(aqueue.size()==0 && pqueue.size()!=0 && equeue.size()==0)
						{
							setglobaltime();
						}
					}
				}
				else
				{	
					initialize_ready_queue(aqueue);
					if(t.dyn_prio == -1)
					{
						t.dyn_prio = t.stat_prio - 1;
						equeue.push_back(t);
					}
					else
						aqueue.push_back(t);
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
//				cout<<"IO : "<<tempio<<endl;
				for(int k=0;k<tempio;k++)
				{
					io[k+t.at]=1;
				}
				t.at +=  tempio;
				t.it +=  tempio;
				t.tempcb = -100;
				t.dyn_prio = t.stat_prio - 1;
				initialize_ready_queue(aqueue);
				enqueue(t);
				if(aqueue.size()==0 && pqueue.size()!=0 && equeue.size()==0)
					setglobaltime();
			}

			else if(quantum > t.tc && quantum > t.tempcb)
			{	
				if(t.tempcb >= t.tc)
				{
					global_time += t.tc;
					t.at += t.tc;
					t.tc = 0;
					t.ft = t.at;
					summary.ft=t.ft;
					t.tt = t.ft - t.begat;
					t.cw = t.tt - t.begtc - t.it;
					finqueue.push_back(t);
					if(aqueue.size()==0 && pqueue.size()!=0 && equeue.size()==0)
						setglobaltime();
				}
				else
				{
					t.tc -= t.tempcb;
					t.at += t.tempcb;
					global_time += t.tempcb;
					if(global_time>t.at) 
						t.at = global_time;
					
					int tempio = myrandom(t.io);
//					cout<<"IO : "<<tempio<<endl;
					for(int k=0;k<tempio;k++)
					{
						io[k+t.at]=1;
					}
					t.at +=  tempio;
					t.it +=  tempio;
					t.tempcb = -100;
					t.dyn_prio = t.stat_prio - 1;
					initialize_ready_queue(aqueue);
					enqueue(t);
					if(aqueue.size()==0 && pqueue.size()!=0 && equeue.size()!=0)
						setglobaltime();

				}
			}

			if(aqueue.size()==0 && pqueue.size()!=0 && equeue.size()==0)
				setglobaltime();

			initialize_ready_queue(aqueue);

			if(aqueue.size()==0 && equeue.size()!=0)
			{
				for(int i=0;i<equeue.size();i++)
					aqueue.push_back(equeue[i]);

				equeue.clear();
			}


			/*if(rqueue.size()>0)
				cout<<rqueue.size()<<" "<<global_time<<" "<<rqueue[0].pid<<endl;*/

		}
		cout<<"PRIO "<<quantum<<endl;
	}

};




////////////////////////////////////////////////////////////////////////////////////////// PRIO ENDS

void readprocesses(string s)
{
	myfile.open(s.c_str());
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
        p_temp.state=0;
        p_temp.it = 0;
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
	myfile.open(s.c_str());
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
		process p = finqueue[i];

		printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
	    p.pid,
	    p.begat, 
	    p.begtc,
	    p.cb, 
	    p.io,
	    p.stat_prio,
	    p.ft, // last time stamp
	    p.tt,
	    p.it,
	    p.cw);

	}
}

void displaysummary()
{	
	summary.createsummary(summary);
	printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
	       summary.ft,
	       summary.cpuUt,
	       summary.ioUt,
	       summary.turnaroundavg,
	       summary.cpuwaitavg, 
	       summary.throughput);
}

void initialize(scheduler **s, char *stype)
{
    if(stype[0] == 'F')
		*s = new fcfs();
	else if(stype[0] == 'S')
		*s = new sjf();
	else if(stype[0] == 'L')
		*s = new lcfs();
	else if(stype[0]== 'P' || stype[0] == 'R')
	{
		int count = 0;
		int i = 1;
		while(stype[i]!='\0')
		{
			count++;
			i++;
		}
		char q[count];
		for(int j=1;j<=count;j++)
		{
			q[j-1] = stype[j];
		}
		quantum = atoi(q);
		if(stype[0] == 'P')
			*s = new prio();
		else
			*s = new rr();
	}

}


int main(int argc, char **argv)
{

 ///////////////// Reading optional arguements
 int c;
 opterr = 0;
 int vflag = 0;
 char *svalue;
 while ((c = getopt (argc, argv, "vs:")) != -1)
 	switch (c)
 {
 	case 'v' :
 		vflag = 1;
 		break;

 	case 's' :
 		svalue = optarg;
 		break;
 	default:
 		abort ();
 }



 ////////////////// Reading input file path and random file path
 string path1 = argv[optind+1];
 string path2 = argv[optind];
 //////////////////////////////////	
 
 readrandomfile(path1);
 readprocesses(path2);
 

 scheduler *s;
 initialize(&s, svalue);
 s->run_scheduler();
 
 sortOutput();
 s->display();

 return 0;
}
