/*
  File:
    matson_scot_programming_project_4.cpp

  Author:
    Scot Matson

  Programming Project 4:
    A 2D digital drawing application

  macOS Build:
    g++ -Wno-depcreated-declarations -framework OpenGL -frameworks GLUT [file.ext]

*/
#include <GLUT/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Display values
#define X_RESOLUTION 1280
#define Y_RESOLUTION 800

// Assigned keyboard keys
#define GLUT_KEY_ESC 27

// Glut Menu Options
#define ALL 0
#define NO_POLYGON 1

// Object menu
#define POINT        0
#define LINE         1
#define RECTANGLE    2
#define ELLIPSE      3
#define BEZIER_CURVE 4

// Fill menu
#define FILLED  5
#define OUTLINE 6

// Color menu options
#define RED     7
#define GREEN   8
#define BLUE    9
#define YELLOW 10
#define PURPLE 11 
#define ORANGE 12
#define WHITE  13
#define BLACK  14

typedef struct Coord {
  GLenum  draw_mode;
  GLenum  polygon_mode;
  GLfloat line_weight;
  GLint   control_points;
  GLfloat x;
  GLfloat y;
  GLfloat z;
  GLfloat r;
  GLfloat g;
  GLfloat b;
  struct Coord *next;
} coord_t;

void menu_init(void);
void scene_init(void);
void main_menu(int);
void object_menu(int);
void polygon_menu(int);
void color_menu(int);
void background_color_menu(int);
void line_weight_menu(int);
void display(void);
void reshape(int, int);
void keyboard(unsigned char, int, int);
void mouse(int, int, int, int);
void motion(int, int);
void trace(void);
void trace_line();
void trace_rectangle();
void trace_ellipse();
void trace_bezier_curve();
void draw(void);
coord_t* draw_point(coord_t*);
coord_t* draw_line(coord_t*);
coord_t* draw_rectangle(coord_t*);
coord_t* draw_ellipse(coord_t*);
coord_t* draw_bezier_curve(coord_t*);
void push(coord_t*);

// Setting perspective
const GLfloat FIELD_OF_VIEW       =  45.0f;
const GLfloat VIEWPORT_LEFT       = -1.0f;
const GLfloat VIEWPORT_RIGHT      =  1.0f;
const GLfloat VIEWPORT_BOTTOM     = -1.0f;
const GLfloat VIEWPORT_TOP        =  1.0f;
const GLfloat NEAR_CLIPPING_PLANE =  0.0001f;
const GLfloat FAR_CLIPPING_PLANE  =  100.0f;

GLboolean mouse_left_active;
GLboolean tracing;

// Making menus globally visable
int menu;
int fills;
int colors;
int background_colors;
int line_weights;
int objects;
int menu_mode;

// Menu toggles
int menuHasFills;


// (x,y) coordinates for drawing
GLfloat x_px;
GLfloat y_px;
const GLfloat z_px = -1.0f;
GLfloat dx_px;
GLfloat dy_px;
GLfloat x_cp0;
GLfloat x_cp1;
GLfloat x_cp2;
GLfloat x_cp3;
GLfloat y_cp0;
GLfloat y_cp1;
GLfloat y_cp2;
GLfloat y_cp3;
GLint number_of_control_points = 0;

// Setting  defaults
GLenum draw_mode       = GL_POINTS;
GLenum polygon_mode    = GL_LINE;
GLfloat line_weight    = 1.0f;
GLfloat selected_red   = 0.0f;
GLfloat selected_green = 0.0f;
GLfloat selected_blue  = 0.0f;

coord_t *head;

/*
 *  Pushes a new coordinate onto the coordinate stack
 */
