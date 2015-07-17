n = n + 1
file = sprintf(data_dir.'/data_%d.dat', n)

title = sprintf('%d', n)
set title title


#set hidden3d
set view 50, 20, 1.0, 1
#set view 90, 0, 1, 1
set zrange[-4:4]
#splot "< bin/remove_boundary ".file matrix every 1:1 w l
set view equal xy
splot file matrix every every_y:every_x w l

if(n<end_time) reread;
