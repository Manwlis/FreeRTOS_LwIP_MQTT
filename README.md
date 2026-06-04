# FreeRTOS_LwIP_MQTT
<!-- what is this project and what it contains -->
MQTT client on LwIP & FreeRTOS. To demonstrate its use, multiple sensor drivers, modules and ethernet throughput tests have been integrated. Implemented on STM32H755ZI.

<!-- Should I say why this client exists? The LwIP MQTT API is not thread safe and receives messages through callbacks. To be used in an RTOS project, such a client must be developed. -->

## MQTT client
<!-- intro -->
The client is build on top of the MQTT API provided by the LwIP stack. It is designed with a multi-task architecture in mind, and communicates with the rest of the codebase using CMSIS-RTOS2 queues. It has a static memory footprint of around a few thousand kilobytes, depending on the selected maximum sizes for the topic names, payloads etc.

### Usage
<!-- API -->
The API consists of the following functions and macros:
- `mqtt_init()`: Initializes the module, OS infrastructure and connects to the MQTT broker.
- `mqtt_get_connection_status()`: Returns true if the client is connected to the broker, false if it is not.
- `mqtt_sub_topic()`: Subscribe to a MQTT topic. User must supply an OS queue to receive the MQTT messages.
- `mqtt_unsub_topic()`: Unsubscribe from an MQTT topic.
- `mqtt_publish_wrapper()`: Wrapper of LwIP's `mqtt_publish()` function. Does error checking and handles TCPIP core locking.
- `mqtt_free_message()`: Return a received message back to the MQTT client.
- `compare_mqtt_payload()`: Helper function to compare a message with a given string. Can be set to complete match, or just the start of the payload.

<!-- data structures -->
Received messages are pushed to the appropriate task queue using the following type:
```c
typedef struct _mqtt_os_message_t
{
	uint8_t data[MQTT_PAYLOAD_MAX_SIZE];    // The payload
	uint32_t len;                           // Size of the payload
	sub_topic_id_t topic_id;                // ID of the topic that received the payload
}mqtt_os_message_t;
```

<!-- Example Usage -->
The following code block exhibits the usage of the API, excluding error checking. 
```c
    ...
	mqtt_init();

     // the task subscribes to two topics using the same queue
	sub_topic_id_t topic_A_id;
	sub_topic_id_t topic_B_id;
	osMessageQueueId_t mqtt_queue = osMessageQueueNew( NUM_MAX_ELEMENTS , sizeof(mqtt_os_message_t*) , NULL );
    mqtt_sub_topic( TOPIC_A , mqtt_queue , &topic_A_id );
    mqtt_sub_topic( TOPIC_B , mqtt_queue , &topic_B_id );

    while(1)
    {
       // wait for a message
		mqtt_os_message_t* message = NULL;
		osStatus_t status = osMessageQueueGet( mqtt_queue , &message , NULL , osWaitForever ); 

        if( message->topic_id = topic_A_id )
        {
            // Do topic A stuff
            mqtt_publish_wrapper( PUBLISH_TOPIC , data , sizeof( data ) , 0 , 0 , NULL , NULL );
        }
        else if( message->topic_id = topic_B_id )
        {
            // Do topic B stuff
            mqtt_publish_wrapper( PUBLISH_TOPIC , data , sizeof( data ) , 0 , 0 , NULL , NULL );
        }

		// free message
		mqtt_free_message( message );
    }
```
### Implementation
<!-- LwIP -->
#### LwIP configuration
The LwIP setup has been inherited by the [LwIP-FreeRTOS-STM32H755ZI](https://github.com/Manwlis/LwIP-FreeRTOS-STM32H755ZI) project. This includes LwIP parameter configuration, DMA & MPU set-up etc. There are two substantial changes:

* Increasing the number of simultaneously active timeouts, as by default LwIP calculates it only based on its active internal modules, not applications built on top of it.
```c
#define MEMP_NUM_SYS_TIMEOUT   (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 1)
```
* Enabling TCPIP core locking. The MQTT API provided by the LwIP stack uses the RAW API, which require core locking to be used from non-TCPIP task context.
```c
#define LWIP_TCPIP_CORE_LOCKING 1
```
<!-- Internal Structure. Do I need to show this? Having it here makes explaining the rest far easier. -->
#### Internal Structure
All information required by the client, is organized with the following structures:
```c
typedef struct _mqtt_sub_topic_t
{
	char name[MQTT_TOPIC_NAME_MAX_SIZE];    // Name of the topic
	osMessageQueueId_t os_queue_id;         // Queue of the topic
	bool valid;                             // Valid flag
}mqtt_sub_topic_t;

typedef struct _mqtt_data_t
{
	mqtt_sub_topic_t sub_topics[MQTT_MAX_SUBBED_TOPICS];    // Array of subbed topics 
	osMemoryPoolId_t os_memory_pool;    // Memory pool of the incoming messages
	mqtt_client_t* client;              // LwIP's MQTT API data structure 
	uint8_t num_topics;                 // Number of subbed topics 
	bool connected;                     // Connected to broker flag
}mqtt_data_t;
```
This information is internal to the module, and is not accessible by the main application. The only variable that can be directly requested is the `connected` flag, through the `mqtt_get_connection_status()` API call.

<!-- Initialization -->
#### Initialization
Function `mqtt_init()` has three responsibilities:
- Allocate and initialize the data-structures shown above.
- Set up the LwIP MQTT stack.
- Connect to the MQTT broker.

The function can be called at any time to reset the module.

<!-- Dynamic subscription -->
<!-- Task can sub to multiple topics using the same queue -->
#### Subscribing to a Topic
Tasks can dynamically subscribe to topics using the `mqtt_sub_topic()` function, as long as the client is connected and there is free space in the array of subbed topics. They must supply the handle of the queue that the received messages will be pushed on. A task can use the same queue for multiple topics.

<!-- Callback structure -->
#### MQTT Callbacks
The MQTT messages are transferred from the LwIP MQTT stack to the client through the callbacks `mqtt_incoming_publish_cb()` & `mqtt_incoming_data_cb()`. The first one identifies which topic the message comes from, while the second callback puts the message to the appropriate queue. To achieve that, it allocates space from the MQTT memory pool. It is expected that the receiving task will free it after no longer needing the message.

## Example Application Structure

### LWL integration
<!-- data can be dump through mqtt -->
<!-- delay to fix waking up from idling problem -->
### Sensor integration
<!-- easy no driver changes, used high level API -->
### Ethernet tests integration
<!-- start stop on demand -->
<!-- dynamicly create/destroy FreeRTOS & lwip resources -->
## Repo Structure
```
code/
├── CM7/    # MQTT application
├── CM4/    # Secondary core default application
mqtt/       # MQTT broker configuration & firmware control script
scripts/    # Non-MQTT scripts required by the project
docs/
server.py  # TCP loopback server
```

## Tools Used
* STM32CubeMX 6.17.0
* STM32Cube FW_H7 V1.13.0
* STM32CubeIDE 2.1.1
* FreeRTOS 10.6.2
* CMSIS-RTOS 2.1.3
* LwIP 2.2.1
* mosquitto 2.1.2