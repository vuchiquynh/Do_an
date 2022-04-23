import paho.mqtt.client as mqtt
import time
import mysql.connector
import json

from datetime import datetime


############

mydb = mysql.connector.connect(
    host="localhost",
    user="root",
    password="123456",
    database="iot_smart"
)
mycursor = mydb.cursor()


def on_connect(client, userdata, flags, rc):
    client.subscribe("Iot/Data")


def on_message(client, userdata, message):
    print("message received ", str(message.payload.decode("utf-8")))

    dulieu = str(message.payload.decode("utf-8"))
    data_in = json.loads(dulieu)

    temp = str(data_in["temp"])
    temp_set = str(data_in["temp_set"])
    time_h =  str(data_in["Time_h"])
    time_p =  str(data_in["Time_p"])
    humi = str(data_in["humi"])

    now = datetime.now()
    cur_time = now.strftime("%Y-%m-%d %H:%M:%M")
    
    sql = "INSERT INTO sensor(temp,temp_set,time_h,time_p, humi, updtime) VALUES(%s, %s, %s,%s, %s, %s)"
    val = (temp,temp_set,time_h,time_p, humi, cur_time)

    mycursor.execute(sql, val)
    mydb.commit()


########################################
broker_address = "broker.hivemq.com"

client = mqtt.Client("mangcbthayuy")


print("Subscribing to topic", "Iot/Data")

client.on_connect = on_connect

client.on_message = on_message

client.connect(broker_address, 1883)  # connect to broker

client.loop_forever()
