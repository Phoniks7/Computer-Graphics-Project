//Gabriel Violette + Geo Kersey
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

#define GL_MULTISAMPLE  0x809D               //this is not defined on windows, so I'm manually defining it.
#define GL_MULTISAMPLE_FILTER_HINT_NV 0x8534 //this is not defined on windows, so I'm manually defining it.

using namespace std;

//-----------------
//GAME OBJECT DATA
//-----------------
Point3 stars[1000];
Planet planets[31];
const int PLANET_COUNT = 31;
Player noob;

Point3 camera_position;

const float GRAVITATIONAL_CONSTANT = 0.25;

//Setup utility function
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
    //cout << linetype << '\n'; //DEBUG

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
    else if ( linetype == "DEATHSTAR" )
    {
      temp_planet.death = true;
    }
    else if ( linetype == "TRANSPARENT" )
    {
      temp_planet.transparent = true;
    }
    else if ( linetype == "VICTORY" )
    {
      temp_planet.win_on_touch = true;
    }
  }

  my_file.close();
}

//Setup function
void setup()
{
  glEnable( GL_DEPTH_TEST );  //enable depth testing
  glEnable( GL_MULTISAMPLE ); //turn on antialiasing
  glHint( GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST ); //we want high quality antialiasing.
  glEnable( GL_LIGHTING );    //enable lighting

  //Initialization logic.
  //Set up camera
  camera_position.x = 0.0;
  camera_position.y = 0.0;
  camera_position.z = 10.0;

  //"Randomly" generate stars
  for ( int i = 0; i < 1000; i++ )
  {
    stars[i].x = rand() % 320 - 160; //-160:160
    stars[i].y = rand() % 200 - 100; //-100:100
    stars[i].z = rand() % 25 - 75;   //-50:-75
  }

  //Read in planets.
  for ( int i = 0; i < PLANET_COUNT; i++ )
  {
    planets[i] = Planet(); //fill array with objects?
    //construct filename string
    char* temp_s = new char();
    string s = "planets/planet";
    s.append( itoa( i, temp_s, 10 ) );
    s.append( ".txt" );
    //load in data.
    readPlanetFromFile( planets[i], s );
  }
}

//Update function
void update( int ignore_me )
{
  //Handles game logic
  //Called once per frame.
  //initialize force to zero
  float player_fx = 0.0;
  float player_fy = 0.0;

  //these will allow the player to glue to a planet
  float old_x;
  float new_x;
  float old_y;
  float new_y;

  for ( int i = 0; i < PLANET_COUNT; i++ )
  {
    float distance_from_center = sqrt( pow( noob.position.x - planets[i].x, 2 ) + pow( noob.position.y - planets[i].y, 2 ) );
	  //record the distance from each planet
	  noob.distances[i] = distance_from_center;
    if ( distance_from_center < planets[i].radius )
    {
      if ( planets[i].death == true )
      {
        //KILL THE PLAYER! and respawn them.
		    noob.position.x = planets[noob.prev_planet].x;
		    noob.position.y = planets[noob.prev_planet].y;
      }
      else if ( planets[i].win_on_touch == true )
      {
        //You win!
        exit( 0 );
      }
      else
      {
        noob.grounded = true;
        noob.prev_planet = i;
        noob.closest_planet = i;
      }
      //set pos to planet pos + r * a unit vector in the direction of the player.
	    noob.position.x = planets[i].radius * cos( atan2( noob.position.y - planets[i].y, noob.position.x - planets[i].x ) ) + planets[i].x;
	    noob.position.y = planets[i].radius * sin( atan2( noob.position.y - planets[i].y, noob.position.x - planets[i].x ) ) + planets[i].y;
    }

    //move the planets in their orbits.
	  float dx = -1*(noob.position.x - planets[i].x);
	  float dy = -1*(noob.position.y - planets[i].y);
	  float g;
	  if ( dx == 0 && dy == 0 ) //division by zero: infinity catch
	  {
		  g = 0.0;
	  }
	  else
	  {
	 	  g = GRAVITATIONAL_CONSTANT * planets[i].mass / ( (dx * dx) + (dy * dy) );
	  }
	  float gx = g * cos( atan2( dy, dx ) );
	  float gy = g * sin( atan2( dy, dx ) );

	  //add to total force
	  player_fx += gx;
	  player_fy += gy;


	  if (i == noob.closest_planet)
	  {
		  if (noob.grounded == true)
		  {
			  //glue to the planet
			  //record old position of planet before update
			  old_x = planets[i].x;
			  old_y = planets[i].y;
		  }
	  }

    //update the planet
    planets[i].update();

	  if (noob.closest_planet == i)
	  {
		  new_x = planets[i].x;
		  new_y = planets[i].y;
	  }

	  //if the player is attached to the planet, move them along with it.
	  if (noob.grounded == true && i == noob.closest_planet)
	  {
		  noob.position.x = noob.position.x + new_x - old_x;
		  noob.position.y = noob.position.y + new_y - old_y;
	  }
  }
  //this approach had a bug where you could snap from large planets to others.
  /*
  int temp = 0;
  float smallest = noob.distances[0];
  for (int i = 0; i < PLANET_COUNT; i++)
  {
	  if (noob.distances[i] < smallest)
	  {
		  smallest = noob.distances[i];
		  temp = i;
	  }
  }
  */

  if ( noob.grounded == false )
  {
	  //free floating, apply gravity.
	  noob.acceleration.x = player_fx;
	  noob.acceleration.y = player_fy;

	  noob.velocity.x += noob.acceleration.x;
	  noob.velocity.y += noob.acceleration.y;

    noob.position.x += noob.velocity.x;
	  noob.position.y += noob.velocity.y;
  }
  else
  {
	  //hit the planet, zero out velocity and acceleration
	  noob.velocity.x = 0.0;
	  noob.velocity.y = 0.0;

	  noob.acceleration.x = 0.0;
	  noob.acceleration.y = 0.0;
  }

  //noob.closest_planet = temp;

  glutTimerFunc( 17, update, 0 );
  glutPostRedisplay();
}

