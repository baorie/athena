#!/usr/bin/python

import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import bisect

#-------------------------------------------------------
# function readdata: reads the data for athena log file.
#   Returns array[4,n], with time, dispersion, width, and velowidth
def readdata(filename):
    print 'Reading file ' + (filename)		#shows that you're reading a file
    with open(filename) as fh:				#filehandling
        content = [x.strip('\n') for x in fh.readlines()]
        content = [x.strip(' ') for x in content]
    fh.close
    match_time  = [s for s in content if "cycle=" in s]
    match_disp  = [s for s in content if "scalar dispersion" in s] 
    ntime       = len(match_time)
    match_time  = match_time[0:ntime-2]
    #print len(match_time),len(match_disp)
   
    l_time  = []
    l_disp  = []
    l_width = []
    l_vwidth= []
    for line in match_time:
        pos  = line.find("cycle=")
        s    = line[pos:pos+40]
        pos2 = s.find("time=")
        l_time.append(float(s[pos2+5:pos2+17]))
    for line in match_disp:
        pos  = line.find("scalar dispersion")
        s1   = line[pos+27:pos+40]
        s2   = line[pos+41:pos+54]
        s3   = line[pos+55:pos+68]
        l_disp.append(float(s1))
        l_width.append(float(s2))
        l_vwidth.append(float(s3))

    n         = len(l_time)
    data      = np.zeros([4,n])
    data[0,:] = l_time
    data[1,:] = l_disp
    data[2,:] = l_width
    data[3,:] = l_vwidth
    data[3,:] = data[3,:]/data[3,0]

    return data

#=======================================================
# main
#=======================================================

imode   = 0
basefnm = '/Users/riellequiambao/kh/run/'
if imode==0:
    vflow   = np.arange(10.0)*0.1+0.1
    drat    = np.zeros(10)+1.0
else:
    vflow   = np.zeros(10)+0.1
    drat    = np.arange(10.0)+1.0
fname   = 'athoutput.kh'
nrun    = len(vflow)
slope   = np.zeros(nrun)
minlw   = 0.5
maxlw   = 1.0
kap     = 1.0
leng    = 2.0
rho1    = 1.0

#------------------------------------------------------

rho2    = rho1*drat
grate   = 4.0*np.pi*kap*vflow/leng * np.sqrt(rho1*rho2)/(rho1+rho2)

plt.subplot(311)
for i in range(nrun):
    run      = 'm'+(str(int(round(vflow[i]*10.0)))).zfill(2)+'r'+(str(int(round(drat[i])))).zfill(2)
    data     = readdata(basefnm+run+'/'+fname)
    time     = data[0,:]
    lwdth    = np.log(data[3,:])
    max1     = np.argmax(lwdth) 
    ilo      = bisect.bisect(lwdth[0:max1],minlw)
    ihi      = bisect.bisect(lwdth[0:max1],maxlw)
    fit      = np.polyfit(time[ilo:ihi],lwdth[ilo:ihi],1)
    fit_fn   = np.poly1d(fit)
    slope[i] = fit[0]
    print i,fit[0],grate[i]
    plt.plot(time,lwdth,'k-')
    plt.plot(time[ilo:ihi],fit_fn(time[ilo:ihi]),'r-')

plt.xlabel('time')
plt.ylabel('ln width')

plt.subplot(312)
if imode==0:
    plt.plot(vflow,slope,'bo',vflow,grate,'k-')
    plt.xlabel('Mach')
else:
    plt.plot(drat,slope,'bo',drat,grate,'k-')
    plt.xlabel('drat')
plt.ylabel('growth rate')

plt.subplot(313)
if imode==0:
    plt.plot(vflow,(slope-grate)/grate,'ro')
    plt.xlabel('Mach')
else:
    plt.plot(drat,(slope-grate)/grate,'ro')
    plt.xlabel('drat')
plt.ylabel('residual')

plt.show()


