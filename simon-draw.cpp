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

// Object menu
#define POINT 0
#define LINE 1
#define RECTANGLE 2
#define ELLIPSE 3
#define BEZIER_CURVE 4

// Fill menu
#define FILLED 5
#define OUTLINE 6

// Color menu options
#define RED 7
#define GREEN 8
#define BLUE 9
#define YELLOW 10
#define PURPLE 11 
#define ORANGE 12
#define WHITE 13
#define BLACK 14

typedef struct Coord {
  GLenum draw_mode;
  GLenum fill_mode;
  GLfloat x;
  GLfloat y;
  GLfloat z;
  GLfloat r;
  GLfloat g;
  GLfloat b;
  struct Coord *next;
} coord_t;

void init(void);
void display(void);
void draw(void);
void trace(void);
coord_t* draw_point(coord_t*);
coord_t* draw_line(coord_t*);
coord_t* draw_rectangle(coord_t*);
coord_t* draw_ellipse(coord_t*);
coord_t* draw_bezier_curve(coord_t*);
void main_menu(int);
void object_menu(int);
void fill_menu(int);
void color_menu(int);
void reshape(int, int);
void keyboard(unsigned char, int, int);
void mouse(int, int, int, int);
void motion(int, int);
void push(coord_t*);

// Setting perspective
const GLfloat FIELD_OF_VIEW   = 45.0f;
const GLfloat VIEWPORT_LEFT   = -1.0f;
const GLfloat VIEWPORT_RIGHT  = 1.0f;
const GLfloat VIEWPORT_BOTTOM = -1.0f;
const GLfloat VIEWPORT_TOP    = 1.0f;
const GLfloat NEAR_CLIPPING_PLANE = 0.0001f;
const GLfloat FAR_CLIPPING_PLANE  = 100.0f;

// Mouse stuff
GLint mouse_left_active;
GLboolean tracing = false;

// (x,y) coordinates for drawing
GLfloat x_px;
GLfloat y_px;
const GLfloat z_px = -1.0f;
GLfloat dx_px;
GLfloat dy_px;

// Setting  defaults
GLenum draw_mode       = GL_POINTS;
GLenum polygon_mode    = GL_LINE;
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
    head->fill_mode = new_coord->fill_mode;
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
    current->next->fill_mode = new_coord->fill_mode; 
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
void init() {
  head = NULL;
  mouse_left_active = false;
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
}

/*
 *  Render the display
 */
void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  if (tracing) {
    trace();
  }
  draw();
  glutSwapBuffers();
}

