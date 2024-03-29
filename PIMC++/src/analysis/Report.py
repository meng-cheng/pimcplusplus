#!/usr/bin/python
###!/usr/bin/env python
from IO import *
import sys
import os
import math
import stats
##import numarray
import TimeLindenman
from AcceptRatio import *
from Energy import *
from nofr import *
from PairCorrelation import *
from HTMLgen import *
from SystemInfo import *
from Josephson import *
from Coupling import *
from Vacancy import *
from PlaneDensity import *
from Pressure import *
from WindingNumber import *
from Langevin import *
from TimeAnalysis import *
from VacancyDensity import *
from SpecificHeat import *
from Hexatic import *
from SpecificHeatA import *
from CycleCount import *
from StructureFactor import *
from SuperfluidFraction import *
import BisectionBlock
##from Conductivity import *

import resource
print resource.getrlimit(resource.RLIMIT_NOFILE)

infiles = IOSectionClassList()
if (sys.argv[1]=='-f'):
     print "NOTE: First argument is -f"
     basename=sys.argv[2]
     listFile=file(basename)
     filesList=listFile.readlines()
     for counter in range(0,len(filesList)):
          filesList[counter]=filesList[counter][:-1]
     print "The file list is ",filesList
     infiles.OpenFilesList(filesList)
     basename=basename+"d"
else:
     basename = sys.argv[1]
     splitBaseName=string.split(basename,'.')
     if splitBaseName[-1]=='h5':
          basename=string.join(splitBaseName[0:-2],'.')
     infiles.OpenFiles(basename);

if (infiles.len() > 1):
     print 'Found ' +repr(infiles.len()) + ' output files.'

dirName = basename
StartCut = None
if (len(sys.argv) > 2):
  StartCut = int(sys.argv[2])
elif (os.access(dirName+"/.pref",os.F_OK)):
  prefFile=IOSectionClass()
  prefFile.OpenFile(dirName+"/.pref")
  StartCut = prefFile.ReadVar("StartCut")
  prefFile.CloseFile() 
else:
  StartCut = 0
print "StartCut = ", StartCut

cutoff = None
if (len(sys.argv) > 3):
  cutoff = int(sys.argv[3])
elif (os.access(dirName+"/.pref",os.F_OK)):
  prefFile=IOSectionClass()
  prefFile.OpenFile(dirName+"/.pref")
  cutoff = prefFile.ReadVar("cutoff")
  prefFile.CloseFile() 
else:
  cutoff=0
print "cutoff = ", cutoff

if not(os.access(dirName,os.F_OK)):
     os.mkdir(dirName)
os.chdir(dirName)
detailedDoc=SeriesDocument()
summaryDoc=SeriesDocument()
summaryDoc.append(Href("detail.html","Detailed HTML Page"))
summaryDoc.append(HR())


#ProcessMove(doc,infiles)

try:
     (_,tau,numTimeSlices,box)=ProcessTopTable(summaryDoc,infiles)
     beta = tau*numTimeSlices
     temp = 1.0/beta
     print beta, temp, box
except:
     print "top table failed"

#infiles.OpenSection("Moves")
#numSections=infiles.CountSections()
#for secNum in range(0,numSections):
#     infiles.OpenSection(secNum)
#     moveName=infiles.GetName()
#     if moveName=="BisectionBlock":
##          print "Processing BisectionBlock"
#          BisectionBlock.Process(infiles,summaryDoc,detailedDoc,StartCut)
#     infiles.CloseSection()
#infiles.CloseSection()
          
#############
#### Moves ##
#############
##infiles.OpenSection("Moves")
##numSections=infiles.CountSections()
###try:
##for secNum in range(0, numSections):
##               infiles.OpenSection(secNum)
###          try:
##               moveName = infiles.GetName()
##               if moveName == "Langevin":
##                    print "Processing Langevin move."
##                    ProcessLangevin(infiles, summaryDoc, detailedDoc, StartCut, beta)
##                    print "Done Langevin."
##               elif moveName=="BisectionBlock":
##                    print "Processing Bisection move."
##                    ProcessBisectionBlock(infiles,summaryDoc,detailedDoc,StartCut)
##               elif moveName=="Displace":
##                    print "Processing Displace move."
##                    ProcessDisplaceMove(infiles,summaryDoc,detailedDoc,StartCut)
##               infiles.CloseSection() # Current move section
###          except:
###               print "Moves also broken"
###               infiles.CloseSection() # Current move section
###except:
###     print "Moves broken"

