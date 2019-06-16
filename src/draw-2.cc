
#ifdef USE_PYTHON
#include <Python.h>
#endif // USE_PYTHON

#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>  // to_string()

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <gtk/gtk.h>
#include <epoxy/gl.h>

#include "globjects.h"
#include "trackball.h"
#include "graphics-info.h"

#include "draw.hh"

gint idle_contour_function(gpointer data);

void init_shaders() {

   GLenum err = glGetError();
   std::cout << "start init_shaders() err is " << err << std::endl;

   std::cout << "----------- parse and create shader " << std::endl;
   shader_program_source sps = parse_shader("Basic.shader");
   err = glGetError();
   std::cout << "init_shaders() parse_shader() err is " << err << std::endl;
   unsigned int programID = CreateShader(sps.VertexSource, sps.FragmentSource);
   err = glGetError();
   std::cout << "init_shaders() CreateShader() returned " << programID << " with err "
             << err << std::endl;
   programID_global = programID;
   std::cout << "----------- created shader program " << programID << std::endl;

   glBindAttribLocation(programID, 0, "position");
   err = glGetError();
   std::cout << "init_shaders() glBindAttribLocation() err is " << err << std::endl;
   if (err == GL_INVALID_VALUE)
      std::cout << "ERROR:: programID " << programID
                << " is not a value generated by OpenGL" << std::endl;

   /* get the location of the "mvp" uniform */
   graphics_info_t::mvp_location = glGetUniformLocation(programID, "mvp");

   err = glGetError();
   std::cout << "finish init_shaders() err is " << err << std::endl;


}

void init_buffers() {

   {
      float positions[18] = {
	 -0.5,  -0.5, -0.0,
	 -0.5,   0.5, -0.0,
	  0.5,   0.5, -0.0,
	  0.8,  -0.4, -0.1,
	 -0.2,  -0.4,  0.1,
	  0.5,   0.4, -0.1
      };

      unsigned int indices[12] { 0,1,1,2,2,0,   3,4,4,5,5,3 };

      GLenum err = glGetError();
      std::cout << "start init_buffers() err is " << err << std::endl;

      // GLuint VertexArrayID;
      glGenVertexArrays(1, &VertexArrayID);
      glBindVertexArray(VertexArrayID);

      std::cout << "init_buffers() binding to VertexArrayID " << VertexArrayID << std::endl;

      GLuint vertexbuffer;
      glGenBuffers(1, &vertexbuffer);
      glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18, &positions[0], GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

      unsigned int ibo;
      glGenBuffers(1, &ibo);
      err = glGetError();
      std::cout << "init_buffers() index glGenBuffers() err is " << err << std::endl;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 12, &indices[0], GL_STATIC_DRAW);
      err = glGetError();
      std::cout << "init_buffers() glBufferData() err is " << err << std::endl;

   }
}

bool graphics_info_t::draw_the_other_things = false;
// GLuint *graphics_info_t::molecules_imol_VertexArrayID = new GLuint[10];
GLuint graphics_info_t::other_triangles_vertexarray_id  = -1;
GLuint graphics_info_t::other_triangles_array_buffer_id = -1;
GLuint graphics_info_t::other_triangles_index_buffer_id = -1;

