#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "structure.h"

ProcessQueue *parse_inputfile(char *filepath, int CPUs, LinkedList *ll);
void finishPrint(ProcessQueue *myQ, int remaining, int runtime);
void n_linear_CPUs_run(int CPUs, ProcessQueue *processes, LinkedList *finishList);
int finishProcess(Process *process, int runTime, int remaining, LinkedList *ll, ProcessQueue *finishQ);
void printRunning(Process *process, int runTime, int coreID);
FinishNode *getPinfo (LinkedList *ll, int ID);
void IDoItMyWay(ProcessQueue *processes, int numberOfCPUs);

int TOTAL_TURNAROUND = 0;
int TOTALQs = 0;
float TOTAL_OVERHEAD = 0;
float MAX_OVERHEAD = 0;

int main(int argc, char *argv[]){
    int cflag = 0;
    int pflag = 0;
    int fflag = 0;
    char *fvalue = NULL;
    int c;
    int processors;

    //Read the input arguments required
    while ((c = getopt(argc, argv, "c:p:f:")) != -1)
        switch (c){
            case 'c':
                cflag = 1;
                processors = atoi(optarg);
                //printf("\n%i processors", processors);
                break; 

            case 'p':
                pflag = 1;
                processors = atoi(optarg);
                //printf("\n%i processors", processors);
                break; 

            case 'f':
                fflag = 1;
                fvalue = optarg;
                //printf("\n%s filename", fvalue);
                break;

            case '?':
                printf("\nError: invalid input commands\n try -p <number of processors> \n -f <filepath>");
                return -1;
                
        }
        /*
        // make sure that the -p function have been called
        if (pflag == 0){
            printf("\nError: have to provide number of proccessors through -p\n");
            return -1;
        }
        // make sure that a filepath has been given
        if (fflag != 1){
            printf("\nA file path has to be given use command -f <filepath> to provide filepath\n");
            return -1;
        }
        */

        LinkedList *PList = createLinkedList();
        ProcessQueue *processes = parse_inputfile(fvalue, processors, PList);
        sortProcessQueue(processes, 0);
        //printProcesses(processes);
        if (pflag == 1){
            n_linear_CPUs_run(processors, processes, PList);
        }else if(cflag == 1){
            IDoItMyWay(processes, processors);
        }
        /*
        if (processors == 1){
            single_processor_scheduler(processes);
        }else{
            printf("\nare we here?");
            n_linear_processors(processors, processes);
        }
        */
    return 0;

}

ProcessQueue *parse_inputfile(char *filepath, int CPUs, LinkedList *ll){
    //reads the textfile of input and puts it into a deque/double linked list data structure
    FILE *fp;
    ProcessQueue *processes = new_queue();
    Process *process;
    int id;
    int dividingint;
    int time_remaining;
    int start_time;
    int time_remaining_split;
    char pchar;
    int p;
    int i;
    int k;

    fp = fopen(filepath, "r");
    if (fp == NULL){
        exit(EXIT_FAILURE);
    }

    while(fscanf(fp, "%i %i %i %c\n", &start_time, &id, &time_remaining, &pchar) != EOF){
        if (pchar == 'p'){
            p = 1;
        }else{
            p = 0;
        }
        if (p == 0){
            process = createProcess(id, start_time, time_remaining, p);
            queueProcess(processes, new_node(process));
        }else if((p == 1) && (CPUs > 1)){
            for (k = 1; k < CPUs; k++){
                if( (time_remaining/k) < 1){
                    k--;
                    break;
                }
            }
            addPFinishChecker(ll, id, k);
            dividingint = ceil((float)time_remaining/(float)k);
            time_remaining_split = dividingint + 1;

            for (i = 0; i < k; i++){
                process = createProcess(id, start_time, time_remaining_split, p);
                process -> subid = i;
                process -> length = time_remaining;
                queueProcess(processes, new_node(process));
            }
        }
    }

    return processes;
}

