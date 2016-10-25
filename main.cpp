//Daniel Sze
//Robert Carneiro
//Brendan Courson
#include "main.h"

int main(int argc, char *argv[])
{
	if(argc < 3) err("Invalid arguments\n");

	std::string inFname=argv[1];
	std::string outFname=argv[2];

    std::ifstream fd(inFname.c_str());
    std::string line;
    std::vector<std::string> lines;
    std::string fileOutput="";

	if(fd != NULL)
	{
        while (std::getline(fd, line)){
            lines.push_back(line);
        }

		std::deque<process> processList;
		process::parse(lines, processList);

		float avgCPUBurstTime=0;
		float avgWaitTime=0;
		float avgTurnAroundTime=0;
		int contextSwitches=0;
		int preemptions=0;

		FCFS(processList, 8, avgCPUBurstTime, avgWaitTime, avgTurnAroundTime, contextSwitches);

		char buffer[10000];
		sprintf(buffer, "Algorithm FCFS\n");
		sprintf(buffer, "%s-- average CPU burst time: %.2f ms\n", buffer, avgCPUBurstTime);
		sprintf(buffer, "%s-- average wait time: %.2f ms\n", buffer, avgWaitTime);
		sprintf(buffer, "%s-- average turnaround time: %.2f ms\n", buffer, avgTurnAroundTime);
		sprintf(buffer, "%s-- total number of context switches: %d\n", buffer, contextSwitches);
		sprintf(buffer, "%s-- total number of preemptions: %d\n", buffer, preemptions);

        //int n = lines.size();
        //int m = 1;
        int t_cs = 8;
        //int t_slice = 84;
		avgCPUBurstTime=0;
		avgWaitTime=0;
		avgTurnAroundTime=0;
		contextSwitches=0;
		preemptions=0;

        SJF(processList, t_cs, avgCPUBurstTime, avgWaitTime, avgTurnAroundTime, contextSwitches);

		sprintf(buffer, "%sAlgorithm SJF\n", buffer);
		sprintf(buffer, "%s-- average CPU burst time: %.2f ms\n", buffer, avgCPUBurstTime);
		sprintf(buffer, "%s-- average wait time: %.2f ms\n", buffer, avgWaitTime);
		sprintf(buffer, "%s-- average turnaround time: %.2f ms\n", buffer, avgTurnAroundTime);
		sprintf(buffer, "%s-- total number of context switches: %d\n", buffer, contextSwitches);
		sprintf(buffer, "%s-- total number of preemptions: %d\n", buffer, preemptions);

		fileOutput+=buffer;

        fd.close();
	}

	std::ofstream fd2(outFname.c_str());

	if(fd2 != NULL)
	{
		fd2.write(fileOutput.c_str(), fileOutput.length());
		fd2.close();
	}

    return 0;
}

std::string intTOstring(int number)
{
   std::stringstream ss;
   ss<<number;
   return ss.str();
}

std::string floatTOstring(float number)
{
   std::stringstream ss;
   ss<<number;
   return ss.str();
}

std::string queueToString(std::deque<process> queue){
    if (queue.empty()){
        return "[Q empty]";
    }
    std::string queueString = "[Q ";

    while (!queue.empty()){
        queueString += queue.front().getID();
        queue.pop_front();

        if (!queue.empty())
            queueString += " ";
    }
    
    queueString += "]";
    return queueString;
}

void sortFCFS(std::deque<process>& processList, std::deque<process>& cpuQ, std::deque<process>& ioQ)
{
	std::sort(processList.begin(), processList.end(), process::FCFSComp);
	std::sort(cpuQ.begin(), cpuQ.end(), process::FCFSComp);
	std::sort(ioQ.begin(), ioQ.end(), process::FCFSComp);
}