void push(coord_t *new_coord) {
  if (head == NULL) {
    head = (coord_t*) malloc(sizeof(coord_t));
    head->draw_mode = new_coord->draw_mode;
    head->polygon_mode = new_coord->polygon_mode;
    head->line_weight = new_coord->line_weight;
    head->control_points = new_coord->control_points;
    head->x = new_coord->x;
    head->y = new_coord->y;
    head->z = new_coord->z;
    head->r = new_coord->r;
    head->g = new_coord->g;
    head->b = new_coord->b;
    head->next = NULL;
  }
  else {
    coord_t *current = head;

    while (current->next != NULL) {
      current = current->next;
    }

    current->next = (coord_t*) malloc(sizeof(coord_t));
    current->next->draw_mode = new_coord->draw_mode;
    current->next->polygon_mode = new_coord->polygon_mode; 
    current->next->line_weight = new_coord->line_weight; 
    current->next->control_points = new_coord->control_points; 
    current->next->x = new_coord->x;
    current->next->y = new_coord->y;
    current->next->z = new_coord->z;
    current->next->r = new_coord->r;
    current->next->g = new_coord->g;
    current->next->b = new_coord->b;
    current->next->next = NULL;
  }
}


/*
 *  Scene intialization
 */
void scene_init() {
  head = NULL;
  mouse_left_active = false;
  GLboolean tracing = false;
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_COLOR_MATERIAL);
  glDepthFunc(GL_NEVER);
}

/*
 *  Renders the display
 */
void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  draw();

  if (tracing) {
    trace();
  }
  glutSwapBuffers();
}

/*
 * Traces to the canvas
 */
void trace() {
  if (draw_mode == GL_LINES) {trace_line();}
  else
  if (draw_mode == GL_QUADS) {trace_rectangle();}
  else
  if (draw_mode == GL_LINE_LOOP || draw_mode == GL_TRIANGLE_FAN) {trace_ellipse();}
  else
  if (draw_mode == GL_LINE_STRIP) {trace_bezier_curve();}
}

/*
 *  Traces a line
 */
void trace_line() {
  glColor3f(selected_red, selected_green, selected_blue);
  glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
  glLineWidth(line_weight);
  glPushMatrix();
  glBegin(draw_mode);
    glVertex3f(x_px, y_px, z_px);
    glVertex3f(dx_px, dy_px, z_px);
  glEnd();
  glPopMatrix();
}

/*
 *  Traces a rectangle
 */
void trace_rectangle() {
  glColor3f(selected_red, selected_green, selected_blue);
  glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
  glLineWidth(line_weight);
  glPushMatrix();
  glBegin(draw_mode);
    glVertex3f(x_px, y_px, z_px);    // Top left
    glVertex3f(dx_px, y_px, z_px);   // Top right
    glVertex3f(dx_px, dy_px, z_px);  // Bottom right
    glVertex3f(x_px, dy_px, z_px);   // Bottom left
  glEnd();
  glPopMatrix();
}

/*
 *  Traces an ellipse
 */
void trace_ellipse() {
  GLint number_of_segments = 100;
  glColor3f(selected_red, selected_green, selected_blue);
  glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
  glLineWidth(line_weight);
  glPushMatrix();
  glBegin(draw_mode);
    for (int i = 0; i < number_of_segments; i++) {
      float theta = 2.0f * M_PI * float(i)/ number_of_segments;

      GLfloat rx = dx_px - x_px;
      GLfloat ry = dy_px - y_px;

      float circle_x = rx * cosf(theta);
      float circle_y = ry * sinf(theta);

      glVertex3f(circle_x + x_px, circle_y + y_px, z_px);
    }
  glEnd();
  glPopMatrix();
}

/*
 *  Traces a Bezier Curve
 */