FinishNode *getPinfo (LinkedList *ll, int ID){
    FinishNode *current;
    current = ll -> head;
    for (int i = 0; i < ll -> length; i++){
        if (current ->id == ID){
            break;
        }
        current = current -> next;
    }

    return current;
}

void finishPrint(ProcessQueue *myQ, int remaining,int runTime){
    Node *current;
    while (myQ -> Nprocesses > 0){
        current = removeHeadQueue(myQ);
        Process *process = current -> process;
        int finish_id = process -> id;
        printf("%i,FINISHED,pid=%i,proc_remaining=%i\n", runTime, finish_id, remaining);
        free(current);
        
    }
}

int finishProcess(Process *process, int runTime, int remaining, LinkedList *ll, ProcessQueue *finishQ){
    int finish_id = process -> id;
    int turnaround;
    float overhead;
    if (process -> P == 0){
        queueProcess(finishQ, new_node(process));
        //printf("%i,FINISHED,pid=%i,proc_remaining=%i\n", runTime, finish_id, remaining);
        turnaround = runTime - process -> start_time;
        overhead = (float) turnaround / (float) process -> length;
        if (overhead > MAX_OVERHEAD){
            MAX_OVERHEAD = overhead;
        }
        TOTAL_OVERHEAD += overhead;
        TOTAL_TURNAROUND += turnaround;
        remaining--;
        return remaining;
    }else if (process -> P ==1){
        FinishNode *myProcess;
        myProcess = getPinfo(ll, finish_id);
        myProcess -> finish_count++;

        if (myProcess -> finish_count >= myProcess -> Np){
            queueProcess(finishQ, new_node(process));
            //printf("%i,FINISHED,pid=%i,proc_remaining=%i\n", runTime, finish_id, remaining);
            turnaround = runTime - process -> start_time;
            overhead = (float) turnaround / (float) process -> length;
            if (overhead > MAX_OVERHEAD){
                MAX_OVERHEAD = overhead;
            }
            TOTAL_OVERHEAD += overhead;

            
            TOTAL_TURNAROUND += turnaround;
            remaining--;
            return remaining;
        }

    }
    return remaining;
}

void printRunning(Process *process, int runTime, int coreID){
    int ID = process -> id;
    int subID = process -> subid;
    int Timeremaining = process -> time_remaining;
    if (process -> P == 0){
        printf("%i,RUNNING,pid=%i,remaining_time=%i,cpu=%i\n",runTime, ID, Timeremaining, coreID);
    }else if(process -> P == 1){
        printf("%i,RUNNING,pid=%i.%i,remaining_time=%i,cpu=%i\n",runTime, ID, subID, Timeremaining, coreID);
    }
}

void n_linear_CPUs_run(int NumberCPUs, ProcessQueue *processes, LinkedList *finishList){
    int processes_remaining = processes -> Nprocesses;
    int TOTALPs = processes -> Nprocesses;
    int proc_qed = 0;
    int id_check = -1;
    ProcessQueue *finishSet;
    System *mySystem = new_system(NumberCPUs);
    int run_time = 0;
    int i;

    Node *currentNode;
    CPU *currentCPU;
    while (processes_remaining > 0){
        //printf("\n -- run time %i -- \n", run_time);
        if (processes -> Nprocesses > 0){
            sortProcessQueue(processes, run_time);
            currentNode = processes -> head;
            while (currentNode -> process -> start_time <= run_time){
                sortSystem(mySystem);
                currentCPU = mySystem -> head;
                addCPUQ(currentCPU, removeHeadQueue(processes));
                if(currentNode -> process -> id != id_check){
                    proc_qed++;
                    TOTALQs++;
                    id_check = currentNode -> process -> id;
                }
                
                currentNode = processes -> head;
                if(processes -> Nprocesses <= 0){
                    break;
                }

            }
        }
        sortSystemforPrint(mySystem);
        currentCPU = mySystem -> head;
       
        for (i = 0; i < NumberCPUs; i++){
            if (currentCPU -> cpuQueue -> Nprocesses > 0){
                if (manageCPUQ(currentCPU, run_time) == 1){
                    
                    printRunning(currentCPU -> running, run_time, currentCPU ->id);
                }
            }
            currentCPU = currentCPU -> next;
        }

        run_time++;
        currentCPU = mySystem -> head;
        finishSet = new_queue();
        for (i = 0; i < NumberCPUs; i++){
            if (currentCPU -> running){
                currentCPU -> running -> time_remaining--;
                currentCPU -> totalTime--;
                if (currentCPU -> running -> time_remaining <= 0){
                    processes_remaining--;
                    proc_qed = finishProcess(currentCPU -> running, run_time, proc_qed, finishList, finishSet);
                    removeHeadQueue(currentCPU->cpuQueue);
                    currentCPU->running = NULL;
                }
            }
            currentCPU = currentCPU -> next;
        }
        finishPrint(finishSet, proc_qed, run_time);
        free(finishSet);

    }

    int average_turnaround = ceil((float) TOTAL_TURNAROUND/ (float) TOTALQs);
    printf("Turnaround time %i\n", average_turnaround);
    float averageoverhead = TOTAL_OVERHEAD/TOTALQs;
    averageoverhead = ((int) (averageoverhead*100 +0.5) /100.0);
    MAX_OVERHEAD = ((int) (MAX_OVERHEAD *100 +0.5) /100.0);
    printf("Time overhead %g %g\n", MAX_OVERHEAD, averageoverhead);
    printf("Makespan %i\n", run_time);
}

