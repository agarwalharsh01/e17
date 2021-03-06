#ifndef EDJE_EDIT_H
#define EDJE_EDIT_H

/* STL */
#include <string>

/* EFL */
#include <Edje.h>
#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT
#include <Edje_Edit.h>

/* EFL++ */
#include <eflxx/Common.h>
#include <eflxx/CountedPtr.h>
#include <einaxx/Einaxx.h>

/* Project */
#include "Part.h"
#include "Object.h"
#include "PartEdit.h"

namespace Edjexx {
  
/*!
 * @todo port to EFLxx!
 * @todo return error states with Exceptions?
 * @file
 * @brief Functions to deal with edje internal object. Don't use in standard
 * situations. The use of any of the edje_edit_* functions can break your
 * theme ability, remember that the program must be separated from the interface!
 *
 * The API can be used to query or set every part of an edje object in real time.
 * You can manage every aspect of parts, part states, programs, script and whatever
 * is contained in the edje file. For a reference of what all parameter means
 * look at the complete @ref edcref.
 *
 * All the functions that deals with part states include the state value inside
 * the returned strings (ex: "defaut 0.00"). For this reason there aren't
 * functions to set/get a state value, you have to pass the value inside the
 * name string (always in the form "state x.xx").
 *
 * Don't forget to free all the strings and the lists returned by any edje_edit_*()
 * functions using edje_edit_string_free() and edje_edit_string_list_free() when
 * you don't need anymore.
 *
 * Example: print all the part in a loaded edje_object
 * @code
 *  Eina_List *parts, *l;
 *
 *  parts = edje_edit_parts_list_get(edje_object);
 *  while(l = parts; l; l = eina_list_nexst(l))
 *  {
 *     printf("Part: %s\n", (char*)eina_list_data_get(l));
 *  }
 *  edje_edit_string_list_free(parts);
 * @endcode
 *
 * Example: Change the color of a rect inside an edje file
 * @code
 * Evas_Object *edje;
 *
 * edje = edje_object_add(evas);
 * edje_object_file_set(edje,"edj/file/name", "group to load");
 * edje_edit_state_color_set(edje, "MyRectName", "default 0.00", 255, 255, 0, 255);
 * edje_edit_save(edje);
 * @endcode
 *
*/
class Edit : public Edjexx::Object
{
public:
  Edit( Evasxx::Canvas &canvas );
  Edit( Evasxx::Canvas &canvas, const std::string &filename, const std::string &groupname );

  //~Object(); // TODO: needed?
  
/******************************************************************************/
/**************************   GENERAL API   ***********************************/
/******************************************************************************/
/** @name General API
 *  General functions that don't fit in other cateories.
 */ //@{
  
#if 0

/** @name General API
 *  General functions that don't fit in other cateories.
 */ //@{

/** Free a generic Eina_List of (char *) allocated by an edje_edit_*_get() function.*/
EAPI void
edje_edit_string_list_free(
   Eina_List *lst          ///< The list to free. Will also free all the strings.
);

/** Free a generic string (char *) allocated by an edje_edit_*_get() function.*/
EAPI void
edje_edit_string_free(
   const char *str         ///< The string to free.
);
#endif
  
  /*! Get the name of the program that compiled the edje file.@n
   *  Can be 'edje_cc' or 'edje_edit'
   */
  const string getCompiler ();
  
  /*! Save the modified edje object back to his file.
   *  Use this function when you are done with your editing, all the change made
   *  to the current loaded group will be saved back to the original file.
   *
   *  NOTE: for now this as limitations
   *     -you will lost your #define in the edc source
   */
  bool save ();
  
  /*! Print on standard output many information about the internal status
   *  of the edje object.
   *  This is probably only usefull to debug.
   */
  void printInternalStatus ();
  
//@}
/******************************************************************************/
/**************************   GROUPS API   ************************************/
/******************************************************************************/
  /** @name Groups API
 *  Functions to deal with groups property (see @ref edcref).
 */ //@{
  
  /*!
   * Create a new empty group in the given edje.
   * If a group with the same name exist none is created.
   *
   * @param name The name for the new empty group
   * @return true on success, false on failure
   */
  bool addGroup (const std::string &name);
 
