import paho.mqtt.client as mqtt
import time
import mysql.connector
import json

from datetime import datetime


############

mydb = mysql.connector.connect( 
    host = "localhost",
    user = "root",
    password = "123456",
    database = "mcb"
)
mycursor = mydb.cursor()

def on_connect(client, userdata, flags, rc):
    client.subscribe("quynhvc/data") 


def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    
    dulieu = str(message.payload.decode("utf-8"))
    data_in = json.loads(dulieu)

    tem = str(data_in["tem"])
    humi = str(data_in["humi"])

    now = datetime.now()
    cur_time = now.strftime("%Y-%m-%d %H:%M:%M")
    
    # sql = "INSERT INTO csdl(Temp, Time, Temp_setup, Humi, updtime) VALUES(%s, %s, %s, %s, %s)"
    # val = (Temp, Time, Temp_setup, Humi, cur_time)

    sql = "INSERT INTO test(tem, humi, updtime) VALUES(%s, %s, %s)"
    val = (tem, humi, cur_time)

    mycursor.execute(sql, val)
    mydb.commit()
    
    
########################################
broker_address="broker.hivemq.com"
#broker_address="iot.eclipse.org"

client = mqtt.Client("mangcbthayuy") 


print("Subscribing to topic","quynhvc/data")

client.on_connect = on_connect

client.on_message=on_message
    
client.connect(broker_address, 1883) #connect to broker

client.loop_forever()


    







