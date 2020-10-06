import tuyapower
#tuyapower.deviceScan(True)

#PLUGID = '01234567891234567890'
#PLUGIP = '10.0.1.99'
#PLUGKEY = '0123456789abcdef'
#PLUGVERS = '3.1'
def truncate(n, decimals=0):
    multiplier = 10 ** decimals
    return int(n * multiplier) / multiplier

data=tuyapower.deviceRaw('id', '10.10.10.10', 'key', '3.3')['dps']
total='101'
volt='20'
amp='18'
watt='19'
switch='1'

old=3676.4

print('Smart Energy Meter Readings')
print('Total Units:\t',truncate(old+data[total]/100,2),' kWh')
print('Voltage:\t',truncate(data[volt]/10,1),'\t  V')
print('Current:\t',data[amp],'\t  mA')
print('Power:\t\t',truncate(data[watt]/10,1),'  W')
print('State:\t\t','On' if data[switch]==True else 'Off')

#deviceJSON=tuyapower.deviceJSON('id', '10.10.10.10', 'key', '3.3')
#(on, w, mA, V, err)=tuyapower.deviceInfo('id', '10.10.10.10', 'key', '3.3')
#tuyapower.devicePrint('id', '10.10.10.10', 'key', '3.3')