//Draw utility functions
void doCameraOffset()
{
  //Translates things that are drawn, so the camera can "move".
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( -1.0 * camera_position.x, -1.0 * camera_position.y, -1.0 * camera_position.z );
}

void sunLight()
{
  //sets up the sun's light
  //set up GL_LIGHT0
  GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light_diffuse[] = { 1.0, 0.9, 0.8, 1.0 };
  GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light_attenuation[] = { 0.001 };

  glLightfv( GL_LIGHT0, GL_POSITION, light_position );
  glLightfv( GL_LIGHT0, GL_AMBIENT, light_ambient );
  glLightfv( GL_LIGHT0, GL_DIFFUSE, light_diffuse );
  glLightfv( GL_LIGHT0, GL_SPECULAR, light_specular );
  glLightfv( GL_LIGHT0, GL_QUADRATIC_ATTENUATION, light_attenuation );

  glEnable( GL_LIGHT0 ); //turn on the light
}

void deathStarLight()
{
  //sets up the other sun's light
  //set up GL_LIGHT1
  GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light_diffuse[] = { 1.0, 0.5, 0.3, 1.0 };
  GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat light_position[] = { 25.0, 8.0, 0.0, 1.0 };
  GLfloat light_attenuation[] = { 0.05 };

  glLightfv( GL_LIGHT1, GL_POSITION, light_position );
  glLightfv( GL_LIGHT1, GL_AMBIENT, light_ambient );
  glLightfv( GL_LIGHT1, GL_DIFFUSE, light_diffuse );
  glLightfv( GL_LIGHT1, GL_SPECULAR, light_specular );
  glLightfv( GL_LIGHT1, GL_LINEAR_ATTENUATION, light_attenuation );

  glEnable( GL_LIGHT1 ); //turn on the light
}

void victoryLight()
{
  //sets up the victory light.
  //set up GL_LIGHT2
  GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light_diffuse[] = { 0.5, 0.0, 1.0, 1.0 };
  GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat light_position[] = { 43.0, 0.0, 0.0, 1.0 };
  GLfloat light_attenuation[] = { 0.01 };

  glLightfv( GL_LIGHT2, GL_POSITION, light_position );
  glLightfv( GL_LIGHT2, GL_AMBIENT, light_ambient );
  glLightfv( GL_LIGHT2, GL_DIFFUSE, light_diffuse );
  glLightfv( GL_LIGHT2, GL_SPECULAR, light_specular );
  glLightfv( GL_LIGHT2, GL_QUADRATIC_ATTENUATION, light_attenuation );

  glEnable( GL_LIGHT2 ); //turn on the light
}

//Draw function
void draw()
{
  //Draw the scene
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the buffers (need to clear both for shading to work right.)

  glLoadIdentity(); //reset orientation

  //draw all the stars in the background
  for ( int i = 0; i < 1000; i++ )
  {
    doCameraOffset(); //do camera offset
    glTranslatef( stars[i].x, stars[i].y, stars[i].z );
    glutSolidSphere( 0.25, 8, 8 );
  }

  //Set up major star lights (sun, death star, wormholes? )
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  sunLight();
  deathStarLight();
  victoryLight();
  glEnable( GL_LIGHTING );   //enable lighting

  camera_position.x = noob.position.x;
  camera_position.y = noob.position.y;

  //draw the planets
  for ( int i = 0; i < PLANET_COUNT; i++ )
  {
    doCameraOffset(); //do camera offset

    //Offset lights
    GLfloat light_position0[] = { -1.0 * planets[i].x, -1.0 * planets[i].y, -1.0 * planets[i].z, 1.0 };
    GLfloat light_position1[] = { -1.0 * planets[i].x + 25.0 * 2.0, -1.0 * planets[i].y + 8.0 * 2.0, -1.0 * planets[i].z, 1.0 };
    GLfloat light_position2[] = { -1.0 * planets[i].x + 43.0 * 2.0, -1.0 * planets[i].y, -1.0 * planets[i].z, 1.0 };
    glLightfv( GL_LIGHT0, GL_POSITION, light_position0 ); //set up GL_LIGHT0
    glLightfv( GL_LIGHT1, GL_POSITION, light_position1 ); //set up GL_LIGHT1
    glLightfv( GL_LIGHT2, GL_POSITION, light_position2 ); //set up GL_LIGHT2

    //draw planets
    planets[i].draw();
  }

  //draw the player
  doCameraOffset();
  noob.draw();

  //glEnd();

  glDisable( GL_LIGHT0 );
  glDisable( GL_LIGHT1 );
  glDisable( GL_LIGHT2 );
  glDisable( GL_LIGHTING );

  glutSwapBuffers(); //swap buffers: display the drawing.
}