  /*!
   * Delete the specified group from the given edje.
   * You can only delete a currently unused group.
   * All the parts and the programs inside the group will be deleted as well,
   * but not image or font embedded in the edje.
   *
   * @param group The group to be deleted
   * @return true on success, false on failure
   */
  bool delGroup (const std::string &group);

  /*!
   * Check if a group with the given name exist in the edje.
   *
   * @return true on success, false on failure
   */
  bool hasGroup (const std::string &group);

  /*!
   * Set a new name for the current open group.
   * You can only rename a group that is currently loaded
   * Note that the relative getter function don't exist as it don't make sense ;)
   *
   * @return true on success, false on failure
   */
  bool setGroupName (const std::string &newName);

  /*!
   * Get the group min width
   *
   * @return min width on success, -1 on errors
   */
  int getGroupMinWidth ();

  /*!
   * Set the group min width
   *
   * @param width The new group min width in pixel
   */
  void setGroupMinWidth (int width);

  /*!
   * Get the group min height
   *
   * @return min width on success, -1 on errors
   */
  int getGroupMinHeight ();

  /*!
   * Set the group min height
   *
   * @param height The new group min height in pixel
   */
  void setGroupMinHeight (int height);

  /*!
   * Get the group max width
   *
   * @return max width on success, -1 on errors
   */
  int getGroupMaxWidth ();

  /*!
   * Set the group max width
   *
   * @param width The new group max width in pixel
   */
  void setGroupMaxWidth (int width);

  /*!
   * Get the group max height
   *
   * @return max height on success, -1 on errors
   */
  int getGroupMaxHeight ();

  /*!
   * Set the group max height
   *
   * @param height The new group max height in pixel
   */
  void setGroupMaxHeight (int height);

#if 0
//@}
/******************************************************************************/
/**************************   DATA API   **************************************/
/******************************************************************************/
/** @name Data API
 *  Functions to deal with data embedded in the edje (see @ref edcref).
 */ //@{

/** Retrieves a list with the item names inside the data block **/
EAPI Eina_List *          ///@return An Eina_List* of string (char *)containing all the data names.
edje_edit_data_list_get(
   Evas_Object *obj       ///< The edje object
);

/**Create a new data object in the given edje
 * If another data with the same name exists nothing is created and FALSE is returned.
 */
EAPI Eina_Bool         ///@return TRUE on success
edje_edit_data_add(
   Evas_Object *obj,       ///< The edje object
   const char *itemname,   ///< The name for the new data
   const char *value       ///< The value for the new data
);

/**Delete the given data object from edje */
EAPI Eina_Bool         ///@return TRUE on success
edje_edit_data_del(
   Evas_Object *obj,       ///< The edje object
   const char *itemname    ///< The name of the data to remove
);

/** Get the data associated with the given itemname **/
EAPI const char *         ///@return The data value
edje_edit_data_value_get(
   Evas_Object * obj,     ///< The edje object
   char *itemname         ///< The name of the data item
);

/** Set the data associated with the given itemname **/
EAPI Eina_Bool        ///@return TRUE on success
edje_edit_data_value_set(
   Evas_Object * obj,     ///< The edje object
   const char *itemname,  ///< The name of the data item
   const char *value      ///< The new value to set
);

/** Change the name of the given data object */
EAPI Eina_Bool        ///@return TRUE on success
edje_edit_data_name_set(
   Evas_Object *obj,     ///< The edje object
   const char *itemname, ///< The name of the data item
   const char *newname   ///< The new name to set
                        );

//@}
/******************************************************************************/
/***********************   COLOR CLASSES API   ********************************/
/******************************************************************************/
/** @name Eflxx::Color Classes API
 *  Functions to deal with Eflxx::Color Classes (see @ref edcref).
 */ //@{

/** Get the list of all the Eflxx::Color Classes in the given edje object.
 *  Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Eina_List *           ///@return An Eina_List* of string (char *)containing all the classes names.
edje_edit_color_classes_list_get(
   Evas_Object * obj       ///< The edje object
);

/** Create a new color class object in the given edje
 *  If another class with the same name exists nothing is created and FALSE is returned.
 */
EAPI Eina_Bool        ///@return TRUE on success
edje_edit_color_class_add(
   Evas_Object *obj,      ///< The edje object
   const char *name       ///< The name of the new color class
);

/** Delete the given class object from edje */
EAPI Eina_Bool        ///@return TRUE on success
edje_edit_color_class_del(
   Evas_Object *obj,      ///< The edje object
   const char *name       ///< The name of the color class to delete
);

/** Get all the colors that compose the class.
 *  You can pass NULL to colors you are not intrested in
 */
EAPI Eina_Bool         ///@return TRUE on success
edje_edit_color_class_colors_get(
   Evas_Object *obj,       ///< The edje object
   const char *class_name, ///< The name of the color class
   int *r,  ///< Where to store the red component of the standard color
   int *g,  ///< Where to store the green component of the standard color
   int *b,  ///< Where to store the blue component of the standard color
   int *a,  ///< Where to store the alpha component of the standard color
   int *r2, ///< Where to store the red component of the second color
   int *g2, ///< Where to store the green component of the second color
   int *b2, ///< Where to store the green component of the second color
   int *a2, ///< Where to store the green component of the second color
   int *r3, ///< Where to store the red component of the third color
   int *g3, ///< Where to store the green component of the third color
   int *b3, ///< Where to store the blue component of the third color
   int *a3  ///< Where to store the alpha component of the third color
);

/** Set the colors for the given color class.
 *  If you set a color to -1 it will not be touched
 */
EAPI Eina_Bool         ///@return TRUE on success
edje_edit_color_class_colors_set(
   Evas_Object *obj,       ///< The edje object
   const char *class_name, ///< The name of the color class
   int r,    ///< The red component of the standard color
   int g,    ///< The green component of the standard color
   int b,    ///< The blue component of the standard color
   int a,    ///< The alpha component of the standard color
   int r2,   ///< The red component of the second color
   int g2,   ///< The green component of the second color
   int b2,   ///< The blue component of the second color
   int a2,   ///< The alpha component of the second color
   int r3,   ///< The red component of the third color
   int g3,   ///< The green component of the third color
   int b3,   ///< The blue component of the third color
   int a3    ///< The alpha component of the third color
);

/** Change the name of a color class */
EAPI Eina_Bool        ///@return TRUE on success
edje_edit_color_class_name_set(
   Evas_Object *obj,      ///< The edje object
   const char *name,      ///< The name of the color class
   const char *newname    ///< The new name to assign
);

//@}
/******************************************************************************/
/**************************   TEXT STYLES *************************************/
/******************************************************************************/
/** @name Text styles API
 *  Functions to deal with text styles (see @ref edcref).
 */ //@{

/** Get the list of all the text styles in the given edje object.
 *  Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Eina_List *           ///@return An Eina_List* of string (char *)containing all the styles name.
edje_edit_styles_list_get(
   Evas_Object *obj        ///< The edje object
);

/** Create a new text style object in the given edje
 *  If another style with the same name exists nothing is created and FALSE is returned.
 */
EAPI Eina_Bool        ///@return TRUE on success, FALSE if the tag can't be created
edje_edit_style_add(
   Evas_Object *obj,      ///< The edje object
   const char *style      ///< The new name for the style
);

/** Delete the given text style and all the child tags. */
EAPI void
edje_edit_style_del(
   Evas_Object *obj,      ///< The edje object
   const char *style      ///< The name for the style to delete
);
   
/** Get the list of all the tags name in the given text style.
 *  Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Eina_List *           ///@return An Eina_List* of string (char *)containing all the tags name.
edje_edit_style_tags_list_get(
  Evas_Object *obj,       ///< The edje object
  const char *style       ///< The name of the style to inspect
);

/** Get the value of the given tag.
 *  Use edje_edit_string_free() when you don't need it anymore.
 */
EAPI const char*           ///@return The value of the tag.
edje_edit_style_tag_value_get(
   Evas_Object *obj,       ///< The edje object
   const char *style,      ///< The name of the style to inspect
   const char *tag         ///< The name of the tag
);

/** Set the value of the given tag. */
EAPI void
edje_edit_style_tag_value_set(
   Evas_Object *obj,       ///< The edje object
   const char *style,      ///< The name of the style to inspect
   const char *tag,        ///< The name of the tag
   const char *new_value   ///< The new tag value
);

/** Set the name of the given tag. */
EAPI void
edje_edit_style_tag_name_set(
   Evas_Object *obj,       ///< The edje object
   const char *style,      ///< The name of the style
   const char *tag,        ///< The name of the tag
   const char *new_name    ///< The new name for tag
);

/** Add a new tag to the given text style.
 *  If another tag with the same name exists nothing is created and FALSE is returned.
 */
EAPI Eina_Bool         ///@return TRUE on success, FALSE if the tag can't be created
edje_edit_style_tag_add(
   Evas_Object *obj,       ///< The edje object
   const char *style,      ///< The name of the style
   const char* tag_name    ///< The name of the new tag
);

/** Delete the given tag. */
EAPI void
edje_edit_style_tag_del(
   Evas_Object *obj,       ///< The edje object
   const char *style,      ///< The name of the style
   const char* tag         ///< The name of the tag to remove
);

//@}
  
