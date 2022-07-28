#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sqlite3.h>

#include <mosquitto.h>

//DEFINES
#define MAX_BDREQ_SIZE 1024

//MQTT
#define ADDRESS    "192.168.43.31"
#define PORT       1883
#define CLIENTNAME "Client01"

//SERVER
#define MAXserveurRequest 1024

//GLOBALS
//SYNC
int sync_end = 1;
struct sync_data
{
  int valid;
  int command_size;
  char* command;
  int topic_size;
  char* topic;
};
struct sync_lightsensor
{
  int valid;
  char* value;
  int msg_size;
};
struct sync_button
{
  int valid;
  char* state;
  int msg_size;
};

struct sync_data sync_msg;
struct sync_lightsensor sync_ls;
struct sync_button sync_btn;


sqlite3* db;

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

void on_connect(struct mosquitto* mosq, void* obj, int rc)
{
  printf("ID: %d\n", *(int*)obj);
  if(rc)
  {
    printf("Error with result code: %d\n", rc);
    exit(-1);
  }
  mosquitto_subscribe(mosq, NULL, "SERVER/light", 0);
  mosquitto_subscribe(mosq, NULL, "SERVER/button", 0);
}

void on_message(struct mosquitto* mosq, void* obj, const struct mosquitto_message* msg)
{
  int size = strlen((char*)msg->payload);
  printf("size: %d\n", size);
  if(strcmp(msg->topic, "SERVER/light") == 0)
  {
    sync_ls.valid = 1;
    sync_ls.value = (char*)malloc(size);
    strcpy(sync_ls.value, (char*)msg->payload);
    sync_ls.msg_size = size;
  }
  else if(strcmp(msg->topic, "SERVER/button") == 0)
  {
    sync_btn.valid = 1;
    sync_btn.state = (char*)malloc(size);
    strcpy(sync_btn.state, (char*)msg->payload);
    sync_btn.msg_size = size;
  }
  else
  {
    printf("No treatment for this topic.\n");
  }
  printf("New message with topic %s: %s\n", msg->topic, (char*)msg->payload);

  //Insert SQLITE
  char* err_msg = 0;
  char req[MAX_BDREQ_SIZE] = "";
  sprintf(req,"INSERT INTO communication_log VALUES ('%s','%s',datetime('now'))",msg->topic, msg->payload);

  char* zErrMsg = 0;
  int rc = sqlite3_exec(db, req, callback, 0, &zErrMsg);
  if(rc != SQLITE_OK)
  {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return;
  }
  
}

void* MQTT_init(void* vargs)
{
  int rc, id = 12;

  mosquitto_lib_init();

  struct mosquitto* mosq;

  mosq = mosquitto_new(CLIENTNAME, true, &id);
  mosquitto_connect_callback_set(mosq, on_connect);
  mosquitto_message_callback_set(mosq, on_message);
  

  if((rc = mosquitto_connect(mosq, ADDRESS, PORT, 10)))
  {
    printf("Could not connect to Broker with return code %d\n", rc);
    exit(-1);
  }

  mosquitto_loop_start(mosq);
  
  while(sync_end)
  {
    if(sync_msg.valid)
    {
      printf("published %s to topic %s\n",sync_msg.command,sync_msg.topic);
      mosquitto_publish(mosq, NULL, sync_msg.topic, sync_msg.command_size, sync_msg.command, 0, false);
      sync_msg.valid = 0;
    }
  }

  mosquitto_loop_stop(mosq, true);

  mosquitto_disconnect(mosq);
  mosquitto_destroy(mosq);

  mosquitto_lib_cleanup();
  printf("[MQTT] mosquitto com closed\n");
}

