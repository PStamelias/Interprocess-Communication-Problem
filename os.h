#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/sem.h>
struct entry {
	unsigned int read;
	unsigned int write;
};
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
};
struct sembuf p = { 0, -1, SEM_UNDO};
struct sembuf v = { 0, +1, SEM_UNDO};