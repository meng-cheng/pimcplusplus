#import numarray
import numpy
import IO
from Tables import *
from HTMLgen import *
from HTMLPlots import *
from GraphDraw import *
import stats

def ProcessPairCorrelation(infiles,summaryDoc,detailedDoc,StartCut,L):
    #acquire data about the correlation section
    species1=infiles.ReadVar("Species1")[0]
    species2=infiles.ReadVar("Species2")[0]
    baseName = "%s-%s_PC" % (species1, species2)
    hlabel="r"    #infiles.ReadVar("xlabel")[0]
    vlabel="g(r)" #infiles.ReadVar("ylabel")[0]
    data=infiles.ReadVar("y")
    currNum = 0
    if data==None or data==[None]:
         return currNum
    s = shape(data[0])
    numProcs = len(data)
    (_,numBins) = shape(data[0])
    mShape = (numProcs, numBins)
    meanArray=zeros(mShape)+0.0
    errorArray=zeros(mShape)+0.0
    proc = 0
    for d in data:
        for bin in range(0,numBins):
            (mean,var,error,kappa)=stats.Stats(d[StartCut:-1,bin])
            meanArray[proc,bin]=mean
            errorArray[proc,bin]=error
        proc = proc + 1
    mean  = zeros(numBins) + 0.0
    error = zeros(numBins) + 0.0
    for bin in range(0,numBins):
        (mean[bin],error[bin])=stats.WeightedAvg(meanArray[:,bin],errorArray[:,bin])
        
    x=infiles.ReadVar("x")[0]
    fillx = x
    revx= x[::-1]
    clf()
    fillx=concatenate((x,revx)) #reverse(x))
#    filly = meanArray+errorArray
    y1=mean+error
    y2=mean-error
    filly=concatenate((y1,y2[::-1]))
    fill (fillx, filly,hold=True)
#    hold on
    plot (x, mean,'r',hold=True)
    xlabel ('r')
    ylabel ('g(r)')
    imageName = baseName + ".png"
    epsName = baseName +".eps"
    savefig(imageName,dpi=60)
    savefig(epsName,dpi=60)
    if (x==None):
         return currNum
    description=infiles.ReadVar("Description")[0]

    currNum=currNum+1

##Produce Image
    #myImg=ProduceCorrelationPicture(x, meanArray,baseName,hlabel,vlabel)
##Produce Ascii file
    asciiName = baseName + '.dat'
    asciiFile = open (asciiName, "w")
    n = len(x)
    for i in range(0,n):
      if x[i] <= L/2:
        asciiFile.write('%20.16e %20.16e %20.16e\n' \
                         % (x[i], mean[i], error[i]))
    asciiFile.close()
    
    #WriteAsciiFile(asciiFileName,x,y)

##create table with file names in it
    fileTable=BuildTable(1,2)
    fileTable.body = [[Href(epsName,'PostScript'), Href(asciiName,'ASCII data')]]
##Write things to document
    img = Image(imageName);
    summaryDoc.append(Heading(2,"%s-%s pair correlation" \
                              % (species1, species2)))
    summaryDoc.append(Heading(4,description))
    summaryDoc.append(img)
    summaryDoc.append(BR())
    summaryDoc.append(fileTable)
    return currNum
