//Gabriel Violette + Geo Kersey
//TODO: reference where you stole the code from (gupta loadtextures.cpp)
//Core Game Code
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <string.h>
#include "planet.h"
#include "player.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#  include <OpenGL/glext.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

Planet planets[10];
Player noob;
float distance_from_center;

void update( int ignore_me )
{
  //handles game logic
  noob.grounded = false;

  for ( int i = 0; i < 10; i++ )
  {
    distance_from_center = sqrt( pow( noob.position.x - planets[i].x, 2 ) + pow( noob.position.y - planets[i].y, 2 ) + pow ( noob.position.z - planets[i].z, 2 ));
    if ( distance_from_center < planets[i].radius )
    {
      noob.grounded = true;
      //set pos to planet pos + r * a unit vector in the direction of the player.
      if (noob.position.x < planets[i].x)
	  {
		  noob.position.x = planets[i].x - (planets[i].x * planets[i].radius / distance_from_center);
	  }
	  else
	  {
		  noob.position.x = planets[i].x + (planets[i].x * planets[i].radius / distance_from_center);
	  }
	  if (noob.position.y < planets[i].y)
	  {
		  noob.position.y = planets[i].y - (planets[i].y * planets[i].radius / distance_from_center);
	  }
	  else
	  {
		  noob.position.y = planets[i].y + (planets[i].y * planets[i].radius / distance_from_center);
	  }
	  if (noob.position.z < planets[i].z)
	  {
		  noob.position.z = planets[i].z - (planets[i].z * planets[i].radius / distance_from_center);
	  }
	  else
	  {
		  noob.position.z = planets[i].z + (planets[i].z * planets[i].radius / distance_from_center);
	  }
      //later, we'll also want to move the player with the planet.
    }

    //move the planets in their orbits.
    //Orbit should be in the form pt, radius, angular velocity. (keep it planar for simplicity?)
    //rotation speed should be in the form of angular velocity. (keep it planar for simplicity?)

    planets[i].update();
    //cout << i << planets[i].x << "\n"; //DEBUG

    //Do gravitation, acceleration, velocity, position!
  }

  glutTimerFunc( 17, update, 0 );
  glutPostRedisplay();
}

void draw()
{
  //Draw the scene
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the buffers (need to clear both for shading to work right.)

  glLoadIdentity(); //reset orientation

  //For each object
  //glBegin(), glEnd()
  //glBegin();

  //draw all the stars in the background
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef( 0.0, 20.0, -75 );
  glutSolidSphere( 0.5, 8, 8 );

  //draw the planets
  for ( int i = 0; i < 10; i++ )
  {
    //do camera offset
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, 0.0 );
    //draw planets
    planets[i].draw();
  }

  //draw the player
  noob.draw();

  //glEnd();

  glutSwapBuffers(); //swap buffers: display the drawing.
}

void resize( int width, int height)
{
  //Handles resizing the window
  glViewport( 0, 0, (GLsizei) width, (GLsizei) height);
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  glFrustum( -16.0, 16.0, -10.0, 10.0, 5.7, 100.0 );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

void handleInput( unsigned char key, int x, int y )
{
  //handles ASCII key input
  if ( key == 65 )
  {
    //TODO: A
  }
  if ( key == 32 )
  {
    //TODO: SPACE
    if ( noob.grounded )
    {
      //jump
    }
  }
}

void specialInput( int key, int x, int y )
{
  //handles non-ASCII key input. (use GLUT_KEY_w/e constants)
}

void readPlanetFromFile( Planet &temp_planet, string filename )
{
  //reads data from file into temp_planet.

  ifstream my_file;
  my_file.open( filename );

  for ( string line; getline(my_file, line); ) //read the file line by line
  {
    istringstream in( line ); //create a stream for the line

    //Figure out what kind of data we're going to be reading in.
    string linetype;
    in >> linetype;
    cout << linetype << '\n';

    if ( linetype == "RADIUS" )
    {
      in >> temp_planet.radius;
    }
    else if ( linetype == "MASS" )
    {
      in >> temp_planet.mass;
    }
    else if ( linetype == "ORBITCENTER" )
    {
      in >> temp_planet.orbit.x;
      in >> temp_planet.orbit.y; 
      in >> temp_planet.orbit.z;
    }
    else if ( linetype == "ORBITRADIUS" )
    {
      in >> temp_planet.orbit.r;
    }
    else if ( linetype == "THETA" )
    {
      in >> temp_planet.orbit.angle;
    }
    else if ( linetype == "OMEGA" )
    {
      in >> temp_planet.orbit.omega;
    }
    else if ( linetype == "AMBIENT" )
    {
      in >> temp_planet.ambient.r;
      in >> temp_planet.ambient.g;
      in >> temp_planet.ambient.b;
      in >> temp_planet.ambient.a;
    }
    else if ( linetype == "DIFFUSE" )
    {
      in >> temp_planet.diffuse.r;
      in >> temp_planet.diffuse.g;
      in >> temp_planet.diffuse.b;
      in >> temp_planet.diffuse.a;
    }
    else if ( linetype == "SPECULAR" )
    {
      in >> temp_planet.specular.r;
      in >> temp_planet.specular.g;
      in >> temp_planet.specular.b;
      in >> temp_planet.specular.a;
    }
    else if ( linetype == "EMISSION" )
    {
      in >> temp_planet.emissive.r;
      in >> temp_planet.emissive.g;
      in >> temp_planet.emissive.b;
      in >> temp_planet.emissive.a;
    }
    else if ( linetype == "SHININESS" )
    {
      in >> temp_planet.shininess;
    }
  }

  my_file.close();
}


void setup()
{
  glEnable(GL_DEPTH_TEST); //enable depth testing
  glEnable(GL_LIGHTING);   //enable lighting

  //Initialization logic.
  for ( int i = 0; i < 10; i++ )
  {
    planets[i] = Planet( i * 3, 0.0, -5.0, 1.0, 100.0 ); //fill array with objects?
    planets[i].orbit.r = i * 3;
    planets[i].orbit.x = 0;
    planets[i].orbit.y = 0;
    planets[i].orbit.z = 0;
  }

  readPlanetFromFile( planets[0], "planet0.txt" );
}

//Main routine
int main(int argc, char** argv)
{
  //Initialization
  glutInit( &argc, argv );  //initialize glut
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB ); //Set display mode

  //Set up window
  glutInitWindowPosition( 0, 0 ); //set window's position
  glutInitWindowSize( 1440, 900 ); //set window's size
  glutCreateWindow("Title!");

  //setup game stuff
  setup();

  //Setup callbacks
  glutTimerFunc( 17, update, 0 );
  glutDisplayFunc(draw);
  glutReshapeFunc(resize);
  glutKeyboardFunc(handleInput);
  glutSpecialFunc(specialInput);
  glutMainLoop();

  return 0; 
}