PGMS=mmult_omp_timing matrix_times_vector hello pi mxv_omp_mpi mmult_mpi_omp

all:	${PGMS}

mmult_mpi_omp:		mmult.o mmult_mpi_omp.o
	mpicc -w -o mmult_mpi_omp -fopenmp -O3 mmult.o mmult_mpi_omp.o

mmult_mpi_omp.o:	mmult_mpi_omp.c
	mpicc -w -c -fopenmp -O3 mmult_mpi_omp.c

mmult_omp_timing:	mmult.o mmult_omp.o mmult_omp_timing.o
	gcc -w -o mmult -fopenmp -O3 mmult.o mmult_omp.o mmult_omp_timing.o -o mmult_omp_timing

mmult.o:	mmult.c
	gcc -w -c -O3 mmult.c

mmult_omp.o:	mmult_omp.c
	gcc -w -c -O3 -fopenmp mmult_omp.c

mmult_omp_timing.o:	mmult_omp_timing.c
	gcc -w -c -O3 mmult_omp_timing.c

matrix_times_vector:	matrix_times_vector.c
	mpicc -w -O3 -o matrix_times_vector matrix_times_vector.c

hello:	hello.c
	mpicc -w -O3 -o hello hello.c

pi:	pi.c
	mpicc -w -O3 -o pi pi.c

mxv_omp_mpi:	mxv_omp_mpi.c
	mpicc -w -fopenmp -O3 -o mxv_omp_mpi mxv_omp_mpi.c mmult.c

clean:
	rm -f *.o
	rm -f ${PGMS}

