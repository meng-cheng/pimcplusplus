import numarray
import IO
from Tables import *
from HTMLgen import *
from HTMLPlots import *
import stats

def ProcessEnergy(infiles,summaryDoc,detailedDoc,StartCut):
    N=infiles.CountVars()
    varList = []
    numProcs=0
    for i in range(0,N):
        data = infiles.ReadVar(i)
        if (type(data[0])==numarray.numarraycore.NumArray):
            varList.append(i)
            numProcs=len(data)
    scalarVarTable=BuildTable(numProcs+1,len(varList)+1)
    scalarVarTable.body[0][0]="Proc"
    scalarTracePageHTMLList=[]
    summaryTable=BuildTable(len(varList)+1,5)
    summaryTable.body[0]=["Energy","Mean","Error","Variance","Kappa"]
    row = 0
    print varList
    for varCounter in varList:
        row = row + 1
        meanList=[]
        errorList=[]
        varList=[]
        kappaList=[]
        data = infiles.ReadVar(varCounter)
        varName = infiles.GetVarName(varCounter)
        baseName=varName+"Energy"
        scalarTracePageHTMLList.append(BuildScalarTracePage(data,baseName,varName,StartCut))
        print row,len(scalarVarTable.body),len(scalarVarTable.body[0])
        scalarVarTable.body[0][row]=varName
        for proc in range(0,numProcs):
            (mean,var,error,kappa)=stats.Stats(data[proc][StartCut:-1])
            meanList.append(mean)
            errorList.append(error)
            varList.append(var)
            kappaList.append(kappa)
            (meanstr, errorstr) = stats.MeanErrorString (mean, error)
            scalarVarTable.body[proc+1][0]=repr(proc)
            scalarVarTable.body[proc+1][row]=meanstr + "+/-" + errorstr
        (totalMean,totalError)=stats.UnweightedAverage(meanList,errorList)
        totalVar=sum(varList)/len(varList)
        totalKappa=sum(kappaList)/len(kappaList)
        (totalMeanStr,totalErrorStr)=stats.MeanErrorString(totalMean,totalError)
        summaryTable.body[row]=[varName,totalMeanStr,totalErrorStr, '%1.2e' % totalVar ,'%1.2f' % totalKappa]
    summaryDoc.append(summaryTable)
    detailedDoc.append(scalarVarTable)
    for pageName in scalarTracePageHTMLList:
        myFrame=IFrame()
        myFrame.src=pageName
        myFrame.width="100%"
        myFrame.height="375"
        detailedDoc.append(myFrame)
    return infiles,summaryDoc,detailedDoc

