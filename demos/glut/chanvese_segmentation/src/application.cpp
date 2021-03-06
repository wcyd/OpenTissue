//
// OpenTissue Template Library Demo
// - A specific demonstration of the flexibility of OTTL.
// Copyright (C) 2008 Department of Computer Science, University of Copenhagen.
//
// OTTL and OTTL Demos are licensed under zlib.
//
#include <OpenTissue/configuration.h>

#define DEFINE_GLUT_MAIN
#include <OpenTissue/utility/glut/glut_perspective_view_application.h>
#undef DEFINE_GLUT_MAIN

#include <OpenTissue/core/containers/grid/io/grid_raw_read.h>
#include <OpenTissue/utility/utility_get_environment_variable.h>

#include <OpenTissue/core/math/math_basic_types.h>
#include <OpenTissue/core/containers/grid/grid.h>

#include <OpenTissue/core/containers/grid/util/grid_crop.h>
#include <OpenTissue/core/containers/grid/util/grid_resample.h>
#include <OpenTissue/core/containers/grid/util/grid_chan_vese.h>
#include <OpenTissue/core/containers/grid/util/grid_ignore_region.h>
#include <OpenTissue/core/containers/grid/util/grid_blockify.h>
#include <OpenTissue/core/containers/grid/util/grid_redistance.h>
#include <OpenTissue/core/containers/grid/util/grid_curvature_flow.h>

#include <OpenTissue/utility/gl/gl_draw_mesh.h>
#include <OpenTissue/core/containers/mesh/trimesh/trimesh.h>
#include <OpenTissue/core/containers/mesh/common/util/mesh_isosurface.h>
#include <OpenTissue/core/containers/mesh/common/util/mesh_compute_angle_weighted_vertex_normals.h>

using namespace OpenTissue;

class Application : public OpenTissue::glut::PerspectiveViewApplication
{
protected:

  typedef OpenTissue::math::BasicMathTypes<double,unsigned int>	 math_types;
  typedef math_types::real_type								                   real_type;
  typedef math_types::vector3_type				                       vector3_type;
  typedef unsigned short									                       value_type;
  typedef OpenTissue::grid::Grid<value_type, math_types>			           input_type;
  typedef OpenTissue::trimesh::TriMesh<>				  			         mesh_type;  
  typedef OpenTissue::grid::Grid<real_type,math_types>                  levelset_type;

  input_type    m_volume;
  mesh_type     m_surface;
  levelset_type m_phi;

public:

  char const * do_get_title() const { return "Chan-Vese Segmentation Application"; }

  void do_display()
  {
    gl::ColorPicker(0.8,0.4,0.8);
    gl::DrawGridAABB(m_volume);

    gl::ColorPicker(0.9, 0.3, 0.1, 1.0, GL_BACK);
    gl::ColorPicker(0.1, 0.3, 0.9, 1.0, GL_FRONT);
    gl::DrawMesh(m_surface);
  }

  void do_action(unsigned char choice)
  {
    switch ( choice )
    {
    case 's':
      {
        real_type lambda = 0.5;
        real_type timestep = 0.05;
        real_type nu = 0.05;
        real_type epsilon = 1.0;
        unsigned int max_iterations = 100;
        chan_vese_auto_in_out(m_phi,m_volume,lambda,0.0,nu,timestep,m_phi,epsilon,max_iterations);
      }
      break;
    case 'm':
      {
        //grid_redistance(m_phi,m_phi); //--- can not handle unsued values:-(
        real_type mu = 0.01; 
        real_type timestep = 0.05;
        OpenTissue::grid::curvature_flow(m_phi,mu,timestep,m_phi);   //---  unsued values may fuck this up!!!!
      }
      break;
    case 'i':
      {
        OpenTissue::grid::ignore_region(m_phi,OpenTissue::grid::inside_region_tag());
        OpenTissue::grid::blockify(m_phi);
      }
      break;
    case 'o':
      {
        OpenTissue::grid::ignore_region(m_phi,OpenTissue::grid::outside_region_tag());
        OpenTissue::grid::blockify(m_phi);
      }
      break;
    case 'r':
      {
        OpenTissue::grid::chan_vese::threshold_initialize(m_volume, 10, 100, m_phi);
      }
      break;
    case 'v':
      {
        real_type volume = 0;
        OpenTissue::grid::chan_vese::compute_volume(m_phi,volume);
        std::cout << "Volume of segmented object is: " << volume << std::endl;
      }
      break;
    case 'e':
      {
        real_type lvl = 0.1;//--- KE 03-06-2003: Magic value, it works...
        mesh::isosurface(m_phi,lvl,m_surface);
        mesh::compute_angle_weighted_vertex_normals(m_surface);
      }
      break;
    };
  }

  void do_init_right_click_menu(int main_menu, void menu(int entry))
  {
    int controls = glutCreateMenu( menu );
    glutAddMenuEntry( "Extract surface          [e]", 'e' );
    glutAddMenuEntry( "Reset segmentation       [r]", 'r' );
    glutAddMenuEntry( "Compute volume           [v]", 'v' );
    glutAddMenuEntry( "ignore inside            [i]", 'i' );
    glutAddMenuEntry( "ignore outside           [o]", 'o' );
    glutAddMenuEntry( "chan vese step           [s]", 's' );
    glutAddMenuEntry( "Mean curvature flow      [m]", 'm' );
    glutSetMenu( main_menu );
    glutAddSubMenu( "Chan-Vese", controls );
  }

  void do_init()
  {
    this->camera().move(95);

    load_knee();
  }

  void do_run(){}

  void do_shutdown(){}

protected:

  void load_teddy()
  {
    std::string path = opentissue_path;
    std::string filename = path + "/demos/data/raw/teddy_bear.raw";
    m_volume.create( 128, 128, 62, 2.8,  2.8, 5.0 );
    OpenTissue::grid::raw_read_8bit_to_16bit(filename, m_volume);
    m_phi.create(m_volume.I(),m_volume.J(),m_volume.K(), m_volume.dx(),m_volume.dy(),m_volume.dz());
    //chan_vese::threshold_initialize(m_volume, 10, 100, m_phi);
    std::fill(m_phi.begin(), m_phi.end(), 0 );
    OpenTissue::grid::blockify(m_phi);
  }

  void load_knee()
  {
    input_type tmp;
    input_type crop_grid;
    std::string path;
    try
    {
      path = OpenTissue::utility::get_environment_variable("DATATISSUE");
    }
    catch(std::exception e)
    {
      std::cout << e.what() << std::endl;
      std::cout << "WARNING: Could not find DATATISSUE, using teddy bear instead" << std::endl;
      load_teddy();
      return;
    }
    std::string filename = path + "/raw/knee_512x512x64_1channels_16bit.raw";
    tmp.create( 512,512,64, 0.35,0.35,1.6);
    OpenTissue::grid::raw_read_16bit(filename, tmp);

    OpenTissue::grid::crop(tmp, crop_grid, 0); //--- Remove black around object.
    OpenTissue::grid::resample(crop_grid, m_volume, 2.0, 2.0, 1.0); //--- get smaller data set
    m_phi.create(m_volume.I(),m_volume.J(),m_volume.K(), m_volume.dx(),m_volume.dy(),m_volume.dz());
    OpenTissue::grid::chan_vese::threshold_initialize(m_volume, 10, 100, m_phi);
  }

};

OpenTissue::glut::instance_pointer init_glut_application(int argc, char **argv)
{
  OpenTissue::glut::instance_pointer instance;
  instance.reset( new Application() );
  return instance;
}
