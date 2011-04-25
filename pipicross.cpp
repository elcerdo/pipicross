#include <vtkCubeSource.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <iostream>
using std::cout;
using std::endl;

int main(int argc,char * argv[])
{
    vtkCubeSource *source=vtkCubeSource::New();

    vtkPolyDataMapper *mapper=vtkPolyDataMapper::New();
    mapper->SetInput(source->GetOutput());
    source->Delete();

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

