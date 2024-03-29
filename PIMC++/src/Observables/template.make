SOURCES = ObservableCorrelation.cc ObservableEnergy.cc ObservableModifiedEnergy.cc AutoCorr.cc ObservableBase.cc PathDump.cc WindingNumber.cc StructureFactor.cc Weight.cc DistanceToHead.cc Time.cc VacancyLocation.cc

objs = ObservableCorrelation.o ObservableEnergy.o ObservableModifiedEnergy.o AutoCorr.o ObservableBase.o PathDump.o WindingNumber.o StructureFactor.o Weight.o DistanceToHead.o Time.o VacancyLocation.o

all:	Observables	

Observables:	${objs}  


clean:
	rm -f *.o

.cc.o: 
	$(CC) $(CCFLAGS) $(DEFS) $(INCL) -o $*.o $< 
.f.o:
	g77 -c -o $*.o $<

newmake:
	+$(MAKE) -f template.make Makefile FRC=force_rebuild

Makefile:	$(FRC)
	rm -f $@
	cp template.make $@
	echo 'Automatically generated dependency list:' >> $@
	$(MAKECC) $(CCFLAGS) $(INCL) $(DEFS) -M $(SOURCES) >> $@
	chmod -w $@


force_rebuild:
