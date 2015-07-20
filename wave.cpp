#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <memory>
#include <cstdlib>

using namespace std;

typedef vector<vector<double> > GridVec;
typedef vector<GridVec> tGridVec;

enum E_TIME {
  PAST=0, PRESENT=1, FUTURE=2
};

class Parameter {
  public:
  int vec_size_x;
  int vec_size_y;
  double system_size_x;
  double system_size_y;
  double dx;
  double dy;
  double dt;
  double v;
  double b0; // boundary value

  double t;
  int n_step;

  Parameter(const double _system_size_x, const double _system_size_y, const double _dx, const double _dy, const double _dt, const double _v)
  {
    system_size_x = _system_size_x;
    system_size_y = _system_size_y;
    dx = _dx;
    dy = _dy;
    dt = _dt;
    v = _v;
    b0 = 0.0;
    t = 0.0;
    n_step = 0;
    vec_size_x = system_size_x / dx +1;
    vec_size_y = system_size_y / dy +1;
    cout << vec_size_x << endl;
    cout << vec_size_y << endl;
  }

  void step()
  {
    t += dt;
    n_step++;
  }
};

class Initialize {
  public:
    virtual double run(Parameter &p, int i, int j) = 0;
};

class Source {
  public:
    virtual void run(Parameter &, tGridVec &) = 0;
};

class Obstacle {
  public:
    virtual void run(Parameter &, tGridVec &) = 0;
};

class Boundary {
  public:
    virtual void run(Parameter &, tGridVec &, const E_TIME &T) = 0;
};

class Init_gauss : public Initialize
{
  public:
  double run(Parameter &p, int i, int j)
  {
    double c = p.system_size_x/2;

    // $B7O$N%5%$%:$N(B1/10
    double sigma = (p.system_size_x * p.system_size_x) * p.dx * p.dy;

    return 
      0.0001 * 
      (1./(sqrt(2.0*M_PI) * sigma)) * 
      exp(-(pow(i*p.dx - c, 2) + pow(j*p.dy - c, 2))/(2*pow(sigma, 2)));
  }
};

class Init_gauss_pulse : public Initialize
{
  public:
  double run(Parameter &p, int i, int j)
  {
    double c = p.system_size_x/2;
    double sigma = pow(p.system_size_x * 0.1, 2);

    return 
      0.01 * 
      (1./(sqrt(2*M_PI) * sigma)) * 
      exp(-(pow(i*p.dx - c,2) + pow(j*p.dy - c, 2))/(2*pow(sigma, 2)));
  }
};


class Init_zero : public Initialize
{
  public:
  double run(Parameter &p, int i, int j)
  {
    return 0;
  }
};

class Init_half : public Initialize
{
  public:
    int k1;
    int k2;
    double t;
    double omega;
    double xL;
    double yL;

    Init_half(const Parameter& p, double t)
    {
      k1=2;
      k2=1;
      this->t = t;

      this->xL = p.system_size_x;
      this->yL = p.system_size_y;
      double omega2 = p.v * p.v * M_PI * M_PI * ((k1/xL)*(k1/xL) + (k2/yL)*(k2/yL));
      this->omega = sqrt(omega2);
    }

    double run(Parameter &p, int i, int j)
    {
      double x = i * p.dx;
      double y = j * p.dy;

      return sin(k1*M_PI*x/xL) * sin(k2*M_PI*y/yL) * cos(omega * t);
    }
};

class Init_half_past : public Initialize
{
  public:
    int k1;
    int k2;
    double t;
    double omega;
    double xL;
    double yL;
    Init_half ih;

    Init_half_past(const Parameter& p, double t) : ih(p, 0.0)
    {
      k1=2;
      k2=1;
      this->t = t;

      this->xL = p.system_size_x;
      this->yL = p.system_size_y;
      double omega2 = p.v * p.v * M_PI * M_PI * ((k1/xL)*(k1/xL) + (k2/yL)*(k2/yL));
      this->omega = sqrt(omega2);
    }

    double run(Parameter &p, int i, int j)
    {
      double x = i * p.dx;
      double y = j * p.dy;
      double alpha2 = pow((p.v * p.dt / p.dx), 2);

      return 
        ih.run(p, i, j)
        +
        t * sin(k1*M_PI*x/xL) * sin(k2*M_PI*y/yL) * omega * sin(omega * 0)
        +
        alpha2/2.0 * (ih.run(p,i+1, j) + ih.run(p,i-1, j) - 2*ih.run(p, i, j) )
        +
        alpha2/2.0 * (ih.run(p,i, j+1) + ih.run(p,i, j-1) - 2*ih.run(p, i, j) )
        ;
    }
};


