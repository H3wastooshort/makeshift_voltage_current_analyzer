import sys,struct
if len(sys.argv) != 4:
    quit("parser.py [filename] [mode] [output]")

last_time=0
def calc_samplerate(time):
    global last_time
    delta = time - last_time
    if delta == 0:
        return None
    freq = 1E9 / delta
    last_time=time
    return freq

to_csv_first=True
def to_csv(outfile, time,pins):
    global to_csv_first
    if to_csv_first:
        to_csv_first=False
        header="timestamp"
        for p in pins.keys():
            header += ",Pin " + str(p)
        outfile.write(header)
    line = str(time)
    for mv in pins.values():
        line += ',' + str(mv)
    outfile.write(line)


def to_pcm(outfile, time, pins):
    #sr = calc_samplerate(time)
    #pn = len(pins)
    for mv in pins.values():
        outfile.write(struct.pack('H',mv))

outmode = 'w'
if sys.argv[2] in ['pcm']:
    outmode = 'wb'

with open(sys.argv[1],'rb') as f:
    with open(sys.argv[3], outmode) as outfile:
        while True:
            header = f.read(5)
            (time,n_readings)=struct.unpack('<IB', header)
            pins =  {}
            if n_readings>0:
                for i in range(0,n_readings):
                    reading = f.read(3)
                    (pin,millivolt)=struct.unpack('<BH', reading)
                    pins[pin]=millivolt
                    #print("Pin %d has %dmV at %dµS"%(pin,millivolt,time))
                if sys.argv[2]=='csv':
                    to_csv(outfile, time,pins)
                elif sys.argv[2]=='pcm':
                    to_pcm(outfile, time, pins)
            else:
                print("REBOOT")