void resize( int width, int height)
{
  //Handles resizing the window
  glViewport( 0, 0, (GLsizei) width, (GLsizei) height);
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  glFrustum( -4.0, 4.0, -2.5, 2.5, 5.0, 90.0 );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

void handleInput( unsigned char key, int x, int y )
{
  //cout << key << '\n'; //DEBUG

  //handles ASCII key input
  /*
  if ( key == 'a' )
  {
    camera_position.x -= 0.4;
  }
  if ( key == 'd' )
  {
    camera_position.x += 0.4;
  }
  if ( key == 'w' )
  {
    camera_position.y += 0.4;
  }
  if ( key == 's' )
  {
    camera_position.y -= 0.4;
  }*/
  if ( key == 'w' )
  {
    camera_position.z -= 0.15;
  }
  if ( key == 's' )
  {
    camera_position.z += 0.15;
  }
  if ( key == 'r' )
  {
    //KILL THE PLAYER! and respawn them.
		noob.position.x = planets[noob.prev_planet].x;
		noob.position.y = planets[noob.prev_planet].y;
  }
  if ( key == 'a' )
  {
    //rotate l
    if ( noob.grounded )
    {
      //
      float angle = atan2( (noob.position.y - planets[noob.closest_planet].y), (noob.position.x - planets[noob.closest_planet].x) );
      angle += 0.0628;
      noob.position.x = planets[noob.closest_planet].x + planets[noob.closest_planet].radius * cos( angle );
      noob.position.y = planets[noob.closest_planet].y + planets[noob.closest_planet].radius * sin( angle );
    }
  }
  if ( key == 'd' )
  {
    //rotate r
    if ( noob.grounded )
    {
      //
      float angle = atan2( (noob.position.y - planets[noob.closest_planet].y), (noob.position.x - planets[noob.closest_planet].x) );
      angle -= 0.0628;
      noob.position.x = planets[noob.closest_planet].x + planets[noob.closest_planet].radius * cos( angle );
      noob.position.y = planets[noob.closest_planet].y + planets[noob.closest_planet].radius * sin( angle );
    }
  }
  if ( key == 32 )
  {
    //TODO: SPACE
    if ( noob.grounded )
    {
      //jump
      noob.position.x += 0.10 * cos( atan2( (noob.position.y - planets[noob.closest_planet].y), (noob.position.x - planets[noob.closest_planet].x) ) );
      noob.position.y += 0.10 * sin( atan2( (noob.position.y - planets[noob.closest_planet].y), (noob.position.x - planets[noob.closest_planet].x) ) );
	    noob.velocity.x += 0.5 * cos( atan2( (noob.position.y - planets[noob.closest_planet].y), (noob.position.x - planets[noob.closest_planet].x) ) );
	    noob.velocity.y += 0.5 * sin( atan2( (noob.position.y - planets[noob.closest_planet].y), (noob.position.x - planets[noob.closest_planet].x) ) );
      noob.acceleration.x += 1.0 * cos( atan2( (noob.position.y - planets[noob.closest_planet].y), (noob.position.x - planets[noob.closest_planet].x) ) );
      noob.acceleration.y += 1.0 * cos( atan2( (noob.position.y - planets[noob.closest_planet].y), (noob.position.x - planets[noob.closest_planet].x) ) );
	    noob.grounded = false;
    }
  }
  if ( key == 27 ) //escape
  {
    exit( 0 ); //end the program
  }
}

void specialInput( int key, int x, int y )
{
  //handles non-ASCII key input. (use GLUT_KEY_w/e constants)
}

//Main routine
int main(int argc, char** argv)
{
  //Initialization
  glutInit( &argc, argv );  //initialize glut
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE ); //Set display mode, enable antialiasing

  //Set up window
  //glutInitWindowPosition( 0, 0 ); //set window's position
  //glutInitWindowSize( 1440, 900 ); //set window's size
  glutCreateWindow("Moon Bounce");
  glutFullScreen(); //go fullscreen!

  //setup game stuff
  setup();

  //Setup callbacks
  glutTimerFunc( 17, update, 0 ); //call update on a delay of 17 ms
  glutDisplayFunc(draw);
  glutReshapeFunc(resize);
  glutKeyboardFunc(handleInput);
  glutSpecialFunc(specialInput);
  glutMainLoop();

  return 0; 
}