  #endif

/******************************************************************************/
/**************************   PARTS API   *************************************/
/******************************************************************************/
/** @name Parts API
 *  Functions to deal with part objects (see @ref edcref).
 */ //@{


  /*!
   * Get the list of all the parts in the given edje object.
   * Use edje_edit_string_list_free() when you don't need it anymore.
   * TODO: how to handle free of string?
   */
  Eflxx::CountedPtr <Einaxx::List <char*>::Iterator> getPartsNameList ();
  
  //Eflxx::CountedPtr <Einaxx::List <Eflxx::CountedPtr <PartEdit> >::Iterator> getPartsList ();
  
  Eflxx::CountedPtr <PartEdit> getPart (const std::string &part);
  
  /*! 
   * Create a new part in the given edje
   * If another part with the same name just exists nothing is created and FALSE is returned.
   * Note that this function also create a default description for the part.
   *
   * @return true on success, false on failure
   * @param name The name for the new part
   * @param type The type of the part to create (One of: EDJE_PART_TYPE_NONE, 
   *             EDJE_PART_TYPE_RECTANGLE, EDJE_PART_TYPE_TEXT,EDJE_PART_TYPE_IMAGE, 
   *             EDJE_PART_TYPE_SWALLOW, EDJE_PART_TYPE_TEXTBLOCK,EDJE_PART_TYPE_GRADIENT 
   *             or EDJE_PART_TYPE_GROUP)
   */  
  bool addPart (const std::string &name, Edje_Part_Type type);
  
  /*!
   * Delete the given part from the edje
   * All the reference to this part will be zeroed.
   * A group must have at least one part, so it's not possible to
   * remove the last remaining part.
   *
   * @param part The name of the part to remove
   *
   * @return true on success, false if the part can't be removed
   */
  bool delPart (const std::string &part);

  /*!
   * Check if a part with the given name exist in the edje object.
   *
   * @param part The name of the part
   *
   * @return true if the part exist, false otherwise.
   */
  bool hasPart (const std::string &part);

//@}
/******************************************************************************/
/**************************   STATES API   ************************************/
/******************************************************************************/
/** @name States API
 *  Functions to deal with part states (see @ref edcref).
 */ //@{

 /*!
  * TODO: how to handle free of string?
  *
  * @param part The name of the part
  *
  * @return An Einaxx::List<char*> containing all the states names found
  *         in part, including the float value (ex: "default 0.00").
  *         Use edje_edit_string_list_free() when you don't need it anymore.
  */
  Eflxx::CountedPtr <Einaxx::List <char*>::Iterator> getPartStatesList (const std::string &part);

