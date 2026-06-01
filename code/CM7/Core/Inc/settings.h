// Eth Test settings ----------------------------------------------------------
#define UDP_TX_BENCHMARK		0
#define TCP_LOOPBACK			1
#define TCP_LOOPBACK_MULTITASK	2

#define CURRENT_TEST	TCP_LOOPBACK

#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK
#define NUM_NETWORK_MESSAGES 16
#endif

// network settings -----------------------------------------------------------
#define MESSAGE_SIZE	1460
#define ETH_SERVER_PORT	55151

#define ETH_SERVER_IP_1	192
#define ETH_SERVER_IP_2	168
#define ETH_SERVER_IP_3	0
#define ETH_SERVER_IP_4	1
#define ETH_SERVER_IP	"192.168.0.1"

// ADC DMA -------------------------------------------------------------------
#define ADC_DMA_FLAG	0x0001U

// LIS3DHTR -------------------------------------------------------------------
#define NO_OS		0
#define FREE_RTOS	1
#define LIS3DHTR_OS	FREE_RTOS
#define I2C_MEM_IT_FLAG		0x0002U
#define I2C_ERR_IT_FLAG		0x0004U

// MQTT -----------------------------------------------------------------------
#define MQTT_HOST_PORT 1883

// Client internals
#define MQTT_MAX_TOPICS				8
#define MQTT_TOPIC_NAME_MAX_SIZE	16
#define MQTT_PAYLOAD_MAX_SIZE		16
#define MQTT_OS_QUEUE_NUM_ELEMENTS	8

// Sub topics
#define MQTT_SUB_LWL_ID			"mod/lwl"
#define MQTT_SUB_LIS3_ID		"sensor/lis3"
#define MQTT_SUB_ALS_ID			"sensor/als"
#define MQTT_SUB_TEMP_ID		"sensor/temp"
#define MQTT_SUB_ETH_TEST_ID	"lwip/eth_test"

// Pub Topics
#define MQTT_PUB_LWL_INDEX_ID	MQTT_SUB_LWL_ID "/meta"
#define MQTT_PUB_LWL_DATA_ID	MQTT_SUB_LWL_ID "/data"
#define MQTT_PUB_ALS_LUX_ID		MQTT_SUB_ALS_ID "/lux"
#define MQTT_PUB_TEMP_INT_ID	MQTT_SUB_TEMP_ID "/int"
#define MQTT_PUB_TEMP_FLOAT_ID	MQTT_SUB_TEMP_ID "/float"
#define MQTT_PUB_LIS3_ACCEL_ID	MQTT_SUB_LIS3_ID "/accel"

// other
#define MQTT_CLIENT_ID			"STM32H755"
#define MQTT_CONNECT_TOPIC		"dev/STM32H755"
#define MQTT_CONNECT_PAYLOAD	"connected"
#define MQTT_WILL_TOPIC			MQTT_CONNECT_TOPIC
#define MQTT_WILL_PAYLOAD		"disconnected"
