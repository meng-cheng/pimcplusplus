#include "Communication.h"

#ifdef USE_MPI

/// Sets this communicator to be that of all the processes
/// (i.e. MPI_WORLD)
void CommunicatorClass::SetWorld()
{
  MPIComm = MPI_COMM_WORLD;
}

int CommunicatorClass::MyProc()
{
  int MyRank;
  MPI_Comm_rank(MPIComm, &MyRank);
  return MyRank;
}

int CommunicatorClass::NumProcs()
{
  int NumProcs;
  MPI_Comm_size(MPIComm, &NumProcs);
  return NumProcs;
}

void CommunicatorClass::Send (void *sendBuf, int count, MPI_Datatype datatype,
			      int dest, int tag)
{
  MPI_Send (sendBuf, count, datatype, dest, tag, MPIComm);
}

void CommunicatorClass::Send (int toProc, Array<double,1> &buff)
{
  Send(buff.data(), buff.size(), MPI_DOUBLE, toProc, 1);
}

void CommunicatorClass::BroadCast (int root, Array<double,1> &buff)
{
  MPI_Bcast(buff.data(), buff.size(), MPI_DOUBLE, root, MPIComm);
}

void CommunicatorClass::Receive (void *recvBuf, int count, 
				 MPI_Datatype datatype, int source, int tag)
{
  MPI_Status status;
  MPI_Recv (recvBuf, count, datatype, source, tag, MPIComm, &status);
}

void CommunicatorClass::Receive (int toProc, Array<double,1> &buff)
{
  Receive(buff.data(), buff.size(), MPI_DOUBLE, toProc, 1);
}

bool CommunicatorClass::Probe(int source, int tag, CommStatusClass &status)
{
  int flag;
  MPI_Status mpiStat;
  
  MPI_Iprobe(source, tag, MPIComm, &flag, &mpiStat);
  status.Source = mpiStat.MPI_SOURCE;
  status.Tag    = mpiStat.MPI_TAG;
  status.Error  = mpiStat.MPI_ERROR;
  status.Length = mpiStat.st_length;
  return (flag != 0);
}


void CommunicatorClass::AllGather(void *sendbuf, int sendcount, 
				  MPI_Datatype sendtype, 
				  void* recvbuf, int recvcount,
				  MPI_Datatype recvtype)
{
  MPI_Allgather (sendbuf, sendcount, sendtype, 
		 recvbuf, recvcount, recvtype, MPIComm);
}

void CommunicatorClass::AllGather (Array<double,1> &SendVec, 
				   Array<double,1> &RecvVec)
{
  int numProcs = NumProcs();
  assert ((RecvVec.size()/SendVec.size())==numProcs);	
  int count = SendVec.size();
  double *sendbuf = SendVec.data();
  double *recvbuf = RecvVec.data();
  
  AllGather(sendbuf, count, MPI_DOUBLE,
	    recvbuf, count, MPI_DOUBLE);
}

void CommunicatorClass::Split (int color, CommunicatorClass &newComm)
{
  MPI_Comm_split(MPIComm, color, 0, &(newComm.MPIComm));
}

void CommunicatorClass::Subset (Array<int,1> ranks, CommunicatorClass &newComm)
{
  MPI_Group myGroup, newGroup;
  MPI_Comm_group (MPIComm, &myGroup);
  MPI_Group_incl(myGroup, ranks.size(), ranks.data(), &newGroup);
  MPI_Comm_create(MPIComm, newGroup, &(newComm.MPIComm));
}


///Sends and receives an array of Vec3
void CommunicatorClass::SendReceive (int sendProc, 
				     const Array<Vec3,1> &sendBuff,
				     int recvProc,Array<Vec3,1> &recvBuff)
{
  double *sendPtr = (double *)sendBuff.data();
  double *recvPtr = (double *)recvBuff.data();
  int numSend = sendBuff.size()*3;
  int numRecv = recvBuff.size()*3;
  MPI_Status status;
  MPI_Sendrecv (sendPtr, numSend, MPI_DOUBLE, sendProc, 1, 
		recvPtr, numRecv, MPI_DOUBLE, recvProc, 1,
		MPIComm, &status);
}


///Sends and receives an array of Vec2
void CommunicatorClass::SendReceive (int sendProc, 
				     const Array<Vec2,1> &sendBuff,
				     int recvProc,Array<Vec2,1> &recvBuff)
{
  double *sendPtr = (double *)sendBuff.data();
  double *recvPtr = (double *)recvBuff.data();
  int numSend = sendBuff.size()*2;
  int numRecv = recvBuff.size()*2;
  MPI_Status status;
  MPI_Sendrecv (sendPtr, numSend, MPI_DOUBLE, sendProc, 1, 
		recvPtr, numRecv, MPI_DOUBLE, recvProc, 1,
		MPIComm, &status);
}



///Sends and receives an array of double
void CommunicatorClass::SendReceive (int sendProc, 
				     const Array<double,1> &sendBuff,
				     int recvProc,
				     Array<double,1> &recvBuff)
{
  double *sendPtr = (double *)sendBuff.data();
  double *recvPtr = recvBuff.data();
  int numSend = sendBuff.size();
  int numRecv = recvBuff.size();
  MPI_Status status;
  MPI_Sendrecv (sendPtr, numSend, MPI_DOUBLE, sendProc, 2, 
		recvPtr, numRecv, MPI_DOUBLE, recvProc, 2,
		MPIComm, &status);
}



///Sends and receives an array of int
void CommunicatorClass::SendReceive (int sendProc,const Array<int,1> &sendBuff,
				     int recvProc, Array<int,1> &recvBuff)
{
  int *sendPtr = (int *) sendBuff.data();
  int *recvPtr = recvBuff.data();
  int numSend = sendBuff.size();
  int numRecv = recvBuff.size();
  MPI_Status status;
  MPI_Sendrecv (sendPtr, numSend, MPI_INT, sendProc, 3, 
		recvPtr, numRecv, MPI_INT, recvProc, 3,
		MPIComm, &status);
}



///Sums up the vectors in sendBuff.  Processor 0 only gets the
///resulting sum.
void CommunicatorClass::Sum (Array<double,1> &sendBuff, Array<double,1> &recvBuff)
{
  double *sendPtr = sendBuff.data();
  double *recvPtr = recvBuff.data();
  int count = sendBuff.size();
  
  MPI_Reduce(sendPtr, recvPtr, count, MPI_DOUBLE, MPI_SUM, 0, 
	     MPIComm);
}

///Sums up the vectors in sendBuff.  Processor 0 only gets the
///resulting sum.
void CommunicatorClass::Sum (Array<int,1> &sendBuff, Array<int,1> &recvBuff)
{
  int *sendPtr = sendBuff.data();
  int *recvPtr = recvBuff.data();
  int count = sendBuff.size();
  
  MPI_Reduce(sendPtr, recvPtr, count, MPI_INT, MPI_SUM, 0, 
	     MPIComm);
}



///Sums up all values a.  Only processor 0 gets the result.  All
///other processors return 0;
double CommunicatorClass::Sum (double a)
{
  double sum;
  MPI_Reduce(&a, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPIComm);
  if (MyProc()==0)
    return sum;
  else
    return (0.0);
}
#endif






