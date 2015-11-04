#!/usr/bin/python
import os
import sys, getopt
import subprocess
import time,atexit
import fcntl
import logging,signal,atexit
import logging.handlers
import zmq

# no need to create class (OOB), define all as global variable
# old config intf mac address and gw mac address
g_CfgOldIntfMacAddr = ''
g_CfgOldGwMacAddr = ''
g_CfgOldIntfType  = ''
# Read definitions Macro static values 
g_pidfile = '/var/run/gwdisc.pid'
g_GwDiscVer = 'Ver 1.0.0'
g_SysLogExecName = '[TranscGwdisc]'
g_SysLogDebugFormat = g_SysLogExecName+'[DEBUG] '
g_SysLogInfoFormat = g_SysLogExecName+'[INFO] '
g_SysLogErrFormat = g_SysLogExecName+'[ERROR] '

        
def fInitSysLog():
    global g_SysLogger
    g_SysLogger = logging.getLogger('MyLogger')
    g_SysLogger.setLevel(logging.INFO)
    handler = logging.handlers.SysLogHandler(address = '/dev/log')
    g_SysLogger.addHandler(handler)
    #g_SysLogger.error('TranscGwdisc Err')
    return 0

def fdelpid():
    g_SysLogger.info(g_SysLogInfoFormat+'remove pid file: '+g_pidfile)
    os.remove(g_pidfile)
    
def fdaemonize (stdin,stdout,stderr):
    '''This forks the current process into a daemon.
    The stdin, stdout, and stderr arguments are file names that
    will be opened and be used to replace the standard file descriptors
    in sys.stdin, sys.stdout, and sys.stderr.
    These arguments are optional and default to /dev/null.
    Note that stderr is opened unbuffered, so
    if it shares a file with stdout then interleaved output
    may not appear in the order that you expect.
    '''
    try:
        pf = file(g_pidfile,'r')
        pid = int(pf.read().strip())
        pf.close()
    except IOError:
        pid = None
    if pid:
        g_SysLogger.error(g_SysLogErrFormat+'pidfile'+g_pidfile+'already exist. Daemon already running?\n')
        sys.exit(1)
    # do the UNIX double-fork magic, see Stevens' "Advanced 
    # Programming in the UNIX Environment" for details (ISBN 0201563177)
    try: 
        pid = os.fork() 
        if pid > 0:
            # exit first parent
            sys.exit(0) 
    except OSError, e: 
        g_SysLogger.error(g_SysLogErrFormat+"fork #1 failed: %d (%s)" % (e.errno, e.strerror))
        sys.exit(1)

    # decouple from parent environment
    os.chdir("/") 
    os.setsid() 
    os.umask(0) 

    # do second fork
    try: 
        pid = os.fork() 
        if pid > 0:
            # exit from second parent, print eventual PID before
            #print "Daemon PID %d" % pid 
            sys.exit(0) 
    except OSError, e: 
        g_SysLogger.error(g_SysLogErrFormat+"fork #2 failed: %d (%s)" % (e.errno, e.strerror))
        sys.exit(1) 

    # Now I am a daemon!

    # Redirect standard file descriptors.
    si = file(stdin, 'r')
    so = file(stdout, 'a+')
    se = file(stderr, 'a+', 0)
    os.dup2(si.fileno(), sys.stdin.fileno())
    os.dup2(so.fileno(), sys.stdout.fileno())
    os.dup2(se.fileno(), sys.stderr.fileno())

    # write pidfile
    atexit.register(fdelpid)
    pid = str(os.getpid())
    file(g_pidfile,'w+').write("%s\n" % pid)