  /*!
   * Copies the state @p from into @p to. If @p to doesn't exist it will be created.
   *
   * @param part The name of the part
   * @param from State to copy from (including state value)
   * @param to State to copy into (not including state value if new)
   *
   * @return true if it could be copied, false if something went wrong.
   */
  bool copyState (const std::string &part, const std::string &from, double valFrom, const std::string &to, double valTo);
  


//@}
/******************************************************************************/
/**************************   TEXT API   ************************************/
/******************************************************************************/
/** @name Text API
 *  Functions to deal with text objects (see @ref edcref).
 */ //@{


//@}
/******************************************************************************/
/**************************   IMAGES API   ************************************/
/******************************************************************************/
/** @name Images API
 *  Functions to deal with image objects (see @ref edcref).
 */ //@{

/*! 
 * Get the list of all the images in the given edje.
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 *
 * @return An Eina_List* of string (char *)containing all the images names found in the edje file.
 */
 Eflxx::CountedPtr <Einaxx::List <char*>::Iterator> getImagesList ();

#if 0
/**Add an new image to the image collection
 *
 * This function add the given image inside the edje. Don't add a new image part
 * but only put the image inside the edje file. It actually write directly to
 * the file so you don't have to save (and you can't undo!).
 * After you have to create a new image_part that use this image. Note that all
 * the parts in the edje share the same image collection, thus you can/must use
 * the same image for different part.
 *
 * The format of the image files that can be loaded depend on the evas engine on your system
 */
EAPI Eina_Bool         ///@return TRUE on success or FALSE on failure
edje_edit_image_add(
   Evas_Object *obj,       ///< The edje object
   const char* path        ///< The name of the image file to include in the edje
);

/**Add an image entry to the image collection
 *
 * This function adds the given image entry to the edje image collection. The
 * image needs to be inside the eet already, with key name "images/id". After
 * you have to create a new image_part that use this image, referring to it as
 * "name". Note that all the parts in the edje share the same image collection,
 * thus you can/must use the same image for different part.
 */
EAPI Eina_Bool        /// @return TRUE on success or FALSE on failure
edje_edit_image_data_add(
   Evas_Object *obj,      ///< The edje object
   const char *name,      ///< The image entry name
   int id                 ///< The image id
);
#endif
  


#if 0
/**Get image id for a given image name. Could be usefull to directly load the image from the eet file.*/
EAPI int                   ///< The ID of the givan image name
edje_edit_image_id_get(
   Evas_Object *obj,       ///< The edje object
   const char *image_name  ///< The image name
);

/**Get compression type for the given image.*/
EAPI Edje_Edit_Image_Comp ///@return One of EDJE_EDIT_IMAGE_COMP_RAW, EDJE_EDIT_IMAGE_COMP_USER, EDJE_EDIT_IMAGE_COMP_COMP or EDJE_EDIT_IMAGE_COMP_LOSSY
edje_edit_image_compression_type_get(
   Evas_Object *obj,      ///< The edje object
   const char *image      ///< The name of the image
);

/**Get compression rate for the given image.*/
EAPI int                  ///@return the compression rate if the image is EDJE_EDIT_IMAGE_COMP_LOSSY. Or < 0 on errors
edje_edit_image_compression_rate_get(
   Evas_Object *obj,      ///< The edje object
   const char *image      ///< The name of the image
);



//@}
/******************************************************************************/
/*************************   SPECTRUM API   ***********************************/
/******************************************************************************/
/** @name Spectrum API
 *  Functions to manage spectrum (see @ref edcref).
 */ //@{

/**Get the list of all the spectrum in the given edje object.
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Eina_List *           ///@return An Eina_List* of string(char *) containing all the spectra names.
edje_edit_spectrum_list_get(
   Evas_Object *obj        ///< The edje object
);

/**Add a new spectra in the given edje object.*/
EAPI Eina_Bool         ///@return 1 on success, 0 otherwise
edje_edit_spectra_add(
   Evas_Object *obj,       ///< The edje object
   const char* name        ///< The name of the new spectra
);

/**Delete the given spectra from the edje object.*/
EAPI Eina_Bool         ///@return 1 on success, 0 otherwise
edje_edit_spectra_del(
   Evas_Object *obj,       ///< The edje object
   const char* spectra     ///< The name of the spectra to delete
);

/**Change the name of the given spectra.*/
EAPI Eina_Bool         ///@return 1 on success, 0 otherwise
edje_edit_spectra_name_set(
   Evas_Object *obj,       ///< The edje object
   const char* spectra,    ///< The current name of the spectra
   const char* name        ///< The new name to assign
);

/**Get the number of stops in the given spectra.*/
EAPI int                   ///@return The number of stops, or 0 on errors
edje_edit_spectra_stop_num_get(
   Evas_Object *obj,       ///< The edje object
   const char* spectra     ///< The name of the spectra
);

/**Set the number of stops in the given spectra.*/
EAPI Eina_Bool         ///@return 1 on success, 0 otherwise
edje_edit_spectra_stop_num_set(
   Evas_Object *obj,       ///< The edje object
   const char* spectra,    ///< The name of the spectra
   int num                 ///< The number of stops you want
);

/**Get the colors of the given stop.*/
EAPI Eina_Bool         ///@return 1 on success, 0 otherwise
edje_edit_spectra_stop_color_get(
   Evas_Object *obj,       ///< The edje object
   const char* spectra,    ///< The name of the spectra
   int stop_number,        ///< The number of the stop
   int *r,                 ///< Where to store the red color value
   int *g,                 ///< Where to store the green color value
   int *b,                 ///< Where to store the blue color value
   int *a,                 ///< Where to store the alpha color value
   int *d                  ///< Where to store the delta stop value
);

/**Set the colors of the given stop.*/
EAPI Eina_Bool         ///@return 1 on success, 0 otherwise
edje_edit_spectra_stop_color_set(
   Evas_Object *obj,       ///< The edje object
   const char* spectra,    ///< The name of the spectra
   int stop_number,        ///< The number of the stop
   int r,                  ///< The red color value to set
   int g,                  ///< The green color value to set
   int b,                  ///< The blue color value to set
   int a,                  ///< The alpha color value to set
   int d                   ///< The delta stop value to set
);


//@}
/******************************************************************************/
/*************************   GRADIENT API   ***********************************/
/******************************************************************************/
/** @name Gradient API
 *  Functions to deal with gradient objects (see @ref edcref).
 */ //@{


//@}
/******************************************************************************/
/*************************   PROGRAMS API   ***********************************/
/******************************************************************************/
/** @name Programs API
 *  Functions to deal with programs (see @ref edcref).
 */ //@{

/**Get the list of all the programs in the given edje object.
 * @param obj The edje object
 * @return An Eina_List* of string (char *)containing all the program names.
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Eina_List *          ///@return A string list containing all the program names
edje_edit_programs_list_get(
   Evas_Object *obj       ///< The edje object
);

/**Add a new program to the edje file
 * If a program with the same name just exist the function will fail.
 */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_add(
   Evas_Object *obj,       ///< The edje object
   const char *name        ///< The name of the new program
);

/**Remove the given program from the edje file.
 */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_del(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The name of the program to remove
);

/**Check if a program with the given name exist in the edje object.
 */
EAPI Eina_Bool         ///< 1 if the program exist, 0 otherwise.
edje_edit_program_exist(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The name of the program
);

/**Run the given program. */
EAPI Eina_Bool         ///< 1 on success, 0 otherwise.
edje_edit_program_run(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The name of the program to execute
);

/**Set a new name for the given program */
EAPI Eina_Bool        ///@return 1 on success or 0 on errors
edje_edit_program_name_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *new_name    ///< The new name to assign
);

/**Get source of a given program. Remember to free the returned string using edje_edit_string_free().*/
EAPI const char *          ///@return The source value for prog
edje_edit_program_source_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set source of the given program. */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_source_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *source      ///< The new source value
);

