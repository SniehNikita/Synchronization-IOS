/** ----------------------------------------
 *  @author     Sniehovskyi Nikita, xsnieh00
 *			    FIT VUTBR.CZ
 *  @date       23.04.2022
 *  project	    IOS-Project-2
 *  @file	    proj2.c
 *  compiler	gcc
 */ 

#define _GNU_SOURCE


#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "error.h"



int *moleculeCount;

int *hydrogen_queue;    // count of waiting for creation
int *hydrogen_expected;  // count of H to be created
int *atom_op_number;   // number of operation
int *molecule_number;   // count of H
int *is_no_more_molecules_available;   // number of H

sem_t *sem_hydrogen_queue = NULL;
sem_t *sem_hydrogen_mol_success = NULL;
sem_t *sem_hydrogen_mol_2H = NULL;
sem_t *sem_hydrogen_mol_3H = NULL;


int *oxygen_queue;    // count of waiting for creation
int *oxygen_expected;  // count of O to be created

sem_t *sem_oxygen_queue = NULL;
sem_t *sem_oxygen_mol_2O = NULL;
sem_t *sem_oxygen_mol_3O = NULL;
sem_t *sem_oxygen_mol_success = NULL;

sem_t *sem_mol_open = NULL;
sem_t *sem_atom_op_number_inc = NULL;
sem_t *sem_molecule_number_inc = NULL;


/**
 * @fn ReadParams
 * @brief Reads input params. Expects: ./proj2 NO NH TI TB
 */
void ReadParams(int argc, char *argv[], int *NO, int *NH, int *TI, int *TB);



void ReadParams(int argc, char *argv[], int *NO, int *NH, int *TI, int *TB) {
    if (argc < 5) {
        fatal_error_msg("Program requires 4 params.");
        exit(1);
    }
    *NO = atoi(argv[1]); // number of oxygen
    *NH = atoi(argv[2]); // number of hydrogen
    *TI = atoi(argv[3]); // time to create an atom
    *TB = atoi(argv[4]); // time to create a molecule

    if (*TI < 0 || *TI > 1000 || *TB < 0 || *TB > 1000) {
        fatal_error_msg("0<TI<1000. 0<TB<1000.");
    }
}



void Free_all_mmap_and_semaphores() {
    if (munmap(hydrogen_queue, sizeof(int)) != 0) {error_msg("munmap error.");}
    if (munmap(moleculeCount, sizeof(int)) != 0) {error_msg("munmap error.");}
    if (munmap(is_no_more_molecules_available, sizeof(int)) != 0) {error_msg("munmap error.");}

    sem_close(sem_hydrogen_queue);
    sem_close(sem_mol_open);
    sem_close(sem_hydrogen_mol_2H);
    sem_close(sem_hydrogen_mol_3H);
    sem_close(sem_hydrogen_mol_success);
    sem_close(sem_oxygen_queue);
    sem_close(sem_oxygen_mol_2O);
    sem_close(sem_oxygen_mol_3O);
    sem_close(sem_oxygen_mol_success);
    sem_close(sem_atom_op_number_inc);
    sem_close(sem_molecule_number_inc);
}



