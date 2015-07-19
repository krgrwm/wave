X = 1.0
Y = 1.0
k1 = 2
k2 = 1
omega2 = 0.3**2 * pi**2 * ((k1/X)**2 + (k2/Y)**2)
omega = sqrt(omega2)
dx = 0.004000
dy = 0.004000
dt = 0.004000
w(x, y) = sin(k1*pi*(x*dx)/X) * sin(k2*pi*(y*dy)/Y) * cos(omega * t)

set isosamples 40
