#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include "os.h"
int main(int argc,char** argv){
	srand(time(NULL));
	int entry_num=atoi(argv[1]);/*entry number of table_entry*/
	int process_num=atoi(argv[2]);/*number of processes*/
	int read_pos=atoi(argv[3]);/*Propability of read_function*/
	int num_turns=atoi(argv[4]);/*how many turn of read-write*/
	int sum_read=0;
	int shm_id; // ID of shared memory
	int glob_id;
	int size_of_glob_var=entry_num*sizeof(int);
	if((glob_id = shmget ((key_t) getpid(), size_of_glob_var, 0666 | IPC_CREAT)) == -1 ) {
    perror("Error in shared memory region setup.\n");
    exit(2);
 	} 
 	int* glob_var=(int*)shmat(glob_id,(int*)0,0);/*table which count for each entry how many readers have at this moment*/
 	for(int i=0;i<entry_num;i++)
 		glob_var[i]=0;
	int status=0;
	int sum_write=0;
	int child_read=0;
	int child_write=0;
	double  mo=0.0;
	double sum_of_delay=0.0;
	int parent_id=getpid();
	struct entry* entry_table;
	srand(time(NULL));
	int SHMSIZE=entry_num*sizeof(struct entry);
	if((shm_id = shmget ((key_t) (getpid()+1), SHMSIZE, 0666 | IPC_CREAT)) == -1 ) {
    printf ("Error in shared memory region setup.\n");
    exit(2);
 	} 
 	/*Initialization of Entry Table*/
 	entry_table=(struct entry*)shmat(shm_id,(struct entry*)0,0);
 	for(int i=0;i<entry_num;i++){
 		struct entry node={0,0};
 		memcpy(entry_table+i*sizeof(struct entry),&node,sizeof(struct entry));
 	}
 	/**********************************************************************/
 	union semun* sem1=malloc(entry_num*sizeof(union semun));
 	union semun* sem2=malloc(entry_num*sizeof(union semun));
 	int* sem1_num=malloc(entry_num*sizeof(int));
 	int* sem2_num=malloc(entry_num*sizeof(int));
 	for(int i=0;i<entry_num;i++){
 		key_t key;
		char *path = "/tmp";
		int id = 'A'+i;
		key = ftok(path, id);
		sem2_num[i]=semget(key, 1, 0666 | IPC_CREAT);
    if(sem2_num[i]<0){
      perror("semget"); 
      exit(11);
    }
    sem2[i].val=1;
    if(semctl(sem2_num[i],0,SETVAL,sem2[i])<0){
     	perror("semctl"); 
     	exit(12);
    }
 	}
 	for(int i=0;i<entry_num;i++){
 		key_t key;
		char *path = "/tmp";
		int id = 'S'+i;
		key = ftok(path, id);
		sem1_num[i]=semget(key, 1, 0666 | IPC_CREAT);
    if(sem1_num[i]<0){
      perror("semget"); 
      exit(11);
    }
    sem1[i].val=1;
    if(semctl(sem1_num[i],0,SETVAL,sem1[i])<0){
     	perror("semctl"); 
     	exit(12);
    }
 	}
 	pid_t* pid=malloc(process_num*sizeof(pid_t));/*Table which holds the pid of forking processes*/
 	for(int i=0;i<entry_num;i++){
 		if(semop(sem1_num[i], &v, 1) < 0){
      perror("semop v"); 
      exit(14);
    }
    if(semop(sem2_num[i],&v, 1) < 0){
      perror("semop v"); 
      exit(14);
    }
 	}
 	for(int i=0;i<process_num;i++)
 		if(getpid()==parent_id)
 			pid[i]=fork();
 	double time_taken=0.0;
 	if(getpid()!=parent_id){
 		for(int i=0;i<num_turns;i++){
 			int pos_in_entry=rand()%entry_num+0;
 			int num_propability=rand()%100+0;
 			clock_t t1 = clock();/*starting counting time*/
 			srand(time(NULL));
 			if(num_propability<=read_pos)/*reader*/{
 				if(semop(sem1_num[pos_in_entry], &p, 1) < 0){
          perror("semop p"); 
          exit(14);
        } 
        glob_var[i]++;
        if(glob_var[i]==1)
        	if(semop(sem2_num[pos_in_entry], &p, 1) < 0){
	          perror("semop p"); 
	          exit(14);
        	}
        if(semop(sem1_num[pos_in_entry], &v, 1) < 0){
          perror("semop v"); 
          exit(14);
        }
        struct entry node;
        memcpy(&node,entry_table+pos_in_entry*sizeof(struct entry),sizeof(struct entry));
        node.read++;
        memcpy(entry_table+pos_in_entry*sizeof(struct entry),&node,sizeof(struct entry));
        child_read++;
        double n = (double)rand()/(double)RAND_MAX;
		   	double ar = (-1)*(log(n));
		   	double T = ar/3;
		   	T = T*8;
		   	sleep(T);
        if(semop(sem1_num[pos_in_entry], &p, 1) < 0){
          perror("semop p"); 
          exit(14);
        }
        glob_var[i]--; 
        if(glob_var[i]==0)
        	if(semop(sem2_num[pos_in_entry],&v,1)<0){
	          perror("semop v"); 
	          exit(14);
        	}
        if(semop(sem1_num[pos_in_entry], &v, 1) < 0){
          perror("semop v"); 
          exit(14);
        }
 			}
 			else/*writer*/{
        if(semop(sem2_num[pos_in_entry], &p, 1) < 0){
          perror("semop p"); 
          exit(14);
        }
        struct entry node;
        memcpy(&node,entry_table+pos_in_entry*sizeof(struct entry),sizeof(struct entry));
        node.write++;
        memcpy(entry_table+pos_in_entry*sizeof(struct entry),&node,sizeof(struct entry));
        child_write++;
      	double n = (double)rand()/(double)RAND_MAX;
		   	double ar = (-1)*(log(n));
		   	double T = ar/3;
		   	T = T*8;
		   	sleep(T);
        if(semop(sem2_num[pos_in_entry],&v,1)<0){
          perror("semop v"); 
          exit(14);
        }
 			}
 			clock_t t2 = clock();/* stop counting time*/
			time_taken = time_taken + (double)(t2 - t1);
 		}
 	}
 	if(getpid()==parent_id){
 		for(int i=0;i<process_num;i++)
 			waitpid(pid[i],&status,0);
 		for(int i=0;i<entry_num;i++){
 			struct entry node;
 			memcpy(&node,entry_table+i*sizeof(struct entry),sizeof(struct entry));
 			sum_read+=node.read;
 			sum_write+=node.write;
 		}
 		printf("Parent Process with ID=%d have sum of reads=%d and sum of writes=%d\n",getpid(),sum_read,sum_write);
 		if(shmdt(entry_table)==-1){
 			fprintf(stderr, "Failed to detach shared memory\n");
    	exit(EXIT_FAILURE);
 		}
 		if(shmdt(glob_var)==-1){
 			fprintf(stderr, "Failed to detach shared memory\n");
    	exit(EXIT_FAILURE);
 		}
 		if(shmctl(shm_id, IPC_RMID,0)==-1)
			fprintf(stderr, "Failed to delete shared memory integer\n");
		if(shmctl(glob_id,IPC_RMID,0)==-1)
			fprintf(stderr, "Failed to delete shared memory integer\n");
		for(int i=0;i<entry_num;i++)
			if(semctl(sem1_num[i], 0, IPC_RMID,0) == -1) 
    		fprintf(stderr, "Failed to delete semaphore for readers\n");
    for(int i=0;i<entry_num;i++)
    	if(semctl(sem2_num[i], 0, IPC_RMID,0) == -1) 
    		perror("Writer Semaphore Deletion");
 	}
 	else{
 		time_taken = time_taken/CLOCKS_PER_SEC;
	  double av_time = time_taken/entry_num;
 		printf("Child Process with Id=%d have average time standby=%f counter_reads=%d and counter_writes=%d\n",getpid(),time_taken,child_read,child_write);
 	}
 	free(pid);
 	free(sem1);
 	free(sem1_num);
 	free(sem2_num);
 	free(sem2);
	return 0;
}