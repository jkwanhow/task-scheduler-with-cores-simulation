#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


typedef struct node Node;
typedef struct process Process;
typedef struct cpu CPU;
typedef struct system System;
typedef struct process_queue ProcessQueue;
typedef struct finishNode FinishNode;
typedef struct linkedList LinkedList;

struct process{
    int id;
    int time_remaining;
    int length;
    int start_time;
    int P;
    int subid;
};

struct linkedList{
    int length;
    FinishNode *head;
};

struct finishNode{
    int id;
    int finish_count;
    int Np;
    FinishNode *next;
};

struct node{
    Process *process;
    Node *next;
    Node *prev;
};

struct cpu{
    Process *running;
    int id;
    ProcessQueue *cpuQueue;
    int totalTime;
    CPU *next;
    CPU *prev;
};


struct system{
    CPU *head;
    CPU *tail;
    int Nprocesses;
    int Ncpu;
    
};

struct process_queue{
    Node *head;
    Node *tail;
    int Nprocesses;

};

LinkedList *createLinkedList(){
    LinkedList *linkedlist = (LinkedList *) malloc(sizeof(*linkedlist));
    assert(linkedlist);
    linkedlist -> head = NULL;
    linkedlist -> length = 0;
    return linkedlist;
}

FinishNode *create_finish_check(int ID, int NumberOfP){
    FinishNode *finishnode = (FinishNode *) malloc(sizeof(*finishnode));
    assert(finishnode);

    finishnode -> id = ID;
    finishnode -> finish_count = 0;
    finishnode -> Np = NumberOfP;
    finishnode -> next = NULL;

    return finishnode;
}

void addPFinishChecker(LinkedList *ll, int ID, int numberOfP){
    FinishNode *new = create_finish_check(ID, numberOfP);
    if (ll -> length < 1){
        ll -> head = new;
        ll -> length++;
        return;
    }else{
        new -> next = ll -> head;
        ll -> head = new;
        ll -> length++;
    }

}

Process *createProcess(float new_id, int time_start, int to_run, int P_bool){
    Process *process;
    process = (Process *) malloc(sizeof(*process));
    assert(process);

    process -> id = new_id;
    process -> start_time = time_start;
    process -> length = to_run;
    process -> time_remaining = to_run;
    process -> P = P_bool;
    
    return process;
}

Node *new_node(Process *processlink){
    //Create a node
    Node *node;
    node = (Node *) malloc(sizeof(*node));
    assert(node);

    node -> process = processlink;
    node -> next = NULL;
    node -> prev = NULL;

    return node;
}

ProcessQueue *new_queue(){
    //Create an empty queue
    ProcessQueue *queue;
    queue = (ProcessQueue*) malloc(sizeof(*queue));
    assert(queue);
    queue -> head = NULL;
    queue -> tail = NULL;
    queue -> Nprocesses = 0;

    return queue;
}

CPU *new_CPU(int ID){
    //Create a CPU
    CPU *cpu;
    cpu = (CPU *) malloc(sizeof(*cpu));
    assert(cpu);

    cpu -> id = ID;
    cpu -> running = NULL;
    cpu -> totalTime = 0;
    cpu -> cpuQueue = new_queue();

    return cpu;
}

System *new_system(int numberOfCPUs){
    System *system;
    system = (System *) malloc(sizeof(*system));
    assert(system);
    system -> Ncpu = numberOfCPUs;
    CPU *aCPU;
    CPU *holder;
    for (int i = 0; i < numberOfCPUs; i++){
        aCPU = new_CPU(i);
        if(i == 0){
            system -> head = aCPU;
            system -> tail = aCPU;
        }else{
            system -> tail = aCPU;
            holder -> next = aCPU;
            aCPU -> prev = holder;
        }
        holder = aCPU;
    }

    return system;
}

void queueProcess(ProcessQueue *processes, Node *node){
    if (processes -> Nprocesses == 0){
        processes -> head = node;
        processes -> tail = node;
    }else{
        node -> prev = processes -> tail;
        node -> next = NULL;
        processes -> tail -> next = node;
        processes -> tail = node;
    }
        processes -> Nprocesses++;

}

Node *removeHeadQueue(ProcessQueue *processes){
    Node *toRemove;
    if(processes -> Nprocesses == 0){
        return NULL;
    }
    if (processes -> Nprocesses == 1){
        toRemove = processes -> head;
        processes -> head = NULL;
        processes -> tail = NULL;
        processes -> Nprocesses--;
        
    }else{
        toRemove = processes -> head;
        processes -> head = toRemove -> next;
        processes -> head -> prev = NULL;
        toRemove -> next = NULL;
        processes -> Nprocesses--;
    }

    return toRemove;
}
Node *removTailQueue(ProcessQueue *processes){
    Node *toRemove;
    if (processes -> Nprocesses == 1){
        toRemove = processes -> head;
        processes -> head = NULL;
        processes -> tail = NULL;
        processes -> Nprocesses--;
        
    }else{
        toRemove = processes -> tail;
        processes -> tail = toRemove -> prev;
        processes -> tail -> next = NULL;
        toRemove -> prev = NULL;
        processes -> Nprocesses--;
    }

    return toRemove;
}