class Source_plane_wave : public Source
{
  public:
  void run(Parameter &p, tGridVec &g)
  {
    double omega = 50;
    for (int j = 0; j < p.vec_size_y; j++) {
      g[PRESENT][j][1] = sin(omega*p.t);
    }
  }
};

class Source_spot : public Source
{
  public:
  void run(Parameter &p, tGridVec &g)
  {
    double omega = 10;
    g[PRESENT][int(p.vec_size_y/2)][int(p.vec_size_x/2)] = sin(omega*p.t);
  }
};


class Source_nothing : public Source
{
  public:
  void run(Parameter &p, tGridVec &g)
  {
  }
};

class Source_rain : public Source
{
  public:
    Source_rain()
    {
      srand(time(0));
    }


    void run(Parameter &p, tGridVec &g)
    {
      int x = rand() % int(p.vec_size_x - 1);
      int y = rand() % int(p.vec_size_y - 1);
      int v = rand() % 10 - 5;
      int test = rand() % 10 - 2;

      if (test < 0) {
        g[PAST][y][x] += v*0.1;
      }
    }
};

class Source_double_spot : public Source
{
  public:
  void run(Parameter &p, tGridVec &g)
  {
    double omega = 50;
    g[PRESENT][int(p.vec_size_y/2)][int(p.vec_size_x/2)-int(p.vec_size_x/3)] = 0.5 * sin(omega*p.t);
    g[PRESENT][int(p.vec_size_y/2)][int(p.vec_size_x/2)+int(p.vec_size_x/3)] = 0.5 * sin(omega*p.t);
  }
};

class Source_ship: public Source
{
  public:
  void run(Parameter &p, tGridVec &g)
  {
    if (p.n_step >= p.vec_size_y) {
      return;
    }
    g[PRESENT][int(p.vec_size_y/2)][p.n_step] = -0.1;
  }
};


class Obstacle_slit : public Obstacle
{
  public:
  void run(Parameter &p, tGridVec &g)
  {
    int slit1 = int(p.vec_size_y/2);
    int d = 5;
    int pos = int(p.vec_size_y/5);
    for (int i = 0; i < slit1; i++) {
      g[PRESENT][i][pos] = 0;
    }
    for (int i = slit1+d; i < p.vec_size_y; i++) {
      g[PRESENT][i][pos] = 0;
    }
  }
};


class Obstacle_double_slit : public Obstacle
{
  public:
  void run(Parameter &p, tGridVec &g)
  {
    int slit = int(p.vec_size_y/2);
    int slit_d = 5;
    int D = 10;
    int pos = int(p.vec_size_y/5);

    for (int i = 0; i < slit-D; i++) {
      g[PRESENT][i][pos] = 0;
    }
    for (int i = slit-D + slit_d; i < slit; i++) {
      g[PRESENT][i][pos] = 0;
    }

    for (int i = slit; i < slit+D - slit_d; i++) {
      g[PRESENT][i][pos] = 0;
    }
    for (int i = slit+D; i < p.vec_size_y; i++) {
      g[PRESENT][i][pos] = 0;
    }
  }
};


class Obstacle_plate : public Obstacle
{
  public:
  void run(Parameter &p, tGridVec &g)
  {
    int center = int(p.vec_size_y/2);
    int pos = int(p.vec_size_y/5);
    for (int i = 0; i < center; i++) {
      g[PRESENT][i][pos] = 0;
    }
  }
};

class Obstacle_nothing : public Obstacle
{
  public:
  void run(Parameter &p, tGridVec &g)
  {
  }
};

class Boundary_fixed : public Boundary
{
  public:
  void run(Parameter &p, tGridVec &g, const E_TIME &T)
  {
    double b0=0.0;

    for (int i = 0; i < p.vec_size_x; i++) {
      g[T][0][i] = b0;
      g[T][p.vec_size_y-1][i] = b0;
    }

    for (int j = 0; j < p.vec_size_y; j++) {
      g[T][j][0] = b0;
      g[T][j][p.vec_size_x-1] = b0;
    }
  }
};

