#ifndef  LIBEXALT_DBUS_PRIVATE_INC
#define  LIBEXALT_DBUS_PRIVATE_INC

#include "Exalt_DBus.h"

/*
 * when we return a valid response, next args are the response
 */
#define EXALT_DBUS_VALID 1
/* when we return an error, the next args have to be an int32 with a error id
 * and the third args have to be a string with a description of the error
 */
#define EXALT_DBUS_ERROR 0

extern int exalt_dbus_log_domain;
#define EXALT_LOG_DOMAIN exalt_dbus_log_domain

typedef struct _exalt_dbus_notify_data exalt_dbus_notify_data;
typedef struct _exalt_dbus_notify_conn_applied_data exalt_dbus_notify_conn_applied_data;
typedef struct _exalt_dbus_scan_notify_data exalt_dbus_scan_notify_data;
typedef struct _exalt_dbus_response_data exalt_dbus_response_data;
typedef struct _exalt_dbus_msg_id Exalt_DBus_Msg_Id;

struct _Exalt_DBus_Conn
{
    E_DBus_Connection *e_conn;
    DBusConnection* conn;

    exalt_dbus_notify_data* notify;
    E_DBus_Signal_Handler * notify_handler;

    exalt_dbus_scan_notify_data* scan_notify;
    E_DBus_Signal_Handler * scan_notify_handler;

    exalt_dbus_response_data* response_notify;
    //define the next message id
    int msg_id;


    E_DBus_Signal_Handler *name_owner_changed_handler;
    E_DBus_Signal_Handler *generated_signal;
    DBusPendingCall *pending_get_name_owner;
    DBusPendingCall *pending_start_service_by_name;
    const char *unique_name;
    Eina_Bool connected : 1;
    Eina_Bool server_started : 1;

    struct 
    {
        Exalt_DBus_Connect_Cb cb;
        void *data;
        Eina_Free_Cb free_data;
    } connect;

    struct 
    {
        Exalt_DBus_Die_Cb cb;
        void *data;
        Eina_Free_Cb free_data;
    } die;
};

struct _exalt_dbus_notify_data
{
    exalt_notify_cb* cb;
    void* user_data;
};

struct _exalt_dbus_scan_notify_data
{
    exalt_scan_notify_cb* cb;
    void* user_data;
};

struct _exalt_dbus_response_data
{
    exalt_response_notify_cb* cb;
    void* user_data;
};


struct _exalt_dbus_msg_id
{
    int id;
    Exalt_DBus_Conn* conn;
};


struct _exalt_dbus_response
{
    int is_error;
    Exalt_DBus_Response_Type type;
    int msg_id;

    int error_id;
    char* error_msg;

    Eina_List* l;

    Exalt_Configuration *c;
    char* iface;
    char* address;
    char* string;
    int is;
};


int exalt_dbus_msg_id_next(Exalt_DBus_Conn* conn);

int exalt_dbus_conf_encaps(Exalt_Configuration* c, DBusMessage *msg);

const char* exalt_dbus_response_string(DBusMessage *msg, int pos);
Eina_List* exalt_dbus_response_strings(DBusMessage *msg, int pos);
void exalt_dbus_string_free(void* data);
int exalt_dbus_response_boolean(DBusMessage *msg, int pos);
int exalt_dbus_response_integer(DBusMessage *msg, int pos);

int exalt_dbus_valid_is(DBusMessage *msg);
int exalt_dbus_error_get_id(DBusMessage *msg);
const char* exalt_dbus_error_get_msg(DBusMessage *msg);


void print_error(const char* file,const char* fct, int line, const char* msg, ...);
const char* dbus_get_eth(DBusMessage* msg);

#endif   /* ----- #ifndef LIBEXALT_DBUS_PRIVATE_INC  ----- */

