#include <vtkCubeSource.h>
#include <vtkGlyph3D.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCallbackCommand.h>
#include <vtkTubeFilter.h>
#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <list>
#include <cassert>
#include <map>
#include <fstream>
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

#include "helper.h"

struct Puzzle {
  struct Cube {
    Cube(int x,int y,int z,const string &color) : x(x), y(y), z(z), color(color) {}
    int x,y,z;
    string color;
  };
  typedef std::list<Cube> Cubes;
  Cubes cubes;

  struct Color {
    Color(unsigned char r=0,unsigned char g=0,unsigned char b=0) : r(r), g(g), b(b) {}
    unsigned char r;
    unsigned char g;
    unsigned char b;
  };
  typedef std::map<string,Color> Colors;
  Colors colors;

  string filename;

  Puzzle() {
    reset();
  }

  void reset() {
    cubes.clear();
    colors.clear();
    addColor(255,255,255,"white");
    addColor(255,0,0,"red");
    addColor(0,255,0,"green");
    addColor(0,0,255,"blue");
    filename = "dessin.pipi";
  }

  void save() const {
    cout << "saving to " << filename << endl;
    ofstream handle(filename.c_str());
    dump(handle);
    assert(handle.good());
    handle.close();
  }

  void load(const string &lfilename) {
    cout << "loading " << lfilename << endl;
    ifstream handle(lfilename.c_str());
    if (!handle.good()) {
      std::cerr << "error while loading " << lfilename << endl;
      return;
    }

    string section;
    Colors lcolors;
    Cubes lcubes;
    while (!handle.eof()) {
      char line[256];
      handle.getline(line,256);

      if (string(line).empty()) continue;

      { // section start
	Matches matches = match_regex("^\\*+ +([A-Z]+) +\\*+$",line);
	if (!matches.empty()) {
	  section = matches[1];
	  //cout << "entering section " << section << endl;
	  continue;
	}
      }

      { // number of element in section
	Matches matches = match_regex("^([0-9]+) +([a-z]+)$",line);
	if (!matches.empty()) {
	  int nfound = atoi(matches[1].c_str());
	  string name = matches[2];
	  //cout << "found " << nfound << " " << name << endl;
	  continue;
	}
      }

      if (section=="COLORS") { // color element
	Matches matches = match_regex("^([0-9]+) +([0-9]+) +([0-9]+) +([a-z]+)$",line);
	if (!matches.empty()) {
	  int red = atoi(matches[1].c_str());
	  int green = atoi(matches[2].c_str());
	  int blue = atoi(matches[3].c_str());
	  string name = matches[4];
	  //cout << "color " << name << endl;
	  lcolors[name] = Color(red,green,blue);
	  continue;
	}
      }

      if (section=="CUBES") { // cube element
	Matches matches = match_regex("^(-?[0-9]+) +(-?[0-9]+) +(-?[0-9]+) +([a-z]+)$",line);
	if (!matches.empty()) {
	  int x = atoi(matches[1].c_str());
	  int y = atoi(matches[2].c_str());
	  int z = atoi(matches[3].c_str());
	  string color = matches[4];
	  //cout << "cube " << x << "," << y << "," << z << endl;
	  lcubes.push_back(Cube(x,y,z,color));
	  continue;
	}
      }

      std::cerr << "syntax error in " << lfilename << " (" << line << ")" << endl;
      return;
    } 
    handle.close();

    filename = lfilename;
    cubes = lcubes;
    colors = lcolors;
  }


  void dump(std::ostream &os) const {
    os << "*** COLORS ***" << endl;
    os << colors.size() << " colors" << endl;
    for (Colors::const_iterator iter=colors.begin(); iter!=colors.end(); iter++) {
      os << static_cast<int>(iter->second.r) << " " << static_cast<int>(iter->second.g) << " " << static_cast<int>(iter->second.b) << " " << iter->first << endl;
    }

    os << "*** CUBES ***" << endl;
    os << cubes.size() << " cubes" << endl;
    for (Cubes::const_iterator iter=cubes.begin(); iter!=cubes.end(); iter++) {
      os << iter->x << " " << iter->y << " " << iter->z << " " << iter->color << endl;
    }
  }

