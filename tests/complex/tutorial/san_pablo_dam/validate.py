import sys, os, math, subprocess

dDay = 1
obsTimes = {}
cntObs = [0,0,0,0,0,0,0,0,0]
verbose = False
    
def readTimes(obsfile):
    times = []
    for line in open(obsfile):
        ll = line.split(':')
        if ll:
            times.append(3600*int(ll[0]) + 60*int(ll[1]) + int(float(ll[2])))
    return times

def parseObsTimes():
    for i in range (0,9): 
       obsTimes[i] = []
    for i in range(1,8):
        if dDay==1 and i==5:
            continue
        if dDay==2 and i==6:
            continue
        obsTimes[i] = readTimes('data/obstimes_%s_%s.txt' % (dDay, i))

    # convert obsTimes[][] into travel-times:
    for i in range(1,8):
        ni = len(obsTimes[i])
        nip = len(obsTimes[i+1])
        cntObs[i] = ni
        if ni==nip and ni > 100: 
            for j in range(ni):
                obsTimes[i][j] = obsTimes[i+1][j] - obsTimes[i][j]

def validate(sumoBinary):
    retcode = subprocess.call([sumoBinary, "-c", "data/spd-road.sumocfg"], stdout=sys.stdout, stderr=sys.stderr)
    sys.stdout.flush()
    sys.stderr.flush()

    # analyzing the results...
    # read the empirical times
    simTimes = {}
    for i in range (0,9): 
       simTimes[i] = {}

    # read the simulated times
    obs2Nr = {'obs1': 1, 'obs2': 2, 'obs3': 3, 'obs4': 4, 'obs5': 5, 'obs6': 6, 'obs7': 7}  
    cnt = [0,0,0,0,0,0,0,0,0]

    for line in open('data/detector.xml'):
        if line.find('<interval')!=-1:
            ll = line.split('"')
            nn = int(ll[7])
            tIn = float(ll[1])
            iObs = obs2Nr[ll[5]]
            if nn>0:
                simTimes[iObs][cnt[iObs]] = tIn
                cnt[iObs] += 1

    # convert simTimes[][] into travel-times:
    for i in range(1,8):
        ni = len(simTimes[i])
        nip = len(simTimes[i+1])
        if ni==nip and ni>100:
            for j in range(ni):
                simTimes[i][j] = simTimes[i+1][j] - simTimes[i][j]

    # compute final statistics
    err = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    averTT = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    errAll = 0.0
    cntAll = 0
    if verbose:
        f = open('data/sumo-obs-error.txt','w')
    for i in range(1,7):
        if cntObs[i]<=100 or cntObs[i+1]<=100:
            continue
        if len(obsTimes[i]) == len(simTimes[i]):
            tmp = 0.0
            for j in range(cntObs[i]):
                d = obsTimes[i][j] - simTimes[i][j]
                tmp += d*d
            err[i] = math.sqrt(tmp/cntObs[i])
            if verbose:
                print >> f, "%s %s" %(i, err[i])
            errAll += err[i]
            cntAll += 1
    if verbose:
        f.close()

    # finally, write the individual travel times into a csv-file
    # this is not really needed when validate is called from calibrate as an intermediate
    # step, but it makes analyzing of the result more simple.
    # first the header
    if verbose:
        c = open('data/compare-tt.csv', 'w')
        c.write('# indx;')
        for i in range(1,7):
                if cntObs[i]>100 and cntObs[i+1]>100:
                        c.write('obs'+repr(i)+';sim'+repr(i)+';')
        c.write('\n')

        # then the data, of course on the ones which are useable
        for line in range(len(simTimes[1])):
                c.write(repr(line)+';')
                for i in range(1,7):
                        if cntObs[i]>100 and cntObs[i+1]>100:
                                ttObs = int(obsTimes[i][line])
                                ttSim = int(simTimes[i][line])
                                c.write(repr(ttObs)+';'+repr(ttSim)+';')
                c.write('\n')
        c.close()
    return errAll/cntAll