void trace_bezier_curve() {
  glColor3f(selected_red, selected_green, selected_blue);
  glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
  glLineWidth(line_weight);
  GLfloat x_bc, y_bc;
  glPushMatrix();
  glBegin(draw_mode);
    for (float t = 0; t < 1; t += 0.01) {
      switch(number_of_control_points) {
        case 1: // Linear Bezier
          x_bc = (1-t) * x_cp0 + t * x_cp1;
          y_bc = (1-t) * y_cp0 + t * y_cp1;
          break;
        case 2: // Quadratic Bezier
          x_bc = pow(1-t, number_of_control_points) * x_cp0 + 
                 number_of_control_points * (1-t) * t * x_cp1 + 
                 pow(t, number_of_control_points) * x_cp2;

          y_bc = pow(1-t, number_of_control_points) * y_cp0 + 
                 number_of_control_points * (1-t) * t * y_cp1 + 
                 pow(t, number_of_control_points) * y_cp2;
          break;
        case 3: // Cubic Bezier
          x_bc = pow(1-t, number_of_control_points) * x_cp0 + 
                 number_of_control_points * pow(1-t, number_of_control_points-1) * t * x_cp1 + 
                 number_of_control_points * (1-t) * pow(t, number_of_control_points-1) * x_cp2 + 
                 pow(t, number_of_control_points) * x_cp3;

          y_bc = pow(1-t, number_of_control_points) * y_cp0 + 
                 number_of_control_points * pow(1-t, number_of_control_points-1) * t * y_cp1 + 
                 number_of_control_points * (1-t) * pow(t, number_of_control_points-1) * y_cp2 + 
                 pow(t, number_of_control_points) * y_cp3;
          break;
        default:
          break;
      }
      glVertex3f(x_bc, y_bc, z_px);
    }
  glEnd();
  glPopMatrix();
}

/*
 *  Draws to the canvas
 */
void draw() {
  coord_t *current = head;
  while (current != NULL) {
    switch (current->draw_mode) {
      case GL_POINTS:
        current = draw_point(current);
        break;
      case GL_LINES:
        current = draw_line(current);
        break;
      case GL_QUADS:
        current = draw_rectangle(current);
        break;
      case GL_LINE_LOOP:
      case GL_TRIANGLE_FAN:
        current = draw_ellipse(current);
        break;
      case GL_LINE_STRIP:
        current = draw_bezier_curve(current);
        break;
    }
  }
}

/*
 *  Draws a point
 */
coord_t* draw_point(coord_t *current) {
  glColor3f(current->r, current->g, current->b);
  glPolygonMode(GL_FRONT_AND_BACK, current->polygon_mode);
  glLineWidth(current->line_weight);
  
  glPushMatrix();
  glBegin(current->draw_mode);
    glVertex3f(current->x, current->y, current->z);
  glEnd();
  glPopMatrix();
  return current->next;
}

/*
 * Draws a line
 */
coord_t* draw_line(coord_t *current) {
  glColor3f(current->r, current->g, current->b);
  glPolygonMode(GL_FRONT_AND_BACK, current->polygon_mode);
  glLineWidth(current->line_weight);

  glPushMatrix();
  glBegin(current->draw_mode);
    glVertex3f(current->x, current->y, current->z);
    current = current->next;
    glVertex3f(current->x, current->y, current->z);
  glEnd();
  glPopMatrix();
  return current->next;
}

/*
 *  Draws a rectangle
 */
coord_t* draw_rectangle(coord_t *current) {
  glColor3f(current->r, current->g, current->b);
  glPolygonMode(GL_FRONT_AND_BACK, current->polygon_mode);
  glLineWidth(current->line_weight);

  glPushMatrix();
  glBegin(current->draw_mode);
    glVertex3f(current->x, current->y, current->z);
    current = current->next;
    glVertex3f(current->x, current->y, current->z);
    current = current->next;
    glVertex3f(current->x, current->y, current->z);
    current = current->next;
    glVertex3f(current->x, current->y, current->z);
  glEnd();
  glPopMatrix();
  return current->next;
}

/*
 *  Draws an ellipse
 */