void init_other_buffers() {

      float positions[18] = {
	 -0.35,  -0.35, -0.2,
	 -0.35,   0.35, -0.2,
	  0.35,   0.35, -0.2,
	  0.68,  -0.34, -0.2,
	 -0.15,  -0.34,  0.2,
	  0.45,   0.34, -0.2
      };

      unsigned int indices[12] { 0,1,1,2,2,0,   3,4,4,5,5,3 };

      // GLuint VertexArrayID;
      glGenVertexArrays(1, &graphics_info_t::other_triangles_vertexarray_id);
      GLenum err = glGetError();
      std::cout << "init_other_buffers(): glGenVertexArrays err " << err << std::endl;
      glBindVertexArray(graphics_info_t::other_triangles_vertexarray_id);
      err = glGetError();
      std::cout << "init_other_buffers(): glBindVertexArray err " << err << std::endl;

      std::cout << "init_other_buffers(): binding to graphics_info_t::other_triangles_vertexarray_id "
		<< graphics_info_t::other_triangles_vertexarray_id << std::endl;



      // GLuint vertexbuffer;
      glGenBuffers(1, &graphics_info_t::other_triangles_array_buffer_id);
      err = glGetError();
      if (err) std::cout << "init_other_buffers(): glGenBuffers() err " << err << std::endl;
      glBindBuffer(GL_ARRAY_BUFFER, graphics_info_t::other_triangles_array_buffer_id);
      err = glGetError();
      if (err) std::cout << "init_other_buffers(): glBindBuffer() err " << err << std::endl;
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18, &positions[0], GL_STATIC_DRAW);
      err = glGetError();
      if (err) std::cout << "init_other_buffers(): glBufferData() err " << err << std::endl;
      glEnableVertexAttribArray(0);
      err = glGetError();
      if (err) std::cout << "init_other_buffers(): glEnableVertexAttribArray() err " << err << std::endl;
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
      err = glGetError();
      if (err) std::cout << "init_other_buffers(): glVertexAttribPointer() err " << err << std::endl;

      // unsigned int ibo;
      glGenBuffers(1, &graphics_info_t::other_triangles_index_buffer_id);
      err = glGetError();
      if (err) std::cout << "init_other_buffers(): glGenBuffers() err " << err << std::endl;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphics_info_t::other_triangles_index_buffer_id);
      err = glGetError();
      if (err) std::cout << "init_other_buffers(): glBindBuffer() err " << err << std::endl;
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 12, &indices[0], GL_STATIC_DRAW);
      err = glGetError();
      if (err) std::cout << "init_other_buffers(): glBufferData() err " << err << std::endl;

      std::cout << "done--------------- init_other_buffers() " << std::endl;
}

void draw_triangle(GtkGLArea *glarea) {

   gtk_gl_area_make_current(glarea);
   glLineWidth(1.0);  // GLv4 antialiasing - OpenGL implementations are not required to support this
   GLenum err = glGetError();
   if (err) std::cout << "draw_triangle() A err " << err << std::endl;

   // To see the possible values of the line width in aliased mode:
   // GLfloat line_width_max_min[2] = {0.0f, 0.0f};
   // glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
   // This may not be possible in GL_LINE_SMOOTH mode.

   if (true) {
      glBindVertexArray(VertexArrayID);
      err = glGetError();
      if (err) std::cout << "draw_triangle() B err " << err << std::endl;
      // std::cout << "draw_triangle() B-2 using program " << programID_global << std::endl;
      glUseProgram(programID_global);
      err = glGetError();
      if (err) std::cout << "draw_triangle() C err " << err << std::endl;

      err = glGetError();
      if (err) std::cout << "draw_triangle() D err " << err << std::endl;

      glm::mat4 mvp_1 = glm::toMat4(graphics_info_t::glm_quat);
      glm::vec3 sc(0.2f, 0.2f, 0.2f);
      glm::mat4 mvp = glm::scale(mvp_1, sc);

      if (false) {
	 std::cout << "debug:: draw_triangle()       local mvp: ";
	 for (unsigned int i=0; i<4; i++)
	    for (unsigned int j=0; j<4; j++)
	       std::cout << std::setw(8) << mvp[i][j] << " ";
	 std::cout << std::endl;
      }
      glUniformMatrix4fv(graphics_info_t::mvp_location, 1, GL_FALSE, &mvp[0][0]);
      err = glGetError();
      if (err) std::cout << "draw_triangle() glUniformMatrix4fv() " << err << std::endl;

      glDrawElements(GL_LINES, 12, GL_UNSIGNED_INT, nullptr);
      err = glGetError();
      if (err) std::cout << "draw_triangle() F glDrawElements() " << err << std::endl;
   }

   glBindVertexArray(0); // unbind
   glUseProgram(0);

}