void *HydrogenProc(int id, int TI) {
//  Start

    // Lock atom_op_number var to use it
    if (sem_wait(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
        printf("%d: H %d: started\n", ++(*atom_op_number), id);
    if (sem_post(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}

//------------------------------------------

    usleep(abs(rand() % TI));

//  Queue  

    // start of creation process
    if (sem_wait(sem_mol_open) == -1)  {printf(" Error of the semaphore\n");}

    if (sem_wait(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
        printf("%d: H %d: going to queue\n", ++(*atom_op_number), id);
    if (sem_post(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}

    // these are being protected my sem_mol_open
    (*hydrogen_queue)++; 
    (*hydrogen_expected)--;
//------------------------------------------

 
//  Molecule 
    // Checks if there enough atoms for at least 1 molecule
    if (*hydrogen_expected+*hydrogen_queue < 2 || *oxygen_expected+*oxygen_queue < 1) {
        *is_no_more_molecules_available = 1;
        if (sem_post(sem_hydrogen_mol_2H) == -1)  {printf(" Error of the semaphore\n");}
        if (sem_post(sem_oxygen_mol_2O) == -1)  {printf(" Error of the semaphore\n");}
    }

    // if enough atoms to create molecule -> create it else open sem for the next molecule to enter
    if (*hydrogen_queue >= 2 && *oxygen_queue >= 1) {
        if (sem_wait(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}
        (*moleculeCount)++;
        if (sem_post(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}

        if (sem_post(sem_hydrogen_mol_2H) == -1)  {printf(" Error of the semaphore\n");} // open gate for 1st hydrogen
        if (sem_post(sem_oxygen_mol_2O) == -1)  {printf(" Error of the semaphore\n");}   // open gate for oxygen
        if (sem_wait(sem_hydrogen_mol_3H) == -1)  {printf(" Error of the semaphore\n");} // wait till first will pass throw
        if (sem_post(sem_hydrogen_mol_2H) == -1)  {printf(" Error of the semaphore\n");} // open gate for itself  
    } else {
        if (sem_post(sem_mol_open) == -1)  {printf(" Error of the semaphore\n");}
    }

    // wait for gate opened by second hydrogen
    if (sem_wait(sem_hydrogen_mol_2H) == -1)  {printf(" Error of the semaphore\n");}

    // check if it's not the last
    if (*is_no_more_molecules_available == 1) {
        if (sem_wait(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
        printf("%d: H %d: not enough O or H\n", ++(*atom_op_number), id);
        if (sem_post(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}

        if (sem_post(sem_hydrogen_mol_2H) == -1)  {printf(" Error of the semaphore\n");}
        if (sem_post(sem_oxygen_mol_2O) == -1)  {printf(" Error of the semaphore\n");}
        
        Free_all_mmap_and_semaphores();
        exit(0);
    }

    if (sem_post(sem_hydrogen_mol_3H) == -1)  {printf(" Error of the semaphore\n");}

    // log
    if (sem_wait(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_wait(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    printf("%d: H %d: creating molecule %d\n", ++(*atom_op_number), id, *moleculeCount);
    if (sem_post(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_post(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}


    // decrease queue
    if (sem_wait(sem_hydrogen_queue) == -1)  {printf(" Error of the semaphore\n");}
    (*hydrogen_queue)--;
    if (sem_post(sem_hydrogen_queue) == -1)  {printf(" Error of the semaphore\n");}

    // waits for oxygen
    if (sem_wait(sem_oxygen_mol_success) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_post(sem_oxygen_mol_success) == -1)  {printf(" Error of the semaphore\n");}

    // log
    if (sem_wait(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_wait(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    printf("%d: H %d: molecule %d created\n", ++(*atom_op_number), id, *moleculeCount);
    if (sem_post(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_post(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    
    if (sem_post(sem_hydrogen_mol_success) == -1)  {printf(" Error of the semaphore\n");} // ROE

//  ------------------------------------------

    Free_all_mmap_and_semaphores();
    exit(0);
}




void *OxygenProc(int id, int TI, int TB) {
//  Start
    if (sem_wait(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    printf("%d: O %d: started\n", ++(*atom_op_number), id);
    if (sem_post(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
//------------------------------------------

    usleep(abs(rand() % TI));

//  Queue
    if (sem_wait(sem_mol_open) == -1)  {printf(" Error of the semaphore\n");}

    if (sem_wait(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    printf("%d: O %d: going to queue\n", ++(*atom_op_number), id);
    if (sem_post(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}

    (*oxygen_queue)++;
    (*oxygen_expected)--;
//------------------------------------------


//  Molecule 
    if (*hydrogen_expected+*hydrogen_queue < 2 || *oxygen_expected+*oxygen_queue < 1) {
        *is_no_more_molecules_available = 1;
        if (sem_post(sem_hydrogen_mol_2H) == -1)  {printf(" Error of the semaphore\n");}
        if (sem_post(sem_oxygen_mol_2O) == -1)  {printf(" Error of the semaphore\n");}
    }

    if (*hydrogen_queue >= 2 && *oxygen_queue >= 1) {
        if (sem_wait(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}
        (*moleculeCount)++;
        if (sem_post(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}

        if (sem_post(sem_hydrogen_mol_2H) == -1)  {printf(" Error of the semaphore\n");} // open gate for 1st hydrogen
        if (sem_post(sem_oxygen_mol_2O) == -1)  {printf(" Error of the semaphore\n");}   // open gate for oxygen
        if (sem_wait(sem_hydrogen_mol_3H) == -1)  {printf(" Error of the semaphore\n");} // wait till first will pass throw
        if (sem_post(sem_hydrogen_mol_2H) == -1)  {printf(" Error of the semaphore\n");} // open gate for itself
    } else {
        if (sem_post(sem_mol_open) == -1)  {printf(" Error of the semaphore\n");}
    }


    // Gate. Only one oxygen can go through
    if (sem_wait(sem_oxygen_mol_2O) == -1)  {printf(" Error of the semaphore\n");}

    if (*is_no_more_molecules_available == 1) {
        if (sem_wait(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
        printf("%d: O %d: not enough H\n", ++(*atom_op_number), id);
        if (sem_post(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}

        if (sem_post(sem_hydrogen_mol_2H) == -1)  {printf(" Error of the semaphore\n");}
        if (sem_post(sem_oxygen_mol_2O) == -1)  {printf(" Error of the semaphore\n");}
        Free_all_mmap_and_semaphores();
        exit(0);
    }

    // log
    if (sem_wait(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_wait(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    printf("%d: O %d: creating molecule %d\n", ++(*atom_op_number), id, *moleculeCount);
    if (sem_post(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_post(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}

    if (sem_wait(sem_oxygen_queue) == -1)  {printf(" Error of the semaphore\n");}
    (*oxygen_queue)--;
    if (sem_post(sem_oxygen_queue) == -1)  {printf(" Error of the semaphore\n");}
    
    usleep(abs(rand() % TB));

    // log
    if (sem_wait(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_wait(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    printf("%d: O %d: molecule %d created\n", ++(*atom_op_number), id, *moleculeCount);
    if (sem_post(sem_molecule_number_inc) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_post(sem_atom_op_number_inc) == -1)  {printf(" Error of the semaphore\n");}

    // signal to hydrogen
    if (sem_post(sem_oxygen_mol_success) == -1)  {printf(" Error of the semaphore\n");}

    if (sem_wait(sem_hydrogen_mol_success) == -1)  {printf(" Error of the semaphore\n");} // waiting for first to pass
    if (sem_wait(sem_hydrogen_mol_success) == -1)  {printf(" Error of the semaphore\n");} // waiting for second to pass

    // checks if there are not enough atom 
    if (sem_wait(sem_hydrogen_queue) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_wait(sem_oxygen_queue) == -1)  {printf(" Error of the semaphore\n");}
    if (*hydrogen_expected+*hydrogen_queue < 2 || *oxygen_expected+*oxygen_queue < 1) {
        *is_no_more_molecules_available = 1;
        if (sem_post(sem_hydrogen_mol_2H) == -1)  {printf(" Error of the semaphore\n");}
        if (sem_post(sem_oxygen_mol_2O) == -1)  {printf(" Error of the semaphore\n");}
    }
    if (sem_post(sem_hydrogen_queue) == -1)  {printf(" Error of the semaphore\n");}
    if (sem_post(sem_oxygen_queue) == -1)  {printf(" Error of the semaphore\n");}

    if (sem_wait(sem_hydrogen_mol_3H) == -1)  {printf(" Error of the semaphore\n");} // close gate
    if (sem_wait(sem_oxygen_mol_success) == -1)  {printf(" Error of the semaphore\n");} // close gate

    if (sem_post(sem_mol_open) == -1)  {printf(" Error of the semaphore\n");}

//  ------------------------------------------

    Free_all_mmap_and_semaphores();
    exit(0);
}



int main (int argc, char **argv) {
    // Oxygen count, Hydrogen count, Time to create Ox/Hyd, Time to create a molecule
    int NO = 0, NH = 0, TI = 0, TB = 0;
    ReadParams(argc,argv,&NO,&NH,&TI,&TB);

    // inicializes random generator
    srand(time(NULL));

    // Shared memory allocation
    // Queue of hydrogen (number)
    hydrogen_queue = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // How many hydrogen is expected to be created
    hydrogen_expected = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    // Number of operation
    atom_op_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //Number of molecule
    molecule_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    is_no_more_molecules_available = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Queue of oxygen (number)
    oxygen_queue = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // How many oxygen is expected to be created
    oxygen_expected = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Molecule Number
    moleculeCount = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    errno = 0;

    // Create semaphores
    // Creates a semaphore for every variable so they won't be changed by two processes in the same time 
    if ((sem_hydrogen_queue = sem_open("/hydrogen", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/hydrogen");
    if ((sem_hydrogen_mol_2H = sem_open("/sem_hydrogen_mol_2H", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/sem_hydrogen_mol_2H");
    if ((sem_hydrogen_mol_3H = sem_open("/sem_hydrogen_mol_3H", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/sem_hydrogen_mol_3H");
    if ((sem_hydrogen_mol_success = sem_open("/sem_hydrogen_mol_success", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/sem_hydrogen_mol_success");

    if ((sem_oxygen_queue = sem_open("/oxygen", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/oxygen");
    if ((sem_oxygen_mol_2O = sem_open("/sem_oxygen_mol_2O", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/sem_oxygen_mol_2O");
    if ((sem_oxygen_mol_3O = sem_open("/sem_oxygen_mol_3O", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/sem_oxygen_mol_3O");
    if ((sem_oxygen_mol_success = sem_open("/sem_oxygen_mol_success", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/sem_oxygen_mol_success");

    if ((sem_atom_op_number_inc = sem_open("/sem_atom_op_number_inc", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/sem_atom_op_number_inc");
    if ((sem_molecule_number_inc = sem_open("/sem_molecule_number_inc", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/sem_molecule_number_inc");
    if ((sem_mol_open = sem_open("/sem_mol_open", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) { fatal_error_msg("Sem open failed. errno:%d\n", errno);}
    sem_unlink("/sem_mol_open");

    *hydrogen_expected = NH;
    *oxygen_expected = NO;


    // Create hydrogen processes
    int h_pids[NH];
    int idH = 0;

    for (int i = 0; i < NH; i++) {
        h_pids[i] = fork();
        if (h_pids[i] == -1) {
            fatal_error_msg("Process creation error.");
            exit(1);
        }

        idH++;
        if (h_pids[i] == 0) {
            HydrogenProc(idH, TI);
            return 0;
        }
    }

    // Create oxygen processes
    int o_pids[NO];
    int idO = 0;

    for (int i = 0; i < NO; i++) {
        o_pids[i] = fork();
        if (o_pids[i] == -1) {
            fatal_error_msg("Process creation error.");
            exit(1);
        }

        idO++;
        if (o_pids[i] == 0) {
            OxygenProc(idO, TI, TB);
            return 0;
        }
    }

    // Delay memory unallocation
    for (int i = 0; i < NH; i++) {
        wait(NULL);
    }
    for (int i = 0; i < NO; i++) {
        wait(NULL);
    }

    Free_all_mmap_and_semaphores();

    return 0;
}