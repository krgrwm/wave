params={'init': 0, 'init_past': 0, 'source': 2, 'obstacle': 0, 'system_size_x': 2, 'system_size_y': 0.08, 
        'time': 4.0, 'v': 1.0, 'boundary': 1, 'energy_flag': 0}

data_dir="data"
set_dir="set"
dx=0.02
dy=0.02
dt=0.01

for i in range(1, 5):
    f_name = "plane_wave_%d" % i
    print(set_dir + "/" + f_name)

    with open(set_dir + "/" + f_name, 'w') as f:
        f.write("data_dir=%s\n" % (data_dir + "/" + f_name))
        for p,v in params.items():
            f.write(p)
            f.write("=")
            f.write(str(v))
            f.write("\n")
        f.write("dx=%f\n" % (dx * 1.0/2**i))
        f.write("dy=%f\n" % (dy * 1.0/2**i))
        f.write("dt=%f\n" % (dt * 1.0/2**i))
