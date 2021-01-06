#!/usr/bin/python
# -*- coding: utf-8 -*-

from flask import Flask, render_template
import datetime as dt
import pickle
import tuyapower
import platform  # For getting the operating system name
import subprocess  # For executing a shell command
import copy


def ping(host):
    param = ('-n' if platform.system().lower() == 'windows' else '-c')
    command = ['ping', param, '2', host]
    return subprocess.call(command) == 0


app = Flask(__name__)


def truncate(n, decimals=0):
    multiplier = 10 ** decimals
    return int(n * multiplier) / multiplier


def leapYear(year):
    if year % 4 == 0:
        if year % 100 == 0:
            if year % 400 == 0:
                return True
            else:
                return False
        else:
            return True
    else:
        return False


def integrate(y_vals, h):
    if len(y_vals) == 0:
        return 0
    i = 1
    total = y_vals[0] + y_vals[-1]
    for y in y_vals[1:-1]:
        if i % 2 == 0:
            total += 2 * y
        else:
            total += 4 * y
        i += 1
    return total * (h / 3.0)


def smooth(array):
    y = copy.deepcopy(array)
    if len(y) == 0:
        return []
    elif len(y) <= 3 or max(y) == 0:
        return y
    t = [round(j - i) for (i, j) in zip(y[:-1], y[1:])]
    t = [0] + t

    d = dict()
    flag = 0
    last = 0
    for i in range(len(t)):
        if y[i] == max(y):
            last = i
            break
        if i == 0 or i == len(t) - 1:
            continue
        elif t[i] < 0 and flag == 0:
            y[i] = round(y[i - 1])
            t = [round(j - i) for (i, j) in zip(y[:-1], y[1:])]
            t = [0] + t

    rest = y[len(y) - 1:last - 1:-1]
    print (len(y), len(y[:last]), len(rest))
    t = [round(j - i) for (i, j) in zip(rest[:-1], rest[1:])]
    t = [0] + t
    for i in range(len(t)):
        if rest[i] == max(rest):
            break
        if i == 0 or i == len(t) - 1:
            continue
        elif t[i] < 0 and flag == 0:
            rest[i] = round(rest[i - 1])
            t = [round(j - i) for (i, j) in zip(rest[:-1], rest[1:])]
            t = [0] + t

    return y[:last] + rest[::-1]


old = 3676.4
data = total = volt = today = amp = watt = switch = 0


@app.route('/')
def meter():
    day = int(dt.datetime.now().strftime('%d'))
    month = int(dt.datetime.now().strftime('%m'))
    year = int(dt.datetime.now().strftime('%y'))

    f = open('/home/pi/pythonTuya/day.dat', 'rb')
    d = pickle.load(f)
    f.close()

    f = open('/home/pi/pythonTuya/current.dat', 'rb')
    power = pickle.load(f)
    f.close()

    col = list(power.keys())
    row = list(power.values())

    ideal = smooth(row)

    powerloss = str(truncate(abs(integrate(ideal, 5) - integrate(row,
                    5)) * 0.017 / 1000, 2))

    s = dict()
    for (i, j) in list(d.items()):
        if i >= 0 and i <= day:
            s[i] = j
    labels = list(s.keys())[1:]
    t = list(s.values())
    values = [truncate(abs(j - i), 2) for (i, j) in zip(t[:-1], t[1:])]

    if ping('192.168.0.86') == False:
        templateData = {
            'volt': '-',
            'amp': '-',
            'watt': '-',
            'state': 'Offline - GridDown / Unreachable',
            'total': str(d[day]) + '  kWh',
            'today': str(truncate(d[day] - d[day - 1], 2)) + '  kWh',
            }
        legend = 'Daily Units'
        return render_template(
            'index.html',
            loss=powerloss + ' kWh',
            ideal=ideal,
            values=values,
            labels=labels,
            legend=legend,
            x=col,
            y=row,
            **templateData
            )

    data = tuyapower.deviceRaw('018065202cf43238c5a8', '192.168.0.86',
                               'ef2d47140c02257b', '3.3')['dps']

    total = str(truncate(old + data['101'] / 100, 2)) + '  kWh'
    volt = str(truncate(data['20'] / 10, 1)) + '      V'
    amp = str(data['18']) + '      mA'
    watt = str(truncate(data['19'] / 10, 1)) + '     W'
    switch = ('Online' if data['18']
              != 0 else 'Offline - No Power / Sundown')

    d[day] = truncate(old + data['101'] / 100, 2)

#       if ((month==2 and day==29) and dt.datetime.now().strftime("%H:%M")[:-1]=="23:5"):
#               d[0]=d[29]
#       elif ((month==2 and day==28) and dt.datetime.now().strftime("%H:%M")[:-1]=="23:5"):
#               d[0]=d[28]
#       elif ((month in [1,3,5,7,8,10,12] and day==31) and dt.datetime.now().strftime("%H:%M")[:-1]=="23:5"):
#               d[0]=d[31]
#       elif ((month not in [1,3,5,7,8,10,12]) and day==30 and dt.datetime.now().strftime("%H:%M")[:-1]=="23:5"):
#               d[0]=d[30]

    if month == 3 and day == 1 and leapYear(year) == True:
        today = str(truncate(old + data['101'] / 100 - d[29], 2)) \
            + '  kWh'
        d[0] = d[29]

    elif month == 3 and day == 1 and leapYear(year) == False:
        today = str(truncate(old + data['101'] / 100 - d[28], 2)) \
            + '  kWh'
        d[0] = d[28]

    elif month==2 and day==1:
        today = str(truncate(old + data['101'] / 100 - d[31], 2)) \
            + '  kWh'
        d[0] = d[31]

    elif month in [5,7,10,12] and day == 1:
        today = str(truncate(old + data['101'] / 100 - d[30], 2)) \
            + '  kWh'
        d[0] = d[30]

    elif month in [1,4,6,8,9,11] and day == 1:
        today = str(truncate(old + data['101'] / 100 - d[31], 2)) \
        + '  kWh'
        d[0] = d[31]
    else:
        today = str(truncate(old + data['101'] / 100 - d[day - 1], 2)) \
            + '  kWh'

    f = open('/home/pi/pythonTuya/day.dat', 'wb')
    pickle.dump(d, f)
    f.close()

    templateData = {
        'volt': volt,
        'amp': amp,
        'watt': watt,
        'state': switch,
        'total': total,
        'today': today,
        }

    legend = 'Daily Units'
    s = dict()
    for (i, j) in list(d.items()):
        if i >= 0 and i <= day:
            s[i] = j
    labels = list(s.keys())[1:]
    t = list(s.values())
    values = [truncate(abs(j - i), 2) for (i, j) in zip(t[:-1], t[1:])]
    return render_template(
        'index.html',
        loss=powerloss + ' kWh',
        ideal=ideal,
        values=values,
        labels=labels,
        legend=legend,
        x=col,
        y=row,
        **templateData
        )

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=90, debug=True)
