# Instrusctions for using the code

To run the flask server, use the following command in the folder dashboard:
```bash
python3 server.py 
```
Then execute the following command in the folder dashboard/app:
```bash
npm run start
```
Before running the esp32c3_sender code update the code with the following parameters:
```bash
    idf.py menuconfig
```
Then change the value of the channel to the one in your wifi with the following command:
```bash
    sudo iwlist wlpXs0 channel
```

For the esp32c3_sender code, you need to set the correct Wi-Fi credentials and the channel number. 

```cpp
    //line 425 change this into your ip address
    // .url = "http://192.168.128.169:3000/update",
    // line 552 the same
    // .url = "http://192.168.128.169:3000/update",
    // Also line 69 to change the buzzer GPIO
```