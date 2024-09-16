import numpy as np
import pandas as pd
import mysql.connector
from mysql.connector import MySQLConnection, Error
import paho.mqtt.client as mqtt
from time import sleep
import pandas as pd
import json

import pymysql
Host = '192.168.25.227' # IP of device where Database is hosted' #127.0.0.1'
Port = '3306'
User = 'smartPets1'
Password = 'smartPets01!'
Database = 'exampledb'
temp=0
    
cnx = mysql.connector.connect(host = Host, user=User, passwd = Password, database=Database,auth_plugin='mysql_native_password')   
mycursor = cnx.cursor()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    client.subscribe("smartPETS/devices/Food/Out")
    print("subsrcibed")
    # client.subscribe("/esp8266/humidity")

def on_message(client, userdata, msg):
    # print("Received message '" + str(msg.payload) + "' on topic '" + msg.topic)
    
    payloadStr = str(msg.payload)
    DecodedPayload = msg.payload.decode(encoding='utf-8') #'utf-8'))
    print(msg.topic + " " + DecodedPayload)
    table = 'petdaily_activity'
    
    json_dict = json.loads(DecodedPayload,encoding='utf-8')
    placeholders = ', '.join(['%s'] * len(json_dict))
    columns = ', '.join(json_dict.keys())
    data=list(json_dict.values())
     
    sqlcmd = "INSERT INTO %s ( %s ) VALUES ( %s )" % (table, columns, placeholders)
    cursor = cnx.cursor() 
    cursor.execute(sqlcmd,data)
    #print('cursor.execute(sqlcmd)', cursor)  
    
    cnx.commit()
    cursor.close()
    # cnx.close()
    return cnx             


    # mycursor.execute(sql,val)
    # cnx.commit()
    # print(mycursor.rowcount, "details inserted")







def main():
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.loop
    mqtt_client.connect('localhost', 1883, 60) 
    # Connect to the MQTT server and process messages in a background thread. 
    mqtt_client.loop_forever() 


if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    main()