void swapProcessData(Node *a, Node *b){
    Process *temp = a -> process;
    a -> process = b -> process;
    b -> process = temp;
}

void sortProcessQueue(ProcessQueue *processes, int currentTime){
    // We want to make sure if the current time has not reached the point,
    // processes with lower run time that start later don't go to the bottom
    // of our queue

    //using bubble sort to sort cuz im lazy
    if (processes -> Nprocesses <= 1){
        return ;
    }
    int swapped = 1;
    Node *n1;
    Node *nl = NULL;
    
    while (swapped == 1){
        n1 = processes -> head;
        swapped = 0;
        while(n1 -> next != nl){
            if (n1 -> process -> time_remaining > n1 -> next ->process -> time_remaining){
                if(n1 -> next -> process -> start_time <= currentTime){
                    swapProcessData(n1, n1->next);
                    swapped = 1;
                }
            }else if(n1 -> process -> time_remaining == n1 ->next ->process -> time_remaining){
                if ((n1 -> process -> id > n1 ->next ->process -> id) 
                && (n1 -> next -> process -> start_time <= currentTime)){
                    swapProcessData(n1, n1->next);
                    swapped = 1;
                }
            }
            n1 = n1 -> next;
        }
        nl = n1;
    }
        n1 = processes -> head;

}

void addCPUQ(CPU *cpu, Node *node){
    ProcessQueue *cpuQ = cpu -> cpuQueue;
    queueProcess(cpuQ, node);
    cpu -> totalTime += node -> process -> time_remaining;
}

int manageCPUQ(CPU *cpu, int run_time){

    Process *oldRunning = cpu -> running;
    ProcessQueue *cpuQ = cpu -> cpuQueue;
    sortProcessQueue(cpuQ, run_time);
    cpu -> running = cpuQ -> head -> process;
    if (cpu -> running == NULL){
        return 0;
    }
    if (cpu -> running != oldRunning){
        return 1;
    }

    return 0;
}
void run(CPU *cpu){

    cpu -> running = cpu ->cpuQueue -> head -> process;
    cpu -> cpuQueue -> head = NULL;
}

void swapCPUData(CPU *a, CPU *b){
    Process *temprunning = a -> running;
    int tempid = a -> id;
    ProcessQueue *tempcpuQueue = a -> cpuQueue;
    int temptotalTime = a -> totalTime;

    a -> running = b -> running;
    a -> cpuQueue = b -> cpuQueue;
    a -> id = b -> id;
    a -> totalTime = b -> totalTime;

    b -> running = temprunning;
    b -> id = tempid;
    b -> cpuQueue = tempcpuQueue;
    b -> totalTime = temptotalTime;

}

void sortSystemforPrint(System *system){
        //Sort system with bubble sort
        if (system -> Ncpu <= 1){
        return ;
    }
    int swapped = 1;
    CPU *s1;
    CPU *sl = NULL;
    
    while (swapped == 1){
        s1 = system -> head;
        swapped = 0;
        while(s1 -> next != sl){
            if (s1 -> id > s1 -> next -> id){
                swapCPUData(s1, s1->next);
                swapped = 1;
            }
            
            s1 = s1 -> next;
        }
        sl = s1;
    }
}

void sortSystem(System *system){
    //Sort system with bubble sort
        if (system -> Ncpu <= 1){
        return ;
    }
    int swapped = 1;
    CPU *s1;
    CPU *sl = NULL;
    
    while (swapped == 1){
        s1 = system -> head;
        swapped = 0;
        while(s1 -> next != sl){
            if (s1 -> totalTime > s1 ->next -> totalTime){
                swapCPUData(s1, s1->next);
                swapped = 1;
            }else if(s1 -> totalTime == s1 ->next -> totalTime){
                if (s1 -> id > s1 ->next -> id){
                    swapCPUData(s1, s1->next);
                    swapped = 1;
                }
            }
            s1 = s1 -> next;
        }
        sl = s1;
    }

}

void printProcesses(ProcessQueue *processes){
    Node *current = processes -> head;
    Process *curproc;
    for (int i = 0;i < processes -> Nprocesses; i++){
        curproc = current -> process;
        printf("\n id = %d start_time = %i run_time = %i", curproc -> id, curproc -> start_time, curproc -> time_remaining);
        current = current -> next;
    }
}

