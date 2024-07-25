import sys,struct
if len(sys.argv) != 2:
    quit("parser.py [filename]")

with open(sys.argv[1],'rb') as f:
    while True:
        dat = f.read(6)
        (voltage,pin,flags,time)=struct.unpack('<HBBH', dat)
        print("Pin %d has %dmV at %dµS"%(pin,voltage,time))