/**Get signal of a given program. Remember to free the returned string using edje_edit_string_free().*/
EAPI const char *          ///@return The signal value for prog
edje_edit_program_signal_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set signal of the given program. */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_signal_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *signal      ///< The new signal value
);

/**Get in.from of a given program.*/
EAPI double                ///@return The delay
edje_edit_program_in_from_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set in.from of a given program.*/
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_in_from_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   double seconds          ///< Number of seconds to delay the program execution
);

/**Get in.range of a given program.*/
EAPI double                ///@return The delay random
edje_edit_program_in_range_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set in.range of a given program.*/
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_in_range_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   double seconds          ///< Max random number of seconds to delay
);

/**Get the action of a given program.
 * Action can be one of EDJE_ACTION_TYPE_NONE, _STATE_SET, ACTION_STOP, SIGNAL_EMIT, DRAG_VAL_SET, _DRAG_VAL_STEP, _DRAG_VAL_PAGE, _SCRIPT
 */
EAPI Edje_Action_Type      ///@return The action type, or -1 on errors
edje_edit_program_action_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the action of a given program.
 * Action can be one of EDJE_ACTION_TYPE_NONE, _STATE_SET, ACTION_STOP, SIGNAL_EMIT, DRAG_VAL_SET, _DRAG_VAL_STEP, _DRAG_VAL_PAGE, _SCRIPT
 */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_action_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   Edje_Action_Type action ///< The new action type
);