void IDoItMyWay(ProcessQueue *processes, int numberOfCPUs){
    int processes_remaining = processes -> Nprocesses;
    int TOTALPs = processes -> Nprocesses;
    Node *bestNode;
    CPU *currentCPU;
    Process *runningProcess;
    int run_time = 0;
    int proc_qed = 0;
    int i;
    System *mySystem = new_system(numberOfCPUs);
    while (processes_remaining > 0){
        if (processes -> Nprocesses > 0){
            currentCPU = mySystem -> head;
            for (i=0;i<numberOfCPUs;i++){
                sortProcessQueue(processes, run_time);
                runningProcess = currentCPU -> running;
                bestNode = processes -> head;
                //printf("\nbug test\n");
                if(runningProcess == NULL){
                    //printf("\nbug test2\n");
                    removeHeadQueue(currentCPU -> cpuQueue);
                    addCPUQ(currentCPU, removeHeadQueue(processes));
                    run(currentCPU);
                    //printf("\nbug test --%i--\n", currentCPU -> running -> id);
                    printf("%i,RUNNING,pid=%i,remaining_time=%i,cpu=%i\n",run_time, currentCPU -> running -> id, 
                    currentCPU->running->time_remaining, currentCPU -> id);
                }else if (runningProcess -> time_remaining > bestNode -> process -> time_remaining){
                    queueProcess(processes, new_node(runningProcess));
                    removeHeadQueue(currentCPU -> cpuQueue);
                    addCPUQ(currentCPU, removeHeadQueue(processes));
                    run(currentCPU);
                    printf("%i,RUNNING,pid=%i,remaining_time=%i,cpu=%i\n",run_time, currentCPU -> running -> id, 
                    currentCPU->running->time_remaining, currentCPU -> id);
                    
                }

                if(processes -> Nprocesses <= 0){
                    break;
                }
                currentCPU = currentCPU -> next;
            }
        }
        
        run_time++;
        currentCPU = mySystem -> head;
            for (i = 0; i < numberOfCPUs; i++){
                if (currentCPU -> running){
                    currentCPU -> running -> time_remaining--;
                    currentCPU -> totalTime--;
                    if (currentCPU -> running -> time_remaining <= 0){
                        runningProcess = currentCPU -> running;
                        processes_remaining--;
                        printf("%i,FINISHED,pid=%i,proc_remaining=NULL\n", run_time, runningProcess -> id);
                        free(runningProcess);
                        currentCPU->running = NULL;
                }
            }
            currentCPU = currentCPU -> next;
        }



    }

    printf("Makespan %i\n", run_time);
    return;
}