class Boundary_free : public Boundary
{
  public:
  void run(Parameter &p, tGridVec &g, const E_TIME &T)
  {
    for (int i = 0; i < p.vec_size_x; i++) {
      g[T][0][i] = g[T][1][i];
      g[T][p.vec_size_y-1][i] = g[T][p.vec_size_y-2][i];
    }

    for (int j = 0; j < p.vec_size_y; j++) {
      g[T][j][0] = g[T][j][1];
      g[T][j][p.vec_size_x-1] = g[T][j][p.vec_size_x-2];
    }
  }
};

class Functions {
  typedef shared_ptr<Initialize>  p_initialize;
  typedef shared_ptr<Source>  p_source;
  typedef shared_ptr<Obstacle>  p_obstacle;
  typedef shared_ptr<Boundary>  p_boundary;
  public:
    p_initialize init;
    p_initialize init_past;
    p_source     source;
    p_obstacle   obstacle;
    p_boundary   boundary;

    vector<p_initialize> vec_init;
    vector<p_source> vec_source;
    vector<p_obstacle>     vec_obstacle;
    vector<p_boundary>     vec_boundary;

    Functions(){}

    Functions(const Parameter& p, const int ninit, const int ninit_past, 
        const int nsource, const int nobstacle, const int nboundary)
    {
      vec_init = vector<p_initialize>({
          p_initialize(new Init_zero()),
          p_initialize(new Init_gauss()),
          p_initialize(new Init_gauss_pulse()),
          p_initialize(new Init_half(p, 0.0)),
          // Init_half_pastもInit_half(p, -p.dt)も同じ結果になる
          // どちらも正しい
//          p_initialize(new Init_half_past(p, -p.dt)),
          p_initialize(new Init_half(p, -p.dt)),
          });

      vec_source = vector<p_source>({
          p_source(new Source_nothing()),
          p_source(new Source_spot()),
          p_source(new Source_plane_wave()),
          p_source(new Source_rain()),
          p_source(new Source_double_spot()),
          p_source(new Source_ship()),
          });

      vec_obstacle = vector<p_obstacle>({
          p_obstacle(new Obstacle_nothing()),
          p_obstacle(new Obstacle_slit()),
          p_obstacle(new Obstacle_double_slit()),
          p_obstacle(new Obstacle_plate()),
          });

      vec_boundary = vector<p_boundary>({
          p_boundary(new Boundary_fixed()),
          p_boundary(new Boundary_free()),
          });

      init = vec_init[ninit];
      init_past = vec_init[ninit_past];
      source = vec_source[nsource];
      obstacle = vec_obstacle[nobstacle];
      boundary = vec_boundary[nboundary];
    }
};

class Grid {
  public:
  tGridVec grid;
  Parameter p;
  Functions f;
  ofstream energy_file;
  ofstream T_file;;
  ofstream potential_file;


  void init_grid_present()
  {
    for (int j = 0; j < p.vec_size_y; j++) {
      for (int i = 0; i < p.vec_size_x; i++) {
        grid[PRESENT][j][i] = (f.init)->run(p, i, j);
      }
    }
  }

  void init_grid_past()
  {
    for (int j = 0; j < p.vec_size_y; j++) {
      for (int i = 0; i < p.vec_size_x; i++) {
        grid[PAST][j][i] = (f.init_past)->run(p, i, j);
      }
    }
  }



  void set_boundary(const E_TIME &T)
  {
    (f.boundary)->run(p, grid, T);
  }

  void update_grid()
  {
    int P = PRESENT;
    //double alpha2 = pow((p.v * p.dt / p.dx), 2);

    for (int j = 1; j < p.vec_size_y-1; j++) {
      for (int i = 1; i < p.vec_size_x-1; i++) {
        grid[FUTURE][j][i] = 
//          alpha2 * (grid[P][j+1][i] + grid[P][j-1][i] + grid[P][j][i+1] + grid[P][j][i-1])
//          +
//          2*grid[P][j][i] * (1 - 2*alpha2)
//          -
//          grid[PAST][j][i]
//          ;

          2.0*grid[P][j][i] - grid[PAST][j][i] +
          (p.v*p.v) * (p.dt*p.dt) * 
          (
           (grid[P][j+1][i] + grid[P][j-1][i] - 2.0*grid[P][j][i]) / (p.dy*p.dy)
           + (grid[P][j][i+1] + grid[P][j][i-1] - 2.0*grid[P][j][i]) / (p.dx*p.dx)
          );
      }
    }
  }

  double diffv(int i, int j)
  {
    double dv=0;

    dv = grid[PRESENT][j][i] - grid[PAST][j][i] / (p.dt);
    return dv;
  }

