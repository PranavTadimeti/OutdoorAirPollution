import urllib
import urllib.request
import json
import time
import csv
import datetime
import matplotlib.pyplot as plot

l_cnt = 0


with open('feeds.csv') as csv_file:
    csv_read = csv.reader(csv_file, delimiter=',')

    temparr = []
    avg = []
    runningavg = []
    d = []

    for i in range(1, 10):
        avg.append(0)
        temparr.append(0)
        runningavg.append([])

    for l in csv_read:

        try:
            date_created = datetime.datetime.strptime(
                l[0], '%Y-%m-%d %H:%M:%S %Z')

            entry_id = int(l[1])
            hum = float(l[2])
            temp = float(l[3])
            p25 = float(l[4])
            p10 = float(l[5])
            co = float(l[6])
            no2 = float(l[7])
            nh3 = float(l[8])
        
        except ValueError:
            continue

        if (hum > 0 and temp > 0 and p25 > 0 and p10 > 0 and co > 0 and no2 > 0 and nh3 > 0):
            

            if l_cnt > 0:
                d.append(date_created)
            
            for j in range(2, 9):
                avg[j] += float(l[j])
                    
                if l_cnt > 0:
                    runningavg[j].append(avg[j]/l_cnt)                        
                    

            l_cnt += 1


for i in range(2, 9):
    avg[i] = avg[i]/(l_cnt)

lbl = ["","","Humidity","Temperature","PM2.5","PM10","CO","NO2","NH3"] 

for i in range(2,9):
    plot.plot(d,runningavg[i],label=lbl[i])


plot.xlabel("X")
plot.ylabel("Y")
plot.legend()
plot.show()

#Calculating Running average from Thingspeak

READ_API_KEY = '8N7EYQT21B0PSUBE'
CHANNEL_ID = '864205'


while True:
    url = urllib.request.urlopen("http://api.thingspeak.com/channels/%s/feeds/last.json?api_key=%s"
                                 % (CHANNEL_ID, READ_API_KEY))

    try:
        response = url.read()
        data = json.loads(response)
    except data < 0:
        continue

    # print(data)

    try:
        a = data['created_at']
        b = float(data['field1'])
        c = float(data['field2'])
        d = float(data['field3'])
        e = float(data['field4'])
        f = float(data['field5'])
        g = float(data['field6'])
        h = float(data['field7'])

    except ValueError:
        continue

    avg[2] = ((avg[2]*l_cnt)+b)/(l_cnt+1)
    avg[3] = ((avg[3]*l_cnt)+c)/(l_cnt+1)
    avg[4] = ((avg[4]*l_cnt)+d)/(l_cnt+1)
    avg[5] = ((avg[5]*l_cnt)+e)/(l_cnt+1)
    avg[6] = ((avg[6]*l_cnt)+f)/(l_cnt+1)
    avg[7] = ((avg[7]*l_cnt)+g)/(l_cnt+1)
    avg[8] = ((avg[8]*l_cnt)+h)/(l_cnt+1)

    print("Humidity: " + str(avg[2]) + "     " + "Temperature: " + str(avg[3]) + "     " + "PM2.5: " + str(avg[4]) + "     "
          + "PM10: " + str(avg[5]) + "     " + "CO: " + str(avg[6]) + "     " + "NO2: " + str(avg[7]) + "     " + "NH3: " + str(avg[8]))

    time.sleep(15)

    url.close()

