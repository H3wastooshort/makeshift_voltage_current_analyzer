voltage_pin=36
current_pin=39

voltage_offset=1683#mV
current_offset=1564#mV
voltage_multi=458/-61#mV/mV
current_multi=134.1/-71#mA/mV

window_size=4000

upper_thresh=200 #real mV
lower_thresh=-200 #real mV

####

import sys,struct,math

if len(sys.argv) not in [3,4]:
    quit("parser.py [filename] [mode] [output]")

parser_mode = sys.argv[2]

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
if parser_mode in ['pcm']:
    outmode = 'wb'

####

def convert_readings(v_reading,i_reading):
    V_mV=voltage_multi*(v_reading-voltage_offset)
    I_mA=current_multi*(i_reading-current_offset)
    return (V_mV, I_mA)

def get_energy(delta_uS,mV,mA):
    return mV*mA*delta_uS / (1E3*1E3*1E5) #energy in J

array=[]
results={'ts':[],'f':[],'P':[],'V_min':[],'V_max':[],'I_min':[],'I_max':[]}
last_uS = 0
def to_parsed(array):
    global last_uS
    min_mV=0xFFFF
    min_mA=0xFFFF
    max_mV=-0xFFFF
    max_mA=-0xFFFF
    E=0
    full_delta=0
    zero_crossings=0
    last_reading_above=False
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
        
        if last_reading_above:
            if mV < lower_thresh:
                zero_crossings+=1
                last_reading_above=False
        else:
            if mV > upper_thresh:
                zero_crossings+=1
                last_reading_above=True
    time_passed=full_delta/1E6
    ts=array[0][0]/1E6
    V_pp=(max_mV-min_mV)/1E3
    I_pp=(max_mA-min_mA)/1E3
    P=E/time_passed
    f=(zero_crossings/2)/time_passed
    
    rv={}
    rv['ts']=ts
    rv['f']=f
    rv['P']=P
    rv['V_min']=min_mV/1E3
    rv['V_max']=max_mV/1E3
    rv['I_min']=min_mA/1E3
    rv['I_max']=max_mA/1E3
    return rv

####



R_load=0
R_dyno=0
if parser_mode == 'inductivity':
    R_dyno = float(input("DC Resistance of Dyno [Ohm]: "))
    R_load = float(input("DCR of load [Ohm]: "))
R_total=R_load+R_dyno

# X_dyno
# ^
# |\
# | \
# |  \
# |   \ Z
# |    \
# |     \
# |      \
#  -------> R_dyno + R_load

def calc_inductivity(V_rms, I_rms, f, R):
    Z = V_rms / I_rms
    X_dyno = math.sqrt(pow(Z,2)-pow(R,2))
    L = X_dyno / ( 2*math.pi * f )
    return (L, X_dyno, Z)

####

def config_axis(ax,offset):
    if offset != 0:
        ax.spines["right"].set_position(("axes", offset))
    ax.set_frame_on(True)
    ax.patch.set_visible(False)
    for sp in ax.spines.values():
        sp.set_visible(False)
    ax.spines["right"].set_visible(True)

