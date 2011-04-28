#include <vtkCubeSource.h>
#include <vtkGlyph3D.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkTextProperty.h>
#include <vtkTextActor.h>
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
#include <sstream>
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

#include "helper.h"

struct Puzzle {
public:
  struct Cube {
    Cube(int x,int y,int z) : x(x), y(y), z(z) {}
    int x,y,z;
  };
  typedef std::map<Cube,string> Cubes;

  struct Color {
    Color(unsigned char r=0,unsigned char g=0,unsigned char b=0) : r(r), g(g), b(b) {}
    unsigned char r;
    unsigned char g;
    unsigned char b;
  };
  typedef std::map<string,Color> Colors;

protected:
  Cubes cubes;
  Colors colors;
  string filename;

public:
  Puzzle() {
    reset();
  }

  const std::string &getFilename() const { return filename; }
  const Colors &getColors() const { return colors; }

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
	Matches matches = matchRegex("^\\*+ +([A-Z]+) +\\*+$",line);
	if (!matches.empty()) {
	  section = matches[1];
	  //cout << "entering section " << section << endl;
	  continue;
	}
      }

      { // number of element in section
	Matches matches = matchRegex("^([0-9]+) +([a-z]+)$",line);
	if (!matches.empty()) {
	  int nfound = convertFromString<int>(matches[1]);
	  string name = matches[2];
	  //cout << "found " << nfound << " " << name << endl;
	  continue;
	}
      }

      if (section=="COLORS") { // color element
	Matches matches = matchRegex("^([0-9]+) +([0-9]+) +([0-9]+) +([a-z]+)$",line);
	if (!matches.empty()) {
	  int red = convertFromString<int>(matches[1]);
	  int green = convertFromString<int>(matches[2]);
	  int blue = convertFromString<int>(matches[3]);
	  string name = matches[4];
	  //cout << "color " << name << endl;
	  lcolors[name] = Color(red,green,blue);
	  continue;
	}
      }

      if (section=="CUBES") { // cube element
	Matches matches = matchRegex("^(-?[0-9]+) +(-?[0-9]+) +(-?[0-9]+) +([a-z]+)$",line);
	if (!matches.empty()) {
	  int x = convertFromString<int>(matches[1]);
	  int y = convertFromString<int>(matches[2]);
	  int z = convertFromString<int>(matches[3]);
	  string color = matches[4];
	  //cout << "cube " << x << "," << y << "," << z << endl;
	  lcubes[Cube(x,y,z)] = color;
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
      os << iter->first.x << " " << iter->first.y << " " << iter->first.z << " " << iter->second << endl;
    }
  }

  vtkPolyData *buildPolyData() const {
    vtkPoints *points = vtkPoints::New();
    vtkUnsignedCharArray *point_colors = vtkUnsignedCharArray::New();
    point_colors->SetNumberOfComponents(3);

    for (Cubes::const_iterator iter=cubes.begin(); iter!=cubes.end(); iter++) {
      points->InsertNextPoint(iter->first.x,iter->first.y,iter->first.z);

      Colors::const_iterator fcolor = colors.find(iter->second);
      if (fcolor==colors.end()) {
	cout << "WARNING " << "can't found color '" << iter->second << "'" << endl;
	point_colors->InsertNextTuple3(255,255,255);
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

  bool hasCube(int x, int y,int z) const {
    Cubes::const_iterator iter = cubes.find(Cube(x,y,z));
    return (iter!=cubes.end());
  }

  void addCube(int x,int y,int z,const string &color) {
    Cube cube(x,y,z);
    assert(cubes.find(cube)==cubes.end()); // not already inserted
    cubes[cube] = color;
  }

  void removeCube(int x, int y,int z) {
    Cubes::iterator iter = cubes.find(Cube(x,y,z));
    assert(iter!=cubes.end());
    cubes.erase(iter);
  }

  void changeCubeColor(int x, int y,int z, const std::string &color) {
    Cubes::iterator iter = cubes.find(Cube(x,y,z));
    assert(iter!=cubes.end());
    iter->second =  color;
  }

  bool hasColor(const std::string &name) {
    Colors::const_iterator iter = colors.find(name);
    return (iter!=colors.end());
  }

  void addColor(unsigned char r,unsigned char g,unsigned char b,const string &name) {
    assert(colors.find(name)==colors.end()); // not already inserted
    colors[name] = Color(r,g,b);
  }
};

bool operator<(const Puzzle::Cube &a,const Puzzle::Cube &b) {
  if (a.z!=b.z) return a.z<b.z;
  if (a.y!=b.y) return a.y<b.y;
  if (a.x!=b.x) return a.x<b.x;
  return false;
}

static Puzzle puzzle;
static Puzzle::Cube cube(0,0,0);
vtkActor *sel_actor = NULL;
vtkRenderWindow *window = NULL;
vtkGlyph3D *glyph = NULL;

typedef std::map<string,string> KeyColors;
static KeyColors keyColors;

void updateSelection() {
  assert(sel_actor);
  assert(window);
  assert(glyph);
  sel_actor->SetPosition(cube.x,cube.y,cube.z);
  vtkPolyData *data = puzzle.buildPolyData();
  glyph->SetInput(data);
  data->Delete();
  window->SetWindowName(puzzle.getFilename().c_str());
  window->Render();
}

void addColorActor(vtkRenderer *renderer) {
  assert(renderer);
  assert(keyColors.empty());

  int y = 24;
  size_t kk = 1;
  for (Puzzle::Colors::const_iterator iter = puzzle.getColors().begin(); iter!=puzzle.getColors().end(); iter++) {
    vtkTextActor *actor = vtkTextActor::New();
    std::stringstream name;
    name << "F" << kk;

    string key = name.str();

    name << " ";
    name << iter->first.c_str();
    
    actor->SetInput(name.str().c_str());
    actor->SetDisplayPosition(20,y);
    actor->GetTextProperty()->SetColor(static_cast<double>(iter->second.r)/255.,static_cast<double>(iter->second.g)/255.,static_cast<double>(iter->second.b)/255.);
    actor->GetTextProperty()->SetBold(true);
    actor->GetTextProperty()->SetFontSize(24);
    renderer->AddActor(actor);
    actor->Delete();

    keyColors[key] = iter->first;

    y+=24;
    kk++;
  }
}

void keyCallback(vtkObject* caller,long unsigned int eventId,void* clientData,void* callData)
{
  vtkRenderWindowInteractor *iren = 
    static_cast<vtkRenderWindowInteractor*>(caller);

  string pressed = iren->GetKeySym();

  //cout << "pressed " << pressed << endl;
  
  // moving around
  if (pressed=="Right") { cube.x++; updateSelection(); return; }
  if (pressed=="Left") { cube.x--; updateSelection(); return; }
  if (pressed=="Up") { cube.y++; updateSelection(); return; }
  if (pressed=="Down") { cube.y--; updateSelection(); return; }
  if (pressed=="Next") { cube.z++; updateSelection(); return; }
  if (pressed=="Prior") { cube.z--; updateSelection(); return; }

  // switch cube on/off
  if (pressed=="space") {
    if (!puzzle.hasCube(cube.x,cube.y,cube.z)) {
      puzzle.addCube(cube.x,cube.y,cube.z,"white");
      cout << "added cube" << endl;
    } else {
      puzzle.removeCube(cube.x,cube.y,cube.z);
      cout << "removed cube" << endl;
    }
    updateSelection();
    return;
  }

  // change color
  for (KeyColors::const_iterator iter=keyColors.begin(); iter!=keyColors.end(); iter++) {
    if (pressed!=iter->first) continue;
    cout << "changing color to " << iter->second << endl;
    assert(puzzle.hasColor(iter->second));
    if (!puzzle.hasCube(cube.x,cube.y,cube.z)) puzzle.addCube(cube.x,cube.y,cube.z,iter->second);
    else puzzle.changeCubeColor(cube.x,cube.y,cube.z,iter->second);
    updateSelection();
    return;
  }

  // save puzzle
  if (pressed=="s") { puzzle.save(); return; }

  cout << "unhandled " << pressed << endl;
}

int main(int argc,char * argv[])
{
  if (argc>1) {
    puzzle.load(argv[1]);
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

  vtkRenderer *renderer = vtkRenderer::New();
  renderer->AddActor(puzzle_actor);
  renderer->AddActor(sel_actor);
  puzzle_actor->Delete();
  sel_actor->Delete();

  vtkRenderWindowInteractor *inter = vtkRenderWindowInteractor::New();
  vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
  inter->SetInteractorStyle(style);
  style->Delete();
  vtkCallbackCommand *callback = vtkCallbackCommand::New();
  callback->SetCallback(keyCallback);
  inter->AddObserver(vtkCommand::KeyPressEvent,callback);

  window = vtkRenderWindow::New();
  window->SetInteractor(inter);
  inter->Delete();
  window->AddRenderer(renderer);
  renderer->Delete();

  window->SetSize(800,800);
  addColorActor(renderer);
  updateSelection();
  inter->Start();

  window->Delete();

  return 0;
}

