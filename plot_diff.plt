set samples 10000
set xrange[0:xr]
print file
plot -a*sin(k*((x+1)*dx - v*(time*dt))), "<./plot_diff_data ".file matrix w l
