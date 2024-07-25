import sys,struct
if len(sys.argv) != 2:
    quit("parser.py [filename]")

with open(sys.argv[1],'rb') as f:
    while True:
        header = f.read(5)
        (time,n_readings)=struct.unpack('<IB', header)
        if n_readings>0:
            for i in range(0,n_readings):
                reading = f.read(3)
                (pin,millivolt)=struct.unpack('<BH', reading)
                print("Pin %d has %dmV at %dµS"%(pin,millivolt,time))
        else:
            print("REBOOT")