void FCFS(std::deque<process> processList, int t_cs,
	float& avgCPUBurstTime, float& avgWaitTime, float& avgTurnAroundTime, int& contextSwitches)
{
	avgCPUBurstTime=0;
	int burstCount=0;
	avgWaitTime=0;
	int waitCount=0;
	avgTurnAroundTime=0;
	contextSwitches=0;

	//int m=1;
	process running=process();

	std::deque<process> cpuQ;
	std::deque<process> ioQ;
	std::cout<<"time 0ms: Simulator started for FCFS ";
	std::cout<<queueToString(cpuQ)<<std::endl;

	//sort the processes
	sortFCFS(processList, cpuQ, ioQ);

	//"run the processes"
	int timeElapsed=0;
	int pSize=processList.size();
	while((processList.size() > 0) ||//arrivals
		(cpuQ.size() > 0) ||//cpu
		(ioQ.size() > 0) ||//io
		(running.getID() != ""))//the cpu is still running
	{
		process ap;
		process cpup;
		process iop;
		
		//arrivals
		if(!processList.empty())
			ap=processList.front();
		//cpu
		if(!cpuQ.empty())
			cpup=cpuQ.front();
		//io
		if(!ioQ.empty())
			iop=ioQ.front();

		//still have arrivals left
		bool taskCompleted=false;
		if(!processList.empty())
		{
			if(((cpuQ.empty()) || (ap.getInitialArrivalTime() < cpup.getArrivalTime())) &&
				((ioQ.empty()) || (ap.getInitialArrivalTime() < iop.getArrivalTime())))
			{
				//set the timeElapsed and print
				timeElapsed=ap.getInitialArrivalTime();
				std::cout<<"time "<<timeElapsed<<"ms: ";
				std::cout<<"Process "<<ap.getID()<<" arrived ";

				//set the new arrival time
				if(!cpuQ.empty())
				{
					ap.setArrivalTime(cpuQ.back().getArrivalTime()+t_cs);
					avgWaitTime+=(cpuQ.back().getArrivalTime()+t_cs)-timeElapsed;
				}
				else
				{
					if(running.getID() != "")
					{
						ap.setArrivalTime(running.getArrivalTime()+(t_cs));
						avgWaitTime+=(running.getArrivalTime()+(t_cs))-timeElapsed;
					}
					else
					{
						ap.setArrivalTime(timeElapsed+(t_cs/2));
						avgWaitTime+=(t_cs/2);
					}
				}

				//set some data
				burstCount+=ap.getNumBursts();
				++waitCount;
				++contextSwitches;
				taskCompleted=true;

				//change the deques
				cpuQ.push_back(ap);
				processList.pop_front();
			}
		}

		if((!taskCompleted) && (running.getID() != ""))//cpu
		{
			if((cpuQ.empty()) || (running.getArrivalTime() < cpup.getArrivalTime()))
			{
				//set the timeElapsed and print
				timeElapsed=running.getArrivalTime();
				std::cout<<"time "<<timeElapsed<<"ms: ";

				//for the turnaround time
				if(running.getNumBurstsLeft() == running.getNumBursts())
					avgTurnAroundTime+=timeElapsed-running.getInitialArrivalTime();

				//do a burst
				running.decrementNumBurstsLeft();

				//just another burst
				if(running.getNumBurstsLeft() > 0)
				{
					std::cout<<"Process "<<running.getID()<<" completed a CPU burst; ";
					std::cout<<running.getNumBurstsLeft()<<" to go ";
					std::cout<<queueToString(cpuQ)<<std::endl;

					//IO blocked
					std::cout<<"time "<<timeElapsed<<"ms: ";
					std::cout<<"Process "<<running.getID()<<" blocked on I/O until time ";
					std::cout<<(timeElapsed+running.getIOTime())<<"ms ";

					//set the new arrival time
					running.setArrivalTime(timeElapsed+running.getIOTime());

					//change the deques
					ioQ.push_back(running);

					//clear running
					running=process();
				}
				else//no more bursts left
				{
					std::cout<<"Process "<<running.getID()<<" terminated ";

					//set the new arrival time
					running.setArrivalTime(timeElapsed+running.getInitialArrivalTime());

					//clear running
					running=process();
				}

				taskCompleted=true;
			}
		}

		if((!taskCompleted) && (running.getID() == ""))//cpu start
		{
			if(!cpuQ.empty())
			{
				if((ioQ.empty()) || (cpup.getArrivalTime() < iop.getArrivalTime()))
				{
					//set the timeElapsed and print
					timeElapsed=cpup.getArrivalTime();
					std::cout<<"time "<<timeElapsed<<"ms: ";

					std::cout<<"Process "<<cpup.getID()<<" started using the CPU ";

					//set the new arrival times
					cpup.setArrivalTime(timeElapsed+cpup.getCPUBurstTime());
					for(unsigned int i=0;i<cpuQ.size();++i)
					{
						cpuQ[i].setArrivalTime(cpuQ[i].getArrivalTime()+cpup.getCPUBurstTime());
						avgWaitTime+=(cpup.getCPUBurstTime());
					}

					//set some data
					avgCPUBurstTime+=cpup.getCPUBurstTime();
					running=cpup;
					taskCompleted=true;

					//change the deques
					cpuQ.pop_front();
				}
			}
		}

		if(!taskCompleted)//io
		{
			if(!ioQ.empty())
			{
				//set the timeElapsed and print
				timeElapsed=iop.getArrivalTime();
				std::cout<<"time "<<timeElapsed<<"ms: ";
				std::cout<<"Process "<<iop.getID()<<" completed I/O ";

				//set the new arrival time
				//set the new arrival time
				if(!cpuQ.empty())
				{
					iop.setArrivalTime(cpuQ.back().getArrivalTime()+t_cs);
					avgWaitTime+=(cpuQ.back().getArrivalTime()+t_cs)-timeElapsed;
				}
				else
				{
					if(running.getID() != "")
					{
						iop.setArrivalTime(running.getArrivalTime()+(t_cs));
						avgWaitTime+=(running.getArrivalTime()+t_cs)-timeElapsed;
					}
					else
					{
						iop.setArrivalTime(timeElapsed+(t_cs/2));
						avgWaitTime+=(t_cs/2);
					}
				}

				//set some data
				++contextSwitches;
				++waitCount;
				taskCompleted=true;

				//change the deques
				cpuQ.push_back(iop);
				ioQ.pop_front();
			}
		}

		std::cout<<queueToString(cpuQ)<<std::endl;

		//sort
		sortFCFS(processList, cpuQ, ioQ);
	}

	timeElapsed+=4;

	std::cout<<"time "<<timeElapsed<<"ms: Simulator ended for FCFS"<<std::endl;

	avgCPUBurstTime/=burstCount;
	avgWaitTime/=waitCount;
	avgTurnAroundTime/=pSize;
}

