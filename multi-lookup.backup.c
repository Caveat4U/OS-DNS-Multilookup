/*
 * File: multi-lookup.c
 * Author: Chris Sterling
 * Original Creators: Andy Sayler
 * Project: OS Programming Assignment 2
 * Description:
 *     This file contains the threaded solution to this assignment.
 *  
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "util.h"
#include "queue.h"

#define MIN_ARGS 3
#define USAGE "./multi-lookup <inputFilePath> <inputFilePath> ... <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MIN_RESOLVER_THREADS 2
#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
#define QUEUE_SIZE 1024

FILE* outputfp;
queue q;

pthread_mutexattr_t attr;
pthread_mutex_t mutex_for_queue;

pthread_mutexattr_t attr2;
pthread_mutex_t mutex_for_output;


/* Function for Each Thread to Run */
void* open_read_file(char* file_name)
{
    /* Setup Local Vars */
    FILE* inputfp = NULL;
    char errorstr[SBUFSIZE];
	char* hostname;
	char* payload_in;
	printf("I'm working on file %s\n", file_name); 
	/* Open Input File */
	inputfp = fopen(file_name, "r");
	if(!inputfp){
		sprintf(errorstr, "Error Opening Input File: %s", file_name);
		perror(errorstr);
	}
	hostname = (char*)malloc((MAX_NAME_LENGTH+1)*sizeof(char));
	/* Read Each File and Process*/
	while(fscanf(inputfp, INPUTFS, hostname) > 0){
		
		/* Write each domain to the queue */
		pthread_mutex_lock(&mutex_for_queue); //lock the queue
		
		if(queue_push(&q, hostname) == QUEUE_FAILURE)
		{
			fprintf(stderr, "ERROR: queue_push failed with value: %s\n", hostname);
		}
		pthread_mutex_unlock(&mutex_for_queue); //unlock the queue
		hostname = (char*)malloc((MAX_NAME_LENGTH+1)*sizeof(char));
		//free(payload_in); //WHERE/WHEN DO I DO THIS??
	 }
	 //after we're done reading the file, close it
	 fclose(inputfp);
	 
	 /* Sleep for 1 to 2 Seconds */
	 usleep((rand()%100)*10000+1000000);
    free(hostname);
    /* Exit, Returning NULL*/
    pthread_exit(NULL);
    return NULL;
}

