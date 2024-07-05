import pymongo
from pymongo import MongoClient
import serial.tools.list_ports
from datetime import datetime

ports = serial.tools.list_ports.comports()
serialInst = serial.Serial()

portsList = []

for onePort in ports:
    portsList.append(str(onePort))
    print(str(onePort))

val = input("Izaberi Port: COM")

for x in range(0,len(portsList)):
    if portsList[x].startswith("COM" + str(val)):
        portVar = "COM" + str(val)
        print(portVar)

serialInst.baudrate = 9600
serialInst.port = portVar
serialInst.open()

while True:
    if serialInst.in_waiting:
        packet = serialInst.readline().decode('utf').rstrip('\n')
        unuTemp, unuVlaz, vanTemp, vanVlaz, pritisak, lux = map(float, packet.split(','))
        vrijemedatum = datetime.now()
        datum = vrijemedatum.strftime("%d-%m-%Y")
        vrijeme = vrijemedatum.strftime("%H:%M:%S")
        cluster = MongoClient("mongodb+srv://diplomski:banjaluka99@cluster0.3rm8jep.mongodb.net/?retryWrites=true&w=majority")
        db = cluster["stanica"]
        collection = db["mjerenja"]
        post = {"Datum": datum, "Vrijeme": vrijeme, "Unutrasnja temperatura": unuTemp, "Unutrasnja vlaznost": unuVlaz, "Vanjska temperatura": vanTemp, "Vanjska vlaznost": vanVlaz, "Pritisak": pritisak, "Nivo osvjetljenosti": lux}
        collection.insert_one(post)