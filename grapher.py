import sys, matplotlib

voltage_pin=36
current_pin=39

voltage_offset=1683#mV
current_offset=1564#mV
voltage_multi=4580/-61#mV/mV
current_multi=134.1/-71#mA/mV

window_size=4000

def convert_readings(v_reading,i_reading):
    V_mA=voltage_multi*(v_reading-voltage_offset)
    I_mA=current_multi*(i_reading-current_offset)
    return (V_mV, I_mA)

def get_energy(delta_uS,mV,mA):
    return mV*mA*delta_uS / (1E3*1E3) #energy in J

results=[]

last_uS = 0
def do_graph(array):
    for i in range(0,len(array),window_size)
        min_mV=0xFFFF
        min_mA=0xFFFF
        max_mV=-0xFFFF
        max_mA=-0xFFFF
        E=0
        full_delta=0
        for j in range(i,i+window_size):
            k=i+j
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
        ts=array[i][0]/1E6
        V_pp=(max_mV-min_mV)/1E3
        I_pp=(max_mA-min_mA)/1E3
        P=E/full_delta
        results.append([ts, V_pp, I_pp, P])