  vtkPolyData *buildPolyData() const {
    vtkPoints *points = vtkPoints::New();
    vtkUnsignedCharArray *point_colors = vtkUnsignedCharArray::New();
    point_colors->SetNumberOfComponents(3);

    for (Cubes::const_iterator iter=cubes.begin(); iter!=cubes.end(); iter++) {
      points->InsertNextPoint(iter->x,iter->y,iter->z);

      Colors::const_iterator fcolor = colors.find(iter->color);
      if (fcolor==colors.end()) {
	cout << "WARNING " << "can't found color '" << iter->color << "'" << endl;
	point_colors->InsertNextTuple3(0,0,0);
	continue;
      }

      point_colors->InsertNextTuple3(fcolor->second.r,fcolor->second.g,fcolor->second.b);
    }

    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(points);
    pd->GetPointData()->SetScalars(point_colors);
    points->Delete();
    point_colors->Delete();

    return pd;
  }

  void addCube(int x,int y,int z,const string &color) {
    //TODO add check 
    cubes.push_back(Cube(x,y,z,color));
  }

  void removeCube(int x, int y,int z) {
    for (Cubes::iterator iter=cubes.begin(); iter!=cubes.end(); iter++) {
      if (x==iter->x && y==iter->y && z==iter->z) {
	cubes.erase(iter);
	return;
      }
    }
  }

  bool hasCube(int x, int y,int z) {
    for (Cubes::const_iterator iter=cubes.begin(); iter!=cubes.end(); iter++) {
      if (x==iter->x && y==iter->y && z==iter->z) return true;
    }
    return false;
  }

  void addColor(unsigned char r,unsigned char g,unsigned char b,const string &name) {
    //TODO add check 
    colors[name] = Color(r,g,b);
  }
};

static Puzzle puzzle;
static Puzzle::Cube cube(0,0,0,"default");
vtkActor *sel_actor = NULL;
vtkRenderWindow *window = NULL;
vtkGlyph3D *glyph = NULL;

void update_sel() {
  assert(sel_actor);
  assert(window);
  assert(glyph);
  sel_actor->SetPosition(cube.x,cube.y,cube.z);
  vtkPolyData *data = puzzle.buildPolyData();
  glyph->SetInput(data);
  data->Delete();
  window->SetWindowName(puzzle.filename.c_str());
  window->Render();
}

void keyCallback(vtkObject* caller,long unsigned int eventId,void* clientData,void* callData)
{
  vtkRenderWindowInteractor *iren = 
    static_cast<vtkRenderWindowInteractor*>(caller);

  string pressed = iren->GetKeySym();

  //cout << "pressed " << pressed << endl;
  if (pressed=="Right") { cube.x++; update_sel(); return; }
  if (pressed=="Left") { cube.x--; update_sel(); return; }
  if (pressed=="Up") { cube.y++; update_sel(); return; }
  if (pressed=="Down") { cube.y--; update_sel(); return; }
  if (pressed=="Next") { cube.z++; update_sel(); return; }
  if (pressed=="Prior") { cube.z--; update_sel(); return; }
  if (pressed=="space") {
    if (!puzzle.hasCube(cube.x,cube.y,cube.z)) {
      puzzle.addCube(cube.x,cube.y,cube.z,"white");
      cout << "added cube" << endl;
      update_sel();
    } else {
      puzzle.removeCube(cube.x,cube.y,cube.z);
      cout << "removed cube" << endl;
      update_sel();
    }
    return;
  }
  if (pressed=="s") { puzzle.save(); return; }

  cout << "unhandled " << pressed << endl;
}

