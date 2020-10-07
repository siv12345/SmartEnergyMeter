from datetime import datetime as dt
import pickle, tuyapower, time
import platform    # For getting the operating system name
import subprocess  # For executing a shell command

def ping(host):
    param = '-n' if platform.system().lower()=='windows' else '-c'
    command = ['ping', param, '2', host]
    return subprocess.call(command) == 0


now=dt.now()

while True:
        d=dict()
        if dt.now().strftime("%d")!=now.strftime("%d"):
                f=open('/home/pi/pythonTuya/current.dat','wb')
                pickle.dump(d,f)
                f.close()
                now=dt.now()

        f=open('/home/pi/pythonTuya/current.dat','rb')
        d=pickle.load(f)
        f.close()

        if ping('192.168.0.86')==True:
                power=tuyapower.deviceRaw('018065202cf43238c5a8', '192.168.0.86', 'ef2d47140c02257b', '3.3')['dps']['19']
        else:
                power=0
                t=dt.now().strftime("%H:%M")
                d[t]=power
                f=open('/home/pi/pythonTuya/current.dat','wb')
                pickle.dump(d,f)
                f.close()
                time.sleep(600)

        if power>1:
                t=dt.now().strftime("%H:%M")
                d[t]=power/10
                f=open('/home/pi/pythonTuya/current.dat','wb')
                pickle.dump(d,f)
                f.close()
                time.sleep(600)