  double diffx(int i, int j)
  {
    double ux=0;

    // 境界のポテンシャルの計算に問題がありそう
    if (0 == i) {
      ux = (grid[PRESENT][j][i+1] - grid[PRESENT][j][i]) / p.dx;
    } else if (p.vec_size_x-1 == i) {
      ux = (grid[PRESENT][j][i] - grid[PRESENT][j][i-1]) / p.dx;
    } else {
//      ux = (grid[PRESENT][j][i+1] - grid[PRESENT][j][i-1]) / (2*p.dx);
      ux = (grid[PRESENT][j][i+1] - grid[PRESENT][j][i]) / p.dx;
    }
    return ux;
  }

  double diffy(int i, int j)
  {
    double uy=0;

    if (0 == j) {
      uy = (grid[PRESENT][j+1][i] - grid[PRESENT][j][i]) / p.dy;
    } else if (p.vec_size_y-1 == j) {
      uy = (grid[PRESENT][j][i] - grid[PRESENT][j-1][i]) / p.dy;
    } else {
//      uy = (grid[PRESENT][j+1][i] - grid[PRESENT][j-1][i]) / (2*p.dy);
      uy = (grid[PRESENT][j+1][i] - grid[PRESENT][j][i]) / p.dy;
    }
    return uy;
  }

  public:
  Grid(int ninit, int ninit_past, int nsource, int nobstacle, int nboundary, double _system_size_x, double _system_size_y, double _dx, double _dy, double _dt, 
      double _v, int energy_flag, string &path)
    : p(_system_size_x, _system_size_y, _dx, _dy, _dt, _v)

  {
    grid = tGridVec(3, GridVec(p.vec_size_y, vector<double>(p.vec_size_x)));
    f = Functions(p, ninit, ninit_past, nsource, nobstacle, nboundary);
    init_grid_past();
    init_grid_present();
    set_boundary(PAST);
    set_boundary(PRESENT);

    stringstream ss;
    stringstream sst;
    stringstream ssp;
    ss   << path << "/" << "energy";
    sst   << path << "/" << "T";
    ssp   << path << "/" << "potential";
    energy_file = ofstream(ss.str().c_str(), ios::trunc);
    T_file = ofstream(sst.str().c_str(), ios::trunc);
    potential_file = ofstream(ssp.str().c_str(), ios::trunc);
  }

  void step()
  {
    (f.source)->run(p, grid);
    (f.obstacle)->run(p, grid);
    set_boundary(PRESENT);
    set_boundary(PAST);
    update_grid();
    set_boundary(FUTURE);

    // shift
    grid[PAST] = grid[PRESENT];
    grid[PRESENT] = grid[FUTURE];

    p.step();
  }


  void print_grid()
  {
    for (int j = 0; j < p.vec_size_y; j++) {
      for (int i = 0; i < p.vec_size_x; i++) {
        cout << grid[PRESENT][j][i] << " ";
      }
      cout << endl;
    }
  }


  void write_data(string &path) {
    stringstream ss;

    double v2  = 0;
    double ux2 = 0;
    double uy2 = 0;
    double energy = 0;
    double T=0;
    double potential = 0;


    string filename = "data";
    string suffix = ".dat";
    string sep = "_";

    ss   << path << "/" << filename << sep << p.n_step << suffix;

    ofstream ofs(ss.str().c_str(), ios::trunc);

    for (int j = 0; j < p.vec_size_y; j++) {
      for (int i = 0; i < p.vec_size_x; i++) {
        ofs << grid[PRESENT][j][i] << " ";
        v2 =  pow(diffv(i, j), 2);
        ux2 = pow(diffx(i, j), 2);
        uy2 = pow(diffy(i, j), 2);
        //energy += v2 + p.v*p.v * (ux2 + uy2);
        T += v2 * p.dx * p.dy;
        potential += (ux2 + uy2) * p.v * p.v * p.dx*p.dy;
        energy += T + potential;
      }
      ofs << endl;
    }
    this->T_file << T << endl;
    this->potential_file << potential << endl;
    this->energy_file << energy << endl;
  }

  void write_only_data(string &path) {
    stringstream ss;

    string filename = "data";
    string suffix = ".dat";
    string sep = "_";

    ss   << path << "/" << filename << sep << p.n_step << suffix;

    ofstream ofs(ss.str().c_str(), ios::trunc);

    for (int j = 0; j < p.vec_size_y; j++) {
      for (int i = 0; i < p.vec_size_x; i++) {
        ofs << grid[PRESENT][j][i] << " ";
      }
      ofs << endl;
    }
  }