coord_t* draw_ellipse(coord_t *current) {

    GLfloat origin_x = current->x;
    GLfloat origin_y = current->y;

    current = current->next;

    GLfloat arc_x = current->x;
    GLfloat arc_y = current->y;

    GLint number_of_segments = 100;
    glColor3f(current->r, current->g, current->b);
    glPolygonMode(GL_FRONT_AND_BACK, current->polygon_mode);
    glLineWidth(current->line_weight);

    glPushMatrix();
    glBegin(current->draw_mode);
      for (int i = 0; i < number_of_segments; i++) {
        float theta = 2.0f * M_PI * float(i) / number_of_segments;

        GLfloat rx = arc_x - origin_x;
        GLfloat ry = arc_y - origin_y;

        float cx = rx * cosf(theta);
        float cy = ry * sinf(theta);

        glVertex3f(cx + origin_x, cy + origin_y, z_px);
      }
    glEnd();
    glPopMatrix();
  return current->next;
}

/*
 *  Draws a Bezier Curve
 */
coord_t* draw_bezier_curve(coord_t *current) {
  GLfloat xcp0, xcp1, xcp2, xcp3, ycp0, ycp1, ycp2, ycp3;
  switch(current->control_points) {
    case 1:
      xcp0 = current->x;
      ycp0 = current->y;
      current = current->next;

      xcp1 = current->x;
      ycp1 = current->y;
      break;
    case 2:
      xcp0 = current->x;
      ycp0 = current->y;
      current = current->next;

      xcp1 = current->x;
      ycp1 = current->y;
      current = current->next;
      
      xcp2 = current->x;
      ycp2 = current->y;
      break;
    case 3:
      xcp0 = current->x;
      ycp0 = current->y;
      current = current->next;

      xcp1 = current->x;
      ycp1 = current->y;
      current = current->next;
      
      xcp2 = current->x;
      ycp2 = current->y;
      current = current->next;

      xcp3 = current->x;
      ycp3 = current->y;
      break;
    default:
      break;
  }

  GLfloat x_bc, y_bc;
  glColor3f(current->r, current->g, current->b);
  glPolygonMode(GL_FRONT_AND_BACK, current->polygon_mode);
  glLineWidth(current->line_weight);

  glPushMatrix();
  glBegin(current->draw_mode);
    for (float t = 0; t < 1; t += 0.01) {
      switch(current->control_points) {
        case 1:
          x_bc = (1-t) * xcp0 + t * xcp1;
          y_bc = (1-t) * ycp0 + t * ycp1;
          break;
        case 2:
          x_bc = pow(1-t, current->control_points) * xcp0 + 
                 current->control_points * (1-t) * t * xcp1 + 
                 pow(t, current->control_points) * xcp2;

          y_bc = pow(1-t, current->control_points) * ycp0 + 
                 current->control_points * (1-t) * t * ycp1 + 
                 pow(t, current->control_points) * ycp2;
          break;
        case 3:
          x_bc = pow(1-t, current->control_points) * xcp0 + 
                 current->control_points * pow(1-t, current->control_points-1) * t * xcp1 + 
                 current->control_points * (1-t) * pow(t, current->control_points-1) * xcp2 + 
                 pow(t, current->control_points) * xcp3;

          y_bc = pow(1-t, current->control_points) * ycp0 + 
                 current->control_points * pow(1-t, current->control_points-1) * t * ycp1 + 
                 current->control_points * (1-t) * pow(t, current->control_points-1) * ycp2 + 
                 pow(t, current->control_points) * ycp3;
          break;
        default:
          break;
      }
      glVertex3f(x_bc, y_bc, z_px);
    }
  glEnd();
  glPopMatrix();
  return current->next;
}

/*
 *  Initializes the menuing system
 */
