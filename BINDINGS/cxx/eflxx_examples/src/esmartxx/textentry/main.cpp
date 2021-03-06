#include <ecorexx/Ecorexx.h>
#include <evasxx/Evasxx.h>
#include <edjexx/Edjexx.h>
#include <esmartxx/TextEntry.h>
#include "../../common/searchFile.h"

#define WIDTH 300
#define HEIGHT 30

using namespace Eflxx;

int main( int argc, const char **argv )
{
  /* Create the application object */
  Ecorexx::Application* app = new Ecorexx::Application( argc, argv, "Esmart Container Test" );

  /* Create the main window, a window with an embedded canvas */
  Ecorexx::EvasWindowSoftwareX11* mw = new Ecorexx::EvasWindowSoftwareX11( Size (WIDTH, HEIGHT) );

  Evasxx::Canvas &evas = mw->getCanvas();

  evas.appendFontPath( searchDataDir () + "/fonts" );

  /* Add some objects to the canvas */
  Evasxx::Rectangle* rect = new Evasxx::Rectangle( evas, Rect (0, 0, WIDTH, HEIGHT) );
  rect->setColor( Color (200, 200, 200, 255) );
  rect->setLayer( 0 );
  rect->show();

  Edjexx::Object* edje = new Edjexx::Object ( evas, searchEdjeFile ("esmart_text_entry_test.edj"), "text_entry" );

  edje->resize( Size (WIDTH, HEIGHT) );
  edje->setLayer( 1 );
  edje->show();

  Esmartxx::TextEntry* passwd = new Esmartxx::TextEntry( evas, Rect (0, 0, WIDTH, HEIGHT) );
  passwd->setEdjePart (edje, "text");
  passwd->setColor( Color (0, 0, 0, 255) );
  passwd->setLayer( 0 );
  passwd->setFocus (true);
  passwd->show();

  mw->show();

  /* Enter the application main loop */
  app->exec();

  /* Delete the application */
  delete app;

  return 0;
}
