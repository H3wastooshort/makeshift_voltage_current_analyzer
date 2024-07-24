import sys,struct
if len(sys.argv) != 2:
    quit("parser.py [filename]")

with open(sys.argv[1],'rb') as f:
    while True:
        dat = f.read(1+4+2)
        (pin,time,voltage)=struct.unpack('<BIH', dat)
        print("Pin %d has %dmV at %dµS"%(pin,voltage,time))