def fParseInput(argv):
    sts = 1
    inputfile = ''
    outputfile = ''
    disctime = 0
    bdaemonize = False
    global g_SysLogger
    try:
        opts, args = getopt.getopt(argv,"hc:o:t:d",["ifile=","ofile=","daemonize"])
        for opt, arg in opts:
            if opt == '-h':
                g_SysLogger.info(g_SysLogInfoFormat+'USAGE: gwdisc.py [-<flag> [<val>],...]')
                g_SysLogger.info(g_SysLogInfoFormat+'gw discovery '+g_GwDiscVer)
                g_SysLogger.info(g_SysLogInfoFormat+'-c <config.yaml>    ,transc config.yaml location')
                g_SysLogger.info(g_SysLogInfoFormat+'-o  <outputfile>    ,sys.yaml outputfile location')
                g_SysLogger.info(g_SysLogInfoFormat+'-t  <interval check>,periodic interval check time')
                g_SysLogger.info(g_SysLogInfoFormat+'-d                  ,daemonize')
                sys.exit()
            elif opt in ("-c", "--ifile"):
                inputfile = arg
            elif opt in ("-o", "--ofile"):
                outputfile = arg
            elif opt in ("-t", "--disctime"):
                disctime = float(arg)
            elif opt in ("-d", "--daemonize"):
                bdaemonize = True
    except getopt.GetoptError:
        g_SysLogger.info(g_SysLogErrFormat+'Error Argument List:', str(sys.argv))
        g_SysLogger.info(g_SysLogErrFormat+'USAGE: gwdisc.py [-<flag> [<val>],...]')
        g_SysLogger.info(g_SysLogErrFormat+'gw discovery '+g_GwDiscVer)
        g_SysLogger.info(g_SysLogErrFormat+'-c <config.yaml>    ,transc config.yaml location')
        g_SysLogger.info(g_SysLogErrFormat+'-o  <outputfile>    ,sys.yaml outputfile location')
        g_SysLogger.info(g_SysLogErrFormat+'-t  <interval check>,periodic interval check time')
        g_SysLogger.info(g_SysLogErrFormat+'-d                  ,daemonize')
    if (('' == inputfile) or ('' == outputfile)):
        sts = 1
    else:
        sts = 0
    if(0 == disctime):
        disctime = 180
    return sts,inputfile,outputfile,disctime,bdaemonize

