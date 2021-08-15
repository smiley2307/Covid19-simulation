#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VERTICES 10000
#define MAX_EDGES 3000

//*****************for priority queue*******************
typedef struct event
{
	int vertex;	//stores the corresponding vertex number
	int time;	//stores day on which the event happend. Priority is highest for the event having lowest time
	char action;	//action can be either 'T' (transmit) or 'R' (recovery)
	struct event* next;		//next event in the priority queue
}event_t;

typedef struct priorityQueue
{
	struct event* head;		
}queue_t;
//******************************************************


//**********for maintaining list S,I,R******************
typedef struct listElement
{
	int vertex;
	int time;
	struct listElement* next;
}listElement_t;

typedef struct list
{
	struct listElement* head;
}list_t;
//******************************************************

//*********for printing the final summary, (number of people*****************
//***************susceptible,infected and recovered day wise)****************
typedef struct dayNode
{
	int dayCount;
	int S;
	int I;
	int R;
	struct dayNode* next;
}day_t;
typedef struct sum
{
	day_t* head;
}summary_t;


//****************for graph*****************************
typedef struct node
{   
    char status;
    int pred_inf_time;
    int inf_time;
    int rec_time;
}node_t;

typedef struct grp
{
    int **mat;
    struct node* stat;
}graph_t;

//function prototypes
event_t* create_new_event(int vertex, int time, char action);
queue_t* create_new_queue();
void pop(queue_t* Q);
void push(queue_t* Q, int vertex, int time, char action);
listElement_t* create_new_ListElement(int vertex, int time);
list_t* create_new_list();
void insert_ListElement_atTail(list_t* L, int vertex, int time);
void delete_listElement(list_t* L, int vertex);
int number_of_listElements(list_t* L);
day_t* create_new_day(int dayCount, int S, int I, int R);
summary_t* create_new_summary();
void insert_day_inSummary(summary_t* D, int dayCount, int S, int I, int R);
void printSummary(summary_t* D);
void initialize(graph_t* G, int numberOfVertices, int maxNumberOfEdges);
void graphLinker(graph_t* G, int numberOfVertices, int maxNumberOfEdges);
int transmitting_time(float tau);
int recovery_time(float gamma);
void find_trans_SIR(graph_t* G, queue_t* Q, int time, float tau, int sourceVertex, int targetVertex, int tmax);
void process_trans_SIR(graph_t* G, int vertex, int time, float tau, float gamma, list_t* S, list_t* I, list_t* R, queue_t* Q, int tmax, int maxNumberOfEdges, summary_t* D);
void process_rec_SIR(graph_t* G,int vertex, int time, list_t* S, list_t* I, list_t* R, summary_t* D);
void fast_SIR(graph_t* G, queue_t* Q, float tau, float gamma, int tmax, int numberOfVertices, int maxNumberOfEdges, list_t* S, list_t* I, list_t* R, summary_t* D);


int main(void)
{
	//to measure the time of execution of the program
	clock_t start, end;	
	double totalTimeOFExecution;
	start = clock();

	graph_t G;
    srand(time(NULL));
    int numberOfVertices = MAX_VERTICES;
    int maxNumberOfEdges = rand()%MAX_EDGES;
    printf("UPDATES:\n");
    printf("1. Graph with number of Vertices = %d\n     and maximum number of edges = %5d (Random number from maximum %d edges) is being initialized....\n",numberOfVertices, maxNumberOfEdges, MAX_EDGES);
    initialize(&G,numberOfVertices,maxNumberOfEdges);
    printf("2. Graph Initialization Finished....\n");
    graphLinker(&G,numberOfVertices,maxNumberOfEdges);
    printf("3. Successfully created edges....\n");

    //list to keep track of susceptible, infected and recovered people
    list_t* S = create_new_list();	//susceptible
    list_t* I = create_new_list();	//infected
    list_t* R = create_new_list();	//recovered

    //list summary to keep track of the number of S,I,R people in each day
    summary_t* D = create_new_summary();

    //priority list creation
    queue_t* Q = create_new_queue();

    //initializing  susceptible list with all the vertices
    int vertexCounter;
    for(vertexCounter = 0; vertexCounter < numberOfVertices; vertexCounter++)
    {
    	insert_ListElement_atTail(S,vertexCounter,0);
    }

    printf("4. Susceptible list initialized.....\n");

    //calling the fast SIR function to spread the disease with parameters
    float tau = 0.5;
    float gamma = 0.2;
    float tmax = 300;

    printf("5. fast_SIR() function call placed.....\n");
    printf("PLEASE WAIT (avg time 190 sec)(maximum time = 6 mins) ...........\n");

    fast_SIR(&G, Q, tau, gamma, tmax, numberOfVertices, maxNumberOfEdges, S, I, R, D);

    printf("6. Printing FINAL Summary......\n");

    printSummary(D);

    end = clock();	//end of the programm
    totalTimeOFExecution = ((double)(end - start))/CLOCKS_PER_SEC;
    printf("End of the programm. Total time of execution is %lfsec (Time in min :%lfmin)\n", totalTimeOFExecution, totalTimeOFExecution/60.0);
	return 0;
}

//*****************************************PRIORITY QUEUE*******************************************************
//this function creates a new event
//arguments to this function being the action to be performed and time of occurrence of the event 
//this function returns the address of the newly formed event
event_t* create_new_event(int vertex, int time, char action)
{
	event_t* new = (event_t*)malloc(sizeof(event_t));

	new->vertex = vertex;
	new->time = time;
	new->action = action;
	new->next = NULL;

	return new;
}

//this function creates a new priority queue
//returns a pointer to the newly formed queue
queue_t* create_new_queue()
{
	queue_t* Q = (queue_t*)malloc(sizeof(queue_t));
	Q->head = NULL;

	return Q;
}


//this function pops the highest priority event from the queue (i.e., the head of the queue)
//Priority is highest for the event having lowest time
void pop(queue_t* Q)
{
	event_t* temp1 = Q->head;
	if(temp1->next == NULL)
	{
		Q->head = NULL;
		free(temp1);
	}
	else
	{			
		event_t* temp2 = Q->head; //head is being stored in two variables, temp1 and temp2
		temp1 = temp1->next;
		free(temp2);	//free the previous head
		Q->head = temp1;
	}
}

//this function creates and inserts a new event into the priority queue based on the priority associated with the event
void push(queue_t* Q, int vertex, int time, char action)
{
	event_t* Qhead = Q->head;
	event_t* prev;
	event_t* newEvent;
	if(Qhead != NULL && Qhead->vertex == vertex)
	{
		Qhead->time = time;
		newEvent = Qhead;
		Q->head = Qhead->next;
	}
	else
	{
		while(Qhead != NULL && Qhead->vertex != vertex)
		{
			prev = Qhead;
			Qhead = Qhead->next;
		}
		if(Qhead != NULL)
		{
			Qhead->time = time;
			prev->next = Qhead->next;
			newEvent = Qhead;
		}
		else
			newEvent = create_new_event(vertex,time,action);
	}

	Qhead = Q->head;
	

	if(Q->head == NULL)
		Q->head = newEvent;

	//if the priority of the new event is greater than the priority of the current event then the head of the priority is updated as the new event
	//i.e, head.time > newEvent.time

	else if(Qhead->time > newEvent->time)
	{
		newEvent->next = Q->head;
		Q->head = newEvent;
	}
	else
	{
		//traversing the list to find the position of the new event based on its priority
		while(Qhead->next != NULL && Qhead->next->time < newEvent->time)
		{
			Qhead = Qhead->next;
		}

		//now Qhead is either NULL or pointing to the required event
		if(Qhead->next == NULL)
		{
			Qhead->next = newEvent;
			newEvent->next = NULL;
		}
		else
		{
			newEvent->next = Qhead->next;
			Qhead->next = newEvent;	
		}		
	}
}

//********************************************PRIORITY QUEUE ENDS**************************************************


//********************************************FOR MAINTAININ S,I,R LIST******************************************** 
//this function creates new listElement for maintaining a separate list for S,I,R
//returns a pointer to a new listElement
listElement_t* create_new_ListElement(int vertex, int time)
{
	listElement_t* new = (listElement_t*)malloc(sizeof(listElement_t));
	new->vertex = vertex;
	new->time = time;
	new->next = NULL;

	return new;
}

//this function creates a new list (susceptible - S, infected - I, recovered - R)
//returns a pointer to the created list
list_t* create_new_list()
{
	list_t* L = (list_t*)malloc(sizeof(list_t));
	L->head = NULL;

	return L;
}

//this function inserts a new listElement at the end of list
void insert_ListElement_atTail(list_t* L, int vertex, int time)
{
	listElement_t* temp = L->head;
	listElement_t* prev;
	if(temp == NULL)
	{
		L->head = create_new_ListElement(vertex,time);
		return;
	}
	else
	{
		while(temp!=NULL)
		{
			prev = temp;
			temp = temp->next;
		}
		listElement_t* new = create_new_ListElement(vertex,time);

		prev->next = new;
		new->next = NULL;
	}
}

//this function deletes a listElement from the given list provided its vertex number
void delete_listElement(list_t* L, int vertex)
{
	listElement_t* temp = L->head;
	listElement_t* previous;

	if(temp == NULL)
		return;
	else if(temp != NULL && temp->vertex == vertex)
	{
		if(temp->next == NULL)
		{
			L->head = NULL;
			free(temp);
			return;
		}
		else
		{
			L->head = temp->next;
			free(temp);
			return;
		}
	}
	else
	{
		while(temp != NULL && temp->vertex!=vertex)
		{
			previous = temp;
			temp = temp->next;
		}

		if(temp != NULL)
		{
			previous->next = temp->next;
			free(temp);
			return;
		}
		else
		{
			printf("Vertex (%d) does not exits in the given list, Delete-Unsuccessful\n",vertex);
			exit(1);
		}

	}
}

//this function returns the number of listElements in the given list
int number_of_listElements(list_t* L)
{
	listElement_t* temp = L->head;
	int noOfListElements = 0;
	while(temp != NULL)
	{
		noOfListElements++;
		temp = temp->next;
	}

	return noOfListElements;
}


//*******************************************END LIST****************************************************************


//*********for printing the final summary, (number of people susceptible,infected and recovered day wise)************
day_t* create_new_day(int dayCount, int S, int I, int R)
{
	day_t* new = malloc(sizeof(day_t));
	new->dayCount = dayCount;
	new->S = S;
	new->I = I;
	new->R = R;
	new->next = NULL;

	return new;
}

summary_t* create_new_summary()
{
	summary_t* D = malloc(sizeof(summary_t));
	D->head = NULL;

	return D;
}

//this function inserts a new day in the summary list
void insert_day_inSummary(summary_t* D, int dayCount, int S, int I, int R)
{
	day_t* temp = D->head;
	if(temp == NULL)
	{
		D->head = create_new_day(dayCount,S,I,R);
		return;
	}
	else
	{
		while(temp->next!=NULL)
		{
			temp = temp->next;
		}
		day_t* new = create_new_day(dayCount,S,I,R);

		temp->next = new;
		new->next = NULL;
	}
}

//prints the summary
void printSummary(summary_t* D)
{
	day_t* temp = D->head;
	if(temp == NULL)
		printf("Summary Empty\n");
	else
	{
		printf("\nDay\t\tSusceptible People\t\tInfected People\t\tRecovered People");
		while(temp != NULL)
		{
			if(temp->next != NULL)
			{
				if(temp->dayCount != temp->next->dayCount)
					printf("\n%3d%25d%30d%25d",temp->dayCount,temp->S,temp->I,temp->R);
			}
			else
			{
				printf("\n%3d%25d%30d%25d",temp->dayCount,temp->S,temp->I,temp->R);
				printf("\nDay\t\tSusceptible People\t\tInfected People\t\tRecovered People");
				printf("\n\n\nTherefor the number of people (as on %dth day):\nSusceptible: %4d\nInfected   : %4d\nRecovered  : %4d\n",temp->dayCount,temp->S,temp->I,temp->R);
			}
			temp = temp->next;
		}
	}	
}

//*********************************************CREATING AND INITIALIZING GRAPH**************************************
void initialize(graph_t* G, int numberOfVertices, int maxNumberOfEdges)
{
    
    if(numberOfVertices == 0)
        numberOfVertices++;

    if((G->mat = (int **)malloc(sizeof(int*)*numberOfVertices)) == NULL)
    {
        printf("Could not allocate memory for graph\n");
        exit(1);
    }

    int vertexCounter = 0;
    int edgeCounter = 0;


    for(vertexCounter = 0; vertexCounter < numberOfVertices; vertexCounter++)
    {
        if(((G->mat)[vertexCounter] = (int*)malloc(sizeof(int)*maxNumberOfEdges)) == NULL)
        {
            printf("Could not allocate memory for vertex\n");
            exit(1);
        }

        //initializing all the vertices with -1 meaning that there are no edges between any vertices of the max numnber of edges
        //          -1 = edge not connected to any other vertex
        //           if contains any number between 0 and numberOfVertices then it implies that there is an edge between these two vertices
        for(edgeCounter = 0; edgeCounter < maxNumberOfEdges; edgeCounter++)
        {
            //by default all the edges of all the vertex are initialized with -1
            (G->mat)[vertexCounter][edgeCounter] = -1;
        }
    }

    if((G->stat = (node_t*)malloc(sizeof(node_t)*numberOfVertices)) == NULL)
    {
        printf("Could not allocate memory to nodes\n");
        exit(1);
    }

    vertexCounter = 0; //initializing vertexCounter back to 0

    //if G->stat->status        == 'S' --------> susceptible
    //                          == 'I' --------> infected
    //                          == 'R' --------> recovered
    //if G->stat->pred_inf_time == 301 days      (meaning all the nodes are initially have a predected infected time as 301 days which is greater than tmax (=300 days))

    for(vertexCounter = 0; vertexCounter < numberOfVertices; vertexCounter++)
    {
        (G->stat)[vertexCounter].status = 'S';
        (G->stat)[vertexCounter].pred_inf_time = 301;
    }
}

//this function starts linking the graph. All vetrices need not have same number of links
void graphLinker(graph_t* G, int numberOfVertices, int maxNumberOfEdges)
{
    int vertexCounter = 0;
    int edgeCounter = 0;

    for (vertexCounter = 0; vertexCounter < numberOfVertices; vertexCounter++)
    {
        //printf("%d:\t",vertexCounter);
        for (edgeCounter=0; edgeCounter < maxNumberOfEdges; edgeCounter++)
        {
            if (rand()%2 == 1)  //link the vertices
            {
                int linkedVertex = rand() % numberOfVertices;
                while(linkedVertex == vertexCounter)
                {
                	linkedVertex = rand() % numberOfVertices;
                }

                (G->mat)[vertexCounter][edgeCounter] = linkedVertex;
            }
            //printf("%d, ", (G->mat)[vertexCounter][edgeCounter]);
        }
        //printf("\n\n");
    }
}

//********************************************TRANSMITTING AND RECOVERY TIME****************************************

//this function returns the number of days in which node v transmits the infection to its neighbors
//no arguments, tau being 0.5 
int transmitting_time(float tau)
{
	int dayCount = 1;

	while(1)
	{
		if(rand()%2)	//probability getting heads if a biased coin is tossed is tau (=0.5)
		{
			return dayCount;
		}
		dayCount++;
	}
}

//this function returns the number of days in which v will recover
int recovery_time(float gamma)
{
	int dayCount = 1;
	while(1)
	{
		if(rand()%10 < 2)	//tossing a biased coin with gamma(=0.2) probability
			return dayCount;
		dayCount++;
	}
}



//***********************************************TRANSMISSION AND RECOVERY FUNCTIONS*******************************

void find_trans_SIR(graph_t* G, queue_t* Q, int time, float tau, int sourceVertex, int targetVertex, int tmax)
{
	if((G->stat)[targetVertex].status == 'S')
	{
		(G->stat)[targetVertex].inf_time = time + transmitting_time(tau);
		if((G->stat)[targetVertex].inf_time < tmax)
		{
			push(Q,targetVertex,(G->stat)[targetVertex].inf_time,'T');
			(G->stat)[targetVertex].pred_inf_time = (G->stat)[targetVertex].inf_time;
		}
	}
}

void process_trans_SIR(graph_t* G, int vertex, int time, float tau, float gamma, list_t* S, list_t* I, list_t* R, queue_t* Q, int tmax, int maxNumberOfEdges, summary_t* D)
{
	(G->stat)[vertex].status = 'I';
	(G->stat)[vertex].rec_time = time + recovery_time(gamma);
	delete_listElement(S,vertex);
	insert_ListElement_atTail(I,vertex,time);
	pop(Q);
	insert_day_inSummary(D,time, number_of_listElements(S), number_of_listElements(I), number_of_listElements(R));

	if((G->stat)[vertex].rec_time < tmax)
	{
		push(Q,vertex,(G->stat)[vertex].rec_time,'R');
	}

	for(int edgeCounter = 0; edgeCounter<maxNumberOfEdges;edgeCounter++)
	{
		if((G->mat)[vertex][edgeCounter] != -1 )
		{
			find_trans_SIR(G,Q,time,tau,vertex,(G->mat)[vertex][edgeCounter],tmax);
		}
	}
}

void process_rec_SIR(graph_t* G,int vertex, int time, list_t* S, list_t* I, list_t* R, summary_t* D)
{
	
	delete_listElement(I,vertex);
	insert_ListElement_atTail(R,vertex,time);
	insert_day_inSummary(D,time, number_of_listElements(S), number_of_listElements(I), number_of_listElements(R));

	(G->stat)[vertex].status = 'R';
	
}

void fast_SIR(graph_t* G, queue_t* Q, float tau, float gamma, int tmax, int numberOfVertices, int maxNumberOfEdges, list_t* S, list_t* I, list_t* R, summary_t* D)
{
	//generating a random vertex number to start the infection
	int vertex = rand()%numberOfVertices;

	(G->stat)[vertex].pred_inf_time = 0;
	(G->stat)[vertex].inf_time = 0;
	push(Q,vertex,(G->stat)[vertex].pred_inf_time,'T');

	event_t* event = Q->head;
	while(event != NULL && event->time < 300)
	{
		if(event->action == 'T')
		{
			if((G->stat)[event->vertex].status == 'S')
			{
				process_trans_SIR(G, event->vertex, event->time, tau, gamma, S, I, R, Q, tmax, maxNumberOfEdges, D);
			}
		}
		else
		{
			process_rec_SIR(G, event->vertex, event->time, S, I, R, D);
			pop(Q);
		}
		event = Q->head;
	}
}
