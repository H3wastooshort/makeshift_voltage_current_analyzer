import sys,struct
if len(sys.argv) != 2:
    quit("parser.py [filename]")

with open(sys.argv[1],'rb') as f:
    while True:
        header = f.read(5)
        (time,n_entries)=struct.unpack('<IB', dat)
        for range(0,n_entries):
            entry = f.read(3)
            (pin,millivolt)=struct.unpack('<BH', dat)
            print("Pin %d has %dmV at %dµS"%(pin,millivolt,time))