void
draw_other_triangle(GtkGLArea *glarea) {

   std::cout << "---- start draw_other_triangle() " << glarea << std::endl;

   gtk_gl_area_make_current(glarea);
   glLineWidth(3.0);
   GLenum err = glGetError();
   if (err) std::cout << "draw_other_triangle() glLineWidth " << err << std::endl;

   glBindVertexArray(graphics_info_t::other_triangles_vertexarray_id);
   err = glGetError();
   if (err) std::cout << "draw_other_triangle() glBindVertexArray " << err << std::endl;

   // no point going on if the bindvertexarray failed.

   if (! err) {

      glBindBuffer(GL_ARRAY_BUFFER, graphics_info_t::other_triangles_array_buffer_id);
      err = glGetError();
      if (err) std::cout << "draw_other_triangle() glBindVertexArray " << err << std::endl;

      if (! err) {

	 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphics_info_t::other_triangles_index_buffer_id);
	 err = glGetError();
	 if (err) std::cout << "draw_other_triangle(): glBindBuffer() index err " << err << std::endl;

	 if (! err) {
	    glUseProgram(programID_global);
	    err = glGetError();
	    if (err) std::cout << "draw_other_triangle() C err " << err << std::endl;

	    glm::mat4 mvp = glm::toMat4(graphics_info_t::glm_quat);

	    std::cout << "debug:: draw_other_triangle() local mvp: ";
	    for (unsigned int i=0; i<4; i++)
	       for (unsigned int j=0; j<4; j++)
		  std::cout << std::setw(8) << mvp[i][j] << " ";
	    std::cout << std::endl;

	    glUniformMatrix4fv(graphics_info_t::mvp_location, 1, GL_FALSE, &mvp[0][0]);
	    err = glGetError();
	    if (err) std::cout << "draw_other_triangle() glUniformMatrix4fv() err " << err << std::endl;

	    glDrawElements(GL_LINES, 12, GL_UNSIGNED_INT, nullptr);
	    err = glGetError();
	    if (err) std::cout << "draw_other_triangle() glDrawElements() " << err << std::endl;
	 }
      }
   }
}

void
gtk3_draw_molecules() {

   // not needed I think
   // gtk_gl_area_make_current(GTK_GL_AREA(graphics_info_t::glarea));

   // std::cout << "in gtk3_draw_molecules() " << std::endl;

   glLineWidth(1.0);
   GLenum err = glGetError();
   if (err) std::cout << "gtk3_draw_molecules() glLineWidth " << err << std::endl;
   
   glUseProgram(programID_global);
   err = glGetError();
   if (err) std::cout << "   gtk3_draw_molecules() glUseProgram with GL err "
                      << err << std::endl;

   glm::mat4 mvp_1 = glm::toMat4(graphics_info_t::glm_quat);
   float z = graphics_info_t::zoom * 0.04;
   glm::vec3 sc(z,z,z);
   glm::mat4 projection_matrix = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 100.0f);
   glm::mat4 view_matrix = glm::mat4(1.0);

   // view_matrix is for translation and scaling only.
   //
   glm::vec3 rc = graphics_info_t::get_rotation_centre();
   view_matrix = glm::translate(view_matrix, -rc);
   view_matrix = glm::scale(view_matrix, sc);
   std::cout << "gtk3_draw_molecules() with rc " << glm::to_string(rc) << std::endl;

   // glm::mat4 mvp = glm::scale(mvp_1, sc);
   glm::mat4 mvp = projection_matrix * view_matrix * mvp_1;

   for (int ii=graphics_info_t::n_molecules()-1; ii>=0; ii--) {
      if (graphics_info_t::molecules[ii].n_vertices_for_VertexArray > 0) {
         if (true)
            std::cout << "   gtk3_draw_molecules(): imol " << ii
                      << " array_id and n_vertices_for_VertexArray: "
		      << graphics_info_t::molecules[ii].m_VertexArrayID << " "
		      << graphics_info_t::molecules[ii].n_vertices_for_VertexArray
		      << std::endl;
         glBindVertexArray(graphics_info_t::molecules[ii].m_VertexArrayID);
         err = glGetError();
         if (err) std::cout << "   gtk3_draw_molecules() glBindVertexArray() "
			    << graphics_info_t::molecules[ii].m_VertexArrayID
			    << " with GL err " << err << std::endl;

         glBindBuffer(GL_ARRAY_BUFFER,         graphics_info_t::molecules[ii].m_VertexBufferID);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphics_info_t::molecules[ii].m_IndexBufferID);

         glUniformMatrix4fv(graphics_info_t::mvp_location, 1, GL_FALSE, &mvp[0][0]);
         err = glGetError();
         if (err) std::cout << "   gtk3_draw_molecules() glUniformMatrix4fv() " << err << std::endl;

         glDrawElements(GL_LINES, graphics_info_t::molecules[ii].n_vertices_for_VertexArray,
                        GL_UNSIGNED_INT, nullptr);
         err = glGetError();
         if (err) std::cout << "   gtk3_draw_molecules() glDrawElements() n_vertices: "
			    << graphics_info_t::molecules[ii].n_vertices_for_VertexArray
			    << " with GL err " << err << std::endl;
      }
   }
}

