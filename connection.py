import serial

with serial.Serial('/dev/ttyACM0', 9600, timeout=10) as ser:
    while True:
        weather_report = ser.readline().decode("utf-8").rstrip()
        print (weather_report) #for testing purposes