def graph_data():
    from matplotlib import pyplot as plt
    fig,(ax01,ax11) = plt.subplots(2)
    fig.subplots_adjust(right=0.75)
  
    ax02=ax01.twinx()
    config_axis(ax02,0)
    ax03=ax01.twinx()
    config_axis(ax03,1.2)
    ax04=ax01.twinx()
    config_axis(ax04,2.4)
    ax12=ax11.twinx()
    config_axis(ax12,0)
    ax13=ax11.twinx()
    config_axis(ax13,1.2)

    ax01.set_xlabel("Time [s]")
    ax01.set_ylabel("Power [W]")
    ax02.set_ylabel("Peak-to-Peak Voltage [V]")
    ax03.set_ylabel("Peak-to-Peak Current [A]")
    ax01.plot(results['ts'],results['P'],color='#0F0F0F',zorder=0)
    ax04.plot(results['ts'],results['f'],color='green',zorder=-1)
    ax02.plot(results['ts'],results['V_min'],color='blue',zorder=-2)
    ax02.plot(results['ts'],results['V_max'],color='blue',zorder=-3)
    ax03.plot(results['ts'],results['I_min'],color='red',zorder=-4)
    ax03.plot(results['ts'],results['I_max'],color='red',zorder=-5)
    lines = [ax01,ax02,ax03,ax04]
    ax01.legend(lines, [l.get_label() for l in lines])
    
    ax11.set_xlabel("Frequency [Hz]")
    ax11.set_ylabel("Power [W]")
    ax12.set_ylabel("Peak-to-Peak Voltage [V]")
    ax13.set_ylabel("Peak-to-Peak Current [A]")
    ax11.scatter(results['f'],results['P'],color='#0F0F0F',zorder=0)
    ax12.scatter(results['f'],results['V_min'],color='blue',zorder=-1)
    ax12.scatter(results['f'],results['V_max'],color='blue',zorder=-2)
    ax13.scatter(results['f'],results['I_min'],color='red',zorder=-3)
    ax13.scatter(results['f'],results['I_max'],color='red',zorder=-4)
    lines = [ax11,ax12,ax13]
    ax11.legend(lines, [l.get_label() for l in lines])
    
    plt.show()

def find_offset(arr):
    v_sum=0
    i_sum=0
    n=0
    for k in arr:
        v_sum += k[1][voltage_pin]
        i_sum += k[1][current_pin]
        n += 1
    cal = (v_sum/n,i_sum/n)
    print("V offset = %.1f  I offset = %.1f" % cal)
    return cal

def read_file(infile,outfile):
    print("reading file")
    global array, last_uS, current_offset, voltage_offset
    first_chunk=True
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
            if parser_mode=='csv':
                to_csv(outfile,time,pins)
            elif parser_mode=='pcm':
                to_pcm(outfile,time,pins)
            elif parser_mode in ['calc_csv', 'graph', 'inductivity']:
                array.append([time,pins])
                if len(array) >= window_size:
                    if first_chunk: #assume first second has 0V and 0A
                        first_chunk=False
                        (voltage_offset,current_offset)=find_offset(array)
                    parsed_chunk = to_parsed(array)
                    if parser_mode == 'graph':
                            results['ts'].append([parsed_chunk['ts']])
                            results['f'].append([parsed_chunk['f']])
                            results['P'].append([parsed_chunk['P']])
                            results['V_min'].append([parsed_chunk['V_min']])
                            results['V_max'].append([parsed_chunk['V_max']])
                            results['I_min'].append([parsed_chunk['I_min']])
                            results['I_max'].append([parsed_chunk['I_max']])
                    elif parser_mode == 'inductivity':
                        # assuming perfect symetrical sinewave
                        V_rms = (parsed_chunk['V_max'] - parsed_chunk['V_min']) * math.sqrt(2)
                        I_rms = (parsed_chunk['I_max'] - parsed_chunk['I_min']) * math.sqrt(2)
                        print("V=%05.1f I=%05.3f"%(V_rms, I_rms))
                        try:
                            (L, X_dyno, Z) = calc_inductivity(V_rms, I_rms, parsed_chunk['f'], R_total)
                            print("L=%EH X=%EOhm Z=%EOhm"%(L, X_dyno, Z))
                        except ValueError:
                            print("invalid (VE)")
                        except ZeroDivisionError:
                            print("invalid (/0)")
                    elif parser_mode == 'calc_csv':
                        pass
                    array=[]
            else:
                quit("unknown mode")
                
        else:
            print("REBOOT")
            if parser_mode=='graph':
                graph_data()
                array=[]
                last_uS=0
                for k in results.keys():
                    results[k]=[]
                print("Reading next section...")

with open(sys.argv[1],'rb') as infile:
    if parser_mode in ['csv','calc_csv','pcm']:
        with open(sys.argv[3], outmode) as outfile:
            read_file(infile,outfile)
    else:
        read_file(infile,None)

if parser_mode=='graph':
    graph_data()