void SJF(std::deque<process> processList, int t_cs, float& avgCPUBurstTime,
        float& avgWaitTime, float& avgTurnAroundTime, int& contextSwitches)
{
	int burstCount=0;
    int waitCount=0;
    int timeElapsed = 0;
    int initialProcesses = 0;
    process *currentProcess = new process();
    std::deque<process> *readyQueue = new std::deque<process>; 
    std::deque<process> *ioQueue = new std::deque<process>;
    bool busy = false;
    bool nonSwitch = true;
    int numNonSwitches = 0;

    //gathering the total number of bursts
    for (std::deque<process>::iterator itr = processList.begin(); itr != processList.end(); itr++){
        burstCount += itr->getCPUBurstTime()*itr->getNumBursts();
    }

    std::cout << "time " << timeElapsed << "ms: Simulator started for SJF " 
        << queueToString(*readyQueue) << std::endl;

    while (1){
        //check to see if any processes are able to arrive
        for (std::deque<process>::iterator itr = processList.begin(); itr != processList.end(); itr++){
            process *newProc = new process(*itr);

            if (itr->getInitialArrivalTime() == timeElapsed){
                readyQueue->push_back(*newProc);
                std::sort(readyQueue->begin(), readyQueue->end(), process::SJTComp);
                std::cout << process::printTime(timeElapsed) << "Process " 
                    << newProc->getID() << " arrived " << queueToString(*readyQueue) << std::endl;
                initialProcesses++;
                processList.erase(itr);
				delete newProc;
            }
            
            if (itr == processList.end())
                break;
        }

        std::sort(readyQueue->begin(), readyQueue->end(), process::SJTComp);

        //process finished
        if (busy && currentProcess->getCPUBurstTime() == currentProcess->getTimeRunning()){
            currentProcess->decrementNumBurstsLeft();

            //no more bursts left
            if (currentProcess->getNumBurstsLeft() == 0){
                std::cout << process::printTime(timeElapsed) << "Process " << 
                   currentProcess->getID() << " terminated " << queueToString(*readyQueue) << std::endl; 
            }
            //process still has some bursts left, block it
            else { 
                initialProcesses++;
                std::cout << process::printTime(timeElapsed) << "Process " << 
                    currentProcess->getID() << " completed a CPU burst; " << 
                    currentProcess->getNumBurstsLeft() << " to go " <<
                    queueToString(*readyQueue) << std::endl;

                int ioBlockTime = timeElapsed +  currentProcess->getIOTime();

                std::cout << process::printTime(timeElapsed) << "Process " <<
                    currentProcess->getID() << " blocked on I/O until time " <<
                    ioBlockTime << "ms " << queueToString(*readyQueue) << std::endl;

                if (readyQueue->empty()){
                    nonSwitch = true;
                }
                
                currentProcess->setIOEnd(ioBlockTime);
                ioQueue->push_back(*currentProcess);
                std::sort(ioQueue->begin(), ioQueue->end(), process::IOComp);
            }

            busy = false;
        }

        //check to see if any IO blocks are finished
        if (!ioQueue->empty()){
            std::sort(ioQueue->begin(), ioQueue->end(), process::IOComp);
            process *io = new process(ioQueue->front());
            if (timeElapsed == io->getIOEnd()){
                if (io->getNumBurstsLeft() == 0){
                    std::cout << process::printTime(timeElapsed) << "Process " <<
                        io->getID() << " terminated " << queueToString(*readyQueue) << std::endl;
                }
                else {
                    io->setTimeRunning(0);

                    if (!busy && readyQueue->empty()){
                        nonSwitch = true;
                    }

                    readyQueue->push_back(*io);
                    std::sort(readyQueue->begin(), readyQueue->end(), process::SJTComp);
                    std::cout << process::printTime(timeElapsed) << "Process " <<
                        io->getID() << " completed I/O " << queueToString(*readyQueue) << std::endl;

                }
                ioQueue->pop_front();
            }
            delete io;
        }

        //can load a new process 
        if (!busy && !readyQueue->empty()){ 
            process *newProc = new process(readyQueue->front());
            *currentProcess = *newProc;
            busy = true; 
            readyQueue->pop_front(); 
            int start = 0; 

            if (nonSwitch){ 
                numNonSwitches++;
                start = timeElapsed + t_cs/2;
                nonSwitch = false; 
            }
            else 
                start = timeElapsed + t_cs; 

            int end = start + currentProcess->getCPUBurstTime();
            currentProcess->setCpuBurstStart(start);
            currentProcess->setCpuBurstEnd(end);
        }
        // can start a process
        else {
            if (timeElapsed == currentProcess->getCpuBurstStart()){
                contextSwitches++;
                std::cout << process::printTime(timeElapsed) << "Process " << currentProcess->getID()
                   << " started using the CPU " << queueToString(*readyQueue) << std::endl; 
            }
        } 

        for (std::deque<process>::iterator itr = readyQueue->begin(); itr != readyQueue->end(); itr++){
            if (currentProcess->getID() != itr->getID())
                waitCount++;
        }

        if (readyQueue->empty() && ioQueue->empty() && !busy){
            timeElapsed += t_cs/2;
            std::cout << process::printTime(timeElapsed) << " Simulator ended for SJF" << std::endl;
            delete currentProcess;
            delete readyQueue;
            delete ioQueue;
            break;
        }

        if (currentProcess->getCpuBurstStart() <= timeElapsed){
            currentProcess->increaseTimeRunning();
        }

        timeElapsed++;
    }

    avgWaitTime = (float)waitCount/(float)contextSwitches;
    avgCPUBurstTime = (float)burstCount/(float)contextSwitches;
    avgTurnAroundTime = ((float)burstCount + (float)waitCount +
            ((float)contextSwitches*(float)t_cs - 0.5*t_cs*numNonSwitches))/(float)contextSwitches;
} 

void roundRobin(const std::deque<process>& processList)
{
	//
}

void err(const char *msg)
{
	fprintf(stderr, "%s", msg);
	exit(EXIT_FAILURE);
}
