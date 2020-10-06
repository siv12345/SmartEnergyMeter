from flask import Flask, render_template
import datetime
import tuyapower
app = Flask(__name__)

def truncate(n, decimals=0):
        multiplier = 10 ** decimals
        return int(n * multiplier) / multiplier


old=3676.4
data=total=volt=amp=watt=switch=0

@app.route("/")
def hello():
        now = datetime.datetime.now()
        timeString = now.strftime("%Y-%m-%d %H:%M")
        data=tuyapower.deviceRaw('018065202cf43238c5a8', '192.168.0.86', 'ef2d47140c02257b', '3.3')['dps']
        total=str(truncate(old+data['101']/100,2))+' kWh'
        volt=str(truncate(data['20']/10,1))+'  V'
        amp=str(data['18'])+'\t  mA'
        watt=str(truncate(data['19']/10,1))+'  W'
        switch='On' if data['1']==True else 'Off'
        templateData = {
        'volt' : volt,
        'amp' : amp,
        'watt' : watt,
        'state' : switch,
        'total' : total
        }
        return render_template('index.html', **templateData)

if __name__ == "__main__":
        app.run(host='0.0.0.0', port=90, debug=True)