GtkWidget *my_gtkglarea(GtkWidget *vbox) {

   GtkWidget *w = gtk_gl_area_new();
   gtk_widget_set_size_request(w, 700, 700);
   gtk_box_pack_start(GTK_BOX(vbox), w, TRUE, TRUE, 2);
   return w;
}

void
on_glarea_realize(GtkGLArea *glarea) {

   std::cout << "realize!" << std::endl;
   gtk_gl_area_make_current(glarea);
   GLenum err = glGetError();
   std::cout << "start on_glarea_realize() err is " << err << std::endl;

   init_shaders();
   err = glGetError();
   std::cout << "on_glarea_realize() post init_shaders() err is " << err << std::endl;

   init_buffers();

   err = glGetError();
   std::cout << "post init_buffer(): err is " << err << std::endl;

   // so, I get to see the other things when I init their buffers on realize, but not after!
   if (true) {
      init_other_buffers();
      graphics_info_t::draw_the_other_things = true;
   }

   glEnable(GL_DEPTH_TEST);

   // Make antialised lines
   glEnable (GL_BLEND);
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_LINE_SMOOTH);

#if 0
   transform = Transform(glm::vec3(0.0, 0.0, 0.0),   // position
			 glm::vec3(0.0, 0.0, 0.0),   // rotation
			 glm::vec3(1.0, 1.0, 1.0));  // scales
#endif

}


gboolean
on_glarea_render(GtkGLArea *glarea) {

   // is this needed?
   gtk_gl_area_make_current(glarea);
   GLenum err = glGetError();
   if (err) std::cout << "on_glarea_render() start " << err << std::endl;

   glClearColor (0.0, 0.0, 0.0, 1.0);
   err = glGetError();
   if (err) std::cout << "on_glarea_render B err " << err << std::endl;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
   err = glGetError();
   if (err) std::cout << "on_glarea_render C err " << err << std::endl;   

   // 

   // get rid of this?
   // if (graphics_info_t::draw_the_other_things)
   // draw_other_triangle(glarea);


   draw_triangle(glarea);

   gtk3_draw_molecules();
   err = glGetError();
   if (err) std::cout << "on_glarea_render gtk3_draw_molecules() " << err << std::endl;

   glFlush ();
   err = glGetError();
   if (err) std::cout << "on_glarea_render E err " << err << std::endl;

  return FALSE;
}


void
on_glarea_resize(GtkGLArea *glarea, gint width, gint height) {

   std::cout << "resize!" << std::endl;
}

