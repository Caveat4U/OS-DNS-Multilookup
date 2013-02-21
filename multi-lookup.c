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

#define MIN_ARGS 3
#define USAGE "<inputFilePath> <inputFilePath> ... <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"


#define NUM_THREADS	5
#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MIN_RESOLVER_THREADS 2
#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
struct file_thread_params {
    long* tid;
    char* file_name;
    FILE* outputfp;
};

/* Function for Each Thread to Run */
void* open_file(void* params)
{
    /* Setup Local Vars and Handle void* */
    //void* threadid, char* file_name, FILE* outputfp;
    
    
    struct file_thread_params *parameters = (struct file_thread_params *) params;
    long* tid = parameters->tid;
    char* file_name = parameters->file_name;
    FILE* outputfp = parameters->outputfp;
    free(parameters); //free up the local copy
    
    
    //long* tid = threadid;
    FILE* inputfp = NULL;
	//char hostname[SBUFSIZE];
    char errorstr[SBUFSIZE];
    //char firstipstr[INET6_ADDRSTRLEN];
	//char file_name[] = "input/names1.txt";
	printf("It's me, thread #%ld! ", *tid);
	printf("I'm working on file %s\n",file_name);
	 
	/* Open Input File */
	inputfp = fopen(file_name, "r");
	if(!inputfp){
		sprintf(errorstr, "Error Opening Input File: %s", file_name);
		perror(errorstr);
	}
	 
	/* Read File and Process*/
	//while(fscanf(inputfp, INPUTFS, hostname) > 0){

		///* Lookup hostname and get IP string */
		//if(dnslookup(hostname, firstipstr, sizeof(firstipstr)) == UTIL_FAILURE)
		//{
			//fprintf(stderr, "dnslookup error: %s\n", hostname);
			//strncpy(firstipstr, "", sizeof(firstipstr));
		//}

		///* Write each resolved IP to the output file */
		////fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
	 //}
	 ////after we're done reading the file, close it
	 fclose(inputfp);
	 
	 /* Sleep for 1 to 2 Seconds */
	 usleep((rand()%100)*10000+1000000);
    
    /* Exit, Returning NULL*/
    return NULL;
}

int main(int argc, char* argv[])
{
	
    /* Local Vars */
    FILE* outputfp = NULL;
    int num_input_files = argc-2; //subtract 1 for the original call and another for resolve.txt
	struct file_thread_params *parameters = malloc(sizeof(struct file_thread_params));
    int i;
    int rc;
    long t;
    
    //pthread_t resolver_threads[MAX_RESOLVER_THREADS];
    //long resolver_threads_tracker[MAX_RESOLVER_THREADS];
	
	pthread_t file_threads[num_input_files];
	long file_thread_tracker[num_input_files];

	
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
		perror("Error Opening Output File");
		return EXIT_FAILURE;
    }

    /* Loop Through Input Files */
    for(t=0; t<num_input_files; t++){ 
	
		printf("In main: creating thread %ld - with file %s\n", t, argv[t+1]);
		
		file_thread_tracker[t] = t;
		parameters->tid=file_thread_tracker[t];
		parameters->file_name=argv[t+1]; //files start at 1
		parameters->outputfp=outputfp;
		rc = pthread_create(&(file_threads[t]), NULL, open_file, &parameters); //pass it the input filename and the output file
		if (rc){ //couldn't create thread successfully
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(EXIT_FAILURE);
		}
	}



    
    /* Wait for All Theads to Finish */
    for(t=0;t<num_input_files;t++)
    {
		//pthread_join(file_thread_tracker[t],NULL);
    }
    
    printf("All of the threads were completed!\n");
	//fclose(outputfp); //close the output file when all the threads are done with it

    return EXIT_SUCCESS;
}