def fGetRouteTableInfo(sysc):
    rtable = ''
    sts = 1
    try:
        p = subprocess.Popen(sysc, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        rtable = p.stdout.readlines()
        sts = p.wait()
    except Exception:
        g_SysLogger.error(g_SysLogErrFormat+'error invoking call to: '+sysc)
    if 0 != sts:
        rtable = ''
    return rtable

def fReadTransCConfigDotYaml(configdotyamlloc):
    outintfline = ''
    modeofoptline = ''
    outputinterface = []
    outputinterfacetype = []
    modeofoperation = ''
    f = None
    try:
        f = open(configdotyamlloc, 'r+')
        fcntl.flock(f.fileno(), fcntl.LOCK_EX)
        for line in f:
            if line.startswith('outgoinginterface:'):
                outintfline = line
            elif line.startswith('modeofoperation:'):
                modeofoptline = line
            if '' != outintfline and '' != modeofoptline:
                break
        f.close()
        f = None
    except IOError:
        g_SysLogger.error(g_SysLogErrFormat+'Error: File does not appear to exist.')
    finally:
        if f:
            f.close()
    if ('' != outintfline):
        arglst = (outintfline.split('\'')[1]).split(';')
        for arg in arglst:
            if (0 != len(arg)):
                val = (arg.replace('"', '').strip()).split(':')
                if (0 < len(val)):
                    outputinterface.append(val[0])
                    outputinterfacetype.append(val[1])
    if ('' != modeofoptline):
        arg = modeofoptline.split('\'')[1]
        if (0 != len(arg)):
            modeofoperation = arg
    return outputinterface,outputinterfacetype,modeofoperation

def fGetGwIPAddrSubnet(sysc):
    #TODO:...
    gwIpv4AddrSubnet = ''
    gwIpv6AddrSubnet = ''
    try:
        p = subprocess.Popen(sysc, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in p.stdout.readlines():
            if 0 < line.find('inet '):
                tempIpv4AddrList = ((line.split('inet ')[1]).split(' ')[0]).split('.')
                #remove last values with CIDR
                i = 0
                for val in tempIpv4AddrList:
                    if(i < 3):
                        gwIpv4AddrSubnet += val+'.'
                    i = i+1
                #print gwIpv4AddrSubnet
            elif 0 < line.find('inet6 '):
                tempIpv6AddrList = ((line.split('inet6 ')[1]).split(' ')[0]).split(':')
                #remove last values with CIDR
                i = 0
                for val in tempIpv6AddrList:
                    if(i < 5):
                        gwIpv6AddrSubnet += val+':'
                    i = i+1
                #print gwIpv6AddrSubnet
        sts = p.wait()
        if (0 != sts):
            gwIpv4AddrSubnet = ''
            gwIpv6AddrSubnet = ''
    except Exception:
        g_SysLogger.error(g_SysLogErrFormat+'error invoking call to: '+sysc)
    return gwIpv4AddrSubnet,gwIpv6AddrSubnet

def fGetGWIPaddrHelper(dfltGwIpAddr,intfRtable,gwIpAddrSubnet):
    gwIpAddr = ''
    for ipaddr in intfRtable:
        if ipaddr.startswith(gwIpAddrSubnet):
            gwIpAddr = ipaddr.rstrip()
            break
    if '' == gwIpAddr:
        for ipaddr in intfRtable:
            if '*'== gwIpAddrSubnet:
                gwIpAddr = dfltGwIpAddr.rstrip()
                break
    return gwIpAddr

def fResolveGwIPAddr(sysc,dfltGwIpAddr,intfRtable):
    gwIpv4Addr = ''
    gwIpv6Addr = ''
    gwIpv4AddrSubnet,gwIpv6AddrSubnet = fGetGwIPAddrSubnet(sysc)
    if '' != gwIpv4AddrSubnet:
        gwIpv4Addr = fGetGWIPaddrHelper(dfltGwIpAddr,intfRtable,gwIpv4AddrSubnet)
    elif '' != gwIpv6AddrSubnet:
        gwIpv6Addr = fGetGWIPaddrHelper(dfltGwIpAddr,intfRtable,gwIpv6AddrSubnet)
    return gwIpv4Addr,gwIpv6Addr

def fGetGwMacAddrHelper(sysc,gwIpAddr,outputinterface):
    #TODO:...
    gwMacAddr = ''
    try:
        p = subprocess.Popen(sysc, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in p.stdout.readlines():
            args = line.split(' ')
            if ((3 < len(args)) and (gwIpAddr == args[0]) and (outputinterface == args[2]) and ('' != args[4])):
                gwMacAddr = args[4]
        sts = p.wait()
        if (0 != sts):
            gwMacAddr = ''
    except Exception:
        g_SysLogger.error(g_SysLogErrFormat+'error invoking call to: '+sysc)
    return gwMacAddr

def fPingGateway(sysc):
    rbuff = ''
    sts = 1
    try:
        p = subprocess.Popen(sysc, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        rbuff = p.stdout.readlines()
        sts = p.wait()
    except Exception:
        g_SysLogger.error(g_SysLogErrFormat+'error invoking call to: '+sysc)
    if 0 != sts:
        rbuff = ''
    return rbuff

def fGetGwMacAddr(outputinterface,dfltGwIpAddr):
    gwMacAddr = ''
    gwUp = False
    intfRtable = fGetRouteTableInfo('route | grep '+outputinterface+' | awk \'{print $2}\'')
    if '' != intfRtable:
        gwIpv4Addr,gwIpv6Addr = fResolveGwIPAddr('ip addr show '+outputinterface,dfltGwIpAddr,intfRtable)
        if ('' != gwIpv4Addr):
            gwMacAddr = fGetGwMacAddrHelper('ip neigh show '+gwIpv4Addr,gwIpv4Addr,outputinterface)
            '''
            rbuff = fPingGateway('arping -c 3 -I '+outputinterface+' '+gwIpv4Addr+' | grep \"eceived\"')
            if 0 != len(rbuff):
                args = rbuff[0].split(' ')
                if (0 == args[1]):
                    gwMacAddr = ''
                else:
                    gwUp = True
            else:
                gwMacAddr = ''
            '''
            gwUp = True
        elif('' != gwIpv6Addr):
            gwMacAddr = fGetGwMacAddrHelper('ip neigh show '+gwIpv6Addr,gwIpv6Addr,outputinterface)
            '''
            rbuff = fPingGateway('ping -c 3 -I '+outputinterface+' '+gwIpv6Addr+' | grep \"eceived\"')
            if 0 != len(rbuff):
                args = (rbuff[0].split(','))[1].split(' ')
                if (0 == args[1]):
                    gwMacAddr = ''
                else:
                    gwUp = True
            else:
                gwMacAddr = ''
            '''
            gwUp = True
    return gwMacAddr,gwUp

def fGetIntfMacAddr(outputinterface):
    intfMacAddr = ''
    try:
        p = subprocess.Popen('cat /sys/class/net/'+outputinterface+'/address', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in p.stdout.readlines():
            intfMacAddr = line.strip()
            break
        sts = p.wait()
        if (0 != sts):
            intfMacAddr = ''
    except Exception:
        g_SysLogger.error(g_SysLogErrFormat+'error trying to access cat /sys/class/net/'+outputinterface+'/address')
    return intfMacAddr


def fwriteSysYaml(outfileloc,modeofoperation,intfType,intfMacAddr,gwMacAddr):
    f = None
    try:
        f = open(outfileloc, 'w')
        fcntl.flock(f.fileno(), fcntl.LOCK_EX)
        f.write('MODEOFOPERATION: \''+modeofoperation+'\'\n')
        g_SysLogger.info(g_SysLogInfoFormat+'MODEOFOPERATION: '+modeofoperation)
        f.write('INTERFACE_TYPE: \''+intfType+'\'\n')
        g_SysLogger.info(g_SysLogInfoFormat+'INTERFACE_TYPE: '+intfType)
        f.write('INTERFACE_MACADDR: \''+intfMacAddr+'\'\n')
        g_SysLogger.info(g_SysLogInfoFormat+'INTERFACE_MACADDR: '+intfMacAddr)
        f.write('GATEWAY_MACADDR: \''+gwMacAddr+'\'\n')
        g_SysLogger.info(g_SysLogInfoFormat+'GATEWAY_MACADDR: '+gwMacAddr)
        f.close()
        f = None
    except IOError:
        g_SysLogger.error(g_SysLogErrFormat+'Error: File does not appear to exist.')
    finally:
        if f:
            f.close()
    return 0

def fPostMsgToZMq():
    try:
        context = zmq.Context()
        socket = context.socket(zmq.REQ)
        #socket.setsockopt(zmq.LINGER, 0)
        socket.connect("tcp://localhost:5555")  # The PORT is fixed for this release and on the same server as the TCS.
        socket.send('500')  # Value is fixed
        msg = socket.recv()  # Have to recv since it blocks...
        if msg == "END":
            g_SysLogger.info(g_SysLogInfoFormat+"Zmq Write: {\"success\": true}")
        else:
            g_SysLogger.info(g_SysLogInfoFormat+"Zmq Write: {\"success\": false}")
    except BaseException:
        g_SysLogger.error(g_SysLogErrFormat+"Zmq exception {\"success\": false}")
    finally:
        socket.close()

def fCfgUpdate(strIntfType,strIntfMacAddr,strGwMacAddr):
    global g_CfgOldIntfType
    global g_CfgOldIntfMacAddr
    global g_CfgOldGwMacAddr
    bCfgUpdate = False
    if( g_CfgOldIntfType != strIntfType):
        g_CfgOldIntfType = strIntfType
        bCfgUpdate = True
    if( g_CfgOldIntfMacAddr != strIntfMacAddr):
        g_CfgOldIntfMacAddr = strIntfMacAddr
        bCfgUpdate = True
    if( g_CfgOldGwMacAddr != strGwMacAddr):
        g_CfgOldGwMacAddr = strGwMacAddr
        bCfgUpdate = True
    if bCfgUpdate:
        g_SysLogger.info(g_SysLogInfoFormat+"pushing sys.yaml update to transc")  
    else:
        g_SysLogger.debug(g_SysLogDebugFormat+"no sys.yaml update necessary")
    return bCfgUpdate

def fFmtString(lstIntfName,lstIntfType):
    idx = 0
    strName = ''
    nlst = len(lstIntfName)
    for intfN in lstIntfName:
        if('null' != lstIntfName):
            if idx < nlst-1:
                strName +='\"'+intfN+':'+lstIntfType[idx]+'\";'
            else:
                strName +='\"'+intfN+':'+lstIntfType[idx]+'\"'
        idx +=1
    return strName    
     
def fGwDisc(configdotyamlloc,outfileloc,bdaemonize,bPostToZmq):
    modeofoperation = ''
    sts             = 1
    lstIntfName     = []
    lstIntfType     = []
    lstIntfMacAddr  = []
    lstGwMacAddr    = []
    typeIdx         = 0
    lstIntfName,lstIntfType,modeofoperation = fReadTransCConfigDotYaml(configdotyamlloc)
    if '' != modeofoperation and ('' != lstIntfName[0] and '' != lstIntfType[0]) and (len(lstIntfName) == len(lstIntfType)):
        for strIntf in lstIntfName:
            strMacAddr = fGetIntfMacAddr(strIntf)
            if ('' != strMacAddr):
                lstIntfMacAddr.append(strMacAddr)
                if 'router' == lstIntfType[typeIdx]:
                    dfltGwIpAddr = fGetRouteTableInfo('route | grep default | grep '+strIntf+' | awk \'{print $2}\'')
                    if '' != dfltGwIpAddr: 
                        g_SysLogger.debug(g_SysLogDebugFormat+'out intf name: '+strIntf)
                        g_SysLogger.debug(g_SysLogDebugFormat+'out intf inject to router (L3) switch')
                        strMacAddr,gwUp = fGetGwMacAddr(strIntf,dfltGwIpAddr)
                        if (('' != strMacAddr) and gwUp):
                            lstGwMacAddr.append(strMacAddr)
                            sts = 0
                        else:
                            lstGwMacAddr.append('00:00:00:00:00:00')
                            g_SysLogger.error(g_SysLogErrFormat+'unable to retrieve Gateway MAC addr')
                    else:
                        lstGwMacAddr.append('00:00:00:00:00:00')
                        g_SysLogger.error(g_SysLogErrFormat+'unable to retrieve Routing Table.')
                elif 'other' == lstIntfType[typeIdx]:
                    lstGwMacAddr.append('00:00:00:00:00:00')
                    g_SysLogger.debug(g_SysLogDebugFormat+'out intf name: '+strIntf)
                    g_SysLogger.debug(g_SysLogDebugFormat+'out intf inject to other (L2) switch')
                    sts = 0
                else:
                    lstGwMacAddr.append('00:00:00:00:00:00')
                    g_SysLogger.error(g_SysLogErrFormat+'unknown interface type')
            else:
                lstIntfName[typeIdx] = 'null'
                lstIntfMacAddr.append('null')
                lstGwMacAddr.append('null')
                g_SysLogger.error(g_SysLogErrFormat+'unable to retrieve local intf MAC addr')
            typeIdx +=1
        if (len(lstIntfName) == len(lstIntfType) == len(lstIntfMacAddr) == len(lstGwMacAddr)):
            strIntfType    = fFmtString(lstIntfName,lstIntfType)
            strIntfMacAddr = fFmtString(lstIntfName,lstIntfMacAddr)
            strGwMacAddr   = fFmtString(lstIntfName,lstGwMacAddr)
            if fCfgUpdate(strIntfType,strIntfMacAddr,strGwMacAddr):
                fwriteSysYaml(outfileloc,modeofoperation,strIntfType,strIntfMacAddr,strGwMacAddr)
                if(bPostToZmq):
                    fPostMsgToZMq()
    else:
        g_SysLogger.error(g_SysLogErrFormat+'retrieveing output interface from '+configdotyamlloc+' or not specified')   
    return sts

def fRunDaemon(configdotyamlloc,outfileloc,disctime,bdaemonize):
    sts = 1
    fdaemonize('/dev/null','/dev/null','/dev/null')
    while True:
        try:
            sts = fGwDisc(configdotyamlloc,outfileloc,bdaemonize,True)
            if (0 != sts):
                g_SysLogger.error(g_SysLogErrFormat+'Error, processing gateway discovery.')
            time.sleep(disctime)
        except (KeyboardInterrupt, SystemExit):
            g_SysLogger.info(g_SysLogInfoFormat+'Ctrl-C caught, Exiting '+g_SysLogExecName +'...')
            break
    return sts

def fRunNonDaemon(configdotyamlloc,outfileloc,bdaemonize):
    sts = fGwDisc(configdotyamlloc,outfileloc,bdaemonize,False)
    if (0 != sts):
        g_SysLogger.error(g_SysLogErrFormat+'Error, processing gateway discovery.')
    return sts
    
def main(argv):
    fInitSysLog()
    sts,configdotyamlloc,outfileloc,disctime,bdaemonize = fParseInput(argv)
    if 0 == sts:
        g_SysLogger.info(g_SysLogInfoFormat+'running gwdisc service' + g_GwDiscVer + ', '+g_pidfile)
        if(bdaemonize):
            sts = fRunDaemon(configdotyamlloc,outfileloc,disctime,bdaemonize)
        else:
            sts = fRunNonDaemon(configdotyamlloc,outfileloc,bdaemonize)
    return sts
                
if __name__ == "__main__":
    sts = main(sys.argv[1:])
    sys.exit(sts)
    
    
    