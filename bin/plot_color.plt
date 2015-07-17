n = n + 1
file = sprintf(data_dir.'/data_%d.dat', n)

title = sprintf('%d', n)
set title title

set cbrange[-0.4:0.4]
plot file matrix every every_y:every_x with image
set pm3d


if(n<end_time) reread;
