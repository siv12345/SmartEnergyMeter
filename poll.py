import tuyapower
data=tuyapower.deviceRaw('id', '10.10.10.10', 'key', '3.3')['dps']
total='101'
volt='20'
amp='19'
watt='18'
switch='1'

old=3676.4

print('Smart Energy Meter Readings')
print('Total Units:\t',old+data[total]/100,'kWh')
print('Voltage:\t',data[volt]/10,'\tV')
print('Current:\t',data[amp],'\tA')
print('Power:\t\t',data[watt],'\tW')
print('State:\t\t','On' if data[switch]==True else 'Off')
