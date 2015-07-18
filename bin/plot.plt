n = n + 1
#n = 140
file = sprintf(data_dir.'/data_%d.dat', n)

title = sprintf('%d', n)
set title title


#set hidden3d
set view 50, 20, 1.0, 1
set view 90, 90, 1.0, 1
set view 90, 0, 1.0, 1
set zrange[-0.05:0.05]
#splot "< bin/remove_boundary ".file matrix every 1:1 w l
set view equal xy
splot file matrix every every_y:every_x w l
#splot "<./intensity ".file matrix every ::50::50 w l

if(n<end_time) reread;