void menu_init() {
  menu_mode = 1;
  objects = glutCreateMenu(object_menu);
  glutAddMenuEntry("Point", POINT);
  glutAddMenuEntry("Line", LINE);
  glutAddMenuEntry("Rectangle", RECTANGLE);
  glutAddMenuEntry("Ellipse", ELLIPSE);
  glutAddMenuEntry("Bezier Curve", BEZIER_CURVE);

  colors = glutCreateMenu(color_menu);
  glutAddMenuEntry("Red", RED);
  glutAddMenuEntry("Green", GREEN);
  glutAddMenuEntry("Blue", BLUE);
  glutAddMenuEntry("Yellow", YELLOW);
  glutAddMenuEntry("Purple", PURPLE);
  glutAddMenuEntry("Orange", ORANGE);
  glutAddMenuEntry("White", WHITE);
  glutAddMenuEntry("Black", BLACK);

  background_colors = glutCreateMenu(background_color_menu);
  glutAddMenuEntry("Red", RED);
  glutAddMenuEntry("Green", GREEN);
  glutAddMenuEntry("Blue", BLUE);
  glutAddMenuEntry("Yellow", YELLOW);
  glutAddMenuEntry("Purple", PURPLE);
  glutAddMenuEntry("Orange", ORANGE);
  glutAddMenuEntry("White", WHITE);
  glutAddMenuEntry("Black", BLACK);

  line_weights = glutCreateMenu(line_weight_menu);
  glutAddMenuEntry("1", 1);
  glutAddMenuEntry("2", 2);
  glutAddMenuEntry("3", 3);
  glutAddMenuEntry("4", 4);
  glutAddMenuEntry("5", 5);
  glutAddMenuEntry("6", 6);
  glutAddMenuEntry("7", 7);
  glutAddMenuEntry("8", 8);
  glutAddMenuEntry("9", 9);
  glutAddMenuEntry("10", 10);

  fills = glutCreateMenu(polygon_menu);
  glutAddMenuEntry("Filled", FILLED);
  glutAddMenuEntry("Outline", OUTLINE);
  menuHasFills = false;

  menu = glutCreateMenu(main_menu);
  glutAddSubMenu("Objects", objects);
  glutAddSubMenu("Colors", colors);
  glutAddSubMenu("Background Colors", background_colors);
  glutAddSubMenu("Line Weights", line_weights);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/*
 *  Create a menu system
 */
void main_menu(int value) {
  glutSetMenu(menu);
  switch(value) {
    case ALL:
      if (menu_mode != 0 && !menuHasFills) {
        glutAddSubMenu("Fills", fills);
        menuHasFills = true;
        menu_mode = 0;
      }
      break;
    case NO_POLYGON:
      if (menu_mode != 1) {
        glutRemoveMenuItem(fills);
        menuHasFills = false;
        menu_mode = 1;
      }
    default:
      break;
  }
}

/*
 *  Handles object selection
 */
void object_menu(int value){
  switch(value) {
    case POINT:
      draw_mode = GL_POINTS;
      polygon_mode = GL_POINTS;
      main_menu(NO_POLYGON);
      break;
    case LINE:
      draw_mode = GL_LINES;
      polygon_mode = GL_LINE;
      main_menu(NO_POLYGON);
      break;
    case RECTANGLE:
      draw_mode = GL_QUADS;
      main_menu(ALL);
      break;
    case ELLIPSE:
      if (polygon_mode == GL_FILL) {draw_mode = GL_TRIANGLE_FAN;}
      else
      if (polygon_mode == GL_LINE) {draw_mode = GL_LINE_LOOP;}
      main_menu(ALL);
      break;
    case BEZIER_CURVE:
      draw_mode    = GL_LINE_STRIP;
      polygon_mode = GL_LINE;
      main_menu(NO_POLYGON);
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

/*
 *  Handles polygon fills selection
 */
void polygon_menu(int value){
  switch(value) {
    case FILLED:
      polygon_mode = GL_FILL;
      if (draw_mode == GL_LINE_LOOP) {draw_mode = GL_TRIANGLE_FAN;}
      break;
    case OUTLINE:
      polygon_mode = GL_LINE;
      if (draw_mode == GL_TRIANGLE_FAN) {draw_mode = GL_LINE_LOOP;}
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

/*
 *  Handles color selections
 */
void color_menu(int value) {
  switch(value) {
    case RED:
      selected_red   = 1.0f;
      selected_green = 0.0f;
      selected_blue  = 0.0f;
      break;
    case GREEN:
      selected_red   = 0.0f;
      selected_green = 1.0f;
      selected_blue  = 0.0f;
      break;
    case BLUE:
      selected_red   = 0.0f;
      selected_green = 0.0f;
      selected_blue  = 1.0f;
      break;
    case YELLOW:
      selected_red   = 1.0f;
      selected_green = 1.0f;
      selected_blue  = 0.0f;
      break;
    case PURPLE:
      selected_red   = 0.5f;
      selected_green = 0.0f;
      selected_blue  = 1.0f;
      break;
    case ORANGE:
      selected_red   = 1.0f;
      selected_green = 0.5f;
      selected_blue  = 0.0f;
      break;
    case WHITE:
      selected_red   = 1.0f;
      selected_green = 1.0f;
      selected_blue  = 1.0f;
      break;
    case BLACK:
      selected_red   = 0.0f;
      selected_green = 0.0f;
      selected_blue  = 0.0f;
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

/*
 *  Handles background color selections
 */
void background_color_menu(int value) {
  switch(value) {
    case RED:
      glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
      break;
    case GREEN:
      glClearColor( 0.0f, 1.0f, 0.0f, 0.0f);
      break;
    case BLUE:
      glClearColor( 0.0f, 0.0f, 1.0f, 1.0f);
      break;
    case YELLOW:
      glClearColor(1.0f, 1.0f, 0.0f, 0.0f);
      break;
    case PURPLE:
      glClearColor(0.5f, 0.0f, 1.0f, 0.0f);
      break;
    case ORANGE:
      glClearColor(1.0f, 0.5f, 0.0f, 0.0f);
      break;
    case WHITE:
      glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
      break;
    case BLACK:
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

void line_weight_menu(int value) {
  switch(value) {
    case 1:
      line_weight = 1.0f;
      break;
    case 2:
      line_weight = 2.0f;
      break;
    case 3:
      line_weight = 3.0f;
      break;
    case 4:
      line_weight = 4.0f;
      break;
    case 5:
      line_weight = 5.0f;
      break;
    case 6:
      line_weight = 6.0f;
      break;
    case 7:
      line_weight = 7.0f;
      break;
    case 8:
      line_weight = 8.0f;
      break;
    case 9:
      line_weight = 9.0f;
      break;
    case 10:
      line_weight = 10.0f;
      break;
    default:
      break;
  }

  glutPostRedisplay();
}


/*
 *  Reshapes the display window
 */
void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(
    -1.0, 1.0, -1.0, 1.0, 
    NEAR_CLIPPING_PLANE, 
    FAR_CLIPPING_PLANE);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/*
 *  Keyboard event handlers
 */
void keyboard(unsigned char key, int x, int y) {
  switch(key) {
    case GLUT_KEY_ESC:
      exit(0);
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

/*
 *  Mouse event handlers
 */
void mouse(int button, int state, int x, int y) {
  if (state == GLUT_DOWN) {
    switch(button) {
      case GLUT_LEFT_BUTTON:
        // Captures the (x, y) coordinate on mousedown
        mouse_left_active = true;
        GLfloat viewport[4];
        glGetFloatv(GL_VIEWPORT, viewport);
        x_px =  ((float)x / (viewport[2] / 2.0f)) - 1.0f;
        y_px = -((float)y / (viewport[3] / 2.0f)) + 1.0f;
        break;
      default:
        break;
    }
    if (draw_mode == GL_LINE_STRIP) {
      switch (number_of_control_points) {
        case 0:
          x_cp0 = x_px;
          y_cp0 = y_px;
          break;
        case 1:
          x_cp2 = x_px;
          y_cp2 = y_px;
          break;
        case 2:
          x_cp3 = x_px;
          y_cp3 = y_px;
          break;
      }
      number_of_control_points += 1;
    }
  }
  if (state == GLUT_UP) {
    // Change mouse button state
    mouse_left_active = false;

    // Calculate final mouse position
    GLfloat viewport[4];
    glGetFloatv(GL_VIEWPORT, viewport);
    dx_px =  ((float)x / (viewport[2] / 2.0f)) - 1.0f;
    dy_px = -((float)y / (viewport[3] / 2.0f)) + 1.0f;

    if (draw_mode == GL_LINES || draw_mode == GL_LINE_LOOP || draw_mode == GL_TRIANGLE_FAN) {
      coord_t *point = (coord_t*) malloc(sizeof(coord_t));
      point->draw_mode = draw_mode;
      point->polygon_mode = polygon_mode;
      point->line_weight = line_weight;
      point->x = x_px;
      point->y = y_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);

      point->draw_mode = draw_mode;
      point->polygon_mode = polygon_mode;
      point->line_weight = line_weight;
      point->x = dx_px;
      point->y = dy_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);
      tracing = false;
      free(point);
    }
    else
    if (draw_mode == GL_QUADS) {
      coord_t *point = (coord_t*) malloc(sizeof(coord_t));
      // Top Left
      point->draw_mode = draw_mode;
      point->polygon_mode = polygon_mode;
      point->line_weight = line_weight;
      point->x = x_px;
      point->y = y_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);

      // Top Right
      point->draw_mode = draw_mode;
      point->polygon_mode = polygon_mode;
      point->line_weight = line_weight;
      point->x = dx_px;
      point->y = y_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);

      // Bottom Right
      point->draw_mode = draw_mode;
      point->polygon_mode = polygon_mode;
      point->line_weight = line_weight;
      point->x = dx_px;
      point->y = dy_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);

      // Bottom Left
      point->draw_mode = draw_mode;
      point->polygon_mode = polygon_mode;
      point->line_weight = line_weight;
      point->x = x_px;
      point->y = dy_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);
      tracing = false;
      free(point);
    }
    else
    if (draw_mode == GL_LINE_STRIP) {
      switch (number_of_control_points) {
        case 1:
          x_cp1 = dx_px;
          y_cp1 = dy_px;
          break;
        case 2:
          // LINEAR BEZIER
          if (x_cp1 == x_cp2 && y_cp1 == y_cp2) {
            coord_t *point = (coord_t*) malloc(sizeof(coord_t));
            point->draw_mode = draw_mode;
            point->polygon_mode = polygon_mode;
            point->line_weight = line_weight;
            point->control_points = number_of_control_points-1;
            point->x = x_cp0;
            point->y = y_cp0;
            point->z = z_px;
            point->r = selected_red;
            point->g = selected_green;
            point->b = selected_blue;
            push(point);

            point->draw_mode = draw_mode;
            point->polygon_mode = polygon_mode;
            point->line_weight = line_weight;
            point->control_points = number_of_control_points-1;
            point->x = x_cp1;
            point->y = y_cp1;
            point->z = z_px;
            point->r = selected_red;
            point->g = selected_green;
            point->b = selected_blue;
            push(point);

            tracing = false;
            free(point);
            number_of_control_points = 0;
          }
          x_cp2 = dx_px;
          y_cp2 = dy_px;
          break;
        case 3:
          if (x_cp2 == x_cp3 && y_cp2 == y_cp3) {
            // QUADRATIC BEZIER
            coord_t *point = (coord_t*) malloc(sizeof(coord_t));
            point->draw_mode = draw_mode;
            point->polygon_mode = polygon_mode;
            point->line_weight = line_weight;
            point->control_points = number_of_control_points-1;
            point->x = x_cp0;
            point->y = y_cp0;
            point->z = z_px;
            point->r = selected_red;
            point->g = selected_green;
            point->b = selected_blue;
            push(point);

            point->draw_mode = draw_mode;
            point->polygon_mode = polygon_mode;
            point->line_weight = line_weight;
            point->control_points = number_of_control_points-1;
            point->x = x_cp1;
            point->y = y_cp1;
            point->z = z_px;
            point->r = selected_red;
            point->g = selected_green;
            point->b = selected_blue;
            push(point);

            point->draw_mode = draw_mode;
            point->polygon_mode = polygon_mode;
            point->line_weight = line_weight;
            point->control_points = number_of_control_points-1;
            point->x = x_cp2;
            point->y = y_cp2;
            point->z = z_px;
            point->r = selected_red;
            point->g = selected_green;
            point->b = selected_blue;
            push(point);

            tracing = false;
            free(point);
            number_of_control_points = 0;
          }
          else {
            // CUBIC BEZIER
            coord_t *point = (coord_t*) malloc(sizeof(coord_t));
            point->draw_mode = draw_mode;
            point->polygon_mode = polygon_mode;
            point->line_weight = line_weight;
            point->control_points = number_of_control_points;
            point->x = x_cp0;
            point->y = y_cp0;
            point->z = z_px;
            point->r = selected_red;
            point->g = selected_green;
            point->b = selected_blue;
            push(point);

            point->draw_mode = draw_mode;
            point->polygon_mode = polygon_mode;
            point->line_weight = line_weight;
            point->control_points = number_of_control_points;
            point->x = x_cp1;
            point->y = y_cp1;
            point->z = z_px;
            point->r = selected_red;
            point->g = selected_green;
            point->b = selected_blue;
            push(point);

            point->draw_mode = draw_mode;
            point->polygon_mode = polygon_mode;
            point->line_weight = line_weight;
            point->control_points = number_of_control_points;
            point->x = x_cp2;
            point->y = y_cp2;
            point->z = z_px;
            point->r = selected_red;
            point->g = selected_green;
            point->b = selected_blue;
            push(point);

            point->draw_mode = draw_mode;
            point->polygon_mode = polygon_mode;
            point->line_weight = line_weight;
            point->control_points = number_of_control_points;
            point->x = x_cp3;
            point->y = y_cp3;
            point->z = z_px;
            point->r = selected_red;
            point->g = selected_green;
            point->b = selected_blue;
            push(point);

            tracing = false;
            number_of_control_points = 0;
            free(point);
          }
          break;
        default:
          break;
      }
    }
  }
  glutPostRedisplay();
}

/*
 *  Mouse motion handler
 */
void motion(int x, int y) {
  if (mouse_left_active) {
    if (draw_mode == GL_POINTS) {
        GLfloat viewport[4];
        glGetFloatv(GL_VIEWPORT, viewport);

        coord_t *point = (coord_t*) malloc(sizeof(coord_t));
        point->draw_mode = draw_mode;
        point->polygon_mode = polygon_mode;
        point->line_weight = line_weight;
        point->x = ((float)x / (viewport[2] / 2.0f)) - 1.0f;
        point->y = -((float)y / (viewport[3] / 2.0f)) + 1.0f;
        point->z = z_px;
        point->r = selected_red;
        point->g = selected_green;
        point->b = selected_blue;
        push(point);
    }
    else {
      tracing = true;
      GLfloat viewport[4];
      glGetFloatv(GL_VIEWPORT, viewport);
      dx_px =  ((float)x / (viewport[2] / 2.0f)) - 1.0f;
      dy_px = -((float)y / (viewport[3] / 2.0f)) + 1.0f;

      // For Bezier Curve control points
      if (draw_mode == GL_LINE_STRIP) {
        switch(number_of_control_points) {
          case 1:
            x_cp1 = dx_px;
            y_cp1 = dy_px;
            break;
          case 2:
            x_cp2 = dx_px;
            y_cp2 = dy_px;
            break;
          case 3:
            x_cp3 = dx_px;
            y_cp3 = dy_px;
            break;
          default:
            break;
        }
      }
    }
  }
  glutPostRedisplay();
}

int main(int argc, char **argv) {
  // GLUT initialization
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

  // Build window
  glutInitWindowSize(X_RESOLUTION, Y_RESOLUTION);
  glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - X_RESOLUTION)/2,
                         (glutGet(GLUT_SCREEN_HEIGHT) - Y_RESOLUTION/2));
  int window = glutCreateWindow("Project 4");

  // Build menu
  menu_init();
  
  // Scene initialization
  scene_init();

  // Register callbacks
  glutDisplayFunc(display);
  glutSetCursor(GLUT_CURSOR_CROSSHAIR);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);

  // Main loop
  glutMainLoop();
  return -1;
}
