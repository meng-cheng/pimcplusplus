#ifndef MCTIME_H
#define MCTIME_H

#include "ObservableBase.h"
#include "../Moves/MoveBase.h"
#include <time.h>

///Currently only owrks in serial mode
class MCTimeClass : public ObservableClass
{

private:
  //Must be initialized
  Array<double,1> MoveTime;
  ObservableVecDouble1 MoveTimeVar;
  Array<double,1> ObservableTime;
  ObservableVecDouble1 ObservableTimeVar;
  ObservableDouble TotalTimeVar;
  int StartTime;
  double TotalTime;
  Array<MoveClass*,1>  &Moves;
  Array<ObservableClass*,1> &Observables;

  int TimesCalled;
  int Freq;
  int DumpFreq;
public:
  void WriteInfo();
  void Accumulate();
  void WriteBlock();
  void Read(IOSectionClass& in);
  MCTimeClass(PathDataClass &myPathData, IOSectionClass &ioSection,
	      Array<MoveClass*,1> &moves,
	      Array<ObservableClass*,1> &observables)
    : ObservableClass(myPathData, ioSection) , 
      MoveTimeVar("MoveTime",IOSection,myPathData.Path.Communicator),
      ObservableTimeVar("ObservableTime",
			IOSection,myPathData.Path.Communicator),
      TotalTimeVar("TotalTime",
		   IOSection,myPathData.Path.Communicator),

      Moves(moves),
      Observables(observables)
  {
    TimesCalled=0;
  }
};


#endif 