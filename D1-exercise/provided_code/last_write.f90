! Created by S. Cozzini and G.P. Brandino for the Course P1.6 Data Management @ MHPC
! Last Revision: November 2015

PROGRAM write_using_set_view

 use mpi
    
 integer :: ierr, i, myrank, thefile, NPE
 integer, parameter :: BUFSIZE=4
 integer, dimension(BUFSIZE) :: buf
 integer(kind=MPI_OFFSET_KIND) :: disp
 integer :: myvec

 call MPI_INIT(ierr)
 call MPI_COMM_RANK(MPI_COMM_WORLD, myrank, ierr)
 call MPI_COMM_SIZE(MPI_COMM_WORLD, NPE, ierr)
 do i = 1, BUFSIZE
    buf(i) = myrank * 10 + i
 enddo

 call MPI_FILE_OPEN(MPI_COMM_WORLD, 'testfile', &
                   MPI_MODE_WRONLY + MPI_MODE_CREATE, &
                   MPI_INFO_NULL, thefile, ierr)

 call MPI_TYPE_SIZE(MPI_INTEGER, intsize, ierr)

 call MPI_TYPE_VECTOR(2, 2, 8, MPI_INTEGER, myvec, ierr)
 call MPI_TYPE_COMMIT(myvec, ierr)

 disp = myrank * BUFSIZE / 2 * intsize

 call MPI_FILE_SET_VIEW(thefile, disp, MPI_INTEGER, &
                       myvec, 'native', &
                       MPI_INFO_NULL, ierr)

 call MPI_FILE_WRITE(thefile, buf, BUFSIZE, MPI_INTEGER, &
                       MPI_STATUS_IGNORE, ierr)

 call MPI_FILE_CLOSE(thefile, ierr)

 call MPI_FINALIZE(ierr)

END PROGRAM write_using_set_view