gboolean
on_glarea_scroll(GtkWidget *widget, GdkEventScroll *event) {

   int direction = 1;
   if (event->direction == GDK_SCROLL_UP)
      direction = -1;

   std::cout << "scroll " << direction << std::endl;

   graphics_info_t g;
   int imol_scroll = graphics_info_t::scroll_wheel_map;
   imol_scroll = 0;

   if (g.is_valid_map_molecule(imol_scroll)) {
      // use direction
      if (direction == 1)
	 graphics_info_t::molecules[imol_scroll].pending_contour_level_change_count--;
      if (direction == -1)
	 graphics_info_t::molecules[imol_scroll].pending_contour_level_change_count++;
      int contour_idle_token = g_idle_add(idle_contour_function, g.glarea);
      std::cout << "####### Now contour level for map " << imol_scroll << "is "
                << g.molecules[imol_scroll].contour_level << std::endl;
      g.set_density_level_string(imol_scroll, g.molecules[imol_scroll].contour_level);
      g.display_density_level_this_image = 1;
      g.update_maps();
      gtk_widget_queue_draw(widget);
   } else {
      std::cout << "No map" << std::endl;
   }
   return TRUE;
}

gboolean
on_glarea_button_press(GtkWidget *widget, GdkEventButton *event) {

   std::cout << "button press!" << std::endl;
   graphics_info_t g;
   g.SetMouseBegin(event->x,event->y);
   return TRUE;
}

gboolean
on_glarea_button_release(GtkWidget *widget, GdkEventButton *event) {

   std::cout << "button release!" << std::endl;
   return TRUE;
}

gboolean
on_glarea_motion_notify(GtkWidget *widget, GdkEventMotion *event) {

   int r = 0;
   graphics_info_t g;

   // split this function up before it gets too big.

   g.mouse_current_x = event->x;
   g.mouse_current_y = event->y;

   if (event->state & GDK_BUTTON1_MASK) {

      GtkAllocation allocation;
      gtk_widget_get_allocation(widget, &allocation);
      int w = allocation.width;
      int h = allocation.height;

      glm::quat tb_quat =
	 g.trackball_to_quaternion((2.0*g.GetMouseBeginX() - w)/w, (h - 2.0*g.GetMouseBeginY())/h,
				   (2.0*g.mouse_current_x - w)/w,  (h - 2.0*g.mouse_current_y)/h,
				   g.get_trackball_size());

      glm::mat4 mat_from_quat = glm::toMat4(tb_quat);

      glm::quat product = tb_quat * graphics_info_t::glm_quat;
      graphics_info_t::glm_quat = glm::normalize(product);
   }


   if (event->state & GDK_BUTTON2_MASK) {

      // View Panning

      // glm::vec3 unProject(glm::vec3 const& win,
      //                     glm::mat4 const& model,
      //                     glm::mat4 const& proj,
      //                     glm::ivec4 const& viewport);

      glm::mat4 mvp_1 = glm::toMat4(graphics_info_t::glm_quat);
      float z = graphics_info_t::zoom * 0.04;
      glm::vec3 sc(z,z,z);
      glm::mat4 projection_matrix = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 100.0f);
      glm::mat4 view_matrix = glm::mat4(1.0);

      glm::vec3 rc = graphics_info_t::get_rotation_centre();
      view_matrix = glm::translate(view_matrix, -rc);
      view_matrix = glm::scale(view_matrix, sc);

      glm::mat4 mvp = projection_matrix * view_matrix * mvp_1;
      glm::mat4 mvp_inv = glm::inverse(mvp);

      glm::vec4 sp1(g.GetMouseBeginX(), g.GetMouseBeginY(), 0.0f, 0.0f);
      glm::vec4 sp2(g.mouse_current_x,  g.mouse_current_y,  0.0f, 0.0f);

      glm::vec4 pt_1 = sp1 * mvp_inv;
      glm::vec4 pt_2 = sp2 * mvp_inv;

      glm::vec4 delta = pt_2 - pt_1;
      std::cout << "delta: " << glm::to_string(delta) << std::endl;

      g.add_to_rotation_centre(delta * 0.1f);
      std::cout << "add a contourer" << std::endl;
      int contour_idle_token = g_idle_add(idle_contour_function, g.glarea);
   }

   if (event->state & GDK_BUTTON3_MASK) {
      double delta_x = event->x - g.GetMouseBeginX();
      double delta_y = event->y - g.GetMouseBeginY();
      double fx = 1.0f +  delta_x/400.0;
      double fy = 1.0f +  delta_y/400.0;
      if (fx > 0.0) g.zoom *= fx;
      if (fy > 0.0) g.zoom *= fy;
      // std::cout << "now zoom: " << g.zoom << std::endl;
   }

   // for next motion
   g.SetMouseBegin(event->x,event->y);
   gtk_widget_queue_draw(widget);
   return TRUE;
}

