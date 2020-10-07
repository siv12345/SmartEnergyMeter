#!/usr/bin/env python3
from flask import Flask, render_template
import datetime as dt, pickle, tuyapower
import platform    # For getting the operating system name
import subprocess  # For executing a shell command

def ping(host):
    param = '-n' if platform.system().lower()=='windows' else '-c'
    command = ['ping', param, '2', host]
    return subprocess.call(command) == 0

app = Flask(__name__)

def truncate(n, decimals=0):
        multiplier = 10 ** decimals
        return int(n * multiplier) / multiplier

def leapYear(year):
    if (year % 4) == 0:
        if (year % 100) == 0:
            if (year % 400) == 0:
                return True
            else:
                return False
        else:
             return True
    else:
        return False


old=3676.4
data=total=volt=today=amp=watt=switch=0

@app.route("/")
def meter():
        day=int(dt.datetime.now().strftime("%d"))
        month=int(dt.datetime.now().strftime("%m"))
        year=int(dt.datetime.now().strftime("%y"))

        f=open("/home/pi/pythonTuya/day.dat","rb")
        d=pickle.load(f)
        f.close()


        if ping('192.168.0.86')==False:
                templateData = {'volt' : '-','amp' : '-','watt' : '-','state' : 'Offline - GridDown','total' : str(d[day]),'today': '-'}
                legend=labels=values=[]
                return render_template('index.html', values=values, labels=labels, legend=legend, **templateData)

        data=tuyapower.deviceRaw('018065202cf43238c5a8', '192.168.0.86', 'ef2d47140c02257b', '3.3')['dps']

        total=str(truncate(old+data['101']/100,2))+'  kWh'
        volt=str(truncate(data['20']/10,1))+'      V'
        amp=str(data['18'])+'      mA'
        watt=str(truncate(data['19']/10,1))+'     W'
        switch='Online' if data['18']!=0 else 'Offline - SunDown'

        d[day]=truncate(old+data['101']/100,2)
        if month==2 and day==29:
                d[0]=d[day]
        elif month==2 and day==28:
                d[0]=d[day]
        elif month not in [1,3,5,7,8,10,12] and day==31:
                d[0]=d[day]
        elif month in [1,3,5,7,8,10,12] and day==30:
                d[0]=d[day]

        if month==3 and day==1 and leapYear(year)==True:
                today=str(truncate(old+data['101']/100-d[29],2))+'  kWh'
        elif month==3 and day==1 and leapYear(year)==False:
                today=str(truncate(old+data['101']/100-d[28],2))+'  kWh'
        elif month not in [1,5,7,8,10,12] and day==1:
                today=str(truncate(old+data['101']/100-d[31],2))+'  kWh'
        elif month in [1,5,7,8,10,12] and day==1:
                today=str(truncate(old+data['101']/100-d[30],2))+'  kWh'
        else:
                today=str(truncate(old+data['101']/100-d[day-1],2))+'  kWh'

        f=open("/home/pi/pythonTuya/day.dat","wb")
        pickle.dump(d,f)
        f.close()

        templateData = {
        'volt' : volt,
        'amp' : amp,
        'watt' : watt,
        'state' : switch,
        'total' : total,
        'today': today
        }

        f=open("/home/pi/pythonTuya/current.dat","rb")
        power=pickle.load(f)
        f.close()

        legend = 'Daily Units'
        s=dict()
        for i,j in list(d.items()):
                if i>=0 and i<=day: s[i]=j
        labels = list(s.keys())[1:]
        t=list(s.values())
        values = [truncate(abs(j-i),2) for i, j in zip(t[:-1], t[1:])]
        col=list(power.keys())
        row=list(power.values())
        return render_template('index.html', values=values, labels=labels, legend=legend, x=col, y=row, **templateData)

if __name__ == "__main__":
        app.run(host='0.0.0.0', port=90, debug=True)