/**Get the list of the targets for the given program
 * Return a list of target name
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Eina_List*            ///@return An Eina_List of char*, or NULL on error
edje_edit_program_targets_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Add a new target name to the list of 'targets' in the given program.
 * If program action is EDJE_ACTION_TYPE_ACTION_STOP then 'target' must be an existing program name.
 * If action is EDJE_ACTION_TYPE_STATE_SET then 'target' must be an existing part name.
 */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_target_add(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *target      ///< The name of another program or another part
);

/**Clear the 'targets' list of the given program */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_targets_clear(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Get the list of action that will be run after the give program
 * Return a list of program name.
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Eina_List*            ///@return An Eina_List of char*, or NULL on error
edje_edit_program_afters_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Add a new program name to the list of 'afters' in the given program.
 * All the programs listed in 'afters' will be executed after program execution.
 */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_after_add(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *after       ///< The name of another program to add to the afters list
);

/**Clear the 'afters' list of the given program */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_afters_clear(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Get the state for the given program
 * In a STATE_SET action this is the name of state to set.
 * In a SIGNAL_EMIT action is the name of the signal to emit.
 */
EAPI const char*           ///@return The name of state for prog
edje_edit_program_state_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the state for the given program
 * In a STATE_SET action this is the name of state to set.
 * In a SIGNAL_EMIT action is the name of the signal to emit.
 */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_state_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *state       ///< The state to set (not including the state value)
);

/**Get the value of state for the given program.
 * In a STATE_SET action this is the value of state to set.
 * Not used on SIGNAL_EMIT action.
 */
EAPI double                ///@return The value of state for prog
edje_edit_program_value_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the value of state for the given program.
 * In a STATE_SET action this is the value of state to set.
 * Not used on SIGNAL_EMIT action.
 */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_value_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   double value            ///< The value to set
);

/**Get the state2 for the given program
 * In a STATE_SET action is not used
 * In a SIGNAL_EMIT action is the source of the emitted signal.
 */
EAPI const char*           ///@return The source to emit for prog
edje_edit_program_state2_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the state2 for the given program
 * In a STATE_SET action is not used
 * In a SIGNAL_EMIT action is the source of the emitted signal.
 */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_state2_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *state2      ///< The state2 value to set
);

/**Get the value of state2 for the given program.
 * I don't know what this is used for. :P
 */
EAPI double                ///@return The value of state2 for prog
edje_edit_program_value2_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the value2 of state for the given program.
 * This is used in DRAG_ACTION
 */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_value2_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   double value            ///< The value to set
);

/**Get the type of transition to use when apply animations.
 * Can be one of: EDJE_TWEEN_MODE_NONE, EDJE_TWEEN_MODE_LINEAR, EDJE_TWEEN_MODE_SINUSOIDAL, EDJE_TWEEN_MODE_ACCELERATE or EDJE_TWEEN_MODE_DECELERATE.
 */
EAPI Edje_Tween_Mode       ///@return The type of transition used by program
edje_edit_program_transition_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the type of transition to use when apply animations.
 * Can be one of: EDJE_TWEEN_MODE_NONE, EDJE_TWEEN_MODE_LINEAR, EDJE_TWEEN_MODE_SINUSOIDAL, EDJE_TWEEN_MODE_ACCELERATE or EDJE_TWEEN_MODE_DECELERATE.
 */
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_transition_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   Edje_Tween_Mode transition ///< The transition type to set
);

/**Get the duration of the transition in seconds.*/
EAPI double                ///@return The duration of the transition
edje_edit_program_transition_time_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the duration of the transition in seconds.*/
EAPI Eina_Bool         ///@return 1 on success or 0 on errors
edje_edit_program_transition_time_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   double seconds          ///< The duration of the transition (in seconds)
);

//@}
/******************************************************************************/
/**************************   SCRIPTS API   ***********************************/
/******************************************************************************/
/** @name Scripts API
 *  Functions to deal with embryo scripts (see @ref edcref).
 */ //@{
EAPI const char* edje_edit_script_get(Evas_Object *obj);
   
#endif
  
};

} // end namespace Edjexx

#endif // EDJE_EDIT_H