##infiles.CloseSection() # "Moves"

#################
## Observables ##
#################
currNum=0
infiles.OpenSection("Observables")
numSections=infiles.CountSections()
#print "Number of sections is ",numSections
for counter in range(0,numSections):
  infiles.OpenSection(counter)
  myName = infiles.GetName()
  myType = infiles.ReadVar("Type")[0]
  print "Currently processing ", myName
  if 1==1:
    if myName=="PairCorrelation":
      ProcessPairCorrelation(infiles,summaryDoc,detailedDoc,StartCut,box[0])
    if myName=="Vacancy":
      ProcessVacancy(infiles,summaryDoc,detailedDoc,StartCut)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="Energy" or myName=="Energies":
      ProcessEnergy(infiles,summaryDoc,detailedDoc,StartCut)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="CycleCount":
      ProcessCycleCount(infiles,summaryDoc,detailedDoc,StartCut)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="SpecificHeat":
      ProcessSpecificHeat(infiles,summaryDoc,detailedDoc,StartCut)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="SpecificHeatA":
      ProcessSpecificHeatA(infiles,summaryDoc,detailedDoc,StartCut)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="nofr":
      Processnofr(infiles,summaryDoc,detailedDoc,StartCut,box)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="Pressure":
      ProcessPressure(infiles,summaryDoc,detailedDoc,StartCut)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="WindingNumber":
      ProcessWindingNumber(infiles,summaryDoc,detailedDoc,StartCut)
    elif myName=="SuperfluidFraction":
      ProcessSuperfluidFraction(infiles,summaryDoc,detailedDoc,StartCut,temp)
    elif myName=="Hexatic":
      ProcessHexatic(infiles,summaryDoc,detailedDoc,StartCut)
    elif myName=="PlaneDensity":
      ProcessPlaneDensity(infiles,summaryDoc,detailedDoc,StartCut,box)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="TimeAnalysis":
      ProcessTimeAnalysis(infiles,summaryDoc,detailedDoc,StartCut)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="TimeLindenman":
      TimeLindenman.Process(infiles,summaryDoc,detailedDoc,StartCut)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="VacancyDensity":
      ProcessVacancyDensity(infiles,summaryDoc,detailedDoc,StartCut)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="StructureFactor":
#     for i in range(0,20):
#       ProcessStructureFactor(infiles,summaryDoc,detailedDoc,i*50)
      ProcessStructureFactor(infiles,summaryDoc,detailedDoc,StartCut)
      summaryDoc.append(HR())
      detailedDoc.append(HR())
    elif myName=="PhiK":
      ProcessJosephson(infiles,summaryDoc,detailedDoc,StartCut,tau,numTimeSlices)
    elif myName=="Conductivity":
      ProcessConductivity(infiles,summaryDoc,detailedDoc, StartCut)
    elif myName=="Coupling":
      ProcessCoupling(infiles,summaryDoc,detailedDoc,StartCut)
    else:
      a=5
  #except:
     #print "Error in processing ", myName
  infiles.CloseSection()
infiles.CloseSection() # "Observables"

summaryDoc.logo=""
summaryDoc.author="Ken and Bryan"
summaryDoc.email="esler@uiuc.edu and bkclark@uiuc.edu"
summaryDoc.banner=("http://esler.physics.uiuc.edu/pimcLogo.png")
summaryDoc.place_nav_buttons=0
summaryDoc.header()

summaryDoc.write("index.html")

detailedDoc.logo=""
detailedDoc.author="Ken and Bryan"
detailedDoc.email="esler@uiuc.edu and bkclark@uiuc.edu"
detailedDoc.banner=("http://esler.physics.uiuc.edu/pimcLogo.png")
detailedDoc.place_nav_buttons=0
detailedDoc.header()

detailedDoc.write("detail.html")

        
