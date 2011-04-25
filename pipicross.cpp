#include <vtkCubeSource.h>
#include <vtkGlyph3D.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <list>
#include <map>
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

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

  Puzzle() {
    addColor(255,255,255,"white");
    addColor(255,0,0,"red");
    addColor(0,255,0,"green");
    addColor(0,0,255,"blue");
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
    dump(cout);

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

  void addColor(unsigned char r,unsigned char g,unsigned char b,const string &name) {
    //TODO add check 
    colors[name] = Color(r,g,b);
  }
};

int main(int argc,char * argv[])
{
  Puzzle puzzle;
  puzzle.addCube(0,0,0,"white");
  puzzle.addCube(1,0,0,"red");
  puzzle.addCube(0,1,0,"green");
  puzzle.addCube(0,0,1,"blue");

  vtkPolyData *data = puzzle.buildPolyData();

  vtkCubeSource *source=vtkCubeSource::New();
  const double length=1;
  source->SetXLength(length);
  source->SetYLength(length);
  source->SetZLength(length);

  vtkGlyph3D *glyph = vtkGlyph3D::New();
  glyph->SetScaleModeToDataScalingOff();
  glyph->SetColorModeToColorByScalar();
  glyph->SetInput(data);
  glyph->SetSource(source->GetOutput());
  data->Delete();
  source->Delete();

  vtkPolyDataMapper *mapper=vtkPolyDataMapper::New();
  mapper->SetInput(glyph->GetOutput());
  glyph->Delete();

  vtkActor *actor=vtkActor::New();
  actor->SetMapper(mapper);
  mapper->Delete();

  vtkRenderer *renderer=vtkRenderer::New();
  renderer->AddActor(actor);
  actor->Delete();

  vtkRenderWindowInteractor *inter=vtkRenderWindowInteractor::New();

  vtkRenderWindow *window=vtkRenderWindow::New();
  window->SetInteractor(inter);
  inter->Delete();
  window->AddRenderer(renderer);
  renderer->Delete();

  window->Render();
  inter->Start();

  window->Delete();

  return 0;
}