void* serveur_com_init(void* vargs)
{
  //FIFOS
  char* s2c_name = "/var/www/SESIBG.com/public_html/s2c_LG";
  char* json_btn_path = "/var/www/SESIBG.com/public_html/btn.json";
  char* json_ls_path = "/var/www/SESIBG.com/public_html/ls.json";

  char serverRequest[MAXserveurRequest];

  fd_set rfds;
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;

  mkfifo(s2c_name, 0666);

  int s2c = open(s2c_name, O_RDWR | O_NONBLOCK);
  
  printf("open fd = %d\n",s2c);
  while(sync_end)
  {
    FD_ZERO(&rfds);
    FD_SET(s2c, &rfds);

    if(select(s2c+1, &rfds, NULL, NULL, &tv) != 0)
    {
      if(FD_ISSET(s2c, &rfds))
      {
	int nbchar;
	if((nbchar = read(s2c, serverRequest, MAXserveurRequest)) == 0) break;

	serverRequest[nbchar] = 0;

	
	printf("Request : %s\n",serverRequest);
	
	if(strcmp(serverRequest, "LEDON") == 0)
	{
	  sync_msg.command_size = 3;
	  sync_msg.command = "ON";
	  sync_msg.topic_size = 8;
	  sync_msg.topic = "ESP/led";
	  sync_msg.valid = 1;
	  printf("Command %s to Topic %s\n",sync_msg.command,sync_msg.topic);
	}
	else if(strcmp(serverRequest, "LEDOFF") == 0)
	{
	  sync_msg.command_size = 4;
	  sync_msg.command = "OFF";
	  sync_msg.topic_size = 8;
	  sync_msg.topic = "ESP/led";
	  sync_msg.valid = 1;
	  printf("Command %s to Topic %s\n",sync_msg.command,sync_msg.topic);
	}
	else if(strcmp(serverRequest, "BTNON") == 0)
	{
	  sync_msg.command_size = 3;
	  sync_msg.command = "ON";
	  sync_msg.topic_size = 11;
	  sync_msg.topic = "ESP/button";
	  sync_msg.valid = 1;
	  printf("Command %s to Topic %s\n",sync_msg.command,sync_msg.topic);
	}
	else if(strcmp(serverRequest, "BTNOFF") == 0)
	{
	  sync_msg.command_size = 4;
	  sync_msg.command = "OFF";
	  sync_msg.topic_size = 11;
	  sync_msg.topic = "ESP/button";
	  sync_msg.valid = 1;
	  printf("Command %s to Topic %s\n",sync_msg.command,sync_msg.topic);
	}
	else if(strcmp(serverRequest, "LSON") == 0)
	{
	  sync_msg.command_size = 3;
	  sync_msg.command = "ON";
	  sync_msg.topic_size = 10;
	  sync_msg.topic = "ESP/light";
	  sync_msg.valid = 1;
	  printf("Command %s to Topic %s\n",sync_msg.command,sync_msg.topic);
	}
	else if(strcmp(serverRequest, "LSOFF") == 0)
	{
	  sync_msg.command_size = 4;
	  sync_msg.command = "OFF";
	  sync_msg.topic_size = 10;
	  sync_msg.topic = "ESP/light";
	  sync_msg.valid = 1;
	  printf("Command %s to Topic %s\n",sync_msg.command,sync_msg.topic);
	}
	else if(strncmp(serverRequest, "LCD#",4) == 0)
	{
	  char* buf = (char*)malloc(sizeof(char)*(nbchar-3));
	  buf = strcpy(buf, serverRequest+4);
	  sync_msg.command_size = nbchar-3;
	  sync_msg.command = buf;
	  sync_msg.topic_size = 8;
	  sync_msg.topic = "ESP/lcd";
	  sync_msg.valid = 1;
	  printf("Command %s to Topic %s\n",sync_msg.command,sync_msg.topic);
	}
	else
	{
	  printf("No command found.\n");
	}

	//Insert SQLITE
	char* err_msg = 0;
	char req[MAX_BDREQ_SIZE] = "";
	
	sprintf(req,"INSERT INTO communication_log VALUES ('%s','%s',datetime('now'))",sync_msg.topic, sync_msg.command);
	printf("REQ : %s\n",req);
	
	char* zErrMsg = 0;
	int rc = sqlite3_exec(db, req, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK)
	{
	  fprintf(stderr, "SQL error: %s\n", zErrMsg);
	  sqlite3_free(zErrMsg);
	  return;
	}
	  
	printf("Bd updated\n");
      }
    }
    if(sync_btn.valid)
    {
      int c2s_btn = open(json_btn_path, O_WRONLY | O_NONBLOCK);
      
      char buffer[100] = "";
      strcat(buffer, "{\"state\":\"");
      strcat(buffer, sync_btn.state);
      strcat(buffer, "\"}  ");
      printf("state: %s\n", buffer);
      write(c2s_btn, buffer, sync_btn.msg_size+14);
      sync_btn.valid = 0;
      close(c2s_btn);
    }
    if(sync_ls.valid)
    {
      int c2s_ls = open(json_ls_path, O_WRONLY | O_NONBLOCK);
      
      char buffer[100] = "";
      strcat(buffer, "{\"value\":\"");
      strcat(buffer, sync_ls.value);
      strcat(buffer, "\"}  ");
      printf("value: %s  %d\n", buffer,sync_ls.msg_size+14);
      write(c2s_ls, buffer, sync_ls.msg_size+14);
      sync_ls.valid = 0;
      close(c2s_ls);
    }
  }
  close(s2c);
  printf("[server] s2c closed\n");
}

int main(int argc, char** argv)
{
  pthread_t thread_id_MQTT, thread_id_serveur;

  //sync init
  sync_end = 1;

  sync_msg.valid = 0;

  //SQLITE init  
   int rc = sqlite3_open("client.db", &db);
   
   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } else {
      fprintf(stdout, "Opened database successfully\n");
   }
  
  pthread_create(&thread_id_MQTT, NULL, MQTT_init, NULL);
  pthread_create(&thread_id_serveur, NULL, serveur_com_init, NULL);

  printf("[main proc] sync value = %d\n",sync_end);
  printf("Press Enter to quit..\n");
  getchar();
  
  sync_end = 0;
  printf("[main proc] sync value = %d\n",sync_end);
  pthread_join(thread_id_MQTT, NULL);
  pthread_join(thread_id_serveur, NULL);
  printf("[main proc] sync value = %d\n",sync_end);

  sqlite3_close(db);
  return 0;
}
