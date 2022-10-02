from importlib.metadata import requires
import requests
import pandas as pd
import schedule
import time
import paho.mqtt.client as mqtt
from keras.models import load_model
from sklearn.preprocessing import MinMaxScaler
import numpy as np

scaler = MinMaxScaler()
Model = load_model('models\speed_predict.h5')
firstTime = True

def sendDataToDevice(active):
    client = mqtt.Client()
    client.connect(broker_address,1883,60)
    if active:
        client.publish("cuton","k")
    else:
        client.publish("cutoff","k")

broker_address = "server.archeindustries.com"
def callParse() :
    response = requests.get('https://services.swpc.noaa.gov/products/solar-wind/mag-1-day.json')
    response1 = requests.get('https://services.swpc.noaa.gov/products/solar-wind/plasma-1-day.json')
    result = response.text;
    result1 = response1.text;

    df = pd.read_json(result1)
    df1 = pd.read_json(result)

    df.to_csv('csvfile.csv',mode='w+', encoding='utf-8', index=False)
    df1.to_csv('csvfile1.csv',mode='w+', encoding='utf-8', index=False)

    df2 = pd.read_csv('csvfile.csv');
    df3 = pd.read_csv('csvfile1.csv');

    parsed_data = pd.merge(df2, df3, on ='0', how ="left")

    parsed_data = parsed_data.rename(columns={'0':'time_tag', '1_x':'density', '2_x':'speed', '3_x':'temperature', '1_y':'bx_gsm', '2_y':'by_gsm', '3_y':'bz_gsm', '4': 'lon_gsm', '5':'lat_gsm', '6':'bt'})

    parsed_data = parsed_data.iloc[1:]

    parsed_data = parsed_data.dropna()

    parsed_data.to_csv("parsed_data_final_.csv", mode='w+', index=False)
    top_row = parsed_data.iloc[0].tolist()
    columnsLength = len(parsed_data.columns)

    for i in range(0,columnsLength):
        if i==0:
            pass
        if i==3:
            top_row.append(int(top_row[i]))
        elif i>0:
            top_row.append(float(top_row[i]))
    top_row = top_row[1:len(top_row)]
    top_row = np.array(top_row)
    top_row = top_row.reshape(-1,1)
    input_ = scaler.fit_transform(top_row)
    output = Model.predict(input_)
    print("output =>" + str(output[0]))
    output = np.array(output[0][0])
    output = output.reshape(-1,1)

    outputtt = scaler.inverse_transform(output)[0][0]
    if outputtt>2000:
        outputtt = outputtt/5
    print(outputtt)
    if outputtt>1000:
        sendDataToDevice(True)
    else:
        sendDataToDevice(False)
    # pass through the NN model
    # check the result
    # if hits the trigger send mqtt disconnect
# schedule.every(6).hours.do(callParse)
schedule.every(10).seconds.do(callParse)

while True:
  if firstTime:
      callParse()
      firstTime = False
  else:
      schedule.run_pending()