int main(int argc,char * argv[])
{
  if (argc>1) {
    puzzle.load(argv[1]);
  } else {
    cube.x = 2;
    puzzle.addCube(0,0,0,"white");
    puzzle.addCube(1,0,0,"red");
    puzzle.addCube(0,1,0,"green");
    puzzle.addCube(0,0,1,"blue");
  }

  vtkActor *puzzle_actor = vtkActor::New();
  {
    vtkPolyData *data = puzzle.buildPolyData();

    vtkCubeSource *source = vtkCubeSource::New();
    const double length=1;
    source->SetXLength(length);
    source->SetYLength(length);
    source->SetZLength(length);

    glyph = vtkGlyph3D::New();
    glyph->SetScaleModeToDataScalingOff();
    glyph->SetColorModeToColorByScalar();
    glyph->SetInput(data);
    glyph->SetSource(source->GetOutput());
    data->Delete();
    source->Delete();

    vtkPolyDataMapper *mapper=vtkPolyDataMapper::New();
    mapper->SetInput(glyph->GetOutput());
    glyph->Delete();

    puzzle_actor->SetMapper(mapper);
    mapper->Delete();
  }

  sel_actor = vtkActor::New();
  {
    const double length=1;
    vtkPoints *points = vtkPoints::New();
    points->InsertNextPoint(-length/2.,-length/2.,-length/2.);
    points->InsertNextPoint(length/2.,-length/2.,-length/2.);
    points->InsertNextPoint(length/2.,length/2.,-length/2.);
    points->InsertNextPoint(-length/2.,length/2.,-length/2.);
    points->InsertNextPoint(-length/2.,-length/2.,length/2.);
    points->InsertNextPoint(length/2.,-length/2.,length/2.);
    points->InsertNextPoint(length/2.,length/2.,length/2.);
    points->InsertNextPoint(-length/2.,length/2.,length/2.);
    vtkCellArray *lines = vtkCellArray::New();
    lines->InsertNextCell(2);
    lines->InsertCellPoint(0);
    lines->InsertCellPoint(1);
    lines->InsertNextCell(2);
    lines->InsertCellPoint(1);
    lines->InsertCellPoint(2);
    lines->InsertNextCell(2);
    lines->InsertCellPoint(2);
    lines->InsertCellPoint(3);
    lines->InsertNextCell(2);
    lines->InsertCellPoint(3);
    lines->InsertCellPoint(0);

    lines->InsertNextCell(2);
    lines->InsertCellPoint(4);
    lines->InsertCellPoint(5);
    lines->InsertNextCell(2);
    lines->InsertCellPoint(5);
    lines->InsertCellPoint(6);
    lines->InsertNextCell(2);
    lines->InsertCellPoint(6);
    lines->InsertCellPoint(7);
    lines->InsertNextCell(2);
    lines->InsertCellPoint(7);
    lines->InsertCellPoint(4);

    lines->InsertNextCell(2);
    lines->InsertCellPoint(0);
    lines->InsertCellPoint(4);
    lines->InsertNextCell(2);
    lines->InsertCellPoint(1);
    lines->InsertCellPoint(5);
    lines->InsertNextCell(2);
    lines->InsertCellPoint(2);
    lines->InsertCellPoint(6);
    lines->InsertNextCell(2);
    lines->InsertCellPoint(3);
    lines->InsertCellPoint(7);

    vtkPolyData *data = vtkPolyData::New();
    data->SetPoints(points);
    points->Delete();
    data->SetLines(lines);
    lines->Delete();

    vtkTubeFilter *filter = vtkTubeFilter::New();
    filter->SetRadius(.1);
    filter->SetNumberOfSides(10);
    filter->SetInput(data);
    data->Delete();

    vtkPolyDataMapper *mapper=vtkPolyDataMapper::New();
    mapper->SetInput(filter->GetOutput());
    filter->Delete();

    sel_actor->SetMapper(mapper);
    mapper->Delete();
  }

  vtkRenderer *renderer=vtkRenderer::New();
  renderer->AddActor(puzzle_actor);
  renderer->AddActor(sel_actor);
  puzzle_actor->Delete();
  sel_actor->Delete();

  vtkRenderWindowInteractor *inter=vtkRenderWindowInteractor::New();
  vtkCallbackCommand *callback = vtkCallbackCommand::New();
  callback->SetCallback(keyCallback);
  inter->AddObserver(vtkCommand::KeyPressEvent,callback);

  window=vtkRenderWindow::New();
  window->SetInteractor(inter);
  inter->Delete();
  window->AddRenderer(renderer);
  renderer->Delete();

  window->SetSize(800,800);
  update_sel();
  inter->Start();

  window->Delete();

  return 0;
}