  void write_param(string &path) 
  {
    stringstream ss;
    string filename = "data";
    string suffix = ".param";
    ss << path << "/" << filename << suffix;
    ofstream ofs(ss.str().c_str(), ios::trunc);

    ofs << "_system_size_x=" << p.system_size_x << endl;
    ofs << "_system_size_y=" << p.system_size_y << endl;
    ofs << "_t="    << p.t    << endl;
    ofs << "_dx="   << p.dx   << endl;
    ofs << "_dy="   << p.dy   << endl;
    ofs << "_dt="   << p.dt   << endl;
    ofs << "_v="    << p.v    << endl;
    ofs << "_vec_size_x=" << p.vec_size_x << endl;
    ofs << "_vec_size_y=" << p.vec_size_y << endl;
    ofs << "_b0="   << p.b0   << endl;
    ofs << "_step="    << p.n_step << endl;
  }
};

// 0: past, 1: present, 2: future





int main(int argc, char const* argv[])
{
  string path;
  int init, init_past, source, obstacle, boundary;
  double system_size_x;
  double system_size_y;
  double time;

  double dx;
  double dy;
  double dt;
  double v;
  int energy_flag;


  if (argc < 15) {
    cout << "path " << "init " << "init_past " << "source " << "obstacle "
      << "system_size_x " << "system_size_y " << "time"
      << "dx" << "dy" << "dt" << "v" << "boundary" << "energy_flag" << "\n";
    exit(1);
  }

  path = string(argv[1]);
  init = atoi(argv[2]);
  init_past = atoi(argv[3]);
  source = atoi(argv[4]);
  obstacle = atoi(argv[5]);
  system_size_x = atof(argv[6]);
  system_size_y = atof(argv[7]);
  time = atof(argv[8]);
  dx = atof(argv[9]);
  dy = atof(argv[10]);
  dt = atof(argv[11]);
  v = atof(argv[12]);
  boundary = atoi(argv[13]);
  energy_flag = atoi(argv[14]);

  int step = time / dt;


  Grid grid = Grid(init, init_past, source, obstacle, boundary, system_size_x, system_size_y, dx, dy, dt, v, energy_flag, path);

  cout << "size: " << grid.p.system_size_x << "x" << grid.p.system_size_y << endl;
  cout << "grid: " << grid.p.vec_size_x << "x" << grid.p.vec_size_y << endl;
  cout << "step: " << step << endl;


  cout << "dt, dx, dy: " << grid.p.dt << ", " << grid.p.dx << ", " << grid.p.dy << endl;


  // energy_flagを誤差のみ出力するかの判定に使う
  // 面倒だから
  // half.set用

  double xL = grid.p.system_size_x;
  double yL = grid.p.system_size_y;
  double x = xL/4;
  double y = yL/2;
  int vec_x = x / grid.p.dx;
  int vec_y = y / grid.p.dy;

  if (!energy_flag) {
    grid.write_only_data(path);
  } else {
  }

  for (int i = 0; i < step; i++) {
    grid.step();

    if (! energy_flag) {
      grid.write_only_data(path);
    }

    if (!(i % 100)) {
      cout << i << endl;
    }
  }

  cout << grid.p.t << endl;
  cout << grid.p.n_step << endl;
  cout << "x, y = " << x << ", " << y << endl;
  cout << "vx, vy = " << vec_x << ", " << vec_y << endl;
  cout << grid.grid[PRESENT][vec_y][vec_x] << endl;

  int k1 = 2;
  int k2 = 1;
  double t = grid.p.n_step * grid.p.dt;
  double omega2 = grid.p.v * grid.p.v * M_PI * M_PI * ((k1/xL)*(k1/xL) + (k2/yL)*(k2/yL));
  double omega = sqrt(omega2);
  double result = 
    sin(k1*M_PI*(vec_x*dx)/xL) * sin(k2*M_PI*(vec_y*dy)/yL) 
    * cos(omega * t);
  cout << "x:" << vec_x*dx << endl;
  cout << "exact: " << result << endl;
  cout << "error:" << endl;
  cout << 
    grid.p.dx << " " <<
    abs(grid.grid[PRESENT][vec_y][vec_x] - result) << endl;

  grid.write_param(path);

  return 0;
}