gboolean
on_glarea_key_press_notify(GtkWidget *widget, GdkEventKey *event) {

   std::cout << "on_glarea_key_press_notify() " << std::endl;
   graphics_info_t g;

   if (event->keyval == GDK_KEY_n) {
      std::cout << "Zoom in " << std::endl;
      graphics_info_t::zoom *= 0.9;
   }
   if (event->keyval == GDK_KEY_m) {
      std::cout << "Zoom out " << std::endl;
      graphics_info_t::zoom *= 1.1;
   }

   if (event->keyval == GDK_KEY_i) {
      float delta = 0.1;
      glm::vec3 EulerAngles(0, delta, 0);
      glm::quat quat_delta(EulerAngles);
      glm::quat normalized_quat_delta(glm::normalize(quat_delta));
      glm::quat product = normalized_quat_delta * graphics_info_t::glm_quat;
      graphics_info_t::glm_quat = glm::normalize(product);
   }

   if (event->keyval == GDK_KEY_minus || event->keyval == GDK_KEY_plus) {
      int s = graphics_info_t::scroll_wheel_map;
      if (graphics_info_t::is_valid_map_molecule(s)) {
         if (event->keyval == GDK_KEY_minus)
	    graphics_info_t::molecules[s].pending_contour_level_change_count--;
         if (event->keyval == GDK_KEY_plus)
	    graphics_info_t::molecules[s].pending_contour_level_change_count++;
	 int contour_idle_token = g_idle_add(idle_contour_function, g.glarea);
	 g.set_density_level_string(s, g.molecules[s].contour_level);
	 g.display_density_level_this_image = 1;
      }
   }

   gtk_widget_queue_draw(widget);

   return TRUE;

}

gboolean
on_glarea_key_release_notify(GtkWidget *widget, GdkEventKey *event) {

   return TRUE;
}

void my_glarea_add_signals_and_events(GtkWidget *glarea) {

   gtk_widget_add_events(glarea, GDK_SCROLL_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON_PRESS_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON_RELEASE_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON1_MOTION_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON2_MOTION_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON3_MOTION_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON1_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON2_MASK);
   gtk_widget_add_events(glarea, GDK_BUTTON3_MASK);
   gtk_widget_add_events(glarea, GDK_KEY_PRESS_MASK);

   // key presses for the glarea:
   gtk_widget_set_can_focus(glarea, TRUE);
   gtk_widget_grab_focus(glarea);

   g_signal_connect(glarea, "realize", G_CALLBACK(on_glarea_realize), NULL);
   g_signal_connect(glarea, "render",  G_CALLBACK(on_glarea_render),  NULL);
   g_signal_connect(glarea, "resize",  G_CALLBACK(on_glarea_resize),  NULL);
   g_signal_connect(glarea, "scroll-event",          G_CALLBACK(on_glarea_scroll),             NULL);
   g_signal_connect(glarea, "button-press-event",    G_CALLBACK(on_glarea_button_press),       NULL);
   g_signal_connect(glarea, "button-release-event",  G_CALLBACK(on_glarea_button_release),     NULL);
   g_signal_connect(glarea, "motion-notify-event",   G_CALLBACK(on_glarea_motion_notify),      NULL);
   g_signal_connect(glarea, "key-press-event",       G_CALLBACK(on_glarea_key_press_notify),   NULL);
   g_signal_connect(glarea, "key-release-event",     G_CALLBACK(on_glarea_key_release_notify), NULL);

}
