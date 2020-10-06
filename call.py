#!/usr/bin/env python3
from flask import Flask, render_template
import datetime as dt, pickle, tuyapower

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

        data=tuyapower.deviceRaw('id', '10.10.10.10', 'key', '3.3')['dps']
        total=str(truncate(old+data['101']/100,2))+'  kWh'
        volt=str(truncate(data['20']/10,1))+'      V'
        amp=str(data['18'])+'      mA'
        watt=str(truncate(data['19']/10,1))+'     W'
        switch='On' if data['1']==True else 'Off'

        d[day]=truncate(old+data['101']/100,2)
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

        f=open("day.dat","wb")
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
        return render_template('index.html', **templateData)

if __name__ == "__main__":
        app.run(host='0.0.0.0', port=90, debug=True)
