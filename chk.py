#!/usr/bin/env python3
import urllib.request as url, datetime as dt, time

time.sleep(30)
while True:
        if int(dt.datetime.now().strftime("%H")) in [3,9,15,18,22]:
                try:
                        url.urlopen("http://133.in:90").getheader('Content-type')
                except:
                        continue