void trace() {
  if (draw_mode == GL_LINES) {
    glPushMatrix();
    glBegin(draw_mode);
      glColor3f(selected_red, selected_green, selected_blue);
      glVertex3f(x_px, y_px, z_px);
      glVertex3f(dx_px, dy_px, z_px);
    glEnd();
    glPopMatrix();
  }
  else
  if (draw_mode == GL_QUADS) {
    glPushMatrix();
    glColor3f(selected_red, selected_green, selected_blue);
    glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
    glBegin(draw_mode);
        glVertex3f(x_px, y_px, z_px);    // Top left
        glVertex3f(dx_px, y_px, z_px);   // Top right
        glVertex3f(dx_px, dy_px, z_px);  // Bottom right
        glVertex3f(x_px, dy_px, z_px);   // Bottom left
    glEnd();
    glPopMatrix();
  }
  else
  if (draw_mode == GL_LINE_LOOP || draw_mode == GL_TRIANGLE_FAN) {
    GLint number_of_segments = 100;
    glPushMatrix();
    glColor3f(selected_red, selected_green, selected_blue);
    //glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
}

/*
 *  Draws to the canvas
 */
void draw() {
  coord_t *current = head;
  while (current != NULL) {
    if (current->draw_mode == GL_POINTS) {current = draw_point(current);}
    else if (current->draw_mode == GL_LINES) {current = draw_line(current);}
    else if (current->draw_mode == GL_QUADS) {current = draw_rectangle(current);}
    else if (current->draw_mode == GL_LINE_LOOP || current->draw_mode == GL_TRIANGLE_FAN) {
        current = draw_ellipse(current);
    }
  }
}

/*
 *  Draws points
 */
coord_t* draw_point(coord_t *current) {
  glPushMatrix();
  glPolygonMode(GL_FRONT_AND_BACK, current->fill_mode);
  glColor3f(current->r, current->g, current->b);
  glBegin(current->draw_mode);
    glVertex3f(current->x, current->y, current->z);
  glEnd();
  glPopMatrix();
  return current->next;
}

coord_t* draw_line(coord_t *current) {
  glPushMatrix();
  glPolygonMode(GL_FRONT_AND_BACK, current->fill_mode);
  glColor3f(current->r, current->g, current->b);

  glBegin(current->draw_mode);
    glVertex3f(current->x, current->y, current->z);
    current = current->next;
    glVertex3f(current->x, current->y, current->z);
  glEnd();
  glPopMatrix();
  return current->next;
}

coord_t* draw_rectangle(coord_t *current) {
  glPushMatrix();
  glPolygonMode(GL_FRONT_AND_BACK, current->fill_mode);
  glColor3f(current->r, current->g, current->b);

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

coord_t* draw_ellipse(coord_t *current) {

    GLfloat origin_x = current->x;
    GLfloat origin_y = current->y;

    current = current->next;

    GLfloat arc_x = current->x;
    GLfloat arc_y = current->y;

    GLint number_of_segments = 100;
    glPolygonMode(GL_FRONT_AND_BACK, current->fill_mode);
    glColor3f(current->r, current->g, current->b);

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

//coord_t* draw_bezier_curve() {}

/*
 *  Create a menu system
 */
void main_menu(int value) {
  int selection;
  switch(value) {}
  glutPostRedisplay();
}

/*
 *  Handles object selection
 */
void object_menu(int value){
  switch(value) {
    case POINT:
      draw_mode = GL_POINTS;
      break;
    case LINE:
      draw_mode = GL_LINES;
      break;
    case RECTANGLE:
      draw_mode = GL_QUADS;
      break;
    case ELLIPSE:
      if (polygon_mode == GL_FILL) {draw_mode = GL_TRIANGLE_FAN;}
      else
      if (polygon_mode == GL_LINE) {draw_mode = GL_LINE_LOOP;}
      break;
    case BEZIER_CURVE:
      //draw_mode = BEZIER_CURVE;
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

/*
 *  Handles polygon fills selection
 */
void fill_menu(int value){
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
  }
  if (state == GLUT_UP) {
    tracing = false;
    mouse_left_active = false;
    coord_t *point = (coord_t*) malloc(sizeof(coord_t));
    if (draw_mode == GL_LINES || draw_mode == GL_LINE_LOOP || draw_mode == GL_TRIANGLE_FAN) {
      point->draw_mode = draw_mode;
      point->fill_mode = polygon_mode;
      point->x = x_px;
      point->y = y_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);

      point->draw_mode = draw_mode;
      point->fill_mode = polygon_mode;
      point->x = dx_px;
      point->y = dy_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);
    }
    else
    if (draw_mode == GL_QUADS) {
      // Top Left
      point->draw_mode = draw_mode;
      point->fill_mode = polygon_mode;
      point->x = x_px;
      point->y = y_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);

      // Top Right
      point->draw_mode = draw_mode;
      point->fill_mode = polygon_mode;
      point->x = dx_px;
      point->y = y_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);

      // Bottom Right
      point->draw_mode = draw_mode;
      point->fill_mode = polygon_mode;
      point->x = dx_px;
      point->y = dy_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);

      // Bottom Left
      point->draw_mode = draw_mode;
      point->fill_mode = polygon_mode;
      point->x = x_px;
      point->y = dy_px;
      point->z = z_px;
      point->r = selected_red;
      point->g = selected_green;
      point->b = selected_blue;
      push(point);
    }
    free(point);
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
        point->fill_mode = polygon_mode;
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
  int objects = glutCreateMenu(object_menu);
  glutAddMenuEntry("Pencil", POINT);
  glutAddMenuEntry("Line", LINE);
  glutAddMenuEntry("Rectangle", RECTANGLE);
  glutAddMenuEntry("Ellipse", ELLIPSE);
  glutAddMenuEntry("Bezier Curve", BEZIER_CURVE);

  int fills = glutCreateMenu(fill_menu);
  glutAddMenuEntry("Filled", FILLED);
  glutAddMenuEntry("Outline", OUTLINE);

  int colors = glutCreateMenu(color_menu);
  glutAddMenuEntry("Red", RED);
  glutAddMenuEntry("Green", GREEN);
  glutAddMenuEntry("Blue", BLUE);
  glutAddMenuEntry("Yellow", YELLOW);
  glutAddMenuEntry("Purple", PURPLE);
  glutAddMenuEntry("Orange", ORANGE);
  glutAddMenuEntry("White", WHITE);
  glutAddMenuEntry("Black", BLACK);

  int menu = glutCreateMenu(main_menu);
  glutAddSubMenu("Objects", objects);
  glutAddSubMenu("Fills", fills);
  glutAddSubMenu("Colors", colors);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // Scene initialization
  init();

  // Register callbacks
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);

  // Main loop
  glutMainLoop();
  return -1;
}
