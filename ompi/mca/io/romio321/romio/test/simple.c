/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*  
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#include "mpi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE (65536)

static void handle_error(int errcode, const char *str)
{
	char msg[MPI_MAX_ERROR_STRING];
	int resultlen;
	MPI_Error_string(errcode, msg, &resultlen);
	fprintf(stderr, "%s: %s\n", str, msg);
	MPI_Abort(MPI_COMM_WORLD, 1);
}
/* Each process writes to separate files and reads them back. 
   The file name is taken as a command-line argument, and the process rank 
   is appended to it. */ 

int main(int argc, char **argv)
{
    int *buf, i, rank, nints, len;
    char *filename, *tmp;
    int  errs = 0, toterrs, errcode;
    MPI_File fh;
    MPI_Status status;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

/* process 0 takes the file name as a command-line argument and 
   broadcasts it to other processes */
    if (!rank) {
	i = 1;
	while ((i < argc) && strcmp("-fname", *argv)) {
	    i++;
	    argv++;
	}
	if (i >= argc) {
	    fprintf(stderr, "\n*#  Usage: simple -fname filename\n\n");
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}
	argv++;
	len = strlen(*argv);
	filename = (char *) malloc(len+10);
	strcpy(filename, *argv);
	MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(filename, len+10, MPI_CHAR, 0, MPI_COMM_WORLD);
    }
    else {
	MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
	filename = (char *) malloc(len+10);
	MPI_Bcast(filename, len+10, MPI_CHAR, 0, MPI_COMM_WORLD);
    }
    

    buf = (int *) malloc(SIZE);
    nints = SIZE/sizeof(int);
    for (i=0; i<nints; i++) buf[i] = rank*100000 + i;

    /* each process opens a separate file called filename.'myrank' */
    tmp = (char *) malloc(len+10);
    strcpy(tmp, filename);
    sprintf(filename, "%s.%d", tmp, rank);

    errcode = MPI_File_open(MPI_COMM_SELF, filename, 
		    MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
    if (errcode != MPI_SUCCESS) handle_error(errcode, "MPI_File_open(1)");

    errcode = MPI_File_write(fh, buf, nints, MPI_INT, &status);
    if (errcode != MPI_SUCCESS) handle_error(errcode, "MPI_File_write");

    errcode = MPI_File_close(&fh);
    if (errcode != MPI_SUCCESS) handle_error(errcode, "MPI_File_clode (1)");

    /* reopen the file and read the data back */

    for (i=0; i<nints; i++) buf[i] = 0;
    errcode = MPI_File_open(MPI_COMM_SELF, filename, 
		    MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
    if (errcode != MPI_SUCCESS) handle_error(errcode, "MPI_File_open(2)");

    errcode = MPI_File_read(fh, buf, nints, MPI_INT, &status);
    if (errcode != MPI_SUCCESS) handle_error(errcode, "MPI_File_read");

    errcode = MPI_File_close(&fh);
    if (errcode != MPI_SUCCESS) handle_error(errcode, "MPI_File_close(2)");

    /* check if the data read is correct */
    for (i=0; i<nints; i++) {
	if (buf[i] != (rank*100000 + i)) {
	    errs++;
	    fprintf(stderr, "Process %d: error, read %d, should be %d\n", 
		    rank, buf[i], rank*100000+i);
	}
    }

    MPI_Allreduce( &errs, &toterrs, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    if (rank == 0) {
	if( toterrs > 0) {
	    fprintf( stderr, "Found %d errors\n", toterrs );
	}
	else {
	    fprintf( stdout, " No Errors\n" );
	}
    }

    free(buf);
    free(filename);
    free(tmp);

    MPI_Finalize();
    return 0; 
}