void* resolve_domain(void* params)
{
	usleep(1000000); //test for now
	
	char *hostname;
    char firstipstr[INET6_ADDRSTRLEN];
 
	//pthread_mutex_lock(&mutex_for_queue);
	/*while(continue&&!all_files_read)
	{
		//pop it off the queue
		//LOCK
		continue = queue_is_empty(&q)
		//UNLOCK
		if(*hostname = (char*)queue_pop(&q) == NULL) //pop off the domain into the queue
		{
			fprintf(stderr, "ERROR: queue_pop failed with value: %s\n", hostname);
			pthread_mutex_unlock(&mutex_for_queue); //unlock the queue for others to use
			pthread_exit(NULL); //Exit, Returning NULL - if it couldn't pop, then the end of the queue must be reached
			return NULL;
		}
		pthread_mutex_unlock(&mutex_for_queue);
		
		// Lookup hostname and get IP string 
		if(dnslookup(hostname, firstipstr, sizeof(firstipstr)) == UTIL_FAILURE)
		{
			fprintf(stderr, "dnslookup error: %s\n", hostname);
			strncpy(firstipstr, "", sizeof(firstipstr)); //copy a NULL value to the IP string
		}
		//WRITE TO THE OUTPUT FILE
		pthread_mutex_lock(&mutex_for_output); //lock the output file
		fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
		pthread_mutex_unlock(&mutex_for_output); //unlock the output file
		free(hostname);
		hostname = NULL;
	}*/
	
	//POP OFF THE QUEUE
	pthread_mutex_lock(&mutex_for_queue); //lock the queue
	if(queue_is_empty(&q))
	{
		fprintf(stderr, "Queue is empty\n");
		pthread_mutex_unlock(&mutex_for_queue); //unlock the queue for others to use
		pthread_exit(NULL); //Exit, Returning NULL
		return NULL;
	}
	/*
	else
	{
		fprintf(stderr, "Queue is NOT empty - THIS IS A PROBLEM\n");
		pthread_mutex_unlock(&mutex_for_queue); //unlock the queue for others to use
		pthread_exit(EXIT_FAILURE); //Exit, RETURN AN ERROR CODE
		return EXIT_FAILURE;
	}
	*/
	if((hostname = (char*)queue_pop(&q)) == NULL) //pop off the domain into the queue
	{
	    fprintf(stderr, "ERROR: queue_pop failed with value: %s\n", hostname);
		pthread_mutex_unlock(&mutex_for_queue); //unlock the queue for others to use
		pthread_exit(NULL); //Exit, Returning NULL - if it couldn't pop, then the end of the queue must be reached
		return NULL;
	}
	pthread_mutex_unlock(&mutex_for_queue); //unlock the queue
	//fprintf(stderr, "The hostname is*: %s\n", *hostname);
	fprintf(stderr, "The hostname is: %s\n", hostname);
	
	/* Lookup hostname and get IP string */
	if(dnslookup(hostname, firstipstr, sizeof(firstipstr)) == UTIL_FAILURE)
	{
		fprintf(stderr, "dnslookup error: %s\n", hostname);
		strncpy(firstipstr, "", sizeof(firstipstr)); //copy a NULL value to the IP string
	}
	//WRITE TO THE OUTPUT FILE
	pthread_mutex_lock(&mutex_for_output); //lock the output file
	fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
	pthread_mutex_unlock(&mutex_for_output); //unlock the output file
	
	free(hostname); //call each time you call pop
	
	/* Exit, Returning NULL*/
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char* argv[])
{
    /* Local Vars */
    outputfp = NULL;
    int num_input_files = argc-2; //subtract 1 for the original call and another for resolve.txt
	const int qSize = QUEUE_SIZE;
    int rc;
    long t;
    
    pthread_t file_threads[num_input_files];
	long file_thread_tracker[num_input_files];
	
    pthread_t resolver_threads[MAX_RESOLVER_THREADS];
    long resolver_thread_tracker[MAX_RESOLVER_THREADS];
	
	pthread_mutexattr_settype(&attr, NULL);
	pthread_mutex_init(&mutex_for_queue, &attr);

	pthread_mutexattr_settype(&attr2, NULL);
	pthread_mutex_init(&mutex_for_output, &attr2);

	
	/* Check Arguments */
	if(argc < MIN_ARGS){
		fprintf(stderr, "Too few arguments: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
		return EXIT_FAILURE;
	}
	
    if(argc > MAX_INPUT_FILES+1){
		fprintf(stderr, "Too many arguments: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
		return EXIT_FAILURE;
    }

    /* Open Output File */
    outputfp = fopen(argv[(argc-1)], "w");
    if(!outputfp){
		perror("Error Opening Output File\n");
		return EXIT_FAILURE;
    }

    /* Initialize Queue */
    if(queue_init(&q, qSize) == QUEUE_FAILURE){
		fprintf(stderr, "error: queue_init failed!\n");
    }
    
    
    /* Loop Through Input Files */
    for(t=0; t<num_input_files; t++){ 
		file_thread_tracker[t] = t;
		
		//create a thread 
		printf("In main: creating thread %ld - with file %s\n", t, argv[t+1]);
		rc = pthread_create(&(file_threads[t]), NULL, open_read_file, argv[t+1]); //pass it the input filename and the output file
		if (rc){ //couldn't create thread successfully
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(EXIT_FAILURE);
		}
	}
	
	/* Begin resolver threads */
	for(t=0; t<MAX_RESOLVER_THREADS; t++){ 
		resolver_thread_tracker[t] = t;
		
		//create a thread 
		printf("In main: creating resolver thread %ld\n", t);
		rc = pthread_create(&(resolver_threads[t]), NULL, resolve_domain, NULL); //pass it the input filename and the output file
		if (rc){ //couldn't create thread successfully
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(EXIT_FAILURE);
		}
	}
	

    /* Wait for All Theads to Finish */
    for(t=0;t<num_input_files;t++)
    {
		pthread_join(file_threads[t], NULL);
    }
    for(t=0;t<num_input_files;t++)
    {
		pthread_join(resolver_threads[t], NULL);
    }
    
    printf("All of the threads were completed!\n");
	fclose(outputfp); //close the output file when all the threads are done with it
	
    queue_cleanup(&q); //cleanup queue
	
	//clean up locks
	pthread_mutex_destroy(&mutex_for_queue);
	pthread_mutex_destroy(&mutex_for_output);
	
    return EXIT_SUCCESS;
}
