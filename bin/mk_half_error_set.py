params={'init': 3, 'init_past': 4, 'source': 0, 'obstacle': 0, 
        'system_size_x': 1.0, 'system_size_y': 1.0, 
        'time': 1.0, 'v': 1.0, 'boundary': 0, 'energy_flag': 1}

data_dir="data"
set_dir="set"
dx=0.001
dy=0.001
dt=0.0005
times=2
at = params['system_size_x'] / 2

for i in range(0, 10):
    f_name = "half_%02d" % i
    print(set_dir + "/" + f_name)
    print("dx=%f" % (dx * times**i), )
    print("dt=%f" % (dt * times**i), )
    print("xn=%f" % (params["system_size_x"] / (dx * times**i)))
    print(" n=%f" % (params["time"] / (dt * times**i)))
    print("vec_x=%f" % (at/(dx*times**i)))

    with open(set_dir + "/" + f_name, 'w') as f:
        f.write("data_dir=%s\n" % (data_dir + "/" + f_name))
        for p,v in params.items():
            f.write(p)
            f.write("=")
            f.write(str(v))
            f.write("\n")
        f.write("dx=%f\n" % (dx * times**i))
        f.write("dy=%f\n" % (dy * times**i))
        f.write("dt=%f\n" % (dt * times**i))
