voltage_pin=36
current_pin=39

voltage_offset=1683#mV
current_offset=1564#mV
voltage_multi=458/-61#mV/mV
current_multi=134.1/-71#mA/mV

window_size=4000

####

import sys,struct

if len(sys.argv) not in [3,4]:
    quit("parser.py [filename] [mode] [output]")

####

last_time=0
def calc_samplerate(time):
    global last_time
    delta = time - last_time
    if delta == 0:
        return None
    freq = 1E9 / delta
    last_time=time
    return freq

####

to_csv_first=True
def to_csv(outfile, time,pins):
    global to_csv_first
    if to_csv_first:
        to_csv_first=False
        header="timestamp"
        for p in pins.keys():
            header += ",Pin " + str(p)
        outfile.write(header + "\n")
    line = str(time)
    for mv in pins.values():
        line += ',' + str(mv)
    outfile.write(line + "\n")

####

def to_pcm(outfile, time, pins):
    #sr = calc_samplerate(time)
    #pn = len(pins)
    for mv in pins.values():
        outfile.write(struct.pack('H',mv))

outmode = 'w'
if sys.argv[2] in ['pcm']:
    outmode = 'wb'

####

def convert_readings(v_reading,i_reading):
    V_mV=voltage_multi*(v_reading-voltage_offset)
    I_mA=current_multi*(i_reading-current_offset)
    return (V_mV, I_mA)

def get_energy(delta_uS,mV,mA):
    return mV*mA*delta_uS / (1E3*1E3*1E5) #energy in J

array=[]
results=[]
last_uS = 0
def to_parsed(array):
    global last_uS
    min_mV=0xFFFF
    min_mA=0xFFFF
    max_mV=-0xFFFF
    max_mA=-0xFFFF
    E=0
    full_delta=0
    for k in range(0,window_size):
        time=array[k][0]
        pins=array[k][1]
        (mV,mA) = convert_readings(pins[voltage_pin],pins[current_pin])
        delta_uS = time - last_uS
        last_uS=time
        
        E += get_energy(delta_uS,mV,mA)
        full_delta+=delta_uS
        
        max_mV=max(max_mV,mV)
        max_mA=max(max_mA,mA)
        min_mV=min(min_mV,mV)
        min_mA=min(min_mA,mA)
    ts=array[0][0]/1E6
    V_pp=(max_mV-min_mV)/1E3
    I_pp=(max_mA-min_mA)/1E3
    P=E/(full_delta/1E6)
    r=[ts, V_pp, I_pp, P]
    print(r)
    results.append(r)

def read_file(infile,outfile):
    global array
    while True:
        header = infile.read(5)
        time=0
        n_readings=0
        try:
            (time,n_readings)=struct.unpack('<IB', header)
        except struct.error:
            print("reached end of file")
            return
        pins =  {}
        if n_readings>0:
            for i in range(0,n_readings):
                reading = infile.read(3)
                (pin,millivolt)=struct.unpack('<BH', reading)
                pins[pin]=millivolt
                #print("Pin %d has %dmV at %dµS"%(pin,millivolt,time))
            if sys.argv[2]=='csv':
                to_csv(outfile,time,pins)
            elif sys.argv[2]=='pcm':
                to_pcm(outfile,time,pins)
            elif sys.argv[2] in ['parse', 'graph']:
                array.append([time,pins])
                if len(array) >= window_size:
                    to_parsed(array)
                    array=[]
            else:
                quit("unknown mode")
                
        else:
            print("REBOOT")

with open(sys.argv[1],'rb') as infile:
    if sys.argv[2] in ['csv','pcm']:
        with open(sys.argv[3], outmode) as outfile:
            read_file(infile,outmode)
    else:
        read_file(infile,None)



if sys.argv[2]=='graph':
    from matplotlib import pyplot as plt
    fig,ax = plt.subplots()
    ax.set_ylabel("Power [W]")
    ax.set_xlabel("Time [s]")
    ax.plot([row[0] for row in results],[row[3] for row in results])
    